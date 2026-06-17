# SmartMet Keyboard Accelerators Survey

This is a short code-based survey of keyboard accelerators defined in the SmartMet workstation resource files and the handlers they trigger.

Main accelerator tables covered:

- `src/SmartMet/SmartMet.rc` -> `IDR_MAINFRAME`
- `src/toolboxdep/SmartMetToolboxDep.rc` -> `IDR_ACCELERATOR1`
- `src/toolboxdep/SmartMetToolboxDep.rc` -> `IDR_ACCELERATOR_SYNTAX_EDIT_CONTROL`
- `src/toolboxdep/SmartMetToolboxDep.rc` -> `IDR_ACCELERATOR_SOUNDING_VIEW`
- `src/dialogs/SmartMetDialogs.rc` -> `IDR_ACCELERATOR2`

Main handler files:

- `src/SmartMet/SmartMetDoc.cpp`
- `src/SmartMet/SmartMetView.cpp`
- `src/toolboxdep/FmiExtraMapViewDlg.cpp`
- `src/toolboxdep/TimeEditValuesDlg.cpp`
- `src/toolboxdep/FmiCrossSectionDlg.cpp`
- `src/toolboxdep/FmiTempDlg.cpp`

## Main application accelerators (`IDR_MAINFRAME`)

### View macros, logging, diagnostics

- `F12`: apply the normal backup view macro.
- `Ctrl+F12`: apply the crash-backup view macro.
- `Shift+F12`: apply the startup view macro.
- `Ctrl+I`: open/store a view macro.
- `Ctrl+L`: open the log viewer.
- `Insert`: run the developer testing hook (`MakeVisualizationImages`).
- `Ctrl+Shift+F1`: start visualization profiling.
- `Ctrl+Shift+F5`: reload all dynamic help/dynamic data.

References:

- `src/SmartMet/SmartMetDoc.cpp:3476`
- `src/SmartMet/SmartMetDoc.cpp:3482`
- `src/SmartMet/SmartMetDoc.cpp:3761`
- `src/SmartMet/SmartMetDoc.cpp:2583`
- `src/SmartMet/SmartMetDoc.cpp:3613`
- `src/SmartMet/SmartMetDoc.cpp:3493`
- `src/SmartMet/SmartMetDoc.cpp:3817`
- `src/SmartMet/SmartMetDoc.cpp:1768`

### Clipboard and information shortcuts

- `Ctrl+M`: copy the active main-map area information to the clipboard.
- `Shift+M`: copy active observation calculation-point information to the clipboard.
- `Ctrl+Shift+M`: copy the active row's source data file paths to the clipboard.
- `Ctrl+C` or `Ctrl+Insert`: normal copy.
- `Ctrl+V` or `Shift+Insert`: normal paste.
- `Delete` or `Ctrl+X`: cut/delete selection, depending on context.

References:

- `src/SmartMet/SmartMetDoc.cpp:3417`
- `src/SmartMet/SmartMetDoc.cpp:3701`
- `src/SmartMet/SmartMetDoc.cpp:3742`

### Main map row and parameter-row shortcuts

- `1..0` or numpad `1..0`: activate map row 1..10 in the main map view.
- `Ctrl+1..0`: borrow parameters from row 1..10 to the active row.
- `Ctrl+Shift+Up`: jump 10 rows upward.
- `Ctrl+Shift+Down`: jump 10 rows downward.
- `Ctrl+Shift+Left`: move all model data on the active row to the previous model run.
- `Ctrl+Shift+Right`: move all model data on the active row to the next model run.

References:

- `src/SmartMet/SmartMetDoc.cpp:2380`
- `src/SmartMet/SmartMetDoc.cpp:2763`
- `src/SmartMet/SmartMetDoc.cpp:3749`
- `src/SmartMet/SmartMetView.cpp:1103`

### Main map time and area shortcuts

- `PgUp` / `PgDn`: move time by time-step set 1 backward/forward.
- `Ctrl+PgUp` / `Ctrl+PgDn`: move time by time-step set 2 backward/forward.
- `Shift+PgUp` / `Shift+PgDn`: move time by time-step set 3 backward/forward.
- `Ctrl+Shift+PgUp` / `Ctrl+Shift+PgDn`: move time by time-step set 4 backward/forward.
- `Home`: set the current view to home time.
- `Ctrl+G`: reset time-filter limits.
- `Space`: swap the currently used map area with the stored swap area.
- `Ctrl+Shift+Space`: same area swap via the secondary shortcut.
- `Ctrl+Space`: store the current map area as the swap-base area.
- `Z`: toggle the animation controls in the time-control area.
- `Ctrl+Shift+T`: move the time box location.

References:

- `src/SmartMet/SmartMetView.cpp:974`
- `src/SmartMet/SmartMetView.cpp:986`
- `src/SmartMet/SmartMetView.cpp:1011`
- `src/SmartMet/SmartMetView.cpp:1016`
- `src/SmartMet/SmartMetView.cpp:1063`
- `src/SmartMet/SmartMetDoc.cpp:3605`
- `src/SmartMet/SmartMetDoc.cpp:3861`

### Main map navigation and measuring

- `Ctrl+Arrow`: pan the map.
- `Shift+Up`: zoom in.
- `Shift+Down`: zoom out.
- `Y`: toggle map range-meter mode on/off.
- `Shift+Y`: change the range-meter color.
- `Ctrl+Y`: toggle the range-meter lock mode.
- `F10`: toggle "keep map ratio".
- `F11`: toggle map-view tooltips.
- `F7`: toggle the help cursor overlay on the map.
- `X`: open the location finder tool.
- `Ctrl+Shift+W`: toggle combined local/WMS map mode.

References:

- `src/SmartMet/SmartMetView.cpp:1125`
- `src/SmartMet/SmartMetView.cpp:1135`
- `src/SmartMet/SmartMetView.cpp:1160`
- `src/SmartMet/SmartMetDoc.cpp:3084`
- `src/SmartMet/SmartMetDoc.cpp:3089`
- `src/SmartMet/SmartMetDoc.cpp:3215`
- `src/SmartMet/SmartMetDoc.cpp:3233`
- `src/SmartMet/SmartMetDoc.cpp:3767`

### Main map row-copy and display toggles

- `Shift+C`: copy all map-view settings/parameters from the current main map.
- `Shift+V`: paste all copied map-view settings/parameters into the current main map.
- `Ctrl+F`: change the active map type.
- `Ctrl+Shift+F`: change the previous-map type.
- `Ctrl+H`: move the parameter window position forward.
- `Ctrl+Shift+H`: move the parameter window position backward.
- `Ctrl+T`: toggle drawing of the time string on the map.
- `Ctrl+E`: toggle grid-point drawing.
- `Shift+E`: change grid-point color.
- `Ctrl+Shift+E`: change grid-point size.
- `Ctrl+W`: toggle masks on the map.
- `Ctrl+R`: toggle projection lines.
- `Ctrl+Q`: toggle land-border draw color.
- `Ctrl+Shift+Q`: toggle land-border pen size.
- `Ctrl+B`: toggle names on the active map.
- `Ctrl+Shift+B`: toggle names on the previous/background map.
- `Shift+B`: toggle overmap foreground/background drawing.

References:

- `src/SmartMet/SmartMetView.cpp:1001`
- `src/SmartMet/SmartMetDoc.cpp:1865`
- `src/SmartMet/SmartMetDoc.cpp:1884`
- `src/SmartMet/SmartMetDoc.cpp:1889`
- `src/SmartMet/SmartMetDoc.cpp:1894`
- `src/SmartMet/SmartMetDoc.cpp:1904`
- `src/SmartMet/SmartMetDoc.cpp:1910`
- `src/SmartMet/SmartMetDoc.cpp:2139`
- `src/SmartMet/SmartMetDoc.cpp:2370`
- `src/SmartMet/SmartMetDoc.cpp:3194`
- `src/SmartMet/SmartMetDoc.cpp:3199`

### Control-point navigation

- `Tab`: select next control point.
- `Ctrl+Tab`: select previous control point.
- `A`, `D`, `W`, `S`: select the nearest control point left, right, up, or down.

References:

- `src/SmartMet/SmartMetDoc.cpp:3775`
- `src/SmartMet/SmartMetDoc.cpp:3810`

### Comparison, analysis, and tool dialogs

- `F2`: toggle observation comparison mode on/off.
- `Shift+F2`: cycle observation-comparison symbol style.
- `Ctrl+F2`: cycle observation-comparison symbol size.
- `Ctrl+Shift+F2`: toggle observation-comparison symbol border drawing.
- `F3`: open synop plot settings.
- `Ctrl+F3`: open the ignore-stations dialog.
- `F4`: toggle cross-section mode.
- `F5`: refresh all views.
- `F6`: open visualization settings.
- `F8`: open parameter-selection dialog.
- `Shift+F8`: open beta production dialog.
- `Ctrl+F8`: open case-study dialog.
- `F9`: open warning center dialog.
- `Ctrl+K`: toggle virtual time mode for the main map view.

References:

- `src/SmartMet/SmartMetDoc.cpp:2276`
- `src/SmartMet/SmartMetDoc.cpp:2319`
- `src/SmartMet/SmartMetDoc.cpp:2343`
- `src/SmartMet/SmartMetDoc.cpp:1014`
- `src/SmartMet/SmartMetDoc.cpp:2754`
- `src/SmartMet/SmartMetDoc.cpp:1762`
- `src/SmartMet/SmartMetDoc.cpp:3832`
- `src/SmartMet/SmartMetDoc.cpp:3884`

### Generic edit/file/help shortcuts

- `Ctrl+N`, `Ctrl+O`, `Ctrl+S`, `Ctrl+P`: standard new/open/save/print.
- `Ctrl+Z`, `Ctrl+Shift+Z`, `Alt+Backspace`: undo/redo.
- `Ctrl+A`, `Ctrl+Shift+A`: select all / deselect all.
- `Shift+F1`: Windows context help.
- `F1`: open the shortcut/help view.
- `Shift+G`: make a grid file.
- `Ctrl+D`: execute the shared "space out" editing action in the current context.

## Extra map accelerators (`IDR_ACCELERATOR1` in map-view 2/3 context)

This table is shared by several toolbox dialogs. In extra-map windows it mainly controls map-view 2 / map-view 3.

### Extra-map rows, params, and time

- `1..0` or numpad `1..0`: activate extra-map row 1..10.
- `Ctrl+1..0`: borrow parameters from extra-map row 1..10.
- `PgUp` / `PgDn`: move extra-map time by step set 1.
- `Ctrl+PgUp` / `Ctrl+PgDn`: move extra-map time by step set 2.
- `Shift+PgUp` / `Shift+PgDn`: move extra-map time by step set 3.
- `Ctrl+Shift+PgUp` / `Ctrl+Shift+PgDn`: move extra-map time by step set 4.
- `Home`: set home time for the extra map.
- `Z`: toggle extra-map animation controls.
- `Ctrl+Shift+T`: move the extra-map time box.

References:

- `src/toolboxdep/FmiExtraMapViewDlg.cpp:524`
- `src/toolboxdep/FmiExtraMapViewDlg.cpp:574`
- `src/toolboxdep/FmiExtraMapViewDlg.cpp:784`
- `src/toolboxdep/FmiExtraMapViewDlg.cpp:860`
- `src/toolboxdep/FmiExtraMapViewDlg.cpp:994`

### Extra-map area, pan, zoom, and locks

- `Space`: swap the extra-map area with its stored swap area.
- `Ctrl+Shift+Space`: same area swap via the secondary shortcut.
- `Ctrl+Space`: store current extra-map area as swap-base area.
- `Ctrl+Arrow`: pan extra map.
- `Shift+Up`: zoom in.
- `Shift+Down`: zoom out.
- `Ctrl+Shift+Up`: jump 10 rows upward.
- `Ctrl+Shift+Down`: jump 10 rows downward.
- `Shift+T`: toggle lock-to-main-map time.
- `Shift+R`: toggle lock-to-main-map row.
- `Ctrl+Shift+Left`: move all model data on the active extra-map row to the previous model run.
- `Ctrl+Shift+Right`: move all model data on the active extra-map row to the next model run.

References:

- `src/toolboxdep/FmiExtraMapViewDlg.cpp:729`
- `src/toolboxdep/FmiExtraMapViewDlg.cpp:742`
- `src/toolboxdep/FmiExtraMapViewDlg.cpp:804`
- `src/toolboxdep/FmiExtraMapViewDlg.cpp:905`

### Extra-map display toggles and dialogs

- `Ctrl+F`: change extra-map type.
- `Ctrl+Shift+F`: change previous/background map type.
- `Ctrl+T`: toggle time-string drawing.
- `Ctrl+E`: toggle grid-point drawing.
- `Shift+E`: toggle grid-point color.
- `Ctrl+Shift+E`: toggle grid-point size.
- `Ctrl+W`: toggle masks on map.
- `Ctrl+R`: toggle projection lines.
- `Ctrl+Q`: toggle land-border draw color.
- `Ctrl+Shift+Q`: toggle land-border pen size.
- `Ctrl+B`: toggle names on the active extra map.
- `Shift+B`: toggle overmap foreground/background.
- `F10`: toggle keep-map-ratio.
- `F11`: toggle tooltip.
- `F2`: toggle observation-comparison mode.
- `F5`: refresh all views.
- `F6`: open visualization settings.
- `F7`: toggle help cursor on extra map.
- `X`: open location finder.
- `Ctrl+L`: open log viewer.
- `Ctrl+K`: toggle virtual time mode for `Map-view-2` or `Map-view-3`.

References:

- `src/toolboxdep/FmiExtraMapViewDlg.cpp:394`
- `src/toolboxdep/FmiExtraMapViewDlg.cpp:483`
- `src/toolboxdep/FmiExtraMapViewDlg.cpp:493`
- `src/toolboxdep/FmiExtraMapViewDlg.cpp:498`
- `src/toolboxdep/FmiExtraMapViewDlg.cpp:504`
- `src/toolboxdep/FmiExtraMapViewDlg.cpp:514`
- `src/toolboxdep/FmiExtraMapViewDlg.cpp:769`
- `src/toolboxdep/FmiExtraMapViewDlg.cpp:779`
- `src/toolboxdep/FmiExtraMapViewDlg.cpp:855`
- `src/toolboxdep/FmiExtraMapViewDlg.cpp:900`
- `src/toolboxdep/FmiExtraMapViewDlg.cpp:1000`
- `src/toolboxdep/FmiExtraMapViewDlg.cpp:1009`

### Extra-map range meter

- `Y`: toggle range-meter mode.
- `Shift+Y`: toggle range-meter color.
- `Ctrl+Y`: toggle range-meter lock mode.

References:

- `src/toolboxdep/FmiExtraMapViewDlg.cpp:915`
- `src/toolboxdep/FmiExtraMapViewDlg.cpp:925`
- `src/toolboxdep/FmiExtraMapViewDlg.cpp:935`

## Time-series accelerators (same `IDR_ACCELERATOR1`, time-series context)

The same accelerator table is reused in the time-series dialog, but some keys are handled differently there.

- `Shift+F`: toggle helper data set 1 in the time-series view.
- `Ctrl+F`: toggle helper data set 2 in the time-series view.
- `Ctrl+Shift+F`: toggle helper data set 3 in the time-series view.
- `Alt+F`: toggle helper data set 4 in the time-series view.
- `F11`: toggle time-series tooltip.
- `Space`: auto-adjust value scale for the active row.
- `Ctrl+Shift+Space`: auto-adjust value scales for all rows.
- `Tab`, `Ctrl+Tab`, `A`, `D`, `W`, `S`: move control-point selection in the time-series view.
- `Ctrl+L`: open log viewer.
- `Ctrl+K`: toggle virtual time mode for `Time-serial-view`.

References:

- `src/toolboxdep/TimeEditValuesDlg.cpp:621`
- `src/toolboxdep/TimeEditValuesDlg.cpp:676`
- `src/toolboxdep/TimeEditValuesDlg.cpp:686`
- `src/toolboxdep/TimeEditValuesDlg.cpp:904`
- `src/toolboxdep/TimeEditValuesDlg.cpp:934`
- `src/toolboxdep/TimeEditValuesDlg.cpp:971`

## Cross-section accelerators (also from `IDR_ACCELERATOR1`, cross-section context)

These shared bindings are handled by the cross-section dialog when it has focus.

- `Ctrl+D`: set the default cross-section axis from the first visible view.
- `Shift+D`: save the current default-axis values.
- `Ctrl+Shift+D`: apply the default cross-section axis to all.
- `Ctrl+S`: set the special cross-section axis from the first visible view.
- `Shift+S`: save the current special-axis values.
- `Ctrl+Shift+S`: apply the special cross-section axis to all.

References:

- `src/toolboxdep/FmiCrossSectionDlg.cpp:473`
- `src/toolboxdep/FmiCrossSectionDlg.cpp:480`
- `src/toolboxdep/FmiCrossSectionDlg.cpp:487`
- `src/toolboxdep/FmiCrossSectionDlg.cpp:494`
- `src/toolboxdep/FmiCrossSectionDlg.cpp:501`
- `src/toolboxdep/FmiCrossSectionDlg.cpp:506`

## Sounding accelerators (`IDR_ACCELERATOR_SOUNDING_VIEW`)

- `Space`: auto-adjust sounding scales.
- `Ctrl+L`: open log viewer.
- `Ctrl+1..0`: toggle/select sounding producer 1..10.
- `Ctrl+D`: increase wind-barb spacing.
- `Ctrl+F`: toggle the secondary-data side view.
- `Ctrl+S`: toggle the stability-index side view.
- `Ctrl+T`: toggle the textual sounding side view.
- `F11`: toggle sounding tooltip.
- `Ctrl+K`: toggle virtual time mode for `Sounding-view`.

References:

- `src/toolboxdep/FmiTempDlg.cpp:756`
- `src/toolboxdep/FmiTempDlg.cpp:816`
- `src/toolboxdep/FmiTempDlg.cpp:822`
- `src/toolboxdep/FmiTempDlg.cpp:831`
- `src/toolboxdep/FmiTempDlg.cpp:840`
- `src/toolboxdep/FmiTempDlg.cpp:848`
- `src/toolboxdep/FmiTempDlg.cpp:856`
- `src/toolboxdep/FmiTempDlg.cpp:955`
- `src/toolboxdep/FmiTempDlg.cpp:962`

## Syntax editor accelerators (`IDR_ACCELERATOR_SYNTAX_EDIT_CONTROL`)

This is a standard text-editing accelerator table used by the syntax editor control:

- `Ctrl+O`: open file.
- `Ctrl+S`: save file.
- `Ctrl+X`, `Ctrl+C`, `Ctrl+V`: cut/copy/paste.
- `Ctrl+Insert`, `Shift+Insert`: copy/paste.
- `Ctrl+Z`, `Ctrl+Shift+Z`, `Alt+Backspace`: undo/redo.

Reference:

- `src/toolboxdep/SmartMetToolboxDep.rc:1617`

## Dialog print accelerator (`IDR_ACCELERATOR2`)

- `Ctrl+P`: print from the dialog that uses `IDR_ACCELERATOR2`.

Reference:

- `src/dialogs/SmartMetDialogs.rc:1038`

## Notes

- Some accelerator IDs are reused across different windows. The meaning depends on which view/dialog currently has focus.
- A few shortcuts are generic MFC edit/file shortcuts rather than meteorology-specific actions, but they are still part of the accelerator tables.
