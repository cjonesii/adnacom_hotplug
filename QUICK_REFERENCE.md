# Quick Reference - Adnacom Hotplug Tool

## Installation

### Self-Extracting Installer (Recommended)
```bash
# On build system
make
make installer

# Copy adnacom-hotplug-installer.sh to target system, then:
sudo ./adnacom-hotplug-installer.sh
```

### Manual Installation
```bash
make clean && make
sudo make install
sudo systemctl daemon-reload
sudo systemctl enable adnacom-hotplug
sudo systemctl start adnacom-hotplug
```

## Basic Commands

```bash
# Check version
adnacom-hp --version

# Run in verbose mode
sudo adnacom-hp -v

# Build from source
make clean && make

# Install to system
sudo make install

# Service management
sudo systemctl start adnacom-hotplug
sudo systemctl stop adnacom-hotplug
sudo systemctl restart adnacom-hotplug
sudo systemctl status adnacom-hotplug
sudo systemctl enable adnacom-hotplug   # Auto-start on boot
sudo systemctl disable adnacom-hotplug  # Disable auto-start

# View logs
sudo journalctl -u adnacom-hotplug -f
sudo journalctl -u adnacom-hotplug --since today
```

## Supported Devices

| Board | PCIe Switch | Vendor ID | Device ID |
|-------|-------------|-----------|-----------|
| H1A   | PEX8608     | 0x10B5    | 0x8608    |
| H18   | PEX8718     | 0x10B5    | 0x8718    |
| H3    | PEX8718     | 0x10B5    | 0x8718    |

## File Locations

| Item | Location |
|------|----------|
| Executable | `/usr/local/sbin/adnacom-hp` |
| Service File | `/etc/systemd/system/adnacom-hotplug.service` |
| Source Code | `/home/cj/adnacom_hotplug/` |

## Kernel Requirements

**Required Kernel Parameters:**

The hotplug tool requires specific kernel parameters to function properly. Add these to your GRUB configuration:

```bash
pci=hpmmioprefsize=2MB,realloc,earlydump pcie_port_pm=off pcie_aspm=off
```

### Check Current Kernel Parameters
```bash
cat /proc/cmdline | grep -i pci
```

### Configure Kernel Parameters (if not set)

1. Edit GRUB configuration:
   ```bash
   sudo nano /etc/default/grub
   ```

2. Add to `GRUB_CMDLINE_LINUX_DEFAULT`:
   ```
   pci=hpmmioprefsize=2MB,realloc,earlydump pcie_port_pm=off pcie_aspm=off
   ```

3. Update GRUB and reboot:
   ```bash
   sudo update-grub
   sudo reboot
   ```

**Parameter Explanation:**
- `pci=hpmmioprefsize=2MB,realloc` - Allocates 2MB prefetchable memory for hotplug and enables resource reallocation
- `pcie_port_pm=off` - Disables PCIe port power management
- `pcie_aspm=off` - Disables Active State Power Management

## Troubleshooting

```bash
# Check if devices are detected
lspci | grep -i plx

# Check if service is running
ps aux | grep adnacom-hp

# Check system messages
dmesg | grep -i pci | tail -20

# Verify kernel parameters are set
cat /proc/cmdline

# Test without installing
sudo ./adnacom-hp
```

## Quick Build Test

```bash
cd /home/cj/adnacom_hotplug
make clean
make
./adnacom-hp --version
```

Expected output:
```
Adnacom Hotplug Tool version 0.0.1
Supports: H1A (PEX8608), H18/H12/H3 (PEX8718)
```
