# Migration from adnacom_h1a_hp to adnacom_hotplug

## Overview

This project has been renamed from `adnacom_h1a_hp` to `adnacom_hotplug` to better reflect its expanded support for multiple Adnacom PCIe host adapter models.

## Changes Summary

### Device Support Added
- **H1A (PEX8608)** - Original support maintained
- **H18 (PEX8718)** - New support added
- **H3 (PEX8718)** - New support added

### Renamed Components

| Old Name | New Name | Description |
|----------|----------|-------------|
| `adnacom_h1a_hp` | `adnacom_hotplug` | Project directory |
| `h1a_hp` | `adnacom-hp` | Executable binary |
| `h1a_hp.service` | `adnacom-hotplug.service` | Systemd service file |
| Program name: `"adna"` | Program name: `"adnacom-hp"` | Internal identifier |

### Code Changes

1. **Device Table Update** (`src/adna.c`):
   ```c
   // Added H18 device to the supported devices table
   struct adnatool_pci_device adnatool_pci_devtbl[] = {
       { .vid = PLX_VENDOR_ID, .did = PLX_H1A_DEVICE_ID, .cls_rev = PCI_CLASS_BRIDGE_PCI, },
       { .vid = PLX_VENDOR_ID, .did = PLX_H18_DEVICE_ID, .cls_rev = PCI_CLASS_BRIDGE_PCI, },
       { .vid = TI_VENDOR_ID,  .did = TI_DEVICE_ID,      .cls_rev = PCI_CLASS_SERIAL_USB, },
       {0}, /* sentinel */
   };
   ```

2. **Updated Headers**: All source file headers now indicate support for H1A, H18, H12, and H3

3. **Version Output**: Updated to show supported devices:
   ```
   Adnacom Hotplug Tool version 0.0.1
   Supports: H1A (PEX8608), H18/H12/H3 (PEX8718)
   ```

### Build System Changes

- **Makefile**: Updated to build `adnacom-hp` binary
- **Service File**: Renamed and updated to reference `/usr/local/sbin/adnacom-hp`
- **Installation Paths**: All paths remain the same (PREFIX=/usr/local)

## Migration for Existing Users

If you have the old `h1a_hp` service installed:

1. **Stop the old service**:
   ```bash
   sudo systemctl stop h1a_hp
   sudo systemctl disable h1a_hp
   ```

2. **Build and install the new version**:
   ```bash
   cd adnacom_hotplug
   make clean
   make
   sudo make install
   ```

3. **Enable and start the new service**:
   ```bash
   sudo systemctl daemon-reload
   sudo systemctl enable adnacom-hotplug
   sudo systemctl start adnacom-hotplug
   ```

4. **Verify operation**:
   ```bash
   sudo systemctl status adnacom-hotplug
   adnacom-hp --version
   ```

5. **Optional: Remove old service file**:
   ```bash
   sudo rm /etc/systemd/system/h1a_hp.service
   ```

## Compatibility Notes

- The hotplug monitoring functionality remains identical
- All H1A-specific features continue to work
- H18 and H3 boards use the same PEX8718 chip and are compatible
- Configuration and behavior are unchanged

## Technical Details

### Device IDs
- **PLX_VENDOR_ID**: 0x10B5
- **PLX_H1A_DEVICE_ID**: 0x8608
- **PLX_H18_DEVICE_ID**: 0x8718

### PCIe Switch Types
- H1A: PEX8608
- H18: PEX8718
- H3: PEX8718

Both H18 and H3 use the PEX8718 switch, which is why they share the same device ID.

## Testing

To test if your device is detected:

```bash
# Run in verbose mode
sudo ./adnacom-hp -v

# Check system logs
sudo journalctl -u adnacom-hotplug -f
```

## Support

For issues or questions:
- GitHub: https://github.com/cjonesii/adnacom_hotplug
- Related project: adnacom_monitor (GUI monitoring tool)

---
**Last Updated**: December 9, 2025
