#!/bin/bash
# Script to create a self-extracting installer for adnacom-hotplug
set -e

INSTALLER_NAME="adnacom-hotplug-installer.sh"
BINARY="adnacom-hp"
SERVICE_FILE="adnacom-hotplug.service"

echo "Creating self-extracting installer: $INSTALLER_NAME"

# Check if binary exists
if [ ! -f "$BINARY" ]; then
    echo "Error: $BINARY not found. Run 'make' first."
    exit 1
fi

# Check if service file exists
if [ ! -f "$SERVICE_FILE" ]; then
    echo "Error: $SERVICE_FILE not found."
    exit 1
fi

# Create the installer script header
cat > "$INSTALLER_NAME" << 'EOF_HEADER'
#!/bin/bash
# Adnacom Hotplug Self-Extracting Installer
# This script installs the adnacom-hotplug service

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Installation paths
INSTALL_DIR="/usr/local/sbin"
SERVICE_DIR="/etc/systemd/system"
BINARY_NAME="adnacom-hp"
SERVICE_NAME="adnacom-hotplug.service"

print_header() {
    echo -e "${BLUE}╔════════════════════════════════════════════════════════╗${NC}"
    echo -e "${BLUE}║     Adnacom Hotplug Service Installer                  ║${NC}"
    echo -e "${BLUE}║     Version 0.0.1                                      ║${NC}"
    echo -e "${BLUE}╚════════════════════════════════════════════════════════╝${NC}"
    echo ""
}

print_success() {
    echo -e "${GREEN}✓${NC} $1" >&2
}

print_error() {
    echo -e "${RED}✗${NC} $1" >&2
}

print_warning() {
    echo -e "${YELLOW}⚠${NC} $1" >&2
}

print_info() {
    echo -e "${BLUE}ℹ${NC} $1" >&2
}

check_root() {
    if [ "$EUID" -ne 0 ]; then
        print_error "This installer must be run as root"
        echo "Please run: sudo $0"
        exit 1
    fi
}

check_kernel_params() {
    print_info "Checking kernel parameters..."
    CMDLINE=$(cat /proc/cmdline)
    
    if echo "$CMDLINE" | grep -q "pci=hpmmioprefsize"; then
        print_success "Kernel parameters are configured correctly"
        return 0
    else
        print_warning "Required kernel parameters are NOT configured!"
        echo ""
        echo "The hotplug service requires the following kernel parameters:"
        echo "  pci=hpmmioprefsize=2MB,realloc,earlydump pcie_port_pm=off pcie_aspm=off"
        echo ""
        echo "To configure:"
        echo "  1. Edit /etc/default/grub"
        echo "  2. Add the parameters to GRUB_CMDLINE_LINUX_DEFAULT"
        echo "  3. Run: sudo update-grub"
        echo "  4. Reboot the system"
        echo ""
        read -p "Continue installation anyway? (y/N): " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            exit 1
        fi
    fi
}

extract_files() {
    print_info "Extracting files..."
    
    # Create temporary directory
    TEMP_DIR=$(mktemp -d)
    
    # Extract archive from this script
    ARCHIVE_LINE=$(awk '/^__ARCHIVE_BELOW__/ {print NR + 1; exit 0; }' "$0")
    tail -n +$ARCHIVE_LINE "$0" | base64 -d | tar -xz -C "$TEMP_DIR"
    
    print_success "Files extracted to temporary directory"
    echo "$TEMP_DIR"
}

install_binary() {
    TEMP_DIR=$1
    print_info "Installing binary to $INSTALL_DIR..."
    
    # Stop service if running
    if systemctl is-active --quiet adnacom-hotplug 2>/dev/null; then
        print_info "Stopping existing service..."
        systemctl stop adnacom-hotplug
    fi
    
    # Install binary
    install -m 755 "$TEMP_DIR/$BINARY_NAME" "$INSTALL_DIR/$BINARY_NAME"
    
    print_success "Binary installed"
}

install_service() {
    TEMP_DIR=$1
    print_info "Installing systemd service..."
    
    # Install service file
    install -m 644 "$TEMP_DIR/$SERVICE_NAME" "$SERVICE_DIR/$SERVICE_NAME"
    
    # Reload systemd
    systemctl daemon-reload
    
    print_success "Service file installed"
}

enable_service() {
    print_info "Enabling and starting service..."
    
    systemctl enable adnacom-hotplug
    systemctl start adnacom-hotplug
    
    sleep 1
    
    if systemctl is-active --quiet adnacom-hotplug; then
        print_success "Service is running"
    else
        print_error "Service failed to start"
        print_info "Check logs with: journalctl -u adnacom-hotplug -n 50"
        exit 1
    fi
}

show_status() {
    echo ""
    echo -e "${BLUE}═══════════════════════════════════════════════════════${NC}"
    echo -e "${GREEN}Installation Complete!${NC}"
    echo -e "${BLUE}═══════════════════════════════════════════════════════${NC}"
    echo ""
    echo "Service Status:"
    systemctl status adnacom-hotplug --no-pager | head -10
    echo ""
    echo "Useful Commands:"
    echo "  View logs:        sudo journalctl -u adnacom-hotplug -f"
    echo "  Stop service:     sudo systemctl stop adnacom-hotplug"
    echo "  Start service:    sudo systemctl start adnacom-hotplug"
    echo "  Restart service:  sudo systemctl restart adnacom-hotplug"
    echo "  Disable service:  sudo systemctl disable adnacom-hotplug"
    echo ""
}

cleanup() {
    if [ -n "$TEMP_DIR" ] && [ -d "$TEMP_DIR" ]; then
        rm -rf "$TEMP_DIR"
    fi
}

# Main installation flow
main() {
    print_header
    
    check_root
    check_kernel_params
    
    TEMP_DIR=$(extract_files)
    trap cleanup EXIT
    
    install_binary "$TEMP_DIR"
    install_service "$TEMP_DIR"
    enable_service
    show_status
    
    print_success "Installation completed successfully!"
}

# Check if we're being executed or sourced
if [ "${BASH_SOURCE[0]}" = "${0}" ]; then
    main "$@"
    exit 0
fi

__ARCHIVE_BELOW__
EOF_HEADER

# Create temporary directory for packaging
TEMP_PKG=$(mktemp -d)
cp "$BINARY" "$TEMP_PKG/"
cp "$SERVICE_FILE" "$TEMP_PKG/"

# Create tar archive, compress, and encode in base64
tar -czf - -C "$TEMP_PKG" . | base64 >> "$INSTALLER_NAME"

# Cleanup
rm -rf "$TEMP_PKG"

# Make installer executable
chmod +x "$INSTALLER_NAME"

# Get file size
SIZE=$(du -h "$INSTALLER_NAME" | cut -f1)

echo ""
echo "✓ Self-extracting installer created: $INSTALLER_NAME"
echo "  Size: $SIZE"
echo ""
echo "To install on target system:"
echo "  sudo ./$INSTALLER_NAME"
echo ""
echo "The installer will:"
echo "  1. Check for root privileges"
echo "  2. Verify kernel parameters"
echo "  3. Install binary to /usr/local/sbin"
echo "  4. Install systemd service"
echo "  5. Enable and start the service"
echo ""
