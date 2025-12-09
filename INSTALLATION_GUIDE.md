# Adnacom Hotplug Service - Installation Guide

## Overview

The Adnacom Hotplug Service automatically monitors and manages PCIe device connections for Adnacom H1A, H18, H12, and H3 adapter cards. This guide will help you install the service on your Linux system.

## System Requirements

- **Operating System**: Linux (Ubuntu, Debian, CentOS, RHEL, or compatible)
- **Architecture**: x86_64 (64-bit)
- **Privileges**: Root/administrator access (sudo)
- **Hardware**: Adnacom PCIe adapter card (H1A, H18, H12, or H3)

## Installation Steps

### Step 1: Download the Installer

You should have received a file named `adnacom-hotplug-installer.sh`. Save this file to a convenient location on your system (e.g., Downloads folder or home directory).

### Step 2: Make the Installer Executable

Open a terminal and navigate to the location where you saved the installer:

```bash
cd ~/Downloads
chmod +x adnacom-hotplug-installer.sh
```

### Step 3: Run the Installer

Execute the installer with root privileges:

```bash
sudo ./adnacom-hotplug-installer.sh
```

You will be prompted for your password. Enter your sudo password to proceed.

### Step 4: Follow the Installation Process

The installer will automatically:

1. ✓ Check that you have proper administrator privileges
2. ✓ Verify kernel parameters are configured (see note below)
3. ✓ Extract and install the service binary
4. ✓ Configure the systemd service
5. ✓ Enable automatic startup on boot
6. ✓ Start the service immediately

The installation typically completes in less than 10 seconds.

### Step 5: Verify Installation

After installation completes, you should see:

```
═══════════════════════════════════════════════════════
Installation Complete!
═══════════════════════════════════════════════════════

Service Status:
● adnacom-hotplug.service - Adnacom PCIe Hotplug Utility
     Loaded: loaded
     Active: active (running)
```

This confirms the service is running and will automatically start on system boot.

## Important: Kernel Parameters

### What Are Kernel Parameters?

The hotplug service requires specific Linux kernel settings to function properly. These settings allocate memory for PCIe hotplug operations.

### Checking Kernel Parameters

The installer will automatically check if the required parameters are configured. If they are **not** configured, you will see a warning:

```
⚠ Required kernel parameters are NOT configured!
```

### Configuring Kernel Parameters (If Needed)

If you see the warning, follow these steps to configure the kernel parameters:

1. **Edit the GRUB configuration:**
   ```bash
   sudo nano /etc/default/grub
   ```

2. **Find the line starting with `GRUB_CMDLINE_LINUX_DEFAULT`**

   It might look like:
   ```
   GRUB_CMDLINE_LINUX_DEFAULT="quiet splash"
   ```

3. **Add the following parameters inside the quotes:**
   ```
   pci=hpmmioprefsize=2MB,realloc,earlydump pcie_port_pm=off pcie_aspm=off
   ```

   After editing, it should look like:
   ```
   GRUB_CMDLINE_LINUX_DEFAULT="quiet splash pci=hpmmioprefsize=2MB,realloc,earlydump pcie_port_pm=off pcie_aspm=off"
   ```

4. **Save the file** (in nano: Press Ctrl+O, then Enter, then Ctrl+X)

5. **Update GRUB:**
   ```bash
   sudo update-grub
   ```

6. **Reboot your system:**
   ```bash
   sudo reboot
   ```

7. **After reboot, verify the parameters are active:**
   ```bash
   cat /proc/cmdline | grep -i pci
   ```

   You should see the parameters you added in the output.

### Can I Install Without Configuring Parameters?

Yes, the installer will allow you to proceed even if kernel parameters are not configured. However, **the hotplug service may not function correctly** without these parameters. We strongly recommend configuring them before using the service in production.

## Post-Installation

### Checking Service Status

To check if the service is running:

```bash
sudo systemctl status adnacom-hotplug
```

You should see `Active: active (running)` in green text.

### Viewing Service Logs

To view real-time logs from the service:

```bash
sudo journalctl -u adnacom-hotplug -f
```

Press Ctrl+C to stop viewing logs.

To view recent log entries:

```bash
sudo journalctl -u adnacom-hotplug -n 50
```

### Managing the Service

**Stop the service:**
```bash
sudo systemctl stop adnacom-hotplug
```

**Start the service:**
```bash
sudo systemctl start adnacom-hotplug
```

**Restart the service:**
```bash
sudo systemctl restart adnacom-hotplug
```

**Disable automatic startup:**
```bash
sudo systemctl disable adnacom-hotplug
```

**Re-enable automatic startup:**
```bash
sudo systemctl enable adnacom-hotplug
```

## Troubleshooting

### Service Won't Start

1. Check logs for errors:
   ```bash
   sudo journalctl -u adnacom-hotplug -n 50
   ```

2. Verify the Adnacom card is detected:
   ```bash
   lspci | grep -i plx
   ```

   You should see your Adnacom device listed.

3. Ensure kernel parameters are configured (see section above)

### No Adnacom Device Detected

1. Verify the card is properly seated in the PCIe slot
2. Power cycle the system
3. Check BIOS settings to ensure PCIe slots are enabled

### Installer Reports Permission Errors

Make sure you're running the installer with `sudo`:
```bash
sudo ./adnacom-hotplug-installer.sh
```

## Uninstallation

If you need to remove the service:

1. **Stop and disable the service:**
   ```bash
   sudo systemctl stop adnacom-hotplug
   sudo systemctl disable adnacom-hotplug
   ```

2. **Remove the service files:**
   ```bash
   sudo rm /etc/systemd/system/adnacom-hotplug.service
   sudo rm /usr/local/sbin/adnacom-hp
   ```

3. **Reload systemd:**
   ```bash
   sudo systemctl daemon-reload
   ```

## Support

If you encounter any issues during installation or operation:

1. Collect the service logs:
   ```bash
   sudo journalctl -u adnacom-hotplug -n 100 > adnacom-logs.txt
   ```

2. Check your kernel parameters:
   ```bash
   cat /proc/cmdline > kernel-params.txt
   ```

3. List detected PCIe devices:
   ```bash
   lspci -v | grep -A 10 -i plx > pci-devices.txt
   ```

4. Contact Adnacom support with these files attached.

## Summary

The Adnacom Hotplug Service installation is designed to be simple and automated:

1. Download `adnacom-hotplug-installer.sh`
2. Run `sudo ./adnacom-hotplug-installer.sh`
3. Configure kernel parameters if prompted (one-time setup)
4. Reboot if you configured kernel parameters
5. Done! The service runs automatically on every boot

Thank you for choosing Adnacom!

---

**Version**: 0.0.1
**Last Updated**: December 2025
**Compatible with**: H1A, H18, H12, H3 adapter cards
