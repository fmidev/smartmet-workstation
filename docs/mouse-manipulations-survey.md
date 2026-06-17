# SmartMet Mouse Manipulations Survey

This is a code-based survey of mouse interactions implemented through `NFmiCtrlView` descendants and the composite views that route events to them.

The survey is intentionally short and practical. It focuses on what the user can do, not every internal branch.

## Base event model

The common mouse API is defined in `NFmiCtrlView`:

- `LeftButtonDown`, `LeftButtonUp`
- `MiddleButtonDown`, `MiddleButtonUp`
- `RightButtonDown`, `RightButtonUp`
- `MouseMove`
- `MouseWheel`
- `LeftDoubleClick`, `RightDoubleClick`

Main reference:

- `libs/fmi/bizcode/ctrlviews/include/NFmiCtrlView.h`

Event routing is often handled by:

- `NFmiCtrlViewList`: forwards events to child views
- `NFmiEditMapView`: top-level map composite
- `NFmiCrossSectionManagerView`: cross-section composite
- `NFmiStationViewHandler`: map/station content handler
- `NFmiParamHandlerView`: parameter box composite

## Map view

Main dispatcher:

- `libs/fmi/bizcode/stationviews/source/NFmiEditMapView.cpp`
- `libs/fmi/bizcode/stationviews/source/NFmiStationViewHandler.cpp`

Short summary:

- Left click in the map delegates either to the time-control subview or the map/station content area.
- Right click also delegates by region: time control, parameter box, map content, or view-specific popups.
- Middle-drag is used for map zoom-box selection.
- `Ctrl` + middle-drag switches the middle-drag behavior from zoom-box to panning.
- Left double-click zooms in if no child subview consumes the event.
- Right double-click zooms out if no child subview consumes the event.
- Mouse move is heavily mode-dependent: normal selection hover, brush editing, control-point moving, cross-section point dragging, temp-view location dragging, and middle-drag zoom/pan preview.

Map/station content details from `NFmiStationViewHandler`:

- Left click selects/updates locations and active view row/time.
- In range-meter mode, left-drag defines the measured range; `Ctrl` + left click moves the range-meter start point instead of ending drag.
- In brush mode, left/right button state controls continuous editing while dragging.
- In control-point mode, drag moves the active control point.
- In cross-section manipulation mode:
  - left click sets the cross-section start point
  - `Ctrl` + left click activates the nearest minor point
  - right click sets the cross-section end point
  - middle click can set the cross-section middle point when cross-section mode reserves the middle button
- Right click on the time box opens a time-box popup.
- Right click on empty/normal map content is also used for location selection logic.
- Mouse wheel over the time box adjusts visual settings:
  - wheel: handled by time-box logic
  - `Ctrl` + wheel: change time-box text size
  - `Ctrl` + `Shift` + wheel: change time-box background alpha
- `Ctrl` + `Shift` + wheel on the map can change all visible hybrid/pressure-like levels together.

Key references:

- `libs/fmi/bizcode/stationviews/source/NFmiEditMapView.cpp:479`
- `libs/fmi/bizcode/stationviews/source/NFmiStationViewHandler.cpp:2208`
- `libs/fmi/bizcode/stationviews/source/NFmiStationViewHandler.cpp:2361`
- `libs/fmi/bizcode/stationviews/source/NFmiStationViewHandler.cpp:2516`
- `libs/fmi/bizcode/stationviews/source/NFmiStationViewHandler.cpp:2578`
- `libs/fmi/bizcode/stationviews/source/NFmiStationViewHandler.cpp:2671`
- `libs/fmi/bizcode/stationviews/source/NFmiStationViewHandler.cpp:2916`
- `libs/fmi/bizcode/stationviews/source/NFmiStationViewHandler.cpp:3136`

## Time-control view

Main implementation:

- `libs/fmi/bizcode/timeviews/source/NFmiTimeControlView.cpp`

Short summary:

- Left click steps time backward.
- Right click steps time forward.
- Middle click sets the selected time directly to the clicked position.
- `Ctrl` + click sets time directly to the clicked position instead of stepping.
- `Ctrl` + `Shift` + left click sets the start time filter boundary to the clicked time.
- `Ctrl` + `Shift` + right click sets the end time filter boundary to the clicked time.
- Left-drag can move:
  - the selected time marker
  - time filter handles
  - the shown time range window
  - the animation time box
- Clicking the full-time-range button resets time filter times.
- Clicking the resolution changer changes resolution:
  - left click one direction
  - right click the other direction
  - wheel also changes it
- Shift + drag can create or resize the animation time box.
- Animation box buttons support play/close/run-mode/lock-mode/delay/last-frame-delay/vertical-control operations.

Mouse wheel behavior:

- Wheel over resolution changer: change time resolution.
- `Ctrl` + wheel over time axis: stretch/shrink the shown time range around the cursor.
- `Shift` + wheel over time axis: move the shown time range left/right in time.
- Wheel over animation delay button: change frame delay.
- Wheel over animation last-frame-delay button: change last-frame delay factor.
- Wheel elsewhere in the time control is also used for time stepping and related controls depending on hit area.

Key references:

- `libs/fmi/bizcode/timeviews/source/NFmiTimeControlView.cpp:1022`
- `libs/fmi/bizcode/timeviews/source/NFmiTimeControlView.cpp:1043`
- `libs/fmi/bizcode/timeviews/source/NFmiTimeControlView.cpp:1281`
- `libs/fmi/bizcode/timeviews/source/NFmiTimeControlView.cpp:1333`
- `libs/fmi/bizcode/timeviews/source/NFmiTimeControlView.cpp:1389`
- `libs/fmi/bizcode/timeviews/source/NFmiTimeControlView.cpp:1394`
- `libs/fmi/bizcode/timeviews/source/NFmiTimeControlView.cpp:1619`

## Parameter box / helper parameter views

Main implementations:

- `libs/fmi/bizcode/ctrlviews/source/NFmiParamHandlerView.cpp`
- `libs/fmi/bizcode/ctrlviews/source/NFmiViewParamsView.cpp`

Short summary:

- Right click on a parameter row opens the parameter/view popup.
- Left click on a parameter row toggles parameter show/hide.
- `Ctrl` + left click activates the clicked parameter.
- `Shift` + left click removes the clicked parameter from the view.
- Left double-click on an active parameter opens the draw-parameter dialog.
- Left double-click on an inactive parameter activates it.
- Left-drag reorders parameter rows in drawing order.
- Mouse wheel on a parameter row:
  - wheel: move parameter in drawing order
  - `Ctrl` + wheel: change active parameter to previous/next
  - `Shift` + wheel: cycle fixed draw settings

Key references:

- `libs/fmi/bizcode/ctrlviews/source/NFmiParamHandlerView.cpp:205`
- `libs/fmi/bizcode/ctrlviews/source/NFmiViewParamsView.cpp:280`
- `libs/fmi/bizcode/ctrlviews/source/NFmiViewParamsView.cpp:317`
- `libs/fmi/bizcode/ctrlviews/source/NFmiViewParamsView.cpp:381`
- `libs/fmi/bizcode/ctrlviews/source/NFmiViewParamsView.cpp:415`
- `libs/fmi/bizcode/ctrlviews/source/NFmiViewParamsView.cpp:468`

## Time-series view

Main implementation:

- `libs/fmi/bizcode/timeviews/source/NFmiTimeSerialView.cpp`

Short summary:

- First click can activate the subview for editing when several time-series subviews exist.
- Left click in the value-axis area adjusts the visible value scale.
- `Ctrl` + left click in the value-axis area moves the value scale window.
- Left click in the modify-factor area decreases the modify-factor scale.
- Mouse drag in the data area edits the time-series modification curve when the parameter is editable.
- In control-point mode, drag writes the final curve values relative to the active control point.
- In analyze-related tools, drag updates the analyze end time.
- Right click in the header/title area opens the time-series popup.
- Right button down mainly reserves the click for the time-series view popup handling.
- Mouse wheel:
  - over modify-factor view: change modify-factor scale
  - over value axis: adjust or move value scale
  - `Ctrl` + wheel outside the scales: change shown parameter
  - `Shift` + wheel: change data level for hybrid/pressure-like data

Key references:

- `libs/fmi/bizcode/timeviews/source/NFmiTimeSerialView.cpp:2414`
- `libs/fmi/bizcode/timeviews/source/NFmiTimeSerialView.cpp:2738`
- `libs/fmi/bizcode/timeviews/source/NFmiTimeSerialView.cpp:2753`
- `libs/fmi/bizcode/timeviews/source/NFmiTimeSerialView.cpp:2839`
- `libs/fmi/bizcode/timeviews/source/NFmiTimeSerialView.cpp:2934`
- `libs/fmi/bizcode/timeviews/source/NFmiTimeSerialView.cpp:4097`

## Sounding / Temp view

Main implementation:

- `libs/fmi/bizcode/ctrlviews/source/NFmiTempView.cpp`

Short summary:

- Left click in the sounding data area modifies dew point.
- Right click in the sounding data area modifies temperature.
- Drag with left/right button held continues dew point / temperature editing.
- `Ctrl` while modifying uses quick-modification logic instead of exact point dragging.
- Left/right click on the temperature axis changes the left/right temperature bounds.
- Left/right click on the pressure axis changes the top/bottom pressure bounds.
- If hodograph is visible, click in the sounding data area sets the hodograph center instead of editing sounding values.
- Clicks on sounding animation controls:
  - left click: move backward in time / previous animation step
  - right click: move forward in time / next animation step

Mouse wheel behavior:

- `Ctrl` + `Shift` + wheel anywhere except textual sounding area changes selected producer index.
- Wheel over hodograph:
  - wheel: change scale max
  - `Ctrl` + wheel: change relative hodograph size
- Wheel over sounding animation button: move soundings in time.
- Wheel over animation-step button: change animation step.
- Wheel over textual sounding side view scrolls text; `Ctrl` and `Ctrl` + `Shift` increase scroll amount.
- Wheel over temperature axis changes temperature axis bounds.
- Wheel over pressure axis changes pressure axis bounds.

Key references:

- `libs/fmi/bizcode/ctrlviews/source/NFmiTempView.cpp:2546`
- `libs/fmi/bizcode/ctrlviews/source/NFmiTempView.cpp:3714`
- `libs/fmi/bizcode/ctrlviews/source/NFmiTempView.cpp:3768`
- `libs/fmi/bizcode/ctrlviews/source/NFmiTempView.cpp:4055`
- `libs/fmi/bizcode/ctrlviews/source/NFmiTempView.cpp:4069`

## Cross-section view

Main implementations:

- `libs/fmi/bizcode/stationviews/source/NFmiCrossSectionManagerView.cpp`
- `libs/fmi/bizcode/stationviews/source/NFmiCrossSectionView.cpp`

Short summary:

- The manager view routes mouse events either to:
  - the time-control view
  - the list of cross-section panels
  - the cross-section system header/footer logic
- In a cross-section panel:
  - left click on the pressure axis changes pressure scale one direction
  - right click on the pressure axis changes pressure scale the opposite direction
  - `Ctrl` makes the pressure-scale change finer
  - right click in the panel opens the cross-section popup
- Left double-click is mainly forwarded to the parameter handler view.
- Mouse wheel:
  - over time control: routed to time control
  - over header/footer: routed to cross-section system
  - over parameter box: routed to parameter handler
  - over pressure axis: changes pressure scale
  - elsewhere in the panel: routed to cross-section system

Key references:

- `libs/fmi/bizcode/stationviews/source/NFmiCrossSectionManagerView.cpp:180`
- `libs/fmi/bizcode/stationviews/source/NFmiCrossSectionManagerView.cpp:253`
- `libs/fmi/bizcode/stationviews/source/NFmiCrossSectionManagerView.cpp:699`
- `libs/fmi/bizcode/stationviews/source/NFmiCrossSectionView.cpp:2027`
- `libs/fmi/bizcode/stationviews/source/NFmiCrossSectionView.cpp:2088`
- `libs/fmi/bizcode/stationviews/source/NFmiCrossSectionView.cpp:2113`
- `libs/fmi/bizcode/stationviews/source/NFmiCrossSectionView.cpp:3127`
- `libs/fmi/bizcode/stationviews/source/NFmiCrossSectionView.cpp:3149`

## Trajectory view

Main implementation:

- `libs/fmi/bizcode/timeviews/source/NFmiTrajectoryView.cpp`

Short summary:

- Left click in the trajectory data area selects the trajectory pressure level.
- Left click on the pressure axis adjusts the pressure axis one direction.
- Right click on the pressure axis adjusts the pressure axis the opposite direction.
- The view also has its own time-control subview for time navigation.

Key references:

- `libs/fmi/bizcode/timeviews/source/NFmiTrajectoryView.cpp:243`
- `libs/fmi/bizcode/timeviews/source/NFmiTrajectoryView.cpp:263`

## Filter/zoom helper views

Main implementation:

- `libs/fmi/bizcode/filterdataviews/source/NFmiZoomView.cpp`

Short summary:

- Left-drag inside the zoom rectangle moves it.
- Left-drag from the bottom-right handle resizes it.
- `Ctrl` + left click scales the zoom rectangle smaller.
- `Ctrl` + `Shift` + left click resets to total area.
- `Ctrl` + right click scales the zoom rectangle larger.

Key references:

- `libs/fmi/bizcode/filterdataviews/source/NFmiZoomView.cpp:148`
- `libs/fmi/bizcode/filterdataviews/source/NFmiZoomView.cpp:166`
- `libs/fmi/bizcode/filterdataviews/source/NFmiZoomView.cpp:195`
- `libs/fmi/bizcode/filterdataviews/source/NFmiZoomView.cpp:215`

## Notes

- Many top-level MFC windows only forward raw mouse messages into these `NFmiCtrlView` handlers. The real interaction logic is mostly in the files listed above.
- Some behavior is highly mode-dependent: brush mode, control-point mode, cross-section mode, range-meter mode, animation mode, and editable/non-editable parameter state all change what the same mouse action does.
- This survey is intentionally selective. It covers the main user-visible interactions for map, time control, time series, sounding/temp, cross-section, trajectory, parameter box, and zoom helper views.
