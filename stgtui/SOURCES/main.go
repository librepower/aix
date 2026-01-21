package main

import (
	"bufio"
	"fmt"
	"os"
	"os/exec"
	"strconv"
	"strings"
	"time"

	"github.com/gdamore/tcell/v2"
	"github.com/rivo/tview"
)

// Config holds user preferences
type Config struct {
	WarnThreshold int // Yellow warning (default 85)
	CritThreshold int // Red critical (default 90)
}

var config = Config{WarnThreshold: 85, CritThreshold: 90}

func loadConfig() {
	home := os.Getenv("HOME")
	if home == "" { return }
	file, err := os.Open(home + "/.stgtuirc")
	if err != nil { return }
	defer file.Close()
	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		line := strings.TrimSpace(scanner.Text())
		if strings.HasPrefix(line, "#") || line == "" { continue }
		parts := strings.SplitN(line, "=", 2)
		if len(parts) != 2 { continue }
		key, val := strings.TrimSpace(parts[0]), strings.TrimSpace(parts[1])
		switch key {
		case "warn_threshold": config.WarnThreshold, _ = strconv.Atoi(val)
		case "crit_threshold": config.CritThreshold, _ = strconv.Atoi(val)
		}
	}
}

func runCmd(cmd string, args ...string) string {
	out, _ := exec.Command(cmd, args...).Output()
	return string(out)
}

func progressBar(percent int, width int) string {
	if percent < 0 { percent = 0 }
	if percent > 100 { percent = 100 }
	filled := (percent * width) / 100
	empty := width - filled
	color := "[green]"
	if percent >= config.CritThreshold { color = "[red]" } else if percent >= config.WarnThreshold { color = "[yellow]" }
	return color + strings.Repeat("█", filled) + "[gray]" + strings.Repeat("░", empty) + "[white]" + fmt.Sprintf(" %3d%%", percent)
}

func parsePercent(s string) int {
	s = strings.TrimSuffix(s, "%")
	pct, _ := strconv.Atoi(s)
	return pct
}

func getPVLunInfo(pv string) (pvid, lunID, vendor, product string) {
	for _, line := range strings.Split(runCmd("lsattr", "-El", pv), "\n") {
		fields := strings.Fields(line)
		if len(fields) >= 2 {
			if fields[0] == "pvid" { pvid = fields[1] }
			if fields[0] == "unique_id" { lunID = fields[1] }
		}
	}
	for _, line := range strings.Split(runCmd("lsmpio", "-ql", pv), "\n") {
		line = strings.TrimSpace(line)
		if strings.HasPrefix(line, "Vendor Id:") { vendor = strings.TrimSpace(strings.TrimPrefix(line, "Vendor Id:")) }
		if strings.HasPrefix(line, "Product Id:") { product = strings.TrimSpace(strings.TrimPrefix(line, "Product Id:")) }
	}
	return
}

func humanSize(mb float64) string {
	if mb >= 1024 { return fmt.Sprintf("%.1fG", mb/1024) }
	return fmt.Sprintf("%.0fM", mb)
}

// getUnusedDisks returns disks not in any VG, with status
// Returns: disk name, hasPVID (has VGDA/remnants), size
func getUnusedDisks() []map[string]string {
	var result []map[string]string
	for _, line := range strings.Split(runCmd("lspv"), "\n") {
		fields := strings.Fields(line)
		if len(fields) >= 3 && strings.HasPrefix(fields[0], "hdisk") {
			vgName := fields[2]
			if vgName == "None" {
				disk := fields[0]
				pvid := fields[1]
				
				// Check if it has PVID (indicates VGDA remnants)
				hasPVID := pvid != "none" && pvid != ""
				
				// Get disk size
				sizeStr := ""
				for _, l := range strings.Split(runCmd("bootinfo", "-s", disk), "\n") {
					if strings.TrimSpace(l) != "" {
						sizeMB, _ := strconv.ParseFloat(strings.TrimSpace(l), 64)
						sizeStr = humanSize(sizeMB)
						break
					}
				}
				
				// Get vendor info
				_, _, vendor, product := getPVLunInfo(disk)
				lunInfo := vendor + " " + product
				if lunInfo == " " { lunInfo = "(virtual)" }
				
				status := "empty"
				if hasPVID { status = "has_vgda" }
				
				result = append(result, map[string]string{
					"disk": disk,
					"pvid": pvid,
					"size": sizeStr,
					"lun":  lunInfo,
					"status": status,
				})
			}
		}
	}
	return result
}

// getFilesystems - compatible with AIX and GNU df, includes NFS
func getFilesystems(includeNFS bool) []map[string]string {
	var result []map[string]string
	dfOutput := runCmd("df", "-m")
	lines := strings.Split(dfOutput, "\n")
	isGNU := len(lines) > 0 && strings.Contains(lines[0], "Available")
	
	for _, line := range lines {
		fields := strings.Fields(line)
		if len(fields) == 0 { continue }
		
		isLocal := strings.HasPrefix(fields[0], "/dev/")
		isNFS := strings.Contains(fields[0], ":")
		
		if !isLocal && !(includeNFS && isNFS) { continue }
		
		var fs map[string]string
		if isGNU && len(fields) >= 6 {
			sizeMB, _ := strconv.ParseFloat(fields[1], 64)
			usedMB, _ := strconv.ParseFloat(fields[2], 64)
			freeMB := sizeMB - usedMB
			fsType := "jfs2"
			if isNFS { fsType = "nfs" }
			fs = map[string]string{
				"device": fields[0], "size": fmt.Sprintf("%.2f", sizeMB),
				"free": fmt.Sprintf("%.2f", freeMB), "pct": fields[4], 
				"mount": fields[5], "type": fsType,
			}
		} else if len(fields) >= 7 {
			fsType := "jfs2"
			if isNFS { fsType = "nfs" }
			fs = map[string]string{
				"device": fields[0], "size": fields[1], "free": fields[2],
				"pct": fields[3], "mount": fields[6], "type": fsType,
			}
		}
		if fs != nil { result = append(result, fs) }
	}
	return result
}

// stripColors removes tview color tags for export
func stripColors(s string) string {
	result := s
	colors := []string{"[green]", "[red]", "[yellow]", "[cyan]", "[gray]", "[white]", "[magenta]",
		"[green::b]", "[yellow::b]", "[red::b]", "[cyan::b]", "[magenta::b]"}
	for _, c := range colors {
		result = strings.ReplaceAll(result, c, "")
	}
	return result
}

// ============ DASHBOARD ============
func getDashboard() string {
	text := fmt.Sprintf(`[yellow::b]
  ███████╗████████╗ ██████╗ ████████╗██╗   ██╗██╗
  ██╔════╝╚══██╔══╝██╔════╝ ╚══██╔══╝██║   ██║██║
  ███████╗   ██║   ██║  ███╗   ██║   ██║   ██║██║
  ╚════██║   ██║   ██║   ██║   ██║   ██║   ██║██║
  ███████║   ██║   ╚██████╔╝   ██║   ╚██████╔╝██║
  ╚══════╝   ╚═╝    ╚═════╝    ╚═╝    ╚═════╝ ╚═╝
[white]       [gray]AIX Storage Explorer v1.1 - LibrePower[white]
[gray]       Thresholds: warn=%d%% crit=%d%% (~/.stgtuirc)[white]

`, config.WarnThreshold, config.CritThreshold)

	fsAlerts := 0
	filesystems := getFilesystems(true)
	for _, fs := range filesystems {
		if parsePercent(fs["pct"]) >= config.WarnThreshold { fsAlerts++ }
	}
	
	healthIcon := "[green]● HEALTHY[white]"
	if fsAlerts > 0 { healthIcon = fmt.Sprintf("[red]● %d FS ALERT(S)[white]", fsAlerts) }
	text += fmt.Sprintf("  Status: %s    [gray]Press 3 for details[white]\n\n", healthIcon)

	// VGs
	text += "[yellow::b]═══ VOLUME GROUPS ═══[white]\n"
	text += fmt.Sprintf("  %-12s %-7s %8s %8s %s\n", "VG", "STATE", "SIZE", "FREE", "PP USAGE")
	text += "  " + strings.Repeat("─", 60) + "\n"
	
	for _, vg := range strings.Split(strings.TrimSpace(runCmd("lsvg")), "\n") {
		if vg == "" { continue }
		vgInfo := runCmd("lsvg", vg)
		var totalPPs, usedPPs, freePPs int
		var state, ppSizeStr string
		for _, line := range strings.Split(vgInfo, "\n") {
			if strings.Contains(line, "VG STATE:") { state = strings.TrimSpace(strings.Fields(strings.Split(line, "VG STATE:")[1])[0]) }
			if strings.Contains(line, "PP SIZE:") { ppSizeStr = strings.Fields(strings.Split(line, "PP SIZE:")[1])[0] }
			if strings.Contains(line, "TOTAL PPs:") { fmt.Sscanf(strings.Fields(strings.Split(line, "TOTAL PPs:")[1])[0], "%d", &totalPPs) }
			if strings.Contains(line, "USED PPs:") { fmt.Sscanf(strings.Fields(strings.Split(line, "USED PPs:")[1])[0], "%d", &usedPPs) }
			if strings.Contains(line, "FREE PPs:") { 
				idx := strings.LastIndex(line, "FREE PPs:")
				if idx >= 0 { fmt.Sscanf(strings.TrimSpace(line[idx+9:]), "%d", &freePPs) }
			}
		}
		ppSize, _ := strconv.Atoi(ppSizeStr)
		pct := 0
		if totalPPs > 0 { pct = (usedPPs * 100) / totalPPs }
		stateIcon := "[green]●[white]"
		if state != "active" { stateIcon = "[red]●[white]" }
		bar := "[cyan]" + strings.Repeat("█", (pct*18)/100) + "[gray]" + strings.Repeat("░", 18-(pct*18)/100) + fmt.Sprintf("[white] %3d%%", pct)
		text += fmt.Sprintf("  %s %-10s %-7s %8s %8s %s\n", stateIcon, vg, state, 
			humanSize(float64(totalPPs*ppSize)), humanSize(float64(freePPs*ppSize)), bar)
	}

	// Unused Disks
	unusedDisks := getUnusedDisks()
	if len(unusedDisks) > 0 {
		text += "\n[yellow::b]═══ UNUSED DISKS ═══[white]\n"
		text += fmt.Sprintf("  %-10s %-8s %8s  %s\n", "DISK", "STATUS", "SIZE", "LUN INFO")
		text += "  " + strings.Repeat("─", 55) + "\n"
		for _, d := range unusedDisks {
			statusIcon := "[green]○[white]"  // Empty circle = completely clean
			statusText := "clean"
			if d["status"] == "has_vgda" {
				statusIcon = "[magenta]◐[white]"  // Half circle = has VGDA remnants
				statusText = "vgda"
			}
			lunInfo := d["lun"]
			if len(lunInfo) > 25 { lunInfo = lunInfo[:25] }
			text += fmt.Sprintf("  %s %-8s [yellow]%-6s[white] %8s  %s\n", statusIcon, d["disk"], statusText, d["size"], lunInfo)
		}
		text += "  [gray]○ clean (ready to use)  ◐ vgda remnants (needs recreatevg/chpv -C)[white]\n"
	}

	// Paging
	text += "\n[yellow::b]═══ PAGING SPACE ═══[white]\n"
	for _, line := range strings.Split(runCmd("lsps", "-a"), "\n") {
		fields := strings.Fields(line)
		if len(fields) >= 5 && strings.HasPrefix(fields[0], "hd") {
			pct := parsePercent(fields[4])
			text += fmt.Sprintf("  %-12s %-10s %8s %s\n", fields[0], fields[2], fields[3], progressBar(pct, 18))
		}
	}

	// Filesystems (including NFS)
	text += "\n[yellow::b]═══ FILESYSTEMS ═══[white]\n"
	text += fmt.Sprintf("  %-20s %-4s %8s %8s %s\n", "MOUNT", "TYPE", "SIZE", "FREE", "USAGE")
	text += "  " + strings.Repeat("─", 62) + "\n"
	
	for _, fs := range filesystems {
		mount := fs["mount"]
		sizeMB, _ := strconv.ParseFloat(fs["size"], 64)
		freeMB, _ := strconv.ParseFloat(fs["free"], 64)
		pct := parsePercent(fs["pct"])
		displayMount := mount
		if len(displayMount) > 20 { displayMount = "..." + displayMount[len(displayMount)-17:] }
		fsType := fs["type"]
		if fsType == "" { fsType = "jfs2" }
		text += fmt.Sprintf("  %-20s %-4s %8s %8s %s\n", displayMount, fsType, humanSize(sizeMB), humanSize(freeMB), progressBar(pct, 16))
	}
	
	return text
}

// ============ IOSTAT VIEW ============
func getIOStats() string {
	text := "[yellow::b]═══ DISK I/O STATISTICS ═══[white]\n"
	text += fmt.Sprintf("[gray]Updated: %s[white]\n\n", time.Now().Format("15:04:05"))
	
	iostatOut := runCmd("iostat", "-d", "1", "1")
	lines := strings.Split(iostatOut, "\n")
	
	text += fmt.Sprintf("  %-10s %8s %10s %8s %12s %12s\n", "DISK", "%TM_ACT", "KBPS", "TPS", "KB_READ", "KB_WRTN")
	text += "  " + strings.Repeat("─", 66) + "\n"
	
	for _, line := range lines {
		fields := strings.Fields(line)
		if len(fields) >= 6 && strings.HasPrefix(fields[0], "hdisk") {
			disk := fields[0]
			tmAct, _ := strconv.ParseFloat(fields[1], 64)
			kbps := fields[2]
			tps := fields[3]
			kbRead := fields[4]
			kbWrtn := fields[5]
			
			// Color based on activity
			actBar := "[green]"
			if tmAct >= 80 { actBar = "[red]" } else if tmAct >= 50 { actBar = "[yellow]" }
			actBar += fmt.Sprintf("%6.1f%%", tmAct) + "[white]"
			
			text += fmt.Sprintf("  %-10s %s %10s %8s %12s %12s\n", disk, actBar, kbps, tps, kbRead, kbWrtn)
		}
	}
	
	text += "\n[gray]Press 'r' to refresh, any other key to go back[white]\n"
	return text
}

// ============ MIRROR STATUS ============
func getMirrorStatus() string {
	text := "[yellow::b]═══ MIRROR STATUS ═══[white]\n\n"
	
	for _, vg := range strings.Split(strings.TrimSpace(runCmd("lsvg")), "\n") {
		if vg == "" { continue }
		text += fmt.Sprintf("[cyan::b]%s[white]\n", vg)
		
		lvOut := runCmd("lsvg", "-l", vg)
		for i, line := range strings.Split(lvOut, "\n") {
			if i < 2 { continue }
			fields := strings.Fields(line)
			if len(fields) >= 6 {
				lvName := fields[0]
				lps, _ := strconv.Atoi(fields[2])
				pps, _ := strconv.Atoi(fields[3])
				copies := 1
				if lps > 0 { copies = pps / lps }
				
				mirrorIcon := "[gray]○[white]"
				mirrorText := "single"
				if copies == 2 {
					mirrorIcon = "[green]◐[white]"
					mirrorText = "2-way"
				} else if copies >= 3 {
					mirrorIcon = "[green]●[white]"
					mirrorText = fmt.Sprintf("%d-way", copies)
				}
				
				state := fields[4]
				stateColor := "[green]"
				if strings.Contains(strings.ToLower(state), "stale") {
					stateColor = "[red]"
				}
				
				text += fmt.Sprintf("  %s %-15s %-6s %s%s[white]\n", mirrorIcon, lvName, mirrorText, stateColor, state)
			}
		}
		text += "\n"
	}
	
	text += "[gray]Legend: ○=single ◐=2-way mirror ●=3-way mirror[white]\n"
	return text
}

// ============ HEALTH CHECK ============
func getHealthCheck() string {
	text := "[yellow::b]═══ STORAGE HEALTH CHECK ═══[white]\n\n"
	issues := 0
	
	// Stale PPs
	text += "[cyan]● Stale Physical Partitions[white]\n"
	hasStale := false
	for _, vg := range strings.Split(strings.TrimSpace(runCmd("lsvg")), "\n") {
		if vg == "" { continue }
		for _, line := range strings.Split(runCmd("lsvg", "-l", vg), "\n") {
			if strings.Contains(strings.ToLower(line), "stale") {
				hasStale = true
				issues++
				text += fmt.Sprintf("  [red]✖ STALE: %s[white]\n", strings.TrimSpace(line))
			}
		}
	}
	if !hasStale { text += "  [green]✓ No stale partitions[white]\n" }
	
	// Quorum
	text += "\n[cyan]● Volume Group Quorum[white]\n"
	for _, vg := range strings.Split(strings.TrimSpace(runCmd("lsvg")), "\n") {
		if vg == "" { continue }
		for _, line := range strings.Split(runCmd("lsvg", vg), "\n") {
			if strings.Contains(line, "QUORUM:") {
				if strings.Contains(line, "Enabled") {
					text += fmt.Sprintf("  [green]✓ %s: Quorum Enabled[white]\n", vg)
				} else {
					text += fmt.Sprintf("  [yellow]⚠ %s: Quorum Disabled[white]\n", vg)
				}
			}
		}
	}
	
	// Multipath
	text += "\n[cyan]● Multipath Status[white]\n"
	pathOut := runCmd("lspath")
	pathCount, failedPaths := 0, 0
	for _, line := range strings.Split(pathOut, "\n") {
		fields := strings.Fields(line)
		if len(fields) >= 2 {
			pathCount++
			if fields[0] != "Enabled" {
				failedPaths++
				issues++
				text += fmt.Sprintf("  [red]✖ %s %s: %s[white]\n", fields[1], fields[2], fields[0])
			}
		}
	}
	if failedPaths == 0 && pathCount > 0 {
		text += fmt.Sprintf("  [green]✓ All %d paths healthy[white]\n", pathCount)
	} else if pathCount == 0 {
		text += "  [gray]- No multipath configured[white]\n"
	}
	
	// Paging
	text += "\n[cyan]● Paging Space[white]\n"
	for _, line := range strings.Split(runCmd("lsps", "-a"), "\n") {
		fields := strings.Fields(line)
		if len(fields) >= 5 && strings.HasPrefix(fields[0], "hd") {
			pct := parsePercent(fields[4])
			if pct >= config.WarnThreshold {
				issues++
				icon := "[yellow]⚠[white]"
				if pct >= config.CritThreshold { icon = "[red]✖[white]" }
				text += fmt.Sprintf("  %s %s: %d%% used[white]\n", icon, fields[0], pct)
			} else {
				text += fmt.Sprintf("  [green]✓ %s: %d%% used[white]\n", fields[0], pct)
			}
		}
	}
	
	// Disk Errors
	text += "\n[cyan]● Recent Disk Errors (errpt)[white]\n"
	errptOut := runCmd("errpt", "-d", "H")
	diskErrors := 0
	for _, line := range strings.Split(errptOut, "\n") {
		if strings.Contains(strings.ToLower(line), "hdisk") {
			diskErrors++
			if diskErrors <= 3 {
				fields := strings.Fields(line)
				if len(fields) >= 6 {
					text += fmt.Sprintf("  [red]✖ %s %s %s[white]\n", fields[0], fields[4], strings.Join(fields[5:], " "))
				}
			}
		}
	}
	if diskErrors > 3 {
		text += fmt.Sprintf("  [red]  ... and %d more disk errors[white]\n", diskErrors-3)
		issues += diskErrors
	} else if diskErrors > 0 {
		issues += diskErrors
	} else {
		text += "  [green]✓ No disk errors in errpt[white]\n"
	}
	
	// Unused disks with VGDA
	text += "\n[cyan]● Unused Disks[white]\n"
	unusedDisks := getUnusedDisks()
	vgdaDisks := 0
	for _, d := range unusedDisks {
		if d["status"] == "has_vgda" {
			vgdaDisks++
			text += fmt.Sprintf("  [yellow]⚠ %s has VGDA remnants (chpv -C to clear)[white]\n", d["disk"])
		}
	}
	if vgdaDisks == 0 && len(unusedDisks) > 0 {
		text += fmt.Sprintf("  [green]✓ %d unused disk(s), all clean[white]\n", len(unusedDisks))
	} else if len(unusedDisks) == 0 {
		text += "  [gray]- No unused disks[white]\n"
	}
	
	// FS Usage
	text += "\n[cyan]● Filesystem Capacity[white]\n"
	fsIssues := 0
	for _, fs := range getFilesystems(true) {
		pct := parsePercent(fs["pct"])
		if pct >= config.WarnThreshold {
			fsIssues++
			issues++
			icon := "[yellow]⚠[white]"
			if pct >= config.CritThreshold { icon = "[red]✖[white]" }
			text += fmt.Sprintf("  %s %s: %d%% full[white]\n", icon, fs["mount"], pct)
		}
	}
	if fsIssues == 0 { text += fmt.Sprintf("  [green]✓ All filesystems below %d%%[white]\n", config.WarnThreshold) }
	
	// Summary
	text += "\n" + strings.Repeat("─", 50) + "\n"
	if issues == 0 {
		text += "[green::b]✓ SYSTEM HEALTHY - No issues detected[white]\n"
	} else {
		text += fmt.Sprintf("[red::b]✖ %d ISSUE(S) REQUIRE ATTENTION[white]\n", issues)
	}
	text += fmt.Sprintf("\n[gray]Thresholds: warn=%d%% crit=%d%% (edit ~/.stgtuirc)[white]\n", config.WarnThreshold, config.CritThreshold)
	
	return text
}

// ============ VG DETAILS ============
func getVGDetails() string {
	text := "[yellow::b]═══ VOLUME GROUP DETAILS ═══[white]\n\n"
	
	for _, vg := range strings.Split(strings.TrimSpace(runCmd("lsvg")), "\n") {
		if vg == "" { continue }
		vgInfo := runCmd("lsvg", vg)
		var totalPPs, usedPPs, freePPs int
		var state, ppSize, quorum string
		for _, line := range strings.Split(vgInfo, "\n") {
			if strings.Contains(line, "VG STATE:") { state = strings.TrimSpace(strings.Fields(strings.Split(line, "VG STATE:")[1])[0]) }
			if strings.Contains(line, "PP SIZE:") { ppSize = strings.Fields(strings.Split(line, "PP SIZE:")[1])[0] + "MB" }
			if strings.Contains(line, "QUORUM:") { 
				parts := strings.Split(line, "QUORUM:")
				if len(parts) > 1 { quorum = strings.TrimSpace(parts[1]) }
			}
			if strings.Contains(line, "TOTAL PPs:") { fmt.Sscanf(strings.Fields(strings.Split(line, "TOTAL PPs:")[1])[0], "%d", &totalPPs) }
			if strings.Contains(line, "USED PPs:") { fmt.Sscanf(strings.Fields(strings.Split(line, "USED PPs:")[1])[0], "%d", &usedPPs) }
			if strings.Contains(line, "FREE PPs:") { 
				idx := strings.LastIndex(line, "FREE PPs:")
				if idx >= 0 { fmt.Sscanf(strings.TrimSpace(line[idx+9:]), "%d", &freePPs) }
			}
		}
		pct := 0
		if totalPPs > 0 { pct = (usedPPs * 100) / totalPPs }
		stateColor := "[green]"
		if state != "active" { stateColor = "[red]" }
		
		text += fmt.Sprintf("[cyan::b]%s[white]  %s%s[white]  PP: %s  Quorum: %s\n", vg, stateColor, state, ppSize, quorum)
		text += fmt.Sprintf("  PPs: %d total, %d used, %d free\n", totalPPs, usedPPs, freePPs)
		bar := "[cyan]" + strings.Repeat("█", (pct*40)/100) + "[gray]" + strings.Repeat("░", 40-(pct*40)/100) + fmt.Sprintf("[white] %3d%%", pct)
		text += fmt.Sprintf("  %s\n\n", bar)
		
		text += "  [gray]Physical Volumes:[white]\n"
		for i, line := range strings.Split(runCmd("lsvg", "-p", vg), "\n") {
			if i < 2 { continue }
			fields := strings.Fields(line)
			if len(fields) >= 5 && strings.HasPrefix(fields[0], "hdisk") {
				pvTotal, _ := strconv.Atoi(fields[2])
				pvFree, _ := strconv.Atoi(fields[3])
				pvUsed := pvTotal - pvFree
				pvPct := 0
				if pvTotal > 0 { pvPct = (pvUsed * 100) / pvTotal }
				pvStateIcon := "[green]●[white]"
				if fields[1] != "active" { pvStateIcon = "[red]●[white]" }
				pvBar := "[cyan]" + strings.Repeat("█", (pvPct*15)/100) + "[gray]" + strings.Repeat("░", 15-(pvPct*15)/100) + fmt.Sprintf("[white] %3d%%", pvPct)
				text += fmt.Sprintf("    %s %-10s %4d/%4d PPs %s\n", pvStateIcon, fields[0], pvUsed, pvTotal, pvBar)
			}
		}
		text += "\n"
	}
	return text
}

// ============ LV STATUS ============
func getLVDetails() string {
	text := "[yellow::b]═══ LOGICAL VOLUME STATUS ═══[white]\n\n"
	text += fmt.Sprintf("  %-15s %-10s %-8s %5s  %-12s %s\n", "LV", "VG", "TYPE", "LPs", "STATE", "MOUNT")
	text += "  " + strings.Repeat("─", 70) + "\n"
	
	for _, vg := range strings.Split(strings.TrimSpace(runCmd("lsvg")), "\n") {
		if vg == "" { continue }
		for i, line := range strings.Split(runCmd("lsvg", "-l", vg), "\n") {
			if i < 2 { continue }
			fields := strings.Fields(line)
			if len(fields) >= 6 {
				lvName, lvType, lps, lvState, mount := fields[0], fields[1], fields[2], fields[4], fields[5]
				if mount == "N/A" { mount = "[gray]N/A[white]" }
				stateIcon := "[green]●[white]"
				stateText := lvState
				if strings.Contains(strings.ToLower(lvState), "stale") {
					stateIcon = "[red]●[white]"
					stateText = "[red]" + lvState + "[white]"
				} else if strings.Contains(lvState, "syncd") {
					stateText = "[green]" + lvState + "[white]"
				}
				text += fmt.Sprintf("  %s %-13s %-10s %-8s %5s  %-12s %s\n", stateIcon, lvName, vg, lvType, lps, stateText, mount)
			}
		}
	}
	return text
}

// ============ FS → LUN MAPPING ============
func mapFStoLUN(mount string) string {
	var device, sizeMB, freeMB string
	var pct int
	for _, fs := range getFilesystems(true) {
		if fs["mount"] == mount {
			device = fs["device"]
			sizeMB = fs["size"]
			freeMB = fs["free"]
			pct = parsePercent(fs["pct"])
			break
		}
	}
	if device == "" { return "[red]Cannot find device for " + mount + "[white]" }
	
	// Check if NFS
	if strings.Contains(device, ":") {
		parts := strings.SplitN(device, ":", 2)
		server, path := parts[0], ""
		if len(parts) > 1 { path = parts[1] }
		sizef, _ := strconv.ParseFloat(sizeMB, 64)
		freef, _ := strconv.ParseFloat(freeMB, 64)
		return fmt.Sprintf(`[yellow::b]═══ NFS MOUNT ═══[white]

[green]▼ FILESYSTEM[white]
  ┌─────────────────────────────────────────────────┐
  │  Mount:    [cyan]%-35s[white]  │
  │  Type:     NFS                                    │
  │  Server:   %-35s  │
  │  Path:     %-35s  │
  │  Size:     %-12s Free: %-12s    │
  │  Usage:    %s  │
  └─────────────────────────────────────────────────┘
`, mount, server, path, humanSize(sizef), humanSize(freef), progressBar(pct, 30))
	}
	
	lv := strings.TrimPrefix(device, "/dev/")
	var vg, lvType, lvState, lps string
	for _, line := range strings.Split(runCmd("lslv", lv), "\n") {
		if strings.Contains(line, "VOLUME GROUP:") { vg = strings.TrimSpace(strings.Fields(strings.Split(line, "VOLUME GROUP:")[1])[0]) }
		if strings.Contains(line, "TYPE:") && !strings.Contains(line, "BB POLICY") { lvType = strings.TrimSpace(strings.Fields(strings.Split(line, "TYPE:")[1])[0]) }
		if strings.Contains(line, "LV STATE:") { lvState = strings.TrimSpace(strings.Split(line, "LV STATE:")[1]) }
		if strings.Contains(line, "LPs:") { lps = strings.TrimSpace(strings.Fields(strings.Split(line, "LPs:")[1])[0]) }
	}
	
	var vgState, ppSize string
	var vgTotalPPs, vgUsedPPs int
	for _, line := range strings.Split(runCmd("lsvg", vg), "\n") {
		if strings.Contains(line, "VG STATE:") { vgState = strings.TrimSpace(strings.Fields(strings.Split(line, "VG STATE:")[1])[0]) }
		if strings.Contains(line, "PP SIZE:") { ppSize = strings.TrimSpace(strings.Split(line, "PP SIZE:")[1]) }
		if strings.Contains(line, "TOTAL PPs:") { fmt.Sscanf(strings.Fields(strings.Split(line, "TOTAL PPs:")[1])[0], "%d", &vgTotalPPs) }
		if strings.Contains(line, "USED PPs:") { fmt.Sscanf(strings.Fields(strings.Split(line, "USED PPs:")[1])[0], "%d", &vgUsedPPs) }
	}
	vgPct := 0
	if vgTotalPPs > 0 { vgPct = (vgUsedPPs * 100) / vgTotalPPs }
	
	pvSet := make(map[string]bool)
	var pvList []string
	for _, line := range strings.Split(runCmd("lslv", "-m", lv), "\n") {
		fields := strings.Fields(line)
		if len(fields) >= 3 && strings.HasPrefix(fields[2], "hdisk") {
			pv := fields[2]
			if !pvSet[pv] { pvSet[pv] = true; pvList = append(pvList, pv) }
		}
	}
	
	sizef, _ := strconv.ParseFloat(sizeMB, 64)
	freef, _ := strconv.ParseFloat(freeMB, 64)
	vgBar := "[cyan]" + strings.Repeat("█", (vgPct*30)/100) + "[gray]" + strings.Repeat("░", 30-(vgPct*30)/100) + fmt.Sprintf("[white] %3d%%", vgPct)
	
	text := fmt.Sprintf(`[yellow::b]═══ FILESYSTEM → STORAGE MAPPING ═══[white]

[green]▼ FILESYSTEM[white]
  ┌─────────────────────────────────────────────────┐
  │  Mount:    [cyan]%-35s[white]  │
  │  Device:   %-35s  │
  │  Size:     %-12s Free: %-12s    │
  │  Usage:    %s  │
  └─────────────────────────────────────────────────┘
           │
           ▼
[green]▼ LOGICAL VOLUME[white]
  ┌─────────────────────────────────────────────────┐
  │  LV Name:  [cyan]%-35s[white]  │
  │  Type:     %-35s  │
  │  State:    %-35s  │
  │  LPs:      %-35s  │
  └─────────────────────────────────────────────────┘
           │
           ▼
[green]▼ VOLUME GROUP[white]
  ┌─────────────────────────────────────────────────┐
  │  VG Name:  [cyan]%-35s[white]  │
  │  State:    %-35s  │
  │  PP Size:  %-35s  │
  │  PP Alloc: %s  │
  └─────────────────────────────────────────────────┘
           │
           ▼
[green]▼ PHYSICAL VOLUMES (%d disk(s))[white]
`, mount, device, humanSize(sizef), humanSize(freef), progressBar(pct, 30),
		lv, lvType, lvState, lps, vg, vgState, ppSize, vgBar, len(pvList))

	for _, pv := range pvList {
		pvid, lunID, vendor, product := getPVLunInfo(pv)
		lunInfo := lunID
		if lunInfo == "" { lunInfo = vendor + " " + product }
		if lunInfo == "" || lunInfo == " " { lunInfo = "(virtual disk)" }
		if len(lunInfo) > 35 { lunInfo = lunInfo[:35] }
		
		text += fmt.Sprintf(`  ┌─────────────────────────────────────────────────┐
  │  Disk:     [cyan]%-35s[white]  │
  │  PVID:     %-35s  │
  │  LUN ID:   %-35s  │
  └─────────────────────────────────────────────────┘
`, pv, pvid, lunInfo)
	}
	
	return text
}

// ============ LUN → FS MAPPING ============
func mapLUNtoFS(pv string) string {
	pvid, lunID, vendor, product := getPVLunInfo(pv)
	lunInfo := lunID
	if lunInfo == "" { lunInfo = vendor + " " + product }
	if lunInfo == "" || lunInfo == " " { lunInfo = "(virtual disk)" }
	
	var totalPPs, freePPs, usedPPs int
	var vgName string
	for _, line := range strings.Split(runCmd("lspv", pv), "\n") {
		if strings.Contains(line, "VOLUME GROUP:") { 
			f := strings.Fields(strings.Split(line, "VOLUME GROUP:")[1])
			if len(f) > 0 { vgName = f[0] }
		}
		if strings.Contains(line, "TOTAL PPs:") { fmt.Sscanf(strings.Fields(strings.Split(line, "TOTAL PPs:")[1])[0], "%d", &totalPPs) }
		if strings.Contains(line, "FREE PPs:") { fmt.Sscanf(strings.Fields(strings.Split(line, "FREE PPs:")[1])[0], "%d", &freePPs) }
	}
	usedPPs = totalPPs - freePPs
	pct := 0
	if totalPPs > 0 { pct = (usedPPs * 100) / totalPPs }
	
	pathInfo := ""
	for _, line := range strings.Split(runCmd("lspath", "-l", pv), "\n") {
		fields := strings.Fields(line)
		if len(fields) >= 3 {
			pathColor := "[green]"
			if fields[0] != "Enabled" { pathColor = "[red]" }
			pathInfo += fmt.Sprintf("%s%s[white] ", pathColor, fields[0])
		}
	}
	if pathInfo == "" { pathInfo = "[gray]single path[white]" }
	
	pvBar := "[cyan]" + strings.Repeat("█", (pct*30)/100) + "[gray]" + strings.Repeat("░", 30-(pct*30)/100) + fmt.Sprintf("[white] %3d%%", pct)
	
	text := fmt.Sprintf(`[yellow::b]═══ STORAGE → FILESYSTEM MAPPING ═══[white]

[green]▼ PHYSICAL VOLUME (LUN)[white]
  ┌─────────────────────────────────────────────────┐
  │  Disk:     [cyan]%-35s[white]  │
  │  PVID:     %-35s  │
  │  LUN ID:   %-35s  │
  │  PPs:      %-4d total, %-4d used, %-4d free      │
  │  Paths:    %-35s  │
  │  PP Alloc: %s  │
  └─────────────────────────────────────────────────┘
           │
           ▼
`, pv, pvid, lunInfo, totalPPs, usedPPs, freePPs, pathInfo, pvBar)

	if vgName == "" || vgName == "None" {
		// Check if it has VGDA remnants
		if pvid != "none" && pvid != "" {
			text += "  [magenta]◐ Disk has VGDA remnants but not assigned to any VG[white]\n"
			text += "  [gray]  Use 'chpv -C " + pv + "' to clear VGDA[white]\n"
		} else {
			text += "  [green]○ Disk is clean and ready to use[white]\n"
			text += "  [gray]  Use 'mkvg -y newvg " + pv + "' to create a VG[white]\n"
		}
		return text
	}
	
	var vgState, ppSize string
	var vgTotalPPs, vgUsedPPs int
	for _, line := range strings.Split(runCmd("lsvg", vgName), "\n") {
		if strings.Contains(line, "VG STATE:") { vgState = strings.TrimSpace(strings.Fields(strings.Split(line, "VG STATE:")[1])[0]) }
		if strings.Contains(line, "PP SIZE:") { ppSize = strings.TrimSpace(strings.Split(line, "PP SIZE:")[1]) }
		if strings.Contains(line, "TOTAL PPs:") { fmt.Sscanf(strings.Fields(strings.Split(line, "TOTAL PPs:")[1])[0], "%d", &vgTotalPPs) }
		if strings.Contains(line, "USED PPs:") { fmt.Sscanf(strings.Fields(strings.Split(line, "USED PPs:")[1])[0], "%d", &vgUsedPPs) }
	}
	vgPct := 0
	if vgTotalPPs > 0 { vgPct = (vgUsedPPs * 100) / vgTotalPPs }
	vgBar := "[cyan]" + strings.Repeat("█", (vgPct*30)/100) + "[gray]" + strings.Repeat("░", 30-(vgPct*30)/100) + fmt.Sprintf("[white] %3d%%", vgPct)
	
	text += fmt.Sprintf(`[green]▼ VOLUME GROUP[white]
  ┌─────────────────────────────────────────────────┐
  │  VG Name:  [cyan]%-35s[white]  │
  │  State:    %-35s  │
  │  PP Size:  %-35s  │
  │  PP Alloc: %s  │
  └─────────────────────────────────────────────────┘
           │
           ▼
[green]▼ LVs & FILESYSTEMS on this disk[white]
`, vgName, vgState, ppSize, vgBar)

	text += fmt.Sprintf("  %-15s %-8s %6s  %-12s %s\n", "LV", "TYPE", "LPs", "STATE", "MOUNT")
	text += "  " + strings.Repeat("─", 60) + "\n"
	
	for i, line := range strings.Split(runCmd("lspv", "-l", pv), "\n") {
		if i < 2 || strings.TrimSpace(line) == "" { continue }
		fields := strings.Fields(line)
		if len(fields) >= 2 {
			lvName, lps := fields[0], fields[1]
			var lvType, mount, lvState string
			for _, l := range strings.Split(runCmd("lslv", lvName), "\n") {
				if strings.Contains(l, "TYPE:") && !strings.Contains(l, "BB") { lvType = strings.TrimSpace(strings.Fields(strings.Split(l, "TYPE:")[1])[0]) }
				if strings.Contains(l, "MOUNT POINT:") { mount = strings.TrimSpace(strings.Split(l, "MOUNT POINT:")[1]) }
				if strings.Contains(l, "LV STATE:") { lvState = strings.TrimSpace(strings.Split(l, "LV STATE:")[1]) }
			}
			if mount == "" { mount = "[gray]N/A[white]" }
			stateColor := "[green]"
			if strings.Contains(strings.ToLower(lvState), "stale") { stateColor = "[red]" }
			text += fmt.Sprintf("  [cyan]%-15s[white] %-8s %6s  %s%-12s[white] %s\n", lvName, lvType, lps, stateColor, lvState, mount)
		}
	}
	
	return text
}

// ============ MAIN ============
func main() {
	loadConfig()
	
	app := tview.NewApplication()
	pages := tview.NewPages()
	
	// Create views
	dashView := tview.NewTextView().SetDynamicColors(true).SetScrollable(true)
	dashView.SetBorder(true).SetTitle(" Dashboard [1] ")
	
	vgView := tview.NewTextView().SetDynamicColors(true).SetScrollable(true)
	vgView.SetBorder(true).SetTitle(" VG Details [2] ")
	
	healthView := tview.NewTextView().SetDynamicColors(true).SetScrollable(true)
	healthView.SetBorder(true).SetTitle(" Health Check [3] ")
	
	lvView := tview.NewTextView().SetDynamicColors(true).SetScrollable(true)
	lvView.SetBorder(true).SetTitle(" LV Status [4] ")
	
	iostatView := tview.NewTextView().SetDynamicColors(true).SetScrollable(true)
	iostatView.SetBorder(true).SetTitle(" I/O Stats [7] ")
	
	mirrorView := tview.NewTextView().SetDynamicColors(true).SetScrollable(true)
	mirrorView.SetBorder(true).SetTitle(" Mirror Status [8] ")
	
	detailView := tview.NewTextView().SetDynamicColors(true).SetScrollable(true)
	detailView.SetBorder(true)
	
	pvList := tview.NewList().ShowSecondaryText(false).SetHighlightFullLine(true)
	pvList.SetBorder(true).SetTitle(" Disk → FS [5] ")
	
	fsList := tview.NewList().ShowSecondaryText(false).SetHighlightFullLine(true)
	fsList.SetBorder(true).SetTitle(" FS → Disk [6] ")
	
	// Search modal
	searchInput := tview.NewInputField().SetLabel("Search: ").SetFieldWidth(30)
	searchModal := tview.NewFlex().SetDirection(tview.FlexRow).
		AddItem(nil, 0, 1, false).
		AddItem(tview.NewFlex().
			AddItem(nil, 0, 1, false).
			AddItem(searchInput, 40, 0, true).
			AddItem(nil, 0, 1, false), 3, 0, true).
		AddItem(nil, 0, 1, false)
	
	help := tview.NewTextView().
		SetText(" [yellow]1-6[white] Views  [yellow]7[white] I/O  [yellow]8[white] Mirror  [yellow]/[white] Search  [yellow]e[white] Export  [yellow]r[white] Refresh  [yellow]j/k[white] Scroll  [yellow]q[white] Quit").
		SetDynamicColors(true)
	
	var currentView string
	var lastExportText string
	
	refresh := func() {
		dashView.SetText(getDashboard())
		vgView.SetText(getVGDetails())
		healthView.SetText(getHealthCheck())
		lvView.SetText(getLVDetails())
		iostatView.SetText(getIOStats())
		mirrorView.SetText(getMirrorStatus())
		
		pvList.Clear()
		// First add unused disks
		for _, d := range getUnusedDisks() {
			statusIcon := "○"
			if d["status"] == "has_vgda" { statusIcon = "◐" }
			lunInfo := d["lun"]
			if len(lunInfo) > 16 { lunInfo = lunInfo[:16] }
			pvList.AddItem(fmt.Sprintf("%s %-10s %-12s %s", statusIcon, d["disk"], "(unused)", lunInfo), "", 0, nil)
		}
		// Then add disks in VGs
		for _, line := range strings.Split(runCmd("lspv"), "\n") {
			fields := strings.Fields(line)
			if len(fields) >= 3 && strings.HasPrefix(fields[0], "hdisk") {
				if fields[2] == "None" { continue } // Skip unused, already added
				_, _, vendor, product := getPVLunInfo(fields[0])
				lunInfo := vendor + " " + product
				if lunInfo == " " { lunInfo = "(virtual)" }
				if len(lunInfo) > 16 { lunInfo = lunInfo[:16] }
				pvList.AddItem(fmt.Sprintf("● %-10s %-12s %s", fields[0], fields[2], lunInfo), "", 0, nil)
			}
		}
		
		fsList.Clear()
		for _, fs := range getFilesystems(true) {
			pct := parsePercent(fs["pct"])
			bar := "[green]"
			if pct >= config.CritThreshold { bar = "[red]" } else if pct >= config.WarnThreshold { bar = "[yellow]" }
			bar += fmt.Sprintf("%3d%%", pct) + "[white]"
			fsType := fs["type"]
			if fsType == "" { fsType = "jfs2" }
			fsList.AddItem(fmt.Sprintf("%-24s %-4s %s", fs["mount"], fsType, bar), "", 0, nil)
		}
	}
	
	// Export function
	exportView := func() {
		var content string
		switch currentView {
		case "dash": content = getDashboard()
		case "vg": content = getVGDetails()
		case "health": content = getHealthCheck()
		case "lv": content = getLVDetails()
		case "iostat": content = getIOStats()
		case "mirror": content = getMirrorStatus()
		case "detail": content = lastExportText
		default: content = getDashboard()
		}
		
		plain := stripColors(content)
		filename := fmt.Sprintf("/tmp/stgtui-report-%s.txt", time.Now().Format("20060102-150405"))
		os.WriteFile(filename, []byte(plain), 0644)
		
		// Show notification
		modal := tview.NewModal().
			SetText(fmt.Sprintf("Exported to:\n%s", filename)).
			AddButtons([]string{"OK"}).
			SetDoneFunc(func(buttonIndex int, buttonLabel string) {
				pages.RemovePage("export-modal")
			})
		pages.AddPage("export-modal", modal, true, true)
	}
	
	// Search function
	doSearch := func(query string) {
		query = strings.ToLower(query)
		results := "[yellow::b]═══ SEARCH RESULTS ═══[white]\n\n"
		results += fmt.Sprintf("Query: [cyan]%s[white]\n\n", query)
		found := 0
		
		// Search VGs
		for _, vg := range strings.Split(strings.TrimSpace(runCmd("lsvg")), "\n") {
			if strings.Contains(strings.ToLower(vg), query) {
				results += fmt.Sprintf("[green]VG:[white] %s\n", vg)
				found++
			}
		}
		
		// Search LVs
		for _, vg := range strings.Split(strings.TrimSpace(runCmd("lsvg")), "\n") {
			for _, line := range strings.Split(runCmd("lsvg", "-l", vg), "\n") {
				if strings.Contains(strings.ToLower(line), query) {
					fields := strings.Fields(line)
					if len(fields) >= 1 {
						results += fmt.Sprintf("[green]LV:[white] %s (VG: %s)\n", fields[0], vg)
						found++
					}
				}
			}
		}
		
		// Search FSs
		for _, fs := range getFilesystems(true) {
			if strings.Contains(strings.ToLower(fs["mount"]), query) ||
			   strings.Contains(strings.ToLower(fs["device"]), query) {
				results += fmt.Sprintf("[green]FS:[white] %s -> %s\n", fs["mount"], fs["device"])
				found++
			}
		}
		
		// Search Disks
		for _, line := range strings.Split(runCmd("lspv"), "\n") {
			if strings.Contains(strings.ToLower(line), query) {
				fields := strings.Fields(line)
				if len(fields) >= 1 {
					results += fmt.Sprintf("[green]PV:[white] %s\n", fields[0])
					found++
				}
			}
		}
		
		if found == 0 {
			results += "[gray]No results found[white]\n"
		} else {
			results += fmt.Sprintf("\n[gray]Found %d result(s)[white]\n", found)
		}
		
		detailView.SetText(results)
		detailView.SetTitle(" Search Results ")
		lastExportText = results
		pages.SwitchToPage("detail")
	}
	
	// Handlers
	pvList.SetSelectedFunc(func(i int, main string, sec string, sh rune) {
		// Extract disk name (skip the status icon)
		parts := strings.Fields(main)
		if len(parts) < 2 { return }
		pv := parts[1]
		content := mapLUNtoFS(pv)
		detailView.SetText(content)
		detailView.SetTitle(fmt.Sprintf(" %s → Filesystems ", pv))
		lastExportText = content
		pages.SwitchToPage("detail")
	})
	
	fsList.SetSelectedFunc(func(i int, main string, sec string, sh rune) {
		mount := strings.Fields(main)[0]
		content := mapFStoLUN(mount)
		detailView.SetText(content)
		detailView.SetTitle(fmt.Sprintf(" %s → Storage ", mount))
		lastExportText = content
		pages.SwitchToPage("detail")
	})
	
	// Setup pages
	for name, view := range map[string]tview.Primitive{
		"dash": dashView, "vg": vgView, "health": healthView, "lv": lvView,
		"pv": pvList, "fs": fsList, "iostat": iostatView, "mirror": mirrorView,
		"detail": detailView,
	} {
		page := tview.NewFlex().SetDirection(tview.FlexRow).AddItem(view, 0, 1, true).AddItem(help, 1, 0, false)
		pages.AddPage(name, page, true, name == "dash")
	}
	pages.AddPage("search", searchModal, true, false)
	
	refresh()
	currentView = "dash"
	
	var lastPage string = "dash"
	
	// Search input handler
	searchInput.SetDoneFunc(func(key tcell.Key) {
		if key == tcell.KeyEnter {
			query := searchInput.GetText()
			pages.HidePage("search")
			if query != "" {
				doSearch(query)
			}
			searchInput.SetText("")
		} else if key == tcell.KeyEscape {
			pages.HidePage("search")
			searchInput.SetText("")
		}
	})
	
	app.SetInputCapture(func(event *tcell.EventKey) *tcell.EventKey {
		// Check if search is active
		if pages.HasPage("search") {
			frontPage, _ := pages.GetFrontPage()
			if frontPage == "search" {
				return event
			}
		}
		
		cp, _ := pages.GetFrontPage()
		if cp != "detail" && cp != "search" { 
			lastPage = cp 
			currentView = cp
		}
		
		// Get current scrollable view for vim keys
		var scrollable *tview.TextView
		switch cp {
		case "dash": scrollable = dashView
		case "vg": scrollable = vgView
		case "health": scrollable = healthView
		case "lv": scrollable = lvView
		case "iostat": scrollable = iostatView
		case "mirror": scrollable = mirrorView
		case "detail": scrollable = detailView
		}
		
		switch event.Rune() {
		case 'q': app.Stop(); return nil
		case '1': pages.SwitchToPage("dash"); currentView = "dash"; return nil
		case '2': pages.SwitchToPage("vg"); currentView = "vg"; return nil
		case '3': pages.SwitchToPage("health"); currentView = "health"; return nil
		case '4': pages.SwitchToPage("lv"); currentView = "lv"; return nil
		case '5': pages.SwitchToPage("pv"); currentView = "pv"; return nil
		case '6': pages.SwitchToPage("fs"); currentView = "fs"; return nil
		case '7': iostatView.SetText(getIOStats()); pages.SwitchToPage("iostat"); currentView = "iostat"; return nil
		case '8': pages.SwitchToPage("mirror"); currentView = "mirror"; return nil
		case 'r': refresh(); return nil
		case 'e': exportView(); return nil
		case '/': pages.ShowPage("search"); app.SetFocus(searchInput); return nil
		// Vim keys
		case 'j': 
			if scrollable != nil {
				row, _ := scrollable.GetScrollOffset()
				scrollable.ScrollTo(row+1, 0)
			}
			return nil
		case 'k':
			if scrollable != nil {
				row, _ := scrollable.GetScrollOffset()
				if row > 0 { scrollable.ScrollTo(row-1, 0) }
			}
			return nil
		case 'g':
			if scrollable != nil { scrollable.ScrollToBeginning() }
			return nil
		case 'G':
			if scrollable != nil { scrollable.ScrollToEnd() }
			return nil
		}
		
		if event.Key() == tcell.KeyEsc { 
			pages.SwitchToPage(lastPage)
			currentView = lastPage
			return nil 
		}
		
		return event
	})
	
	if err := app.SetRoot(pages, true).EnableMouse(false).Run(); err != nil { panic(err) }
}
