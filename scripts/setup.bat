@echo off
setlocal
echo ============================================================
echo  Omnisoniq Studio — First-Time Setup
echo ============================================================
echo.

:: ── 1. Check for Git ──
git --version >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Git is not installed or not in PATH.
    echo         Download from: https://git-scm.com/download/win
    pause & exit /b 1
)

:: ── 2. Check for CMake ──
cmake --version >nul 2>&1
if errorlevel 1 (
    echo [ERROR] CMake is not installed or not in PATH.
    echo         Download from: https://cmake.org/download/
    echo         During install, choose "Add CMake to PATH".
    pause & exit /b 1
)

:: ── 3. Check for Visual Studio / cl.exe ──
where cl.exe >nul 2>&1
if errorlevel 1 (
    echo [WARNING] cl.exe (MSVC) not found in PATH.
    echo           Make sure Visual Studio 2022 with "Desktop development with C++"
    echo           workload is installed, then run this script from:
    echo           x64 Native Tools Command Prompt for VS 2022
    echo.
)

:: ── 4. Check for Python ──
python --version >nul 2>&1
if errorlevel 1 (
    echo [WARNING] Python not found. Stem separation will not work.
    echo           Install Python 3.10+ from https://python.org
    echo.
) else (
    echo [INFO] Installing Python dependencies for stem separation...
    pip install -r "%~dp0..\python\requirements.txt"
    if errorlevel 1 (
        echo [WARNING] Some Python packages failed to install.
        echo           GPU-accelerated separation requires a CUDA-compatible GPU.
        echo           CPU-only will still work (slower).
    ) else (
        echo [OK] Python dependencies installed.
    )
)

:: ── 5. Configure CMake ──
cd /d "%~dp0.."
echo.
echo [INFO] Configuring CMake project (this will download JUCE ~500MB)...
cmake -B build -G "Visual Studio 17 2022" -A x64
if errorlevel 1 (
    echo [ERROR] CMake configuration failed.
    pause & exit /b 1
)

echo.
echo ============================================================
echo  Setup complete! Now run scripts\build.bat to compile.
echo ============================================================
pause
