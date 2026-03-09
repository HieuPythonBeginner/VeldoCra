# File Association Guide for VeldoCra (.vel)

This document describes how to set up file icons and associations for `.vel` files across different platforms and editors.

## Overview

VeldoCra uses the `.vel` file extension for its source code files. This guide covers:

1. **VSCode** - Icon theme for .vel files
2. **Windows** - File association and icon in Explorer
3. **Linux** - File association and icon in file managers (Nautilus, Dolphin, etc.)

---

## 1. VSCode Icon Theme Setup

### Files Created

- `icons/veldora-icons.json` - VSCode icon theme definition
- `package.json` - Updated with icon theme contribution

### Manual VSCode Configuration

If using as a VSCode extension:

1. Copy `icons/veldora-icons.json` to your VSCode extension's icons folder
2. Update `package.json` to point to the correct path
3. Package and install the extension

### Testing in VSCode

Add this to your VSCode settings (`settings.json`):

```json
{
  "workbench.iconTheme": "veldora-dragon-icons"
}
```

---

## 2. Windows File Association

### Files to Create

Create the following files in `install-icons/windows/` folder:

1. **Windows Registry File** (`.reg`) - Associates .vel extension with VeldoCra
2. **Icon File** (`.ico`) - Custom icon for .vel files
3. **Installer Script** (`.bat`) - Automated installation

### Registry Entry Template

```reg
Windows Registry Editor Version 5.00

; VeldoCra .vel File Association
[HKEY_CLASSES_ROOT\.vel]
@="VeldoCraSourceFile"
"Content Type"="text/x-vel"

[HKEY_CLASSES_ROOT\VeldoCraSourceFile]
@="VeldoCra Source File"

[HKEY_CLASSES_ROOT\VeldoCraSourceFile\DefaultIcon]
@="\"%LOCALAPPDATA%\\VeldoCra\\icons\\vel.ico\",0"

[HKEY_CLASSES_ROOT\VeldoCraSourceFile\shell]
@="open"

[HKEY_CLASSES_ROOT\VeldoCraSourceFile\shell\open]
@="Open with VeldoCra"

[HKEY_CLASSES_ROOT\VeldoCraSourceFile\shell\open\command]
@="\"%LOCALAPPDATA%\\VeldoCra\\velc.exe\" \"%1\""
```

### Windows Installer Script

```batch
@echo off
setlocal

echo ============================================
echo   VeldoCra File Association Installer
echo   For Windows
echo ============================================
echo.

:: Check for admin privileges
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo This script requires Administrator privileges.
    echo Please right-click and select "Run as administrator"
    pause
    exit /b 1
)

:: Create VeldoCra app data directory
if not exist "%LOCALAPPDATA%\VeldoCra" mkdir "%LOCALAPPDATA%\VeldoCra"
if not exist "%LOCALAPPDATA%\VeldoCra\icons" mkdir "%LOCALAPPDATA%\VeldoCra\icons"

:: Copy icon file
echo Installing VeldoCra icon...
copy /Y "icons\vel.ico" "%LOCALAPPDATA%\VeldoCra\icons\" >nul

:: Copy compiler
echo Installing VeldoCra compiler...
copy /Y "build\bin\velc.exe" "%LOCALAPPDATA%\VeldoCra\" >nul

:: Register file association
echo Registering .vel file association...
reg import "install-icons\windows\veldocra.reg" >nul

echo.
echo ============================================
echo   Installation Complete!
echo ============================================
echo.
echo Please restart Explorer or log out/in to see changes.
pause
```

---

## 3. Linux File Association

### Files to Create

Create the following files in `install-icons/linux/` folder:

1. **MIME Type XML** - Defines .vel as a custom file type
2. **.desktop File** - Desktop entry for VeldoCra
3. **Installer Script** (Shell) - Automated installation

### MIME Type XML

Create `install-icons/linux/veldocra.xml`:

```xml
<?xml version="1.0" encoding="utf-8"?>
<mime-info xmlns="http://www.freedesktop.org/standards/shared-mime-info">
  <mime-type type="text/x-vel">
    <comment>VeldoCra Source File</comment>
    <glob pattern="*.vel"/>
    <sub-class-of type="text/plain"/>
  </mime-type>
</mime-info>
```

### Desktop Entry File

Create `install-icons/linux/veldora.desktop`:

```desktop
[Desktop Entry]
Version=1.0
Type=Application
Name=VeldoCra
Comment=VeldoCra Programming Language Compiler
Exec=/usr/local/bin/velc %f
Icon=veldora
Terminal=true
Categories=Development;Interpreter;
MimeType=text/x-vel;
Keywords=vel;compiler;programming;
```

### Linux Installer Script

```bash
#!/bin/bash

echo "============================================"
echo "  VeldoCra File Association Installer"
echo "  For Linux"
echo "============================================"
echo

# Check for root privileges
if [ "$EUID" -ne 0 ]; then
    echo "This script requires root privileges."
    echo "Please run with sudo: sudo $0"
    exit 1
fi

# Install MIME type
echo "Installing MIME type..."
install -Dm644 install-icons/linux/veldocra.xml \
    /usr/share/mime/packages/veldocra.xml

# Update MIME database
echo "Updating MIME database..."
update-mime-database /usr/share/mime

# Install icon
echo "Installing icon..."
install -Dm644 icons/vel.png /usr/share/icons/hicolor/256x256/apps/veldora.png

# Update icon cache
echo "Updating icon cache..."
gtk-update-icon-cache -f -t /usr/share/icons/hicolor

# Install desktop entry
echo "Installing desktop entry..."
install -Dm644 install-icons/linux/veld    /usr/share/applications/veldora.desktop

# Install compiler
echo "Installing Velora.desktop \
doCra compiler..."
install -Dm755 build/bin/velc /usr/local/bin/velc

# Update desktop database
echo "Updating desktop database..."
update-desktop-database /usr/share/applications

echo
echo "============================================"
echo "  Installation Complete!"
echo "============================================"
echo
echo "Please restart your file manager to see changes."
echo "For GNOME: nautilus -q && nautilus &"
echo "For KDE: kbuildsycoca6"
echo "For XFCE: thunar -q && thunar &"
```

---

## 4. Cross-Platform Auto-Installer

### Main Installer Script

Create `install-icons/install.sh`:

```bash
#!/bin/bash

# VeldoCra Cross-Platform Installer
# Detects OS and installs appropriate file associations

set -e

echo "============================================"
echo "  VeldoCra File Association Installer"
echo "============================================"
echo

# Detect OS
detect_os() {
    case "$OSTYPE" in
        msys*|cygwin*|win32*) echo "windows" ;;
        linux*) echo "linux" ;;
        darwin*) echo "macos" ;;
        *) echo "unknown" ;;
    esac
}

OS=$(detect_os)

echo "Detected OS: $OS"
echo

case "$OS" in
    windows)
        echo "Running Windows installer..."
        cmd.exe /c "install-icons\windows\install.bat"
        ;;
    linux)
        echo "Running Linux installer..."
        bash install-icons/linux/install.sh
        ;;
    macos)
        echo "macOS support coming soon..."
        ;;
    *)
        echo "Unsupported OS: $OSTYPE"
        exit 1
        ;;
esac

echo
echo "Installation complete!"
```

---

## 5. Icon File Requirements

### Required Icon Formats

| Platform | Format | Location |
|----------|--------|----------|
| VSCode | SVG, PNG | `icons/` folder |
| Windows | ICO (16x16, 32x32, 48x48, 256x256) | `install-icons/windows/` |
| Linux | PNG (multiple sizes) | `icons/` folder |

### Icon Sizes for Linux

- 16x16: `16x16/apps/veldora.png`
- 22x22: `22x22/apps/veldora.png`
- 24x24: `24x24/apps/veldora.png`
- 32x32: `32x32/apps/veldora.png`
- 48x48: `48x48/apps/veldora.png`
- 64x64: `64x64/apps/veldora.png`
- 128x128: `128x128/apps/veldora.png`
- 256x256: `256x256/apps/veldora.png`

---

## 6. Usage

### Build and Install

```bash
# Build the compiler
cd build && make

# Install file associations (Linux)
sudo ./install-icons/linux/install.sh

# Install file associations (Windows - Run as Administrator)
install-icons\windows\install.bat
```

### Verify Installation

#### Linux
```bash
# Check MIME type
xdg-mime query filetype example.vel

# Should output: text/x-vel
```

#### Windows
```bash
# Right-click a .vel file
# Should show VeldoCra icon and "Open with VeldoCra"
```

---

## 7. Troubleshooting

### VSCode Icons Not Showing

1. Open VSCode Settings
2. Search for "icon theme"
3. Select "Veldora Dragon Icons"
4. Restart VSCode

### Windows Icons Not Updating

1. Open Task Manager
2. End process "Explorer.exe"
3. File → Run new Explorer window

### Linux Icons Not Updating

1. Clear icon cache: `rm -rf ~/.cache/icon*`
2. Restart file manager
3. Log out and log back in

---

## License

This file association setup is part of VeldoCra - The official programming language for JuraTempestFederationOS.

