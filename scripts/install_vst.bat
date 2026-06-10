@echo off
setlocal
echo ============================================================
echo  Omnisoniq Studio — VST3 Installer
echo ============================================================
echo.

:: Check for admin rights
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo [!] This installer needs Administrator rights.
    echo     Right-click install_vst.bat and choose "Run as administrator"
    pause
    exit /b 1
)

set VST3_SRC=%~dp0Omnisoniq Studio.vst3
set VST3_DEST=%COMMONPROGRAMFILES%\VST3\Omnisoniq Studio.vst3

if not exist "%VST3_SRC%" (
    echo [ERROR] "Omnisoniq Studio.vst3" not found next to this script.
    echo Make sure you extracted the full zip and run install_vst.bat from inside it.
    pause & exit /b 1
)

echo [1/3] Copying VST3 plugin...
if exist "%VST3_DEST%" rmdir /s /q "%VST3_DEST%"
xcopy /E /I /Y "%VST3_SRC%" "%VST3_DEST%\"
if errorlevel 1 ( echo [ERROR] Copy failed. & pause & exit /b 1 )
echo      Done: %VST3_DEST%

echo [2/3] Copying Python stem-separation script...
if exist "%~dp0python" (
    xcopy /E /I /Y "%~dp0python" "%VST3_DEST%\python\"
    echo      Done: %VST3_DEST%\python\
) else (
    echo      [SKIP] python folder not found (stem separation will require manual setup)
)

echo [3/3] Registering plugin path...
reg add "HKLM\SOFTWARE\VST3" /v "OmnisoniqStudio" /t REG_SZ /d "%VST3_DEST%" /f >nul 2>&1

echo.
echo ============================================================
echo  INSTALLATION COMPLETE!
echo ============================================================
echo.
echo  Plugin installed to:
echo  %VST3_DEST%
echo.
echo  Next steps:
echo  1. Open FL Studio (or your DAW)
echo  2. Go to Options > Manage Plugins > Start Scan
echo     (or press F10 > Plugins > Start Scan in FL Studio)
echo  3. Omnisoniq Studio will appear in your plugin list
echo.
echo  For AI stem separation, also run:
echo  pip install demucs
echo  (requires Python to be installed)
echo.
pause
