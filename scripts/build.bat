@echo off
setlocal
echo ============================================================
echo  Omnisoniq Studio — Building VST3 Plugin
echo ============================================================

cd /d "%~dp0.."

if not exist "build" (
    echo [ERROR] Build folder not found. Run setup.bat first.
    pause & exit /b 1
)

echo [INFO] Building Release...
cmake --build build --config Release --target Omnisoniq_VST3 -j 4

if errorlevel 1 (
    echo [ERROR] Build failed. Check errors above.
    pause & exit /b 1
)

:: ── Find the built VST3 ──
set VST3_PATH=build\Omnisoniq_artefacts\Release\VST3\Omnisoniq Studio.vst3

if exist "%VST3_PATH%" (
    echo.
    echo ============================================================
    echo  BUILD SUCCESSFUL!
    echo ============================================================
    echo.
    echo  VST3 plugin location:
    echo  %CD%\%VST3_PATH%
    echo.
    echo  Installing to VST3 folder...

    set INSTALL_DIR=%COMMONPROGRAMFILES%\VST3
    if not exist "%INSTALL_DIR%" mkdir "%INSTALL_DIR%"

    xcopy /E /I /Y "%VST3_PATH%" "%INSTALL_DIR%\Omnisoniq Studio.vst3\"
    echo  Installed to: %INSTALL_DIR%\Omnisoniq Studio.vst3

    echo.
    echo  Also copying python\ folder next to the VST3...
    xcopy /E /I /Y "python" "%INSTALL_DIR%\Omnisoniq Studio.vst3\python\"
    echo.
    echo  Restart your DAW and scan for new plugins.
    echo  Python path setting can be configured in the plugin settings.
    echo ============================================================
) else (
    echo [ERROR] VST3 not found at expected path. Check build output.
)

pause
