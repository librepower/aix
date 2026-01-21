package main

import (
	"fmt"
	"os/exec"
	"strings"

	"github.com/gdamore/tcell/v2"
	"github.com/rivo/tview"
)

type VGInfo struct {
	Name      string
	State     string
	PPSize    string
	TotalPPs  string
	FreePPs   string
	UsedPPs   string
	LVCount   string
	PVCount   string
}

func getVolumeGroups() []string {
	out, err := exec.Command("lsvg").Output()
	if err != nil {
		return []string{}
	}
	return strings.Split(strings.TrimSpace(string(out)), "\n")
}

func getVGInfo(vg string) *VGInfo {
	out, err := exec.Command("lsvg", vg).Output()
	if err != nil {
		return nil
	}
	
	info := &VGInfo{Name: vg}
	lines := strings.Split(string(out), "\n")
	
	for _, line := range lines {
		if strings.Contains(line, "VG STATE:") {
			parts := strings.Split(line, "VG STATE:")
			if len(parts) > 1 {
				info.State = strings.TrimSpace(strings.Split(parts[1], "PP SIZE:")[0])
			}
		}
		if strings.Contains(line, "PP SIZE:") {
			parts := strings.Split(line, "PP SIZE:")
			if len(parts) > 1 {
				info.PPSize = strings.TrimSpace(parts[1])
			}
		}
		if strings.Contains(line, "TOTAL PPs:") {
			parts := strings.Split(line, "TOTAL PPs:")
			if len(parts) > 1 {
				info.TotalPPs = strings.TrimSpace(strings.Split(parts[1], "FREE PPs:")[0])
			}
		}
		if strings.Contains(line, "FREE PPs:") {
			parts := strings.Split(line, "FREE PPs:")
			if len(parts) > 1 {
				info.FreePPs = strings.TrimSpace(parts[1])
			}
		}
		if strings.Contains(line, "USED PPs:") {
			parts := strings.Split(line, "USED PPs:")
			if len(parts) > 1 {
				info.UsedPPs = strings.TrimSpace(parts[1])
			}
		}
		if strings.Contains(line, "LVs:") && !strings.Contains(line, "OPEN") && !strings.Contains(line, "MAX") {
			parts := strings.Split(line, "LVs:")
			if len(parts) > 1 {
				info.LVCount = strings.TrimSpace(strings.Split(parts[1], "USED PPs:")[0])
			}
		}
		if strings.Contains(line, "TOTAL PVs:") {
			parts := strings.Split(line, "TOTAL PVs:")
			if len(parts) > 1 {
				info.PVCount = strings.TrimSpace(strings.Split(parts[1], "VG DESCRIPTORS:")[0])
			}
		}
	}
	return info
}

func getLVsForVG(vg string) []string {
	out, err := exec.Command("lsvg", "-l", vg).Output()
	if err != nil {
		return []string{}
	}
	lines := strings.Split(string(out), "\n")
	var lvs []string
	for i, line := range lines {
		if i < 2 || strings.TrimSpace(line) == "" {
			continue
		}
		fields := strings.Fields(line)
		if len(fields) > 0 {
			lvs = append(lvs, fields[0])
		}
	}
	return lvs
}

func getPVsForVG(vg string) []string {
	out, err := exec.Command("lsvg", "-p", vg).Output()
	if err != nil {
		return []string{}
	}
	lines := strings.Split(string(out), "\n")
	var pvs []string
	for i, line := range lines {
		if i < 2 || strings.TrimSpace(line) == "" {
			continue
		}
		fields := strings.Fields(line)
		if len(fields) > 0 {
			pvs = append(pvs, fields[0])
		}
	}
	return pvs
}

func main() {
	app := tview.NewApplication()
	
	// Main flex layout
	flex := tview.NewFlex()
	
	// Left panel - VG list
	vgList := tview.NewList().
		ShowSecondaryText(false).
		SetHighlightFullLine(true)
	vgList.SetBorder(true).SetTitle(" Volume Groups ")
	
	// Right panel - Details
	details := tview.NewTextView().
		SetDynamicColors(true).
		SetScrollable(true)
	details.SetBorder(true).SetTitle(" Details ")
	
	// Bottom panel - Help
	help := tview.NewTextView().
		SetText(" [yellow]↑/↓[white] Navigate  [yellow]Enter[white] Select  [yellow]l[white] LVs  [yellow]p[white] PVs  [yellow]q[white] Quit").
		SetDynamicColors(true)
	
	// Populate VG list
	vgs := getVolumeGroups()
	for _, vg := range vgs {
		vgList.AddItem(vg, "", 0, nil)
	}
	
	// Update details when VG selected
	updateDetails := func(vgName string) {
		info := getVGInfo(vgName)
		if info == nil {
			details.SetText("Error getting VG info")
			return
		}
		
		text := fmt.Sprintf(`[yellow]Volume Group: [white]%s

[green]Status[white]
  State:     %s
  PP Size:   %s

[green]Capacity[white]
  Total:     %s
  Used:      %s
  Free:      %s

[green]Contents[white]
  LVs:       %s
  PVs:       %s
`,
			info.Name,
			info.State,
			info.PPSize,
			info.TotalPPs,
			info.UsedPPs,
			info.FreePPs,
			info.LVCount,
			info.PVCount)
		
		details.SetText(text)
	}
	
	vgList.SetChangedFunc(func(index int, mainText string, secondaryText string, shortcut rune) {
		updateDetails(mainText)
	})
	
	// Show LVs for selected VG
	showLVs := func() {
		idx := vgList.GetCurrentItem()
		if idx < 0 {
			return
		}
		vgName, _ := vgList.GetItemText(idx)
		lvs := getLVsForVG(vgName)
		
		text := fmt.Sprintf("[yellow]Logical Volumes in %s:[white]\n\n", vgName)
		for _, lv := range lvs {
			text += fmt.Sprintf("  • %s\n", lv)
		}
		details.SetText(text)
		details.SetTitle(fmt.Sprintf(" LVs in %s ", vgName))
	}
	
	// Show PVs for selected VG
	showPVs := func() {
		idx := vgList.GetCurrentItem()
		if idx < 0 {
			return
		}
		vgName, _ := vgList.GetItemText(idx)
		pvs := getPVsForVG(vgName)
		
		text := fmt.Sprintf("[yellow]Physical Volumes in %s:[white]\n\n", vgName)
		for _, pv := range pvs {
			text += fmt.Sprintf("  • %s\n", pv)
		}
		details.SetText(text)
		details.SetTitle(fmt.Sprintf(" PVs in %s ", vgName))
	}
	
	// Keyboard handling
	app.SetInputCapture(func(event *tcell.EventKey) *tcell.EventKey {
		switch event.Rune() {
		case 'q':
			app.Stop()
			return nil
		case 'l':
			showLVs()
			return nil
		case 'p':
			showPVs()
			return nil
		}
		return event
	})
	
	// Initial selection
	if len(vgs) > 0 {
		updateDetails(vgs[0])
	}
	
	// Layout
	mainFlex := tview.NewFlex().
		AddItem(vgList, 25, 0, true).
		AddItem(details, 0, 1, false)
	
	flex.SetDirection(tview.FlexRow).
		AddItem(mainFlex, 0, 1, true).
		AddItem(help, 1, 0, false)
	
	if err := app.SetRoot(flex, true).EnableMouse(false).Run(); err != nil {
		panic(err)
	}
}
