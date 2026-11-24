# MITAB

MITAB is an open source C++ library for reading and writing MapInfo TAB (binary) and MIF/MID files. This tree is a continuation of the MapGears MITAB library. It keeps the Visual Studio 2013 / Windows XP toolset for the Win32 DLL and carries selected MapInfo fixes ported from GDAL.

The library version string is `2.0.0-dev (2008-10)` (`MITAB_VERSION_INT` `2000000` in `mitab/mitab.h`).

## Formats

- **TAB**: binary MapInfo table (`.tab`, `.map`, `.dat`, `.id`, and `.ind` when an index is present).
- **MIF/MID**: MapInfo Interchange Format. Geometry is in the `.mif` file and attributes are in the `.mid` file.

## Build

The supported Visual Studio build is `MITAB.sln` with toolset `v120_xp` (Visual Studio 2013, Windows SDK 7.1).

```
msbuild MITAB.sln /p:Configuration=Release /p:Platform=Win32
```

Use MSBuild 12.0 so the `v120_xp` toolset is the one that actually compiles the tree. The solution defines `Debug` and `Release` for `Win32` and `x64`. The configuration that this tree treats as the compatibility build is `Release|Win32`.

Outputs:

| Project | Output |
|---|---|
| `CPL.vcxproj` | static `cpl.lib` |
| `OGR.vcxproj` | static `ogr.lib` |
| `MITAB.vcxproj` | `mitab.dll` |
| `tab2tab.vcxproj`, `tabdump.vcxproj`, `ogrinfo.vcxproj`, `ogr2ogr.vcxproj` | console programs in `bin\<Platform>\<Configuration>\` |

`CMakeLists.txt` can configure the same three libraries with `-T v120_xp`. It does not build the console utilities. The runtime is the static CRT (`/MT` in Release, `/MTd` in Debug).

`Release|Win32` links with subsystem 5.01. That marks the image as compatible with the Windows XP subsystem. Running the binaries on Windows XP itself is a separate check and is not implied by a successful build on a newer Windows host.

`Release|x64` uses the same `v120_xp` toolset. The linker warning `LNK4010` means subsystem 5.01 is not valid for a 64-bit image, and the linker substitutes subsystem 6.00. An x64 build on this tree is not a Windows XP image.

## DLL and C API

`mitab/mitab_capi.h` is the public C API: 93 `mitab_c_*` functions. On Win32 they are `__stdcall`, and the export names are decorated as `_mitab_c_<name>@<bytes>`. On x64 the same declarations are used; MSVC does not decorate `__stdcall` names, so the exports are plain `mitab_c_<name>`.

Do not change those signatures, the decorated stack sizes, or the set of `mitab_c_*` exports. External programs bind to this C API. `CPL_DLL` in `cpl/cpl_port.h` defaults to `dllexport`, so `mitab.dll` also exports CPL and OGR symbols. Those extra exports are part of the current binary and are not an invitation to treat the C++ classes as a stable ABI.

Language bindings in `contrib/` (`mitab.pas`, `mitab_cdecl.pas`, `mitab_dyn.pas`, `mitab.vb`, `mitab.def`, `contrib/dotnet/MiApi.cs`) follow the C API. Coverage is not identical in every file: `contrib/mitab_dyn.pas` loads only part of the function set.

## Console utilities

`tab2tab`, `ogrinfo`, `ogr2ogr`, and `tabdump` are in `MITAB.sln`. They call MITAB C++ classes. `mitab.dll` exports the stdcall C API, so the utilities do not link its import library. `MITAB.Utility.props` adds the object files already produced for the DLL, and CPL and OGR stay static libraries. Build the `MITAB` project before the utilities.

```
tab2tab <src> <dst>
ogrinfo <datasource> [layer]
ogr2ogr -f "MapInfo File" <dst> <src>
```

`tabdump` is compiled without `DEBUG`. Its `main` then prints that tabdump is available only when MITAB is compiled with `-DDEBUG`, and returns -1. `DEBUG` is not defined for the DLL: under that macro the headers declare extra virtual `Dump` methods, which changes the C++ vtable layout of the DLL.

## GDAL fixes

Fixes ported from the GDAL MapInfo driver are tracked in `docs/gdal-migration/PROGRESS.md`. The comparison with GDAL and the per-item checks are in `docs/gdal-migration/ANALYSIS.md`. A fix is `VERIFIED` only after a `Release|Win32` `v120_xp` build and the behavior check named in the analysis. Items still marked `TODO`, `BLOCKED`, or `SKIPPED` are not done.

## License

The MIT-style license is in `LICENSE` and `README.TXT`. `NOTICE` has the separate terms for `ogr/swq.c`, `ogr/swq.h`, and `cpl/cpl_recode_stub.cpp`, and lists the GDAL MapInfo commits ported into this tree. Each source file keeps its own copyright header. `contrib/dotnet/MiApi.cs`, `contrib/dotnet/MiWrapper.cs`, and `ogr/ogr_srs_esri_names.h` do not contain the `LICENSE` permission notice.
