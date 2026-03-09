#!/bin/bash
# =============================================
#   VeldoCra File Association Installer
#   For Linux
# =============================================

echo "============================================"
echo "  VeldoCra File Association Installer"
echo "  For Linux"
echo "============================================"
echo

# Check for root privileges
if [ "$EUID" -ne 0 ]; then
    echo "[ERROR] This script requires root privileges."
    echo "Please run with sudo: sudo $0"
    exit 1
fi

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

echo "[1/5] Installing MIME type..."
# Create MIME type XML
cat > /tmp/veldocra.xml << 'EOF'
<?xml version="1.0" encoding="utf-8"?>
<mime-info xmlns="http://www.freedesktop.org/standards/shared-mime-info">
  <mime-type type="text/x-vel">
    <comment>VeldoCra Source File</comment>
    <glob pattern="*.vel"/>
    <sub-class-of type="text/plain"/>
    <magic>
      <match type="string" offset="0" value="#!/usr/bin/velc"/>
      <match type="string" offset="0" value="// VeldoCra"/>
      <match type="string" offset="0" value="# VeldoCra"/>
    </magic>
  </mime-type>
</mime-info>
EOF

install -Dm644 /tmp/veldocra.xml /usr/share/mime/packages/veldocra.xml
rm /tmp/veldocra.xml
echo "       - MIME type installed"

# Update MIME database
echo "[2/5] Updating MIME database..."
update-mime-database /usr/share/mime 2>/dev/null || true
echo "       - MIME database updated"

# Create icon directories
echo "[3/5] Installing VeldoCra icon..."
ICONS_DIRS=(
    "/usr/share/icons/hicolor/16x16/apps"
    "/usr/share/icons/hicolor/22x22/apps"
    "/usr/share/icons/hicolor/24x24/apps"
    "/usr/share/icons/hicolor/32x32/apps"
    "/usr/share/icons/hicolor/48x48/apps"
    "/usr/share/icons/hicolor/64x64/apps"
    "/usr/share/icons/hicolor/128x128/apps"
    "/usr/share/icons/hicolor/256x256/apps"
    "/usr/share/icons/hicolor/512x512/apps"
    "/usr/share/icons/hicolor/scalable/apps"
)

for dir in "${ICONS_DIRS[@]}"; do
    mkdir -p "$dir"
done

# Copy icon if exists
if [ -f "$PROJECT_ROOT/VeldAni.png" ]; then
    # Install original PNG
    install -Dm644 "$PROJECT_ROOT/VeldAni.png" /usr/share/icons/hicolor/256x256/apps/veldora.png
    install -Dm644 "$PROJECT_ROOT/VeldAni.png" /usr/share/icons/hicolor/512x512/apps/veldora.png
    
    # Create symlinks for other sizes
    for size in 16 22 24 32 48 64 128; do
        ln -sf /usr/share/icons/hicolor/256x256/apps/veldora.png \
            "/usr/share/icons/hicolor/${size}x${size}/apps/veldora.png" 2>/dev/null || true
    done
    
    # Create scalable symlink
    ln -sf /usr/share/icons/hicolor/256x256/apps/veldora.png \
        /usr/share/icons/hicolor/scalable/apps/veldora.png 2>/dev/null || true
    
    echo "       - Icon installed"
elif [ -f "$PROJECT_ROOT/icons/vel.png" ]; then
    # Fallback to vel.png
    install -Dm644 "$PROJECT_ROOT/icons/vel.png" /usr/share/icons/hicolor/256x256/apps/veldora.png
    install -Dm644 "$PROJECT_ROOT/icons/vel.png" /usr/share/icons/hicolor/512x512/apps/veldora.png
    
    for size in 16 22 24 32 48 64 128; do
        ln -sf /usr/share/icons/hicolor/256x256/apps/veldora.png \
            "/usr/share/icons/hicolor/${size}x${size}/apps/veldora.png" 2>/dev/null || true
    done
    
    ln -sf /usr/share/icons/hicolor/256x256/apps/veldora.png \
        /usr/share/icons/hicolor/scalable/apps/veldora.png 2>/dev/null || true
    
    echo "       - Icon installed (vel.png)"
else
    echo "       - Warning: VeldAni.png not found, using default icon"
fi

# Update icon cache
gtk-update-icon-cache -f -t /usr/share/icons/hicolor 2>/dev/null || true
echo "       - Icon cache updated"

# Create .desktop file
echo "[4/5] Installing desktop entry..."
cat > /tmp/veldora.desktop << 'EOF'
[Desktop Entry]
Version=1.0
Type=Application
Name=VeldoCra
GenericName=VeldoCra Compiler
Comment=VeldoCra Programming Language Compiler
Exec=/usr/local/bin/velc %f
Icon=veldora
Terminal=true
Categories=Development;Interpreter;ProgrammingLanguage;
MimeType=text/x-vel;
Keywords=vel;compiler;programming;script;
StartupNotify=true
EOF

install -Dm644 /tmp/veldora.desktop /usr/share/applications/veldora.desktop
rm /tmp/veldora.desktop
echo "       - Desktop entry installed"

# Update desktop database
update-desktop-database /usr/share/applications 2>/dev/null || true

# Install compiler
echo "[5/5] Installing VeldoCra compiler..."
if [ -f "$PROJECT_ROOT/build/bin/velc" ]; then
    install -Dm755 "$PROJECT_ROOT/build/bin/velc" /usr/local/bin/velc
    echo "       - Compiler installed to /usr/local/bin/velc"
else
    echo "       - Warning: velc not found. Please build first with: cd build && make"
fi

echo
echo "============================================"
echo "  Installation Complete!"
echo "============================================"
echo
echo "Please restart your file manager to see changes."
echo
echo "For GNOME: nautilus -q && nautilus &"
echo "For KDE: kbuildsycoca6"
echo "For XFCE: thunar -q && thunar &"
echo "For generic: rm -rf ~/.cache/icon* && logout"
echo
echo "To verify MIME type:"
echo "  xdg-mime query filetype example.vel"
echo "  (should show: text/x-vel)"
echo

