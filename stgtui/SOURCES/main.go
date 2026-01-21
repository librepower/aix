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
	
	for _, vg := range strings.Split(strings.TrimSpace(runCmd("lsvg")), "\n") {
		if vg == "" { continue }
		
		vgInfo := runCmd("lsvg", vg)
		var totalPPs, usedPPs, freePPs int
		var state string
		
		for _, line := range strings.Split(vgInfo, "\n") {
			if strings.Contains(line, "VG STATE:") {
				parts := strings.Split(line, "VG STATE:")
				if len(parts) > 1 { state = strings.TrimSpace(strings.Fields(parts[1])[0]) }
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
		
		text += fmt.Sprintf("  %s [cyan]%-12s[white] %s\n", stateIcon, vg, progressBar(pct, 25))
	}
	
	// Filesystem Summary with bars
	text += "\n[yellow::b]═══ FILESYSTEMS ═══[white]\n\n"
	
	dfOut := runCmd("df", "-m")
	for _, line := range strings.Split(dfOut, "\n") {
		fields := strings.Fields(line)
		if len(fields) >= 6 && strings.HasPrefix(fields[0], "/dev/") {
			mount := fields[len(fields)-1]
			var pct int
			fmt.Sscanf(fields[3], "%d", &pct)
			
			// Truncate long mount points
			displayMount := mount
			if len(displayMount) > 20 { displayMount = "..." + displayMount[len(displayMount)-17:] }
			
			text += fmt.Sprintf("  %-20s %s\n", displayMount, progressBar(pct, 25))
		}
	}
	
	// Disk count
	pvCount := 0
	for _, line := range strings.Split(runCmd("lspv"), "\n") {
		if strings.HasPrefix(strings.TrimSpace(line), "hdisk") { pvCount++ }
	}
	
	text += fmt.Sprintf("\n[gray]  Total disks: %d[white]\n", pvCount)
	
	return text
}

// ============ MAPPING: FS → LV → VG → PV → LUN ============
func mapFStoLUN(mount string) string {
	dfOut := runCmd("df", "-m", mount)
	var device, sizeMB, freeMB, usePct string
	for _, line := range strings.Split(dfOut, "\n") {
		fields := strings.Fields(line)
		if len(fields) >= 6 && strings.HasPrefix(fields[0], "/dev/") {
			device = fields[0]
			sizeMB = fields[1]
			freeMB = fields[2]
			usePct = fields[3]
			break
		}
	}
	if device == "" { return "[red]Cannot find device for " + mount + "[white]" }
	
	lv := strings.TrimPrefix(device, "/dev/")
	
	var pct int
	fmt.Sscanf(usePct, "%d", &pct)
	
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
	for _, line := range strings.Split(vgInfo, "\n") {
		if strings.Contains(line, "VG STATE:") {
			parts := strings.Split(line, "VG STATE:")
			if len(parts) > 1 { vgState = strings.TrimSpace(strings.Fields(parts[1])[0]) }
		}
		if strings.Contains(line, "PP SIZE:") {
			parts := strings.Split(line, "PP SIZE:")
			if len(parts) > 1 { ppSize = strings.TrimSpace(parts[1]) }
		}
	}
	
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
  └─────────────────────────────────────────────────┘
           │
           ▼
[green]▼ PHYSICAL VOLUMES (LUNs)[white]
`, mount, device, sizeMB+"MB", freeMB+"MB", progressBar(pct, 30),
		lv, lvType, lvState, lps, vg, vgState, ppSize)

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
	var pvState, totalPPs, freePPs, usedPPs int
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
	
	_ = pvState
	
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
	for _, line := range strings.Split(vgInfo, "\n") {
		if strings.Contains(line, "VG STATE:") {
			parts := strings.Split(line, "VG STATE:")
			if len(parts) > 1 { vgState = strings.TrimSpace(strings.Fields(parts[1])[0]) }
		}
		if strings.Contains(line, "PP SIZE:") {
			parts := strings.Split(line, "PP SIZE:")
			if len(parts) > 1 { ppSize = strings.TrimSpace(parts[1]) }
		}
	}
	
	text += fmt.Sprintf(`[green]▼ VOLUME GROUP[white]
  ┌─────────────────────────────────────────────────┐
  │  VG Name:  [cyan]%-35s[white]  │
  │  State:    %-35s  │
  │  PP Size:  %-35s  │
  └─────────────────────────────────────────────────┘
           │
           ▼
[green]▼ LVs & FILESYSTEMS on this disk[white]
`, vgName, vgState, ppSize)

	text += fmt.Sprintf("  %-15s %-8s %8s %-20s\n", "LV", "TYPE", "LPs", "MOUNT")
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
			
			text += fmt.Sprintf("  [cyan]%-15s[white] %-8s %8s %-20s\n", lvName, lvType, lps, mount)
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
	
	detailView := tview.NewTextView().SetDynamicColors(true).SetScrollable(true)
	detailView.SetBorder(true)
	
	pvList := tview.NewList().ShowSecondaryText(false).SetHighlightFullLine(true)
	pvList.SetBorder(true).SetTitle(" Disks → FS [2] ")
	
	fsList := tview.NewList().ShowSecondaryText(false).SetHighlightFullLine(true)
	fsList.SetBorder(true).SetTitle(" FS → Disks [3] ")
	
	help := tview.NewTextView().
		SetText(" [yellow]1[white] Dashboard  [yellow]2[white] Disk→FS  [yellow]3[white] FS→Disk  [yellow]r[white] Refresh  [yellow]Esc[white] Back  [yellow]q[white] Quit").
		SetDynamicColors(true)
	
	// Populate
	refresh := func() {
		dashView.SetText(getDashboard())
		
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
			if len(fields) >= 6 && strings.HasPrefix(fields[0], "/dev/") {
				mount := fields[len(fields)-1]
				var pct int
				fmt.Sscanf(fields[3], "%d", &pct)
				
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
		case '2': pages.SwitchToPage("pv"); return nil
		case '3': pages.SwitchToPage("fs"); return nil
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
