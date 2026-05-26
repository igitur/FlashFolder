# FlashFolder

FlashFolder hooks the file open/save dialogs of Windows applications and adds a
toolbar with frequently used folders, favorites, history and plugins.

This branch contains the Visual Studio 2022 / MSVC v143 conversion of the
original Visual Studio 2005 solution. The build is **x64-only** (Win32 was
dropped during the conversion).

## Requirements

- Visual Studio 2022 with the "Desktop development with C++" workload
  - MSVC v143 toolset
  - Windows 10/11 SDK (projects target 10.0.26100.0)
  - MFC (used by `CommonLib`, `CommonUtilsLib`, `FFConfig`, `FFService` and
    `TestGui`)
- WiX Toolset 3.x for building the MSI installer

## Building

Use an MSBuild from a Visual Studio developer environment (`dotnet build` does
not support C++ projects):

```bat
"%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat" -arch=x64
msbuild FlashFolder.sln /p:Configuration=Release /p:Platform=x64
```

Or build everything, including the MSI installer, through the umbrella
script:

```bat
msbuild ALL.msbuild /p:Configuration=Release
```

`Configuration` can be `Debug` or `Release`; the platform is always x64.

## Outputs

Binaries are written to `exe\`:

| File | Project |
|------|---------|
| `FlashFolder64.exe` | `FFService` |
| `FFConfig64.exe` | `FFConfig` |
| `fflib6439_64.dll` | `FFLib` |
| `plugins64\Default\DefFileMgr.ffplugin` | `FileMgrPlugin` |
| `CustomActions_x64.dll` | `Installer\CustomActions` |
| `TestGui64.exe` | `z_TestGui` |

Static libraries are written to `common\lib`, `commonUtils\lib`,
`Installer\wcautil\lib`; the installer is written to `Installer\msi\en-US`.

## Project layout

| Project | Type |
|---------|------|
| `common\CommonLib.vcxproj` | Static library |
| `commonUtils\CommonUtilsLib.vcxproj` | Static library |
| `FFLib\FFLib.vcxproj` | DLL |
| `FFConfig\FFConfig.vcxproj` | Application |
| `FFService\FFService.vcxproj` | Application |
| `FileMgrPlugin\FileMgrPlugin.vcxproj` | DLL |
| `z_TestGui\TestGui.vcxproj` | Application (MFC, static) |
| `Installer\CustomActions\CustomActions.vcxproj` | DLL (WiX custom actions) |
| `Installer\wcautil\wcautil.vcxproj` | Static library |
| `Installer\MkVersionFile\MkVersionFile.vcxproj` | Console tool |
| `Installer\Installer.vcxproj` | Makefile project that builds the MSI |

## Notes

- `_version.h` holds the product version. The installer build uses
  `Installer\MkVersionFile` to expand `Installer\_version.wxi.templ` into
  `Installer\_version.wxi` (generated, not tracked).
- The WiX path defaults to a scoop installation of WiX 3.x if present,
  otherwise to the standard WiX 3.x MSBuild extensions directory. Override it
  with `WixToolPath`/`WixTargetsPath` if needed.
- The documentation files `CONVERSION_CHECKLIST.md`,
  `CONVERSION_TECHNICAL_REFERENCE.md`, `DELIVERABLES_SUMMARY.md` and
  `VCPROJ_CONVERSION_REPORT.md` describe the conversion process.
