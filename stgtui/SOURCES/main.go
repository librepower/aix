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

// ASCII progress bar with color
func progressBar(percent int, width int) string {
	if percent < 0 { percent = 0 }
	if percent > 100 { percent = 100 }
	
	filled := (percent * width) / 100
	empty := width - filled
	
	color := "[green]"
	if percent >= 90 {
		color = "[red]"
	} else if percent >= 80 {
		color = "[yellow]"
	}
	
	bar := color + strings.Repeat("█", filled) + "[gray]" + strings.Repeat("░", empty) + "[white]"
	return fmt.Sprintf("%s %3d%%", bar, percent)
}

// Get LUN info for a PV
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
		if strings.HasPrefix(line, "Vendor Id:") {
			vendor = strings.TrimSpace(strings.TrimPrefix(line, "Vendor Id:"))
		}
		if strings.HasPrefix(line, "Product Id:") {
			product = strings.TrimSpace(strings.TrimPrefix(line, "Product Id:"))
		}
	}
	return
}

// Parse percentage from string like "67%" or "67"
func parsePercent(s string) int {
	s = strings.TrimSuffix(s, "%")
	pct, _ := strconv.Atoi(s)
	return pct
}

// ============ DASHBOARD with visual bars ============
func getDashboard() string {
	text := `[yellow::b]
  ███████╗████████╗ ██████╗ ████████╗██╗   ██╗██╗
  ██╔════╝╚══██╔══╝██╔════╝ ╚══██╔══╝██║   ██║██║
  ███████╗   ██║   ██║  ███╗   ██║   ██║   ██║██║
  ╚════██║   ██║   ██║   ██║   ██║   ██║   ██║██║
  ███████║   ██║   ╚██████╔╝   ██║   ╚██████╔╝██║
  ╚══════╝   ╚═╝    ╚═════╝    ╚═╝    ╚═════╝ ╚═╝
[white]        [gray]AIX Storage Explorer - LibrePower[white]

`
	// VG Summary with bars
	text += "[yellow::b]═══ VOLUME GROUPS ═══[white]\n\n"
	text += fmt.Sprintf("  %-12s %-8s %8s %8s %s\n", "VG", "STATE", "TOTAL", "FREE", "USAGE")
	text += "  " + strings.Repeat("─", 65) + "\n"
	
	for _, vg := range strings.Split(strings.TrimSpace(runCmd("lsvg")), "\n") {
		if vg == "" { continue }
		
		vgInfo := runCmd("lsvg", vg)
		var totalPPs, usedPPs, freePPs int
		var state, ppSize string
		
		for _, line := range strings.Split(vgInfo, "\n") {
			if strings.Contains(line, "VG STATE:") {
				parts := strings.Split(line, "VG STATE:")
				if len(parts) > 1 { state = strings.TrimSpace(strings.Fields(parts[1])[0]) }
			}
			if strings.Contains(line, "PP SIZE:") {
				parts := strings.Split(line, "PP SIZE:")
				if len(parts) > 1 { 
					f := strings.Fields(parts[1])
					if len(f) >= 1 { ppSize = f[0] }
				}
			}
			if strings.Contains(line, "TOTAL PPs:") {
				parts := strings.Split(line, "TOTAL PPs:")
				if len(parts) > 1 { fmt.Sscanf(strings.Fields(parts[1])[0], "%d", &totalPPs) }
			}
			if strings.Contains(line, "USED PPs:") {
				parts := strings.Split(line, "USED PPs:")
				if len(parts) > 1 { fmt.Sscanf(strings.Fields(parts[1])[0], "%d", &usedPPs) }
			}
			if strings.Contains(line, "FREE PPs:") {
				idx := strings.LastIndex(line, "FREE PPs:")
				if idx >= 0 {
					rest := line[idx+9:]
					fmt.Sscanf(strings.TrimSpace(rest), "%d", &freePPs)
				}
			}
		}
		
		pct := 0
		if totalPPs > 0 { pct = (usedPPs * 100) / totalPPs }
		
		stateIcon := "[green]●[white]"
		if state != "active" { stateIcon = "[red]●[white]" }
		
		totalMB := totalPPs * parsePercent(ppSize)
		freeMB := freePPs * parsePercent(ppSize)
		
		totalStr := fmt.Sprintf("%dG", totalMB/1024)
		freeStr := fmt.Sprintf("%dG", freeMB/1024)
		if freeMB < 1024 { freeStr = fmt.Sprintf("%dM", freeMB) }
		
		text += fmt.Sprintf("  %s %-10s %-8s %8s %8s %s\n", stateIcon, vg, state, totalStr, freeStr, progressBar(pct, 20))
	}
	
	// Filesystem Summary with bars
	text += "\n[yellow::b]═══ FILESYSTEMS ═══[white]\n\n"
	text += fmt.Sprintf("  %-20s %8s %8s %s\n", "MOUNT", "SIZE", "FREE", "USAGE")
	text += "  " + strings.Repeat("─", 65) + "\n"
	
	dfOut := runCmd("df", "-m")
	for _, line := range strings.Split(dfOut, "\n") {
		fields := strings.Fields(line)
		// AIX df -m format: Filesystem  MB_blocks  Free  %Used  Iused  %Iused  Mounted
		if len(fields) >= 7 && strings.HasPrefix(fields[0], "/dev/") {
			mount := fields[len(fields)-1]
			
			sizeMB, _ := strconv.ParseFloat(fields[1], 64)
			freeMB, _ := strconv.ParseFloat(fields[2], 64)
			pct := parsePercent(fields[3])
			
			// Format size
			sizeStr := fmt.Sprintf("%.0fM", sizeMB)
			if sizeMB >= 1024 { sizeStr = fmt.Sprintf("%.1fG", sizeMB/1024) }
			
			freeStr := fmt.Sprintf("%.0fM", freeMB)
			if freeMB >= 1024 { freeStr = fmt.Sprintf("%.1fG", freeMB/1024) }
			
			// Truncate long mount points
			displayMount := mount
			if len(displayMount) > 20 { displayMount = "..." + displayMount[len(displayMount)-17:] }
			
			text += fmt.Sprintf("  %-20s %8s %8s %s\n", displayMount, sizeStr, freeStr, progressBar(pct, 20))
		}
	}
	
	// Disk count and warnings
	pvCount := 0
	for _, line := range strings.Split(runCmd("lspv"), "\n") {
		if strings.HasPrefix(strings.TrimSpace(line), "hdisk") { pvCount++ }
	}
	
	text += fmt.Sprintf("\n[gray]  Total disks: %d[white]\n", pvCount)
	
	return text
}

// ============ VG Details View ============
func getVGDetails() string {
	text := "[yellow::b]═══ VOLUME GROUP DETAILS ═══[white]\n\n"
	
	for _, vg := range strings.Split(strings.TrimSpace(runCmd("lsvg")), "\n") {
		if vg == "" { continue }
		
		vgInfo := runCmd("lsvg", vg)
		var totalPPs, usedPPs, freePPs int
		var state, ppSize string
		
		for _, line := range strings.Split(vgInfo, "\n") {
			if strings.Contains(line, "VG STATE:") {
				parts := strings.Split(line, "VG STATE:")
				if len(parts) > 1 { state = strings.TrimSpace(strings.Fields(parts[1])[0]) }
			}
			if strings.Contains(line, "PP SIZE:") {
				parts := strings.Split(line, "PP SIZE:")
				if len(parts) > 1 { 
					f := strings.Fields(parts[1])
					if len(f) >= 1 { ppSize = f[0] + "MB" }
				}
			}
			if strings.Contains(line, "TOTAL PPs:") {
				parts := strings.Split(line, "TOTAL PPs:")
				if len(parts) > 1 { fmt.Sscanf(strings.Fields(parts[1])[0], "%d", &totalPPs) }
			}
			if strings.Contains(line, "USED PPs:") {
				parts := strings.Split(line, "USED PPs:")
				if len(parts) > 1 { fmt.Sscanf(strings.Fields(parts[1])[0], "%d", &usedPPs) }
			}
			if strings.Contains(line, "FREE PPs:") {
				idx := strings.LastIndex(line, "FREE PPs:")
				if idx >= 0 {
					rest := line[idx+9:]
					fmt.Sscanf(strings.TrimSpace(rest), "%d", &freePPs)
				}
			}
		}
		
		pct := 0
		if totalPPs > 0 { pct = (usedPPs * 100) / totalPPs }
		
		stateColor := "[green]"
		if state != "active" { stateColor = "[red]" }
		
		text += fmt.Sprintf("[cyan::b]%s[white]  %s%s[white]  PP: %s\n", vg, stateColor, state, ppSize)
		text += fmt.Sprintf("  PPs: %d total, %d used, %d free\n", totalPPs, usedPPs, freePPs)
		text += fmt.Sprintf("  %s\n\n", progressBar(pct, 40))
		
		// Show PVs in this VG
		text += "  [gray]Physical Volumes:[white]\n"
		pvOut := runCmd("lsvg", "-p", vg)
		for i, line := range strings.Split(pvOut, "\n") {
			if i < 2 { continue }
			fields := strings.Fields(line)
			if len(fields) >= 5 && strings.HasPrefix(fields[0], "hdisk") {
				pv := fields[0]
				pvState := fields[1]
				pvTotal, _ := strconv.Atoi(fields[2])
				pvFree, _ := strconv.Atoi(fields[3])
				pvUsed := pvTotal - pvFree
				pvPct := 0
				if pvTotal > 0 { pvPct = (pvUsed * 100) / pvTotal }
				
				pvStateIcon := "[green]●[white]"
				if pvState != "active" { pvStateIcon = "[red]●[white]" }
				
				text += fmt.Sprintf("    %s %-10s %4d/%4d PPs %s\n", pvStateIcon, pv, pvUsed, pvTotal, progressBar(pvPct, 15))
			}
		}
		
		text += "\n"
	}
	
	return text
}

// ============ ALERTS View ============
func getAlerts() string {
	text := "[yellow::b]═══ STORAGE ALERTS ═══[white]\n\n"
	
	hasAlerts := false
	
	// Check VGs
	for _, vg := range strings.Split(strings.TrimSpace(runCmd("lsvg")), "\n") {
		if vg == "" { continue }
		
		vgInfo := runCmd("lsvg", vg)
		var totalPPs, usedPPs int
		
		for _, line := range strings.Split(vgInfo, "\n") {
			if strings.Contains(line, "TOTAL PPs:") {
				parts := strings.Split(line, "TOTAL PPs:")
				if len(parts) > 1 { fmt.Sscanf(strings.Fields(parts[1])[0], "%d", &totalPPs) }
			}
			if strings.Contains(line, "USED PPs:") {
				parts := strings.Split(line, "USED PPs:")
				if len(parts) > 1 { fmt.Sscanf(strings.Fields(parts[1])[0], "%d", &usedPPs) }
			}
		}
		
		pct := 0
		if totalPPs > 0 { pct = (usedPPs * 100) / totalPPs }
		
		if pct >= 80 {
			hasAlerts = true
			icon := "[yellow]⚠[white]"
			if pct >= 90 { icon = "[red]✖[white]" }
			text += fmt.Sprintf("%s [cyan]VG %s[white] is %d%% full (%d/%d PPs)\n", icon, vg, pct, usedPPs, totalPPs)
		}
	}
	
	// Check Filesystems
	dfOut := runCmd("df", "-m")
	for _, line := range strings.Split(dfOut, "\n") {
		fields := strings.Fields(line)
		if len(fields) >= 7 && strings.HasPrefix(fields[0], "/dev/") {
			mount := fields[len(fields)-1]
			pct := parsePercent(fields[3])
			
			if pct >= 80 {
				hasAlerts = true
				icon := "[yellow]⚠[white]"
				if pct >= 90 { icon = "[red]✖[white]" }
				text += fmt.Sprintf("%s [cyan]FS %s[white] is %d%% full\n", icon, mount, pct)
			}
		}
	}
	
	if !hasAlerts {
		text += "[green]✓ No storage alerts - all systems healthy[white]\n"
	}
	
	text += "\n[gray]Legend: [yellow]⚠[gray] = 80-89%  [red]✖[gray] = 90%+[white]\n"
	
	return text
}

// ============ MAPPING: FS → LV → VG → PV → LUN ============
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
	
	// Get LV info
	lvInfo := runCmd("lslv", lv)
	var vg, lvType, lvState, lps string
	for _, line := range strings.Split(lvInfo, "\n") {
		if strings.Contains(line, "VOLUME GROUP:") {
			parts := strings.Split(line, "VOLUME GROUP:")
			if len(parts) > 1 { vg = strings.TrimSpace(strings.Fields(parts[1])[0]) }
		}
		if strings.Contains(line, "TYPE:") && !strings.Contains(line, "BB POLICY") {
			parts := strings.Split(line, "TYPE:")
			if len(parts) > 1 { lvType = strings.TrimSpace(strings.Fields(parts[1])[0]) }
		}
		if strings.Contains(line, "LV STATE:") {
			parts := strings.Split(line, "LV STATE:")
			if len(parts) > 1 { lvState = strings.TrimSpace(parts[1]) }
		}
		if strings.Contains(line, "LPs:") {
			parts := strings.Split(line, "LPs:")
			if len(parts) > 1 { lps = strings.TrimSpace(strings.Fields(parts[1])[0]) }
		}
	}
	
	// Get VG info
	vgInfo := runCmd("lsvg", vg)
	var vgState, ppSize string
	var vgTotalPPs, vgUsedPPs int
	for _, line := range strings.Split(vgInfo, "\n") {
		if strings.Contains(line, "VG STATE:") {
			parts := strings.Split(line, "VG STATE:")
			if len(parts) > 1 { vgState = strings.TrimSpace(strings.Fields(parts[1])[0]) }
		}
		if strings.Contains(line, "PP SIZE:") {
			parts := strings.Split(line, "PP SIZE:")
			if len(parts) > 1 { ppSize = strings.TrimSpace(parts[1]) }
		}
		if strings.Contains(line, "TOTAL PPs:") {
			parts := strings.Split(line, "TOTAL PPs:")
			if len(parts) > 1 { fmt.Sscanf(strings.Fields(parts[1])[0], "%d", &vgTotalPPs) }
		}
		if strings.Contains(line, "USED PPs:") {
			parts := strings.Split(line, "USED PPs:")
			if len(parts) > 1 { fmt.Sscanf(strings.Fields(parts[1])[0], "%d", &vgUsedPPs) }
		}
	}
	vgPct := 0
	if vgTotalPPs > 0 { vgPct = (vgUsedPPs * 100) / vgTotalPPs }
	
	// Get PVs
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
	
	text := fmt.Sprintf(`[yellow::b]═══ FILESYSTEM → STORAGE MAPPING ═══[white]

[green]▼ FILESYSTEM[white]
  ┌─────────────────────────────────────────────────┐
  │  Mount:    [cyan]%-35s[white]  │
  │  Device:   %-35s  │
  │  Size:     %-10s  Free: %-10s       │
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
[green]▼ PHYSICAL VOLUMES (LUNs)[white]
`, mount, device, sizeMB+"MB", freeMB+"MB", progressBar(pct, 30),
		lv, lvType, lvState, lps, vg, vgState, ppSize, progressBar(vgPct, 30))

	for _, pv := range pvList {
		pvid, lunID, vendor, product := getPVLunInfo(pv)
		lunInfo := lunID
		if lunInfo == "" { lunInfo = vendor + " " + product }
		if lunInfo == "" || lunInfo == " " { lunInfo = "(virtual)" }
		if len(lunInfo) > 35 { lunInfo = lunInfo[:35] }
		
		text += fmt.Sprintf(`  ┌─────────────────────────────────────────────────┐
  │  Disk:     [cyan]%-35s[white]  │
  │  PVID:     %-35s  │
  │  LUN:      %-35s  │
  └─────────────────────────────────────────────────┘
`, pv, pvid, lunInfo)
	}
	
	return text
}

// ============ MAPPING: LUN/PV → VG → LV → FS ============
func mapLUNtoFS(pv string) string {
	pvid, lunID, vendor, product := getPVLunInfo(pv)
	lunInfo := lunID
	if lunInfo == "" { lunInfo = vendor + " " + product }
	if lunInfo == "" || lunInfo == " " { lunInfo = "(virtual)" }
	
	pvOut := runCmd("lspv", pv)
	var totalPPs, freePPs, usedPPs int
	var vgName string
	for _, line := range strings.Split(pvOut, "\n") {
		if strings.Contains(line, "VOLUME GROUP:") {
			parts := strings.Split(line, "VOLUME GROUP:")
			if len(parts) > 1 { 
				f := strings.Fields(parts[1])
				if len(f) > 0 { vgName = f[0] }
			}
		}
		if strings.Contains(line, "TOTAL PPs:") {
			parts := strings.Split(line, "TOTAL PPs:")
			if len(parts) > 1 { fmt.Sscanf(strings.Fields(parts[1])[0], "%d", &totalPPs) }
		}
		if strings.Contains(line, "FREE PPs:") {
			parts := strings.Split(line, "FREE PPs:")
			if len(parts) > 1 { fmt.Sscanf(strings.Fields(parts[1])[0], "%d", &freePPs) }
		}
	}
	usedPPs = totalPPs - freePPs
	pct := 0
	if totalPPs > 0 { pct = (usedPPs * 100) / totalPPs }
	
	text := fmt.Sprintf(`[yellow::b]═══ STORAGE → FILESYSTEM MAPPING ═══[white]

[green]▼ PHYSICAL VOLUME (LUN)[white]
  ┌─────────────────────────────────────────────────┐
  │  Disk:     [cyan]%-35s[white]  │
  │  PVID:     %-35s  │
  │  LUN:      %-35s  │
  │  PPs:      %-4d total, %-4d used, %-4d free      │
  │  Usage:    %s  │
  └─────────────────────────────────────────────────┘
           │
           ▼
`, pv, pvid, lunInfo, totalPPs, usedPPs, freePPs, progressBar(pct, 30))

	if vgName == "" || vgName == "None" {
		text += "  [yellow]Disk not assigned to any Volume Group[white]\n"
		return text
	}
	
	// VG info
	vgInfo := runCmd("lsvg", vgName)
	var vgState, ppSize string
	var vgTotalPPs, vgUsedPPs int
	for _, line := range strings.Split(vgInfo, "\n") {
		if strings.Contains(line, "VG STATE:") {
			parts := strings.Split(line, "VG STATE:")
			if len(parts) > 1 { vgState = strings.TrimSpace(strings.Fields(parts[1])[0]) }
		}
		if strings.Contains(line, "PP SIZE:") {
			parts := strings.Split(line, "PP SIZE:")
			if len(parts) > 1 { ppSize = strings.TrimSpace(parts[1]) }
		}
		if strings.Contains(line, "TOTAL PPs:") {
			parts := strings.Split(line, "TOTAL PPs:")
			if len(parts) > 1 { fmt.Sscanf(strings.Fields(parts[1])[0], "%d", &vgTotalPPs) }
		}
		if strings.Contains(line, "USED PPs:") {
			parts := strings.Split(line, "USED PPs:")
			if len(parts) > 1 { fmt.Sscanf(strings.Fields(parts[1])[0], "%d", &vgUsedPPs) }
		}
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

	text += fmt.Sprintf("  %-15s %-8s %6s %-20s\n", "LV", "TYPE", "LPs", "MOUNT")
	text += "  " + strings.Repeat("─", 55) + "\n"
	
	lspvlOut := runCmd("lspv", "-l", pv)
	for i, line := range strings.Split(lspvlOut, "\n") {
		if i < 2 || strings.TrimSpace(line) == "" { continue }
		fields := strings.Fields(line)
		if len(fields) >= 2 {
			lvName := fields[0]
			lps := fields[1]
			
			lvInfo := runCmd("lslv", lvName)
			var lvType, mount string
			for _, l := range strings.Split(lvInfo, "\n") {
				if strings.Contains(l, "TYPE:") && !strings.Contains(l, "BB") {
					parts := strings.Split(l, "TYPE:")
					if len(parts) > 1 { lvType = strings.TrimSpace(strings.Fields(parts[1])[0]) }
				}
				if strings.Contains(l, "MOUNT POINT:") {
					parts := strings.Split(l, "MOUNT POINT:")
					if len(parts) > 1 { mount = strings.TrimSpace(parts[1]) }
				}
			}
			if mount == "" { mount = "[gray]N/A[white]" }
			
			text += fmt.Sprintf("  [cyan]%-15s[white] %-8s %6s %-20s\n", lvName, lvType, lps, mount)
		}
	}
	
	return text
}

func main() {
	app := tview.NewApplication()
	pages := tview.NewPages()
	
	// Views
	dashView := tview.NewTextView().SetDynamicColors(true).SetScrollable(true)
	dashView.SetBorder(true).SetTitle(" Dashboard [1] ")
	
	vgView := tview.NewTextView().SetDynamicColors(true).SetScrollable(true)
	vgView.SetBorder(true).SetTitle(" VG Details [2] ")
	
	alertView := tview.NewTextView().SetDynamicColors(true).SetScrollable(true)
	alertView.SetBorder(true).SetTitle(" Alerts [3] ")
	
	detailView := tview.NewTextView().SetDynamicColors(true).SetScrollable(true)
	detailView.SetBorder(true)
	
	pvList := tview.NewList().ShowSecondaryText(false).SetHighlightFullLine(true)
	pvList.SetBorder(true).SetTitle(" Disk → FS [4] ")
	
	fsList := tview.NewList().ShowSecondaryText(false).SetHighlightFullLine(true)
	fsList.SetBorder(true).SetTitle(" FS → Disk [5] ")
	
	help := tview.NewTextView().
		SetText(" [yellow]1[white] Dash [yellow]2[white] VGs [yellow]3[white] Alerts [yellow]4[white] Disk→FS [yellow]5[white] FS→Disk [yellow]r[white] Refresh [yellow]Esc[white] Back [yellow]q[white] Quit").
		SetDynamicColors(true)
	
	// Populate
	refresh := func() {
		dashView.SetText(getDashboard())
		vgView.SetText(getVGDetails())
		alertView.SetText(getAlerts())
		
		pvList.Clear()
		for _, line := range strings.Split(runCmd("lspv"), "\n") {
			fields := strings.Fields(line)
			if len(fields) >= 3 && strings.HasPrefix(fields[0], "hdisk") {
				_, _, vendor, product := getPVLunInfo(fields[0])
				lunInfo := vendor + " " + product
				if lunInfo == " " { lunInfo = "(virtual)" }
				if len(lunInfo) > 18 { lunInfo = lunInfo[:18] }
				pvList.AddItem(fmt.Sprintf("%-10s %-10s %s", fields[0], fields[2], lunInfo), "", 0, nil)
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
				
				fsList.AddItem(fmt.Sprintf("%-28s %s", mount, bar), "", 0, nil)
			}
		}
	}
	
	// Handlers
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
	
	// Pages
	dashPage := tview.NewFlex().SetDirection(tview.FlexRow).
		AddItem(dashView, 0, 1, true).AddItem(help, 1, 0, false)
	pages.AddPage("dash", dashPage, true, true)
	
	vgPage := tview.NewFlex().SetDirection(tview.FlexRow).
		AddItem(vgView, 0, 1, true).AddItem(help, 1, 0, false)
	pages.AddPage("vg", vgPage, true, false)
	
	alertPage := tview.NewFlex().SetDirection(tview.FlexRow).
		AddItem(alertView, 0, 1, true).AddItem(help, 1, 0, false)
	pages.AddPage("alert", alertPage, true, false)
	
	pvPage := tview.NewFlex().SetDirection(tview.FlexRow).
		AddItem(pvList, 0, 1, true).AddItem(help, 1, 0, false)
	pages.AddPage("pv", pvPage, true, false)
	
	fsPage := tview.NewFlex().SetDirection(tview.FlexRow).
		AddItem(fsList, 0, 1, true).AddItem(help, 1, 0, false)
	pages.AddPage("fs", fsPage, true, false)
	
	detailPage := tview.NewFlex().SetDirection(tview.FlexRow).
		AddItem(detailView, 0, 1, true).AddItem(help, 1, 0, false)
	pages.AddPage("detail", detailPage, true, false)
	
	refresh()
	
	var lastPage string = "dash"
	
	app.SetInputCapture(func(event *tcell.EventKey) *tcell.EventKey {
		cp, _ := pages.GetFrontPage()
		if cp != "detail" { lastPage = cp }
		
		switch event.Rune() {
		case 'q': app.Stop(); return nil
		case '1': pages.SwitchToPage("dash"); return nil
		case '2': pages.SwitchToPage("vg"); return nil
		case '3': pages.SwitchToPage("alert"); return nil
		case '4': pages.SwitchToPage("pv"); return nil
		case '5': pages.SwitchToPage("fs"); return nil
		case 'r': refresh(); return nil
		}
		if event.Key() == tcell.KeyEsc {
			pages.SwitchToPage(lastPage)
			return nil
		}
		return event
	})
	
	if err := app.SetRoot(pages, true).EnableMouse(false).Run(); err != nil {
		panic(err)
	}
}
