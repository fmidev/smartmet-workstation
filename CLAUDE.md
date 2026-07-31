# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

SmartMet Workstation is a desktop weather analysis and visualization application developed by the Finnish Meteorological Institute (FMI). Originally a Windows MFC application, it is being actively ported to Linux using Qt6. The codebase is ~2,500 C++ files with dual-platform support from a single source tree.

## Build commands

### Linux (primary development target)

```bash
make configure    # CMake configure (RelWithDebInfo, disables Windows-only features)
make build        # Build with all CPU cores
make clean        # Remove build directory
```

Or manually:
```bash
cmake -S . -B build_linux \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DDISABLE_CPPRESTSDK=ON \
  -DDISABLE_EXTREME_TOOLKITPRO=ON \
  -DDISABLE_UNIRAS_TOOLMASTER=ON \
  -DUNIX_BUILD=ON
cmake --build build_linux -j$(nproc)
```

Output binary: `build_linux/runtime_output/SmartMet`

### Running

```bash
# Demo mode (no data required)
./build_linux/runtime_output/SmartMet

# With real querydata
./build_linux/runtime_output/SmartMet --data path/to/file.sqd

# With control directory (loads the querydata listed in its helpdatainfo config)
./build_linux/runtime_output/SmartMet -p control_linux
```

The window has a menu bar, a toolbar with parameter/level/time combo boxes, a dock
listing the loaded data and their parameters, and a status bar with the cursor lat/lon
and value readout. Keyboard navigation (also in the Data menu): ←/→ time step, ↑/↓
parameter, PgUp/PgDn level, N/P loaded data. These shortcuts are scoped to the map view
so they do not steal the arrow keys from the tree and the combo boxes.

Mouse: wheel zooms around the cursor, dragging with the left button pans, double click
or R (View - Reset zoom) returns to the whole grid. The visible area is a
`WeatherRenderer::GridView` rectangle in normalized grid coordinates (u west to east,
v north to south) that the map view owns and passes to every renderer call, so zooming
resamples the data rather than scaling a bitmap.

### Windows

Uses Visual Studio 2019+ with CMake. Solution generated at `smartmet-workstation_msvc/SmartMet.sln`. Requires downloading `libs.zip` from GitHub releases into `libs/3rd/`.

## Architecture

### Platform split

The codebase uses `#ifdef UNIX` / `#ifndef UNIX` throughout. On Linux, three Windows-only features are always disabled via CMake defines: `DISABLE_CPPRESTSDK`, `DISABLE_EXTREME_TOOLKITPRO`, `DISABLE_UNIRAS_TOOLMASTER`.

- **Windows**: MFC UI, GDI+ rendering, Extreme Toolkit Pro (formula editor), Uniras ToolMaster (contouring), CppRestSdk (WMS), bundled 3rd-party libs in `libs/3rd/`
- **Linux**: Qt6 UI, Cairo rendering via FMI's trax/giza/imagine2 libraries, system-installed smartmet libraries from `/usr/lib64/libsmartmet-*.so`

### Platform abstraction layer (`libs/fmi/bizcode/platform/`)

Critical directory for the Linux port. Contains:
- `linux_compat.h` — Windows type stubs and compatibility macros
- `gdiplus_stub.h` — GDI+ API stubs for Linux compilation
- `registry_value_linux.h` — File-based settings replacing Windows registry
- `qt_main_window.{h,cpp}` — `QMainWindow` with menu bar, navigation toolbar, data dock
  and status bar (the Linux counterpart of the MFC `CMainFrame`)
- `qt_map_view.{h,cpp}` — the map canvas: QImage backing store, legend overlay and the
  cursor lat/lon + value readout
- `weather_data_model.{h,cpp}` — Qt-free view state: which data/parameter/level/time is
  shown and the grid extracted for it. The window and `main()` share this one object
- `weather_renderer.{h,cpp}` — Cairo/Trax-based isoline and color grid rendering
- `help_data_loader.{h,cpp}` — loads the querydata the control directory's
  `MetEditor::HelpData` settings list (the Windows help data loading threads' stand-in)
- `cairo_qt_bridge.h` — Cairo surface to QImage conversion
- `qt_application_interface.{h,cpp}` — `ApplicationInterface` implementation the document
  calls back into (the Qt counterpart of `ApplicationInterfaceForSmartMet`)
- `qt_document_view.{h,cpp}` — hosts SmartMet's real `NFmiEditMapView` on a QPainter
- `linux_view_stubs.cpp` — placeholder definitions for view-stack symbols whose bodies
  are inside `#ifndef UNIX` blocks; each logs once when hit

### Bringing up the real view stack (`--document`)

`NFmiEditMapGeneralDataDoc::Init()` runs on Linux and the Qt shell can host it, but the
real map view cannot be created yet. `NFmiEditMapView`'s constructor immediately does
`GetCtrlViewDocumentInterface()->GetMapHandlerInterface(index)->Area()->XYArea()`, and
`CtrlViewDocumentInterfaceForGeneralDataDoc::GetMapHandlerInterface` returns `nullptr`
on Linux (`CtrlViewDocumentInterfaceForGeneralDataDoc.cpp:275`). It returns a
`GdiPlusMapHandlerInterface` wrapping `NFmiGdiPlusImageMapHandler`, and both are among
the `GdiPlus*` sources that `libs/fmi/bizcode/smetutils/CMakeLists.txt` excludes on
Linux — about 900 lines that own the map projection, background map bitmap and zooming.

Porting that map handler is the next blocking step. `--document` brings the document up
and reports this rather than crashing; the standalone renderer keeps drawing.

Note the image formats: Cairo's `ARGB32` is premultiplied and maps to
`QImage::Format_ARGB32_Premultiplied`, but `renderColorGrid()` takes plain ARGB from its
colour function and must use non-premultiplied `QImage::Format_ARGB32`. Querydata grid
row 0 is the southernmost, so the renderer flips the vertical axis to draw north up.

### Source organization

```
src/SmartMet/          Main executable (main_linux.cpp for Linux, MFC for Windows)
src/applicationdatabase/  App database handling
src/processes/         Process management
src/threads/           Threading logic
src/dialogs/           Windows-only MFC dialogs
src/toolboxdep/        Windows-only toolbox dependencies
src/mfcutils/          Windows-only MFC utilities

modules/               Core algorithmic libraries (local copies for Windows, system libs on Linux)
  newbase/             QueryData format — FMI's native gridded weather data
  smarttools/          Weather analysis SmartTool formula/macro system
  macgyver/            General utilities
  calculator/          Time series and area calculations
  textgen/             Forecast text generation
  imagine/             Visualization/imaging
  cppext/, cppback/    C++ extensions

libs/fmi/bizcode/      Business logic (25 subdirectories)
  smetdata/            Core SmartMet data layer
  smetutils/           SmartMet utility functions
  ctrlviews/           Control views
  ctrlviewutils/       View utility functions
  toolbox/             Toolbox core
  toolboxviews/        Toolbox view components
  editingdatautils/    Data editing logic
  modifyediteddata/    Edited data modification
  platform/            Linux/Windows compatibility (see above)
  winregistryutils/    Settings persistence (registry on Windows, file on Linux)

libs/fmi/              Other library collections
  datautilities/       Data handling
  webclient/           Web client
  catlog/              Logging (CatLog)
  wmssupport/          WMS support (Windows-only, requires CppRestSdk)
```

### Dependency model

On Linux, core SmartMet libraries (`macgyver`, `newbase`, `calculator`, `smarttools`, `imagine`, `textgen`) are imported as system-installed shared libraries via the `smartmet_system_library()` CMake macro in the root `CMakeLists.txt`. Additional system libraries: `trax`, `giza`, `imagine2`, `gis`. Headers are at `/usr/include/smartmet/`.

On Windows, these same libraries are built from local source in `modules/` and linked statically.

### Build dependency chain

Internal FMI libraries are built as static archives. The main `SmartMet` executable links them all. The `CMakeLists.txt` `add_subdirectory()` order defines the build sequence — later targets depend on earlier ones. Many subdirectories are `if(NOT UNIX)` guarded (Windows-only).

### Configuration at runtime

- Linux settings: `~/.config/smartmet/registry.conf` (via NFmiSettings)
- Control directory: `control_linux/` (searched automatically, or via `-p` flag)
- Factory settings: `src/SmartMet/factory_settings/`

## Key patterns

- **C++17** standard, compiled with `-fpermissive` on Linux (required for some system header quirks)
- The `.clangd` file at project root configures the LSP with all necessary include paths and defines for Linux development
- `NFmiQueryData` / `NFmiFastQueryInfo` are the central data access types (from newbase)
- `NFmiDataMatrix<float>` holds gridded weather data indexed as `[x][y]`
- `kFloatMissing` is the sentinel for missing data values
- The `WeatherRenderer` class (platform layer) uses Cairo surfaces for color grids and the Trax library for marching-squares contouring (isolines/isobands)
- Business logic libraries expose include dirs as CMake cache variables (e.g., `${SmetData_INCLUDE_DIRS}`)

## License

MIT with attribution requirements for specific files in `src/atlutils/`, `src/mfcutils/`, `libs/fmi/bizcode/gridctrl/`, `libs/fmi/bizcode/casestudylib/`, and `libs/fmi/xmlliteutils/`. See README.md for the full list.
