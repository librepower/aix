package main

import (
	"fmt"
	"os/exec"
	"strconv"
	"strings"

	"github.com/gdamore/tcell/v2"
	"github.com/rivo/tview"
)

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
	if percent >= 90 { color = "[red]" } else if percent >= 80 { color = "[yellow]" }
	return color + strings.Repeat("█", filled) + "[gray]" + strings.Repeat("░", empty) + "[white]" + fmt.Sprintf(" %3d%%", percent)
}

func parsePercent(s string) int {
	s = strings.TrimSuffix(s, "%")
	pct, _ := strconv.Atoi(s)
	return pct
}

func getPVLunInfo(pv string) (pvid, lunID, vendor, product string) {
	attrOut := runCmd("lsattr", "-El", pv)
	for _, line := range strings.Split(attrOut, "\n") {
		fields := strings.Fields(line)
		if len(fields) >= 2 {
			if fields[0] == "pvid" { pvid = fields[1] }
			if fields[0] == "unique_id" { lunID = fields[1] }
		}
	}
	mpioOut := runCmd("lsmpio", "-ql", pv)
	for _, line := range strings.Split(mpioOut, "\n") {
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

// ============ DASHBOARD ============
func getDashboard() string {
	text := `[yellow::b]
  ███████╗████████╗ ██████╗ ████████╗██╗   ██╗██╗
  ██╔════╝╚══██╔══╝██╔════╝ ╚══██╔══╝██║   ██║██║
  ███████╗   ██║   ██║  ███╗   ██║   ██║   ██║██║
  ╚════██║   ██║   ██║   ██║   ██║   ██║   ██║██║
  ███████║   ██║   ╚██████╔╝   ██║   ╚██████╔╝██║
  ╚══════╝   ╚═╝    ╚═════╝    ╚═╝    ╚═════╝ ╚═╝
[white]        [gray]AIX Storage Explorer v1.4 - LibrePower[white]

`
	// Quick Health Check
	alerts := 0
	for _, vg := range strings.Split(strings.TrimSpace(runCmd("lsvg")), "\n") {
		if vg == "" { continue }
		vgInfo := runCmd("lsvg", vg)
		var totalPPs, usedPPs int
		for _, line := range strings.Split(vgInfo, "\n") {
			if strings.Contains(line, "TOTAL PPs:") { fmt.Sscanf(strings.Fields(strings.Split(line, "TOTAL PPs:")[1])[0], "%d", &totalPPs) }
			if strings.Contains(line, "USED PPs:") { fmt.Sscanf(strings.Fields(strings.Split(line, "USED PPs:")[1])[0], "%d", &usedPPs) }
		}
		if totalPPs > 0 && (usedPPs*100)/totalPPs >= 90 { alerts++ }
	}
	dfOut := runCmd("df", "-m")
	for _, line := range strings.Split(dfOut, "\n") {
		fields := strings.Fields(line)
		if len(fields) >= 7 && strings.HasPrefix(fields[0], "/dev/") {
			if parsePercent(fields[3]) >= 90 { alerts++ }
		}
	}
	
	healthIcon := "[green]● HEALTHY[white]"
	if alerts > 0 { healthIcon = fmt.Sprintf("[red]● %d ALERTS[white]", alerts) }
	text += fmt.Sprintf("  System Status: %s    [gray]Press 3 for details[white]\n\n", healthIcon)

	// VGs
	text += "[yellow::b]═══ VOLUME GROUPS ═══[white]\n"
	text += fmt.Sprintf("  %-12s %-7s %7s %7s %s\n", "VG", "STATE", "SIZE", "FREE", "USAGE")
	text += "  " + strings.Repeat("─", 58) + "\n"
	
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
		text += fmt.Sprintf("  %s %-10s %-7s %7s %7s %s\n", stateIcon, vg, state, 
			humanSize(float64(totalPPs*ppSize)), humanSize(float64(freePPs*ppSize)), progressBar(pct, 18))
	}

	// Paging Space
	text += "\n[yellow::b]═══ PAGING SPACE ═══[white]\n"
	psOut := runCmd("lsps", "-a")
	for _, line := range strings.Split(psOut, "\n") {
		fields := strings.Fields(line)
		if len(fields) >= 5 && strings.HasPrefix(fields[0], "hd") {
			pct := parsePercent(fields[4])
			text += fmt.Sprintf("  %-12s %-10s %7s %s\n", fields[0], fields[2], fields[3], progressBar(pct, 18))
		}
	}

	// Filesystems
	text += "\n[yellow::b]═══ FILESYSTEMS ═══[white]\n"
	text += fmt.Sprintf("  %-18s %7s %7s %s\n", "MOUNT", "SIZE", "FREE", "USAGE")
	text += "  " + strings.Repeat("─", 58) + "\n"
	
	for _, line := range strings.Split(dfOut, "\n") {
		fields := strings.Fields(line)
		if len(fields) >= 7 && strings.HasPrefix(fields[0], "/dev/") {
			mount := fields[len(fields)-1]
			sizeMB, _ := strconv.ParseFloat(fields[1], 64)
			freeMB, _ := strconv.ParseFloat(fields[2], 64)
			pct := parsePercent(fields[3])
			displayMount := mount
			if len(displayMount) > 18 { displayMount = "..." + displayMount[len(displayMount)-15:] }
			text += fmt.Sprintf("  %-18s %7s %7s %s\n", displayMount, humanSize(sizeMB), humanSize(freeMB), progressBar(pct, 18))
		}
	}
	
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
		text += fmt.Sprintf("  %s\n\n", progressBar(pct, 40))
		
		text += "  [gray]Physical Volumes:[white]\n"
		pvOut := runCmd("lsvg", "-p", vg)
		for i, line := range strings.Split(pvOut, "\n") {
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
				text += fmt.Sprintf("    %s %-10s %4d/%4d PPs %s\n", pvStateIcon, fields[0], pvUsed, pvTotal, progressBar(pvPct, 15))
			}
		}
		text += "\n"
	}
	return text
}

// ============ HEALTH / ALERTS ============
func getHealthCheck() string {
	text := "[yellow::b]═══ STORAGE HEALTH CHECK ═══[white]\n\n"
	issues := 0
	
	// Check Stale PPs
	text += "[cyan]● Stale Physical Partitions[white]\n"
	hasStale := false
	for _, vg := range strings.Split(strings.TrimSpace(runCmd("lsvg")), "\n") {
		if vg == "" { continue }
		lvOut := runCmd("lsvg", "-l", vg)
		for _, line := range strings.Split(lvOut, "\n") {
			if strings.Contains(strings.ToLower(line), "stale") {
				hasStale = true
				issues++
				text += fmt.Sprintf("  [red]✖ STALE: %s[white]\n", strings.TrimSpace(line))
			}
		}
	}
	if !hasStale { text += "  [green]✓ No stale partitions[white]\n" }
	
	// Check Quorum
	text += "\n[cyan]● Volume Group Quorum[white]\n"
	for _, vg := range strings.Split(strings.TrimSpace(runCmd("lsvg")), "\n") {
		if vg == "" { continue }
		vgInfo := runCmd("lsvg", vg)
		for _, line := range strings.Split(vgInfo, "\n") {
			if strings.Contains(line, "QUORUM:") {
				if strings.Contains(line, "Enabled") {
					text += fmt.Sprintf("  [green]✓ %s: Quorum Enabled[white]\n", vg)
				} else {
					text += fmt.Sprintf("  [yellow]⚠ %s: Quorum Disabled[white]\n", vg)
				}
			}
		}
	}
	
	// Check Multipath
	text += "\n[cyan]● Multipath Status[white]\n"
	pathOut := runCmd("lspath")
	pathCount := 0
	failedPaths := 0
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
		text += "  [gray]No multipath configured[white]\n"
	}
	
	// Check Paging
	text += "\n[cyan]● Paging Space[white]\n"
	psOut := runCmd("lsps", "-a")
	for _, line := range strings.Split(psOut, "\n") {
		fields := strings.Fields(line)
		if len(fields) >= 5 && strings.HasPrefix(fields[0], "hd") {
			pct := parsePercent(fields[4])
			if pct >= 80 {
				issues++
				icon := "[yellow]⚠[white]"
				if pct >= 90 { icon = "[red]✖[white]" }
				text += fmt.Sprintf("  %s %s: %d%% used[white]\n", icon, fields[0], pct)
			} else {
				text += fmt.Sprintf("  [green]✓ %s: %d%% used[white]\n", fields[0], pct)
			}
		}
	}
	
	// Check VG Usage
	text += "\n[cyan]● Volume Group Capacity[white]\n"
	for _, vg := range strings.Split(strings.TrimSpace(runCmd("lsvg")), "\n") {
		if vg == "" { continue }
		vgInfo := runCmd("lsvg", vg)
		var totalPPs, usedPPs int
		for _, line := range strings.Split(vgInfo, "\n") {
			if strings.Contains(line, "TOTAL PPs:") { fmt.Sscanf(strings.Fields(strings.Split(line, "TOTAL PPs:")[1])[0], "%d", &totalPPs) }
			if strings.Contains(line, "USED PPs:") { fmt.Sscanf(strings.Fields(strings.Split(line, "USED PPs:")[1])[0], "%d", &usedPPs) }
		}
		pct := 0
		if totalPPs > 0 { pct = (usedPPs * 100) / totalPPs }
		if pct >= 80 {
			issues++
			icon := "[yellow]⚠[white]"
			if pct >= 90 { icon = "[red]✖[white]" }
			text += fmt.Sprintf("  %s %s: %d%% full[white]\n", icon, vg, pct)
		} else {
			text += fmt.Sprintf("  [green]✓ %s: %d%% full[white]\n", vg, pct)
		}
	}
	
	// Check FS Usage
	text += "\n[cyan]● Filesystem Capacity[white]\n"
	dfOut := runCmd("df", "-m")
	fsIssues := 0
	for _, line := range strings.Split(dfOut, "\n") {
		fields := strings.Fields(line)
		if len(fields) >= 7 && strings.HasPrefix(fields[0], "/dev/") {
			mount := fields[len(fields)-1]
			pct := parsePercent(fields[3])
			if pct >= 80 {
				fsIssues++
				issues++
				icon := "[yellow]⚠[white]"
				if pct >= 90 { icon = "[red]✖[white]" }
				text += fmt.Sprintf("  %s %s: %d%% full[white]\n", icon, mount, pct)
			}
		}
	}
	if fsIssues == 0 { text += "  [green]✓ All filesystems below 80%[white]\n" }
	
	// Summary
	text += "\n" + strings.Repeat("─", 50) + "\n"
	if issues == 0 {
		text += "[green::b]✓ SYSTEM HEALTHY - No issues detected[white]\n"
	} else {
		text += fmt.Sprintf("[red::b]✖ %d ISSUE(S) REQUIRE ATTENTION[white]\n", issues)
	}
	text += "\n[gray]Legend: [green]✓[gray]=OK  [yellow]⚠[gray]=Warning(80-89%%)  [red]✖[gray]=Critical(90%%+)[white]\n"
	
	return text
}

// ============ LV DETAILS ============
func getLVDetails() string {
	text := "[yellow::b]═══ LOGICAL VOLUME STATUS ═══[white]\n\n"
	text += fmt.Sprintf("  %-15s %-10s %-8s %5s  %-12s %s\n", "LV", "VG", "TYPE", "LPs", "STATE", "MOUNT")
	text += "  " + strings.Repeat("─", 70) + "\n"
	
	for _, vg := range strings.Split(strings.TrimSpace(runCmd("lsvg")), "\n") {
		if vg == "" { continue }
		lvOut := runCmd("lsvg", "-l", vg)
		for i, line := range strings.Split(lvOut, "\n") {
			if i < 2 { continue }
			fields := strings.Fields(line)
			if len(fields) >= 6 {
				lvName := fields[0]
				lvType := fields[1]
				lps := fields[2]
				lvState := fields[4]
				mount := fields[5]
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

// ============ FS → LUN Mapping ============
func mapFStoLUN(mount string) string {
	dfOut := runCmd("df", "-m", mount)
	var device, sizeMB, freeMB string
	var pct int
	for _, line := range strings.Split(dfOut, "\n") {
		fields := strings.Fields(line)
		if len(fields) >= 7 && strings.HasPrefix(fields[0], "/dev/") {
			device = fields[0]
			sizeMB = fields[1]
			freeMB = fields[2]
			pct = parsePercent(fields[3])
			break
		}
	}
	if device == "" { return "[red]Cannot find device for " + mount + "[white]" }
	
	lv := strings.TrimPrefix(device, "/dev/")
	lvInfo := runCmd("lslv", lv)
	var vg, lvType, lvState, lps string
	for _, line := range strings.Split(lvInfo, "\n") {
		if strings.Contains(line, "VOLUME GROUP:") { vg = strings.TrimSpace(strings.Fields(strings.Split(line, "VOLUME GROUP:")[1])[0]) }
		if strings.Contains(line, "TYPE:") && !strings.Contains(line, "BB POLICY") { lvType = strings.TrimSpace(strings.Fields(strings.Split(line, "TYPE:")[1])[0]) }
		if strings.Contains(line, "LV STATE:") { lvState = strings.TrimSpace(strings.Split(line, "LV STATE:")[1]) }
		if strings.Contains(line, "LPs:") { lps = strings.TrimSpace(strings.Fields(strings.Split(line, "LPs:")[1])[0]) }
	}
	
	vgInfo := runCmd("lsvg", vg)
	var vgState, ppSize string
	var vgTotalPPs, vgUsedPPs int
	for _, line := range strings.Split(vgInfo, "\n") {
		if strings.Contains(line, "VG STATE:") { vgState = strings.TrimSpace(strings.Fields(strings.Split(line, "VG STATE:")[1])[0]) }
		if strings.Contains(line, "PP SIZE:") { ppSize = strings.TrimSpace(strings.Split(line, "PP SIZE:")[1]) }
		if strings.Contains(line, "TOTAL PPs:") { fmt.Sscanf(strings.Fields(strings.Split(line, "TOTAL PPs:")[1])[0], "%d", &vgTotalPPs) }
		if strings.Contains(line, "USED PPs:") { fmt.Sscanf(strings.Fields(strings.Split(line, "USED PPs:")[1])[0], "%d", &vgUsedPPs) }
	}
	vgPct := 0
	if vgTotalPPs > 0 { vgPct = (vgUsedPPs * 100) / vgTotalPPs }
	
	lslvmOut := runCmd("lslv", "-m", lv)
	pvSet := make(map[string]bool)
	var pvList []string
	for _, line := range strings.Split(lslvmOut, "\n") {
		fields := strings.Fields(line)
		if len(fields) >= 3 && strings.HasPrefix(fields[2], "hdisk") {
			pv := fields[2]
			if !pvSet[pv] { pvSet[pv] = true; pvList = append(pvList, pv) }
		}
	}
	
	sizef, _ := strconv.ParseFloat(sizeMB, 64)
	freef, _ := strconv.ParseFloat(freeMB, 64)
	
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
  │  VG Usage: %s  │
  └─────────────────────────────────────────────────┘
           │
           ▼
[green]▼ PHYSICAL VOLUMES (%d disk(s))[white]
`, mount, device, humanSize(sizef), humanSize(freef), progressBar(pct, 30),
		lv, lvType, lvState, lps, vg, vgState, ppSize, progressBar(vgPct, 30), len(pvList))

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

// ============ LUN → FS Mapping ============
func mapLUNtoFS(pv string) string {
	pvid, lunID, vendor, product := getPVLunInfo(pv)
	lunInfo := lunID
	if lunInfo == "" { lunInfo = vendor + " " + product }
	if lunInfo == "" || lunInfo == " " { lunInfo = "(virtual disk)" }
	
	pvOut := runCmd("lspv", pv)
	var totalPPs, freePPs, usedPPs int
	var vgName string
	for _, line := range strings.Split(pvOut, "\n") {
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
	
	// Path info
	pathInfo := ""
	pathOut := runCmd("lspath", "-l", pv)
	for _, line := range strings.Split(pathOut, "\n") {
		fields := strings.Fields(line)
		if len(fields) >= 3 {
			pathColor := "[green]"
			if fields[0] != "Enabled" { pathColor = "[red]" }
			pathInfo += fmt.Sprintf("%s%s[white] ", pathColor, fields[0])
		}
	}
	if pathInfo == "" { pathInfo = "[gray]single path[white]" }
	
	text := fmt.Sprintf(`[yellow::b]═══ STORAGE → FILESYSTEM MAPPING ═══[white]

[green]▼ PHYSICAL VOLUME (LUN)[white]
  ┌─────────────────────────────────────────────────┐
  │  Disk:     [cyan]%-35s[white]  │
  │  PVID:     %-35s  │
  │  LUN ID:   %-35s  │
  │  PPs:      %-4d total, %-4d used, %-4d free      │
  │  Paths:    %-35s  │
  │  Usage:    %s  │
  └─────────────────────────────────────────────────┘
           │
           ▼
`, pv, pvid, lunInfo, totalPPs, usedPPs, freePPs, pathInfo, progressBar(pct, 30))

	if vgName == "" || vgName == "None" {
		text += "  [yellow]Disk not assigned to any Volume Group[white]\n"
		return text
	}
	
	vgInfo := runCmd("lsvg", vgName)
	var vgState, ppSize string
	var vgTotalPPs, vgUsedPPs int
	for _, line := range strings.Split(vgInfo, "\n") {
		if strings.Contains(line, "VG STATE:") { vgState = strings.TrimSpace(strings.Fields(strings.Split(line, "VG STATE:")[1])[0]) }
		if strings.Contains(line, "PP SIZE:") { ppSize = strings.TrimSpace(strings.Split(line, "PP SIZE:")[1]) }
		if strings.Contains(line, "TOTAL PPs:") { fmt.Sscanf(strings.Fields(strings.Split(line, "TOTAL PPs:")[1])[0], "%d", &vgTotalPPs) }
		if strings.Contains(line, "USED PPs:") { fmt.Sscanf(strings.Fields(strings.Split(line, "USED PPs:")[1])[0], "%d", &vgUsedPPs) }
	}
	vgPct := 0
	if vgTotalPPs > 0 { vgPct = (vgUsedPPs * 100) / vgTotalPPs }
	
	text += fmt.Sprintf(`[green]▼ VOLUME GROUP[white]
  ┌─────────────────────────────────────────────────┐
  │  VG Name:  [cyan]%-35s[white]  │
  │  State:    %-35s  │
  │  PP Size:  %-35s  │
  │  VG Usage: %s  │
  └─────────────────────────────────────────────────┘
           │
           ▼
[green]▼ LVs & FILESYSTEMS on this disk[white]
`, vgName, vgState, ppSize, progressBar(vgPct, 30))

	text += fmt.Sprintf("  %-15s %-8s %6s  %-12s %s\n", "LV", "TYPE", "LPs", "STATE", "MOUNT")
	text += "  " + strings.Repeat("─", 60) + "\n"
	
	lspvlOut := runCmd("lspv", "-l", pv)
	for i, line := range strings.Split(lspvlOut, "\n") {
		if i < 2 || strings.TrimSpace(line) == "" { continue }
		fields := strings.Fields(line)
		if len(fields) >= 2 {
			lvName := fields[0]
			lps := fields[1]
			lvInfo := runCmd("lslv", lvName)
			var lvType, mount, lvState string
			for _, l := range strings.Split(lvInfo, "\n") {
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

func main() {
	app := tview.NewApplication()
	pages := tview.NewPages()
	
	dashView := tview.NewTextView().SetDynamicColors(true).SetScrollable(true)
	dashView.SetBorder(true).SetTitle(" Dashboard [1] ")
	
	vgView := tview.NewTextView().SetDynamicColors(true).SetScrollable(true)
	vgView.SetBorder(true).SetTitle(" VG Details [2] ")
	
	healthView := tview.NewTextView().SetDynamicColors(true).SetScrollable(true)
	healthView.SetBorder(true).SetTitle(" Health Check [3] ")
	
	lvView := tview.NewTextView().SetDynamicColors(true).SetScrollable(true)
	lvView.SetBorder(true).SetTitle(" LV Status [4] ")
	
	detailView := tview.NewTextView().SetDynamicColors(true).SetScrollable(true)
	detailView.SetBorder(true)
	
	pvList := tview.NewList().ShowSecondaryText(false).SetHighlightFullLine(true)
	pvList.SetBorder(true).SetTitle(" Disk → FS [5] ")
	
	fsList := tview.NewList().ShowSecondaryText(false).SetHighlightFullLine(true)
	fsList.SetBorder(true).SetTitle(" FS → Disk [6] ")
	
	help := tview.NewTextView().
		SetText(" [yellow]1[white]Dash [yellow]2[white]VGs [yellow]3[white]Health [yellow]4[white]LVs [yellow]5[white]Disk→FS [yellow]6[white]FS→Disk [yellow]r[white]Refresh [yellow]q[white]Quit").
		SetDynamicColors(true)
	
	refresh := func() {
		dashView.SetText(getDashboard())
		vgView.SetText(getVGDetails())
		healthView.SetText(getHealthCheck())
		lvView.SetText(getLVDetails())
		
		pvList.Clear()
		for _, line := range strings.Split(runCmd("lspv"), "\n") {
			fields := strings.Fields(line)
			if len(fields) >= 3 && strings.HasPrefix(fields[0], "hdisk") {
				_, _, vendor, product := getPVLunInfo(fields[0])
				lunInfo := vendor + " " + product
				if lunInfo == " " { lunInfo = "(virtual)" }
				if len(lunInfo) > 16 { lunInfo = lunInfo[:16] }
				pvList.AddItem(fmt.Sprintf("%-10s %-12s %s", fields[0], fields[2], lunInfo), "", 0, nil)
			}
		}
		
		fsList.Clear()
		for _, line := range strings.Split(runCmd("df", "-m"), "\n") {
			fields := strings.Fields(line)
			if len(fields) >= 7 && strings.HasPrefix(fields[0], "/dev/") {
				mount := fields[len(fields)-1]
				pct := parsePercent(fields[3])
				bar := "[green]"
				if pct >= 90 { bar = "[red]" } else if pct >= 80 { bar = "[yellow]" }
				bar += fmt.Sprintf("%3d%%", pct) + "[white]"
				fsList.AddItem(fmt.Sprintf("%-26s %s", mount, bar), "", 0, nil)
			}
		}
	}
	
	pvList.SetSelectedFunc(func(i int, main string, sec string, sh rune) {
		pv := strings.Fields(main)[0]
		detailView.SetText(mapLUNtoFS(pv))
		detailView.SetTitle(fmt.Sprintf(" %s → Filesystems ", pv))
		pages.SwitchToPage("detail")
	})
	
	fsList.SetSelectedFunc(func(i int, main string, sec string, sh rune) {
		mount := strings.Fields(main)[0]
		detailView.SetText(mapFStoLUN(mount))
		detailView.SetTitle(fmt.Sprintf(" %s → LUNs ", mount))
		pages.SwitchToPage("detail")
	})
	
	for name, view := range map[string]tview.Primitive{
		"dash": dashView, "vg": vgView, "health": healthView, "lv": lvView, "pv": pvList, "fs": fsList, "detail": detailView,
	} {
		page := tview.NewFlex().SetDirection(tview.FlexRow).AddItem(view, 0, 1, true).AddItem(help, 1, 0, false)
		pages.AddPage(name, page, true, name == "dash")
	}
	
	refresh()
	
	var lastPage string = "dash"
	app.SetInputCapture(func(event *tcell.EventKey) *tcell.EventKey {
		cp, _ := pages.GetFrontPage()
		if cp != "detail" { lastPage = cp }
		switch event.Rune() {
		case 'q': app.Stop(); return nil
		case '1': pages.SwitchToPage("dash"); return nil
		case '2': pages.SwitchToPage("vg"); return nil
		case '3': pages.SwitchToPage("health"); return nil
		case '4': pages.SwitchToPage("lv"); return nil
		case '5': pages.SwitchToPage("pv"); return nil
		case '6': pages.SwitchToPage("fs"); return nil
		case 'r': refresh(); return nil
		}
		if event.Key() == tcell.KeyEsc { pages.SwitchToPage(lastPage); return nil }
		return event
	})
	
	if err := app.SetRoot(pages, true).EnableMouse(false).Run(); err != nil { panic(err) }
}
