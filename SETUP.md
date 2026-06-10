# Omnisoniq Studio — VST3 Build Guide

## What You Need (One-Time Install)

### 1. Visual Studio 2022 Community (Free)
- Download: https://visualstudio.microsoft.com/vs/community/
- During install, select: **"Desktop development with C++"**

### 2. CMake (Free)
- Download: https://cmake.org/download/ → Windows x64 Installer
- During install: tick **"Add CMake to the system PATH"**

### 3. Git (Free)
- Download: https://git-scm.com/download/win
- Use all defaults during install

### 4. Python 3.10+ (For stem separation)
- Download: https://python.org → Windows installer
- During install: tick **"Add Python to PATH"**

---

## Build Steps

### Step 1 — Open the right terminal
Open **"x64 Native Tools Command Prompt for VS 2022"**  
(search it in the Start menu — this sets up MSVC paths)

### Step 2 — Run setup
```
cd C:\Users\desan\Omnisoniq
scripts\setup.bat
```
This will:
- Download JUCE (~500 MB, one time only)  
- Install Python deps (Demucs, PyTorch — ~2 GB)
- Configure the CMake project

### Step 3 — Build
```
scripts\build.bat
```
This compiles the VST3 and installs it to `C:\Program Files\Common Files\VST3\`.

---

## Loading in Your DAW

After build:
- **FL Studio**: Options → Manage plugins → Scan → "Fast scan"
- **Ableton**: Options → Preferences → Plug-Ins → VST3 folder: `C:\Program Files\Common Files\VST3`
- **Reaper**: Options → Preferences → Plug-ins → VST → Add path
- **Cubase / Studio One / Pro Tools**: Scan automatically on launch

---

## Features

| Feature | Status |
|---------|--------|
| Local file browser (drag & drop) | ✅ Full |
| BPM detection (autocorrelation) | ✅ Full |
| Key detection (Krumhansl-Schmuckler) | ✅ Full |
| Waveform display with playhead | ✅ Full |
| Stem playback (Solo/Mute/Volume) | ✅ Full |
| Transport (Play/Pause/Stop/Loop) | ✅ Full |
| Host BPM sync | ✅ Full |
| Stem separation (Demucs AI) | ✅ Full (needs Python) |
| Export stems as WAV | ✅ Full |
| Dark UI matching design | ✅ Full |

---

## Stem Separation Notes

Stem separation requires Python + Demucs. First-time separation will download the Demucs model (~380 MB). After that it's fast.

- **With GPU (NVIDIA)**: ~10-30 seconds per song
- **CPU only**: ~2-5 minutes per song

To install/reinstall Demucs:
```
scripts\install_python_deps.bat
```

---

## Troubleshooting

**"python not found"** — Make sure Python is in PATH. The plugin defaults to `python`. You can override the path in the plugin settings panel.

**VST3 not showing in DAW** — Make sure the build completed (check `build.bat` output). Rescan plugins in your DAW.

**CMake error about generator** — Make sure you're running from the "x64 Native Tools Command Prompt for VS 2022", not a regular terminal.
