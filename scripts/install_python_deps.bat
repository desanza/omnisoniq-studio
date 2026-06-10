@echo off
echo Installing Demucs and dependencies...
echo This may take a while (PyTorch is ~2GB).
echo.
pip install demucs
if errorlevel 1 (
    echo.
    echo [ERROR] Installation failed.
    echo Try running as Administrator, or check your Python/pip installation.
) else (
    echo.
    echo [OK] Demucs installed successfully!
    echo You can now use One-Click Separate Stems in Omnisoniq Studio.
)
pause
