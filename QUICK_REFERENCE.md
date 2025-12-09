# Quick Reference - Adnacom Hotplug Tool

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

## Troubleshooting

```bash
# Check if devices are detected
lspci | grep -i plx

# Check if service is running
ps aux | grep adnacom-hp

# Check system messages
dmesg | grep -i pci | tail -20

# Test without installing
sudo ./adnacom-hp
```

## Migration from h1a_hp

```bash
# 1. Stop old service
sudo systemctl stop h1a_hp
sudo systemctl disable h1a_hp

# 2. Build and install new version
cd /home/cj/adnacom_hotplug
make clean && make
sudo make install

# 3. Start new service
sudo systemctl daemon-reload
sudo systemctl enable adnacom-hotplug
sudo systemctl start adnacom-hotplug

# 4. Verify
sudo systemctl status adnacom-hotplug
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
