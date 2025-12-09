# H12 Board Support Addition

## Date: December 9, 2025

---

## ✅ H12 Support Successfully Added to Both Projects

### Summary

H12 board support has been added to both `adnacom_hotplug` and verified in `adnacom_monitor`. The H12 uses the same **PLX PEX8718** PCIe switch as H18 and H3, so it shares the device ID **0x8718**.

---

## 📋 H12 Specifications

From **PCIe Gen3 Command Interface** documentation (Table 4):

| Parameter | Value |
|-----------|-------|
| **Board ID** | 4 |
| **PCIe Switch** | PEX8718 |
| **Device ID** | 0x8718 |
| **Vendor ID** | 0x10B5 (PLX) |
| **Max Downstream Ports** | 2 |
| **Transceiver Type** | SFP, CSFP |
| **Max Transceivers** | 2 |
| **Max Transceiver Channels** | 1 or 2 (depending on type) |
| **Number of DIP Switches** | 1 |
| **Mode Selection** | NA (Host only) |

---

## 🔧 Changes Made

### adnacom_hotplug Project

#### Files Modified:
1. **src/adna.c** - Updated header comments
   - Changed: "Supports Adnacom H1A, H18, and H3"
   - To: "Supports Adnacom H1A, H18, H12, and H3"

2. **src/adna.h** - Updated header comments
   - Changed: "Supports Adnacom H1A, H18, and H3"
   - To: "Supports Adnacom H1A, H18, H12, and H3"

3. **src/main.c** - Updated version output
   - Changed: `puts("Supports: H1A (PEX8608), H18/H3 (PEX8718)");`
   - To: `puts("Supports: H1A (PEX8608), H18/H3/H12 (PEX8718)");`

4. **README.md** - Added H12 to supported devices list
   ```
   Supported Devices:
   - H1A  (PLX PEX8608)
   - H18  (PLX PEX8718)
   - H12  (PLX PEX8718)
   - H3   (PLX PEX8718)
   ```

5. **SUMMARY.md** - Updated device support matrix
6. **MIGRATION.md** - Updated references to supported boards
7. **QUICK_REFERENCE.md** - Updated board listings

#### Device Table:
**No changes needed!** The existing device table already supports H12:
```c
struct adnatool_pci_device adnatool_pci_devtbl[] = {
    { .vid = PLX_VENDOR_ID, .did = PLX_H1A_DEVICE_ID, .cls_rev = PCI_CLASS_BRIDGE_PCI, },
    { .vid = PLX_VENDOR_ID, .did = PLX_H18_DEVICE_ID, .cls_rev = PCI_CLASS_BRIDGE_PCI, }, // Covers H18, H12, H3
    { .vid = TI_VENDOR_ID,  .did = TI_DEVICE_ID,      .cls_rev = PCI_CLASS_SERIAL_USB, },
    {0}, /* sentinel */
};
```

### adnacom_monitor Project

#### Status: ✅ Already Supported

H12 was **already fully supported** in adnacom_monitor:

1. **Protocol.py** - Board ID defined:
   ```python
   BOARD_IDS = {1: "H18", 2: "R34", 3: "H14", 4: "H12", 5: "H3"}
   ```

2. **adnacom_monitor.py** - QSFP count configured:
   ```python
   "H12": 2,  # 2 transceivers
   ```

3. **Uses same device ID** (0x8718) as H18/H3, so device detection works automatically

---

## 🎯 Technical Notes

### Why H12 "Just Works"

Since H12 uses the **same PLX PEX8718 chip** as H18 and H3:
- Same PCI **Vendor ID**: 0x10B5
- Same PCI **Device ID**: 0x8718
- Same PCIe capabilities and registers
- Compatible hotplug monitoring mechanism

The only differences are:
- **Board ID** field in firmware (ID=4)
- **Transceiver type** (SFP/CSFP vs QSFP)
- **Number of ports** (2 vs 4)

These board-specific details are handled at the application level (in adnacom_monitor), not at the PCIe detection level (in adnacom_hotplug).

---

## ✅ Verification

### Build Test (adnacom_hotplug):
```bash
$ cd /home/cj/adnacom_hotplug
$ make clean && make
# Build successful

$ ./adnacom-hp --version
Adnacom Hotplug Tool version 0.0.1
Supports: H1A (PEX8608), H18/H3/H12 (PEX8718)
```

### Code Verification (adnacom_monitor):
```bash
$ grep "H12" adnacom_monitor/Protocol.py
BOARD_IDS = {1: "H18", 2: "R34", 3: "H14", 4: "H12", 5: "H3"}

$ grep "H12" adnacom_monitor/adnacom_monitor.py
"H12": 2,  # 2 transceivers
```

---

## 📊 Complete Board Support Matrix

| Board | PCIe Switch | Vendor ID | Device ID | Board ID | adnacom_hotplug | adnacom_monitor |
|-------|-------------|-----------|-----------|----------|-----------------|-----------------|
| H1A   | PEX8608     | 0x10B5    | 0x8608    | -        | ✅              | ❌              |
| H18   | PEX8718     | 0x10B5    | 0x8718    | 1        | ✅              | ✅              |
| H14   | PEX8718     | 0x10B5    | 0x8718    | 3        | ✅              | ✅              |
| H12   | PEX8718     | 0x10B5    | 0x8718    | 4        | ✅              | ✅              |
| H3    | PEX8718     | 0x10B5    | 0x8718    | 5        | ✅              | ✅              |
| R34   | PEX8734     | 0x10B5    | 0x8734    | 2        | ❌              | ✅              |

**Note**: 
- adnacom_hotplug: Monitors PCIe link status and provides automatic recovery
- adnacom_monitor: GUI for diagnostics, QSFP monitoring, and firmware updates

---

## 🚀 Board-Specific Features

### H12 Unique Characteristics

1. **Transceiver Support**:
   - SFP (Single channel - 1 TX/RX)
   - CSFP (Dual channel - 2 TX/RX)
   - Unlike H18/H3 which use QSFP (4 channels)

2. **Port Configuration**:
   - 2 downstream ports (same as H14)
   - Each port can be x2
   - Total: 2 downstream ports x2

3. **DIP Switch**:
   - 1 DIP switch (S1)
   - No mode selection (Host only)
   - Configuration: Always 2 downstream ports x2

4. **Application**:
   - Designed for SFP-based connectivity
   - Lower port count than H18 (4 ports) or H3 (4 ports)
   - Similar to H14 but with different transceiver type

---

## 📚 Reference Documentation

- **Source**: PCIe Gen3 Command Interface-2025-10-27-ia.docx
- **Location**: /home/cj/adnacom_monitor/docs/
- **Key Tables**:
  - Table 4: Board comparison (Parameters)
  - Table 6: Board types and transceiver counts
  - Table 8: Port configurations

---

## 🎓 Next Steps (Optional)

If you encounter an H12 board and need to test:

1. **Detection**:
   ```bash
   lspci | grep -i plx
   # Should show: 10b5:8718
   ```

2. **Hotplug Monitoring**:
   ```bash
   sudo systemctl start adnacom-hotplug
   sudo journalctl -u adnacom-hotplug -f
   ```

3. **GUI Monitoring** (if applicable):
   ```bash
   sudo adnacom-monitor
   # Select H12 from dropdown
   ```

---

**Status**: ✅ H12 Fully Supported in Both Projects  
**Date Added**: December 9, 2025  
**Build Verified**: Yes  
**Documentation Updated**: Yes
