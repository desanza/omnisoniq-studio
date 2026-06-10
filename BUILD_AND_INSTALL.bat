@echo off
setlocal EnableDelayedExpansion
title Omnisoniq Studio — Build and Install
color 0A

echo.
echo  ██████╗ ███╗   ███╗███╗   ██╗██╗███████╗ ██████╗ ███╗   ██╗██╗ ██████╗
echo  ██╔═══██╗████╗ ████║████╗  ██║██║██╔════╝██╔═══██╗████╗  ██║██║██╔═══██╗
echo  ██║   ██║██╔████╔██║██╔██╗ ██║██║███████╗██║   ██║██╔██╗ ██║██║██║   ██║
echo  ██║   ██║██║╚██╔╝██║██║╚██╗██║██║╚════██║██║   ██║██║╚██╗██║██║██║▄▄ ██║
echo  ╚██████╔╝██║ ╚═╝ ██║██║ ╚████║██║███████║╚██████╔╝██║ ╚████║██║╚██████╔╝
echo   ╚═════╝ ╚═╝     ╚═╝╚═╝  ╚═══╝╚═╝╚══════╝ ╚═════╝ ╚═╝  ╚═══╝╚═╝ ╚══▀▀═╝
echo.
echo                    S T U D I O   —   VST3 Builder
echo.
echo ================================================================
echo   This script will:
echo   1. Install CMake (C++ build tool)         ~5 min
echo   2. Install Visual Studio Build Tools      ~20-30 min
echo   3. Download JUCE framework                ~10 min
echo   4. Compile Omnisoniq Studio VST3          ~5 min
echo   5. Install to FL Studio / all DAWs        instant
echo ================================================================
echo.
echo   Total time: ~40 minutes (mostly downloading)
echo   Requires: ~6 GB free disk space, internet connection
echo.
pause

:: ── Check winget ──────────────────────────────────────────────────────────
winget --version >nul 2>&1
if errorlevel 1 (
    echo.
    echo [ERROR] winget not found. Please update Windows to version 1809 or later.
    echo         Or install manually from: https://aka.ms/getwinget
    pause & exit /b 1
)

:: ── Install CMake ─────────────────────────────────────────────────────────
echo.
echo [STEP 1/5] Installing CMake...
cmake --version >nul 2>&1
if not errorlevel 1 (
    echo   CMake already installed. Skipping.
) else (
    winget install Kitware.CMake --accept-source-agreements --accept-package-agreements
    if errorlevel 1 (
        echo [ERROR] Failed to install CMake.
        echo         Try manually: https://cmake.org/download/
        pause & exit /b 1
    )
    :: Refresh PATH
    for /f "tokens=*" %%i in ('reg query "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v Path 2^>nul') do set SYS_PATH=%%i
    set "PATH=%PATH%;C:\Program Files\CMake\bin"
    echo   CMake installed.
)

:: ── Install VS Build Tools ────────────────────────────────────────────────
echo.
echo [STEP 2/5] Installing Visual Studio Build Tools (C++ compiler)...
echo   This is the big download (~3 GB). Please wait...
echo.

if exist "C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC" (
    echo   Visual Studio Build Tools already installed. Skipping.
) else (
    winget install Microsoft.VisualStudio.2022.BuildTools --accept-source-agreements --accept-package-agreements --override "--add Microsoft.VisualStudio.Workload.VCTools --includeRecommended --quiet --wait"
    if errorlevel 1 (
        echo [ERROR] VS Build Tools install failed or requires a restart.
        echo         If prompted, restart your PC, then re-run this script.
        pause & exit /b 1
    )
    echo   Visual Studio Build Tools installed.
)

:: ── Find vcvars64.bat ─────────────────────────────────────────────────────
set VCVARS=
for %%p in (
    "C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
    "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
    "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
    "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
) do (
    if exist %%p set VCVARS=%%p
)

if "%VCVARS%"=="" (
    echo [ERROR] Could not find vcvars64.bat. Visual Studio may need a restart to complete.
    echo         Please restart your computer and run this script again.
    pause & exit /b 1
)

echo.
echo [STEP 3/5] Setting up compiler environment...
call "%VCVARS%"
if errorlevel 1 ( echo [ERROR] Failed to init compiler. & pause & exit /b 1 )
echo   Compiler ready.

:: ── Set paths ─────────────────────────────────────────────────────────────
set PATH=%PATH%;C:\Program Files\CMake\bin

:: Confirm cmake works now
cmake --version >nul 2>&1
if errorlevel 1 (
    echo [ERROR] CMake not found in PATH after install.
    echo         Please restart this script from a new command prompt.
    pause & exit /b 1
)

:: ── Configure and Build ───────────────────────────────────────────────────
echo.
echo [STEP 4/5] Downloading JUCE and building plugin...
echo   (JUCE download: ~500 MB, then compile: ~5 min)
echo.

cd /d "%~dp0"

cmake -B build -G "Visual Studio 17 2022" -A x64
if errorlevel 1 ( echo [ERROR] CMake configure failed. & pause & exit /b 1 )

cmake --build build --config Release --target Omnisoniq_VST3 -j 4
if errorlevel 1 ( echo [ERROR] Build failed. & pause & exit /b 1 )

echo   Build complete!

:: ── Find VST3 ─────────────────────────────────────────────────────────────
set VST3_SRC=
for /r "build" %%f in (*.vst3) do (
    if "%%~xf"==".vst3" set VST3_SRC=%%f
)

:: Also try the artefacts folder (JUCE output)
if exist "build\Omnisoniq_artefacts\Release\VST3\Omnisoniq Studio.vst3" (
    set VST3_SRC=build\Omnisoniq_artefacts\Release\VST3\Omnisoniq Studio.vst3
)

if "%VST3_SRC%"=="" (
    echo [ERROR] VST3 output not found. Check build output above.
    pause & exit /b 1
)

:: ── Install VST3 ───────────────────────────────────────────────────────────
echo.
echo [STEP 5/5] Installing VST3 plugin...

set VST3_DEST=%COMMONPROGRAMFILES%\VST3\Omnisoniq Studio.vst3

if exist "%VST3_DEST%" rmdir /s /q "%VST3_DEST%"
xcopy /E /I /Y "%VST3_SRC%" "%VST3_DEST%\"
xcopy /E /I /Y "python" "%VST3_DEST%\python\" >nul 2>&1

echo   Installed to: %VST3_DEST%

:: ── Package for sharing ────────────────────────────────────────────────────
echo.
echo Packaging distributable zip for friends...

set DIST=OmnisoniqStudio_v1.0_Windows_x64
if exist "%DIST%" rmdir /s /q "%DIST%"
mkdir "%DIST%"
xcopy /E /I /Y "%VST3_SRC%" "%DIST%\Omnisoniq Studio.vst3\" >nul
xcopy /E /I /Y "python" "%DIST%\python\" >nul 2>&1
copy "scripts\install_vst.bat" "%DIST%\" >nul
echo To install: extract this zip then RIGHT-CLICK install_vst.bat and choose "Run as administrator" > "%DIST%\HOW_TO_INSTALL.txt"
echo. >> "%DIST%\HOW_TO_INSTALL.txt"
echo After installing, open FL Studio and go to: Options > Manage Plugins > Start Scan >> "%DIST%\HOW_TO_INSTALL.txt"
echo. >> "%DIST%\HOW_TO_INSTALL.txt"
echo Stem separation requires Python + Demucs: pip install demucs >> "%DIST%\HOW_TO_INSTALL.txt"

powershell -Command "Compress-Archive -Path '%DIST%\*' -DestinationPath '%DIST%.zip' -Force"
if exist "%DIST%.zip" (
    echo   Zip created: %CD%\%DIST%.zip
    echo   Share this zip with friends!
)

echo.
echo ================================================================
echo   ALL DONE!
echo ================================================================
echo.
echo   Plugin installed. Now in FL Studio:
echo   1. Go to Options (top menu)
echo   2. Click "Manage plugins"
echo   3. Click "Start scan" (or "Fast scan")
echo   4. Search for "Omnisoniq" — it should appear!
echo.
echo   Shareable zip: %CD%\%DIST%.zip
echo.
pause
