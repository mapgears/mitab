# Contributing to MITAB

This tree keeps a Win32 DLL that existing callers can load, and it takes MapInfo bug fixes from GDAL one at a time. Changes should stay inside that scope.

## Build before sending a change

The required build is `Release|Win32`, toolset `v120_xp`, MSBuild 12.0:

```
msbuild MITAB.sln /t:Rebuild /p:Configuration=Release /p:Platform=Win32
```

`Debug|Win32`, `Release|x64`, and `Debug|x64` are present in `MITAB.sln`. Build an x64 configuration when the change can depend on pointer size. Do not describe a configuration as tested unless that build actually ran. A Win32 image with subsystem 5.01 is not, by itself, a test on Windows XP. An x64 link that reports `LNK4010` and writes subsystem 6.00 is not a Windows XP image.

Use C++11. Keep `GInt32` for 32-bit quantities so the same change builds for Win32 and x64. Use the integer types already used in `mitab/`.

## What not to change without a separate decision

- Public signatures and the set of exports in `mitab/mitab_capi.h`. Win32 stack sizes must keep matching the `_name@N` exports.
- `cpl/` and `ogr/`. Items B1–B6 in `docs/gdal-migration/PROGRESS.md` stay `BLOCKED` or `SKIPPED` until that decision is made.
- A global `DEBUG` preprocessor definition. It adds virtual `Dump` methods and changes the C++ ABI of `mitab.dll`. That is why `tabdump` in the Visual Studio build only prints its DEBUG limitation.
- The console utilities' link line: MITAB object files plus static `cpl.lib` and `ogr.lib`. Do not switch them to the DLL import library as a drive-by cleanup.

## Porting a fix from GDAL

Read `docs/gdal-migration/ANALYSIS.md` and `docs/gdal-migration/PROGRESS.md` before editing. Do not repeat the full GDAL history walk when the item is already described there. Inspect only the GDAL commit named for that item, and edit `mitab/` in this tree. The GDAL checkout is read-only.

After the edit, update that item's row: status, MITAB commit (leave the commit cell empty until a commit exists), and what the build and the check in `ANALYSIS.md` actually showed. `VERIFIED` requires `Release|Win32` `v120_xp` and the named behavior check. A compile by itself is `IMPLEMENTED` at most. Item 23 stays as written until there is a separate decision; do not copy that GDAL change in as a mechanical port. Item 22 waits on `TABFile::SetFeature`.

## Console utilities

`tab2tab` converts TAB and MIF. The destination extension selects the output. `ogrinfo` opens a dataset with the `MapInfo File` driver and, when a layer name is given, prints the layer and its features. `ogr2ogr` converts with `-f "MapInfo File"`. `tabdump` needs a MITAB build with `DEBUG`, which this solution does not enable.

A small MIF/MID pair in a temporary directory is enough to see that the tools start and that a conversion round-trips. Do not commit those files.

## Tests and compatibility

There is no GDAL autotest suite in this tree. For a behavior change, record the input, the command or the API call, and the result in `docs/gdal-migration/PROGRESS.md`. Compare `mitab.dll` exports with `mitab/mitab_capi.h` when a change could affect the C API: 93 `mitab_c_*` functions, decorated on Win32 and undecorated on x64.

Bindings under `contrib/` are updated to follow the header. They are not a second public API, and a binding-only gap is not a reason to add or remove a DLL export.

## Git

Do not commit `bin/`, `obj/`, `*.dll`, `*.lib`, `*.obj`, `*.exe`, `*.pdb`, or `*.ilk`. Do not commit `mitab.code-workspace` when it contains absolute paths to other checkouts. Do not commit an unrelated product's sources into this repository.

Match the existing commit style: one short imperative sentence that says why the change exists. Keep build, library fixes, utilities, and documentation in separate commits when they land together.
