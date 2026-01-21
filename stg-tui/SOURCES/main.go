package main

import (
	"fmt"
	"os/exec"
	"strings"

	"github.com/gdamore/tcell/v2"
	"github.com/rivo/tview"
)

func runCmd(cmd string, args ...string) string {
	out, _ := exec.Command(cmd, args...).Output()
	return string(out)
}

func getVGTable() string {
	out := runCmd("lsvg")
	vgs := strings.Split(strings.TrimSpace(out), "\n")
	
	text := "[yellow]VOLUME GROUPS[white]\n"
	text += fmt.Sprintf("%-12s %-8s %10s %10s %10s %5s %5s\n", "VG", "STATE", "TOTAL", "USED", "FREE", "LVs", "PVs")
	text += strings.Repeat("─", 70) + "\n"
	
	for _, vg := range vgs {
		if vg == "" {
			continue
		}
		info := runCmd("lsvg", vg)
		
		state, ppSize, total, used, free, lvs, pvs := "", "", "", "", "", "", ""
		
		for _, line := range strings.Split(info, "\n") {
			if strings.Contains(line, "VG STATE:") {
				parts := strings.Split(line, "VG STATE:")
				if len(parts) > 1 {
					state = strings.TrimSpace(strings.Fields(parts[1])[0])
				}
			}
			if strings.Contains(line, "PP SIZE:") {
				parts := strings.Split(line, "PP SIZE:")
				if len(parts) > 1 {
					ppSize = strings.TrimSpace(strings.Fields(parts[1])[0])
				}
			}
			if strings.Contains(line, "TOTAL PPs:") && strings.Contains(line, "megabyte") {
				// Extract size in parentheses
				start := strings.Index(line, "(")
				end := strings.Index(line, "megabyte")
				if start > 0 && end > start {
					total = strings.TrimSpace(line[start+1:end]) + "MB"
				}
			}
			if strings.Contains(line, "USED PPs:") && strings.Contains(line, "megabyte") {
				start := strings.Index(line, "(")
				end := strings.Index(line, "megabyte")
				if start > 0 && end > start {
					used = strings.TrimSpace(line[start+1:end]) + "MB"
				}
			}
			if strings.Contains(line, "FREE PPs:") && strings.Contains(line, "megabyte") {
				start := strings.LastIndex(line, "(")
				end := strings.LastIndex(line, "megabyte")
				if start > 0 && end > start {
					free = strings.TrimSpace(line[start+1:end]) + "MB"
				}
			}
			if strings.Contains(line, "LVs:") && !strings.Contains(line, "OPEN") && !strings.Contains(line, "MAX") {
				parts := strings.Split(line, "LVs:")
				if len(parts) > 1 {
					lvs = strings.TrimSpace(strings.Fields(parts[1])[0])
				}
			}
			if strings.Contains(line, "TOTAL PVs:") {
				parts := strings.Split(line, "TOTAL PVs:")
				if len(parts) > 1 {
					pvs = strings.TrimSpace(strings.Fields(parts[1])[0])
				}
			}
		}
		
		stateColor := "[green]"
		if state != "active" {
			stateColor = "[red]"
		}
		
		_ = ppSize // Available if needed
		text += fmt.Sprintf("%-12s %s%-8s[white] %10s %10s %10s %5s %5s\n", 
			vg, stateColor, state, total, used, free, lvs, pvs)
	}
	return text
}

func getPVTable() string {
	out := runCmd("lspv")
	
	text := "[yellow]PHYSICAL VOLUMES (DISKS)[white]\n"
	text += fmt.Sprintf("%-10s %-36s %-12s %-10s %-20s\n", "DISK", "PVID", "VG", "STATE", "LUN/VENDOR")
	text += strings.Repeat("─", 95) + "\n"
	
	for _, line := range strings.Split(out, "\n") {
		fields := strings.Fields(line)
		if len(fields) < 3 || !strings.HasPrefix(fields[0], "hdisk") {
			continue
		}
		
		disk := fields[0]
		pvid := fields[1]
		vg := fields[2]
		state := "active"
		if len(fields) > 3 {
			state = fields[3]
		}
		
		// Get LUN info
		lunInfo := ""
		mpioOut := runCmd("lsmpio", "-ql", disk)
		for _, mline := range strings.Split(mpioOut, "\n") {
			mline = strings.TrimSpace(mline)
			if strings.HasPrefix(mline, "Vendor Id:") {
				lunInfo = strings.TrimSpace(strings.TrimPrefix(mline, "Vendor Id:"))
			}
			if strings.HasPrefix(mline, "Product Id:") {
				lunInfo += " " + strings.TrimSpace(strings.TrimPrefix(mline, "Product Id:"))
			}
		}
		
		// Try unique_id if no vendor info
		if lunInfo == "" {
			attrOut := runCmd("lsattr", "-El", disk)
			for _, aline := range strings.Split(attrOut, "\n") {
				if strings.HasPrefix(aline, "unique_id") {
					parts := strings.Fields(aline)
					if len(parts) > 1 {
						uid := parts[1]
						if len(uid) > 18 {
							uid = uid[:18] + "..."
						}
						lunInfo = uid
					}
				}
			}
		}
		
		if lunInfo == "" {
			lunInfo = "[gray]N/A[white]"
		}
		if len(lunInfo) > 20 {
			lunInfo = lunInfo[:20]
		}
		
		stateColor := "[green]"
		if state != "active" {
			stateColor = "[yellow]"
		}
		if vg == "None" {
			vg = "[gray]None[white]"
		}
		
		text += fmt.Sprintf("%-10s %-36s %-12s %s%-10s[white] %-20s\n",
			disk, pvid, vg, stateColor, state, lunInfo)
	}
	return text
}

func getLVTable(vg string) string {
	var vgs []string
	if vg == "" {
		out := runCmd("lsvg")
		vgs = strings.Split(strings.TrimSpace(out), "\n")
	} else {
		vgs = []string{vg}
	}
	
	text := "[yellow]LOGICAL VOLUMES[white]\n"
	text += fmt.Sprintf("%-15s %-10s %-8s %8s %8s %-10s %-15s\n", "LV", "VG", "TYPE", "LPs", "SIZE", "STATE", "MOUNT")
	text += strings.Repeat("─", 85) + "\n"
	
	for _, v := range vgs {
		if v == "" {
			continue
		}
		out := runCmd("lsvg", "-l", v)
		lines := strings.Split(out, "\n")
		
		// Get PP size for this VG to calculate size
		vgInfo := runCmd("lsvg", v)
		ppSize := 128 // default
		for _, line := range strings.Split(vgInfo, "\n") {
			if strings.Contains(line, "PP SIZE:") {
				parts := strings.Split(line, "PP SIZE:")
				if len(parts) > 1 {
					fmt.Sscanf(strings.Fields(parts[1])[0], "%d", &ppSize)
				}
			}
		}
		
		for i, line := range lines {
			if i < 2 || strings.TrimSpace(line) == "" {
				continue
			}
			fields := strings.Fields(line)
			if len(fields) < 6 {
				continue
			}
			
			lv := fields[0]
			lvType := fields[1]
			lps := fields[2]
			state := fields[5]
			mount := ""
			if len(fields) > 6 {
				mount = fields[6]
			}
			
			// Calculate size
			var lpCount int
			fmt.Sscanf(lps, "%d", &lpCount)
			sizeMB := lpCount * ppSize
			sizeStr := fmt.Sprintf("%dMB", sizeMB)
			if sizeMB >= 1024 {
				sizeStr = fmt.Sprintf("%.1fGB", float64(sizeMB)/1024)
			}
			
			stateColor := "[green]"
			if state == "closed/syncd" || state == "open/syncd" {
				stateColor = "[green]"
			} else if strings.Contains(state, "stale") {
				stateColor = "[red]"
			}
			
			text += fmt.Sprintf("%-15s %-10s %-8s %8s %8s %s%-10s[white] %-15s\n",
				lv, v, lvType, lps, sizeStr, stateColor, state, mount)
		}
	}
	return text
}

func getFSTable() string {
	out := runCmd("lsfs", "-c")
	
	text := "[yellow]FILESYSTEMS[white]\n"
	text += fmt.Sprintf("%-20s %-15s %10s %10s %6s %-10s\n", "MOUNT", "DEVICE", "SIZE", "FREE", "USE%", "TYPE")
	text += strings.Repeat("─", 80) + "\n"
	
	// Also get df for usage
	dfOut := runCmd("df", "-m")
	dfMap := make(map[string][]string)
	for _, line := range strings.Split(dfOut, "\n") {
		fields := strings.Fields(line)
		if len(fields) >= 4 && strings.HasPrefix(fields[0], "/dev/") {
			mount := fields[len(fields)-1]
			dfMap[mount] = fields
		}
	}
	
	for _, line := range strings.Split(out, "\n") {
		if strings.HasPrefix(line, "#") || strings.TrimSpace(line) == "" {
			continue
		}
		parts := strings.Split(line, ":")
		if len(parts) < 4 {
			continue
		}
		
		mount := parts[0]
		device := parts[1]
		fsType := parts[3]
		
		size, free, usePct := "", "", ""
		if df, ok := dfMap[mount]; ok && len(df) >= 4 {
			size = df[1] + "MB"
			free = df[2] + "MB"
			usePct = df[3]
		}
		
		// Color code usage
		useColor := "[green]"
		if usePct != "" {
			var pct int
			fmt.Sscanf(usePct, "%d", &pct)
			if pct > 90 {
				useColor = "[red]"
			} else if pct > 80 {
				useColor = "[yellow]"
			}
		}
		
		if device == "" {
			device = "[gray]N/A[white]"
		}
		
		text += fmt.Sprintf("%-20s %-15s %10s %10s %s%6s[white] %-10s\n",
			mount, device, size, free, useColor, usePct, fsType)
	}
	return text
}

func getDashboard() string {
	text := "[yellow::b]═══════════════════════════════════════════════════════════════════════════════[white]\n"
	text += "[yellow::b]                        AIX STORAGE DASHBOARD - stg-tui                        [white]\n"
	text += "[yellow::b]═══════════════════════════════════════════════════════════════════════════════[white]\n\n"
	
	// Quick summary
	vgOut := runCmd("lsvg")
	vgCount := len(strings.Split(strings.TrimSpace(vgOut), "\n"))
	
	pvOut := runCmd("lspv")
	pvCount := 0
	for _, line := range strings.Split(pvOut, "\n") {
		if strings.HasPrefix(strings.TrimSpace(line), "hdisk") {
			pvCount++
		}
	}
	
	fsOut := runCmd("lsfs", "-c")
	fsCount := 0
	for _, line := range strings.Split(fsOut, "\n") {
		if !strings.HasPrefix(line, "#") && strings.TrimSpace(line) != "" {
			fsCount++
		}
	}
	
	text += fmt.Sprintf("[green]Summary:[white] %d VGs | %d Disks | %d Filesystems\n\n", vgCount, pvCount, fsCount)
	
	return text
}

func main() {
	app := tview.NewApplication()
	
	pages := tview.NewPages()
	
	// Dashboard view
	dashboard := tview.NewTextView().
		SetDynamicColors(true).
		SetScrollable(true)
	dashboard.SetBorder(true).SetTitle(" Dashboard [1] ")
	
	// VG view
	vgView := tview.NewTextView().
		SetDynamicColors(true).
		SetScrollable(true)
	vgView.SetBorder(true).SetTitle(" Volume Groups [2] ")
	
	// PV view
	pvView := tview.NewTextView().
		SetDynamicColors(true).
		SetScrollable(true)
	pvView.SetBorder(true).SetTitle(" Physical Volumes [3] ")
	
	// LV view
	lvView := tview.NewTextView().
		SetDynamicColors(true).
		SetScrollable(true)
	lvView.SetBorder(true).SetTitle(" Logical Volumes [4] ")
	
	// FS view
	fsView := tview.NewTextView().
		SetDynamicColors(true).
		SetScrollable(true)
	fsView.SetBorder(true).SetTitle(" Filesystems [5] ")
	
	// Help bar
	help := tview.NewTextView().
		SetText(" [yellow]1[white] Dashboard  [yellow]2[white] VGs  [yellow]3[white] Disks/PVs  [yellow]4[white] LVs  [yellow]5[white] Filesystems  [yellow]r[white] Refresh  [yellow]q[white] Quit").
		SetDynamicColors(true)
	
	// Refresh function
	refresh := func() {
		dashText := getDashboard()
		dashText += getVGTable() + "\n"
		dashText += getPVTable()
		dashboard.SetText(dashText)
		
		vgView.SetText(getVGTable())
		pvView.SetText(getPVTable())
		lvView.SetText(getLVTable(""))
		fsView.SetText(getFSTable())
	}
	
	// Build pages
	for name, view := range map[string]tview.Primitive{
		"dashboard": dashboard,
		"vg":        vgView,
		"pv":        pvView,
		"lv":        lvView,
		"fs":        fsView,
	} {
		flex := tview.NewFlex().SetDirection(tview.FlexRow).
			AddItem(view, 0, 1, true).
			AddItem(help, 1, 0, false)
		pages.AddPage(name, flex, true, name == "dashboard")
	}
	
	// Initial load
	refresh()
	
	// Key handling
	app.SetInputCapture(func(event *tcell.EventKey) *tcell.EventKey {
		switch event.Rune() {
		case 'q':
			app.Stop()
			return nil
		case '1':
			pages.SwitchToPage("dashboard")
			return nil
		case '2':
			pages.SwitchToPage("vg")
			return nil
		case '3':
			pages.SwitchToPage("pv")
			return nil
		case '4':
			pages.SwitchToPage("lv")
			return nil
		case '5':
			pages.SwitchToPage("fs")
			return nil
		case 'r':
			refresh()
			return nil
		}
		return event
	})
	
	if err := app.SetRoot(pages, true).EnableMouse(false).Run(); err != nil {
		panic(err)
	}
}
