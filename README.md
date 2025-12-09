Information
~~~~~~~~~~~

This is the Adnacom PCIe Hotplug Tool. It monitors the link status of
Adnacom downstream ports and performs appropriate actions to rescan or
re-enumerate the bus where the Adnacom device is installed whenever any
downstream link status changes.

Supported Devices:
- H1A  (PLX PEX8608)
- H18  (PLX PEX8718)
- H12  (PLX PEX8718)
- H3   (PLX PEX8718)

Copyright (C) 2022, 2023 Adnacom Inc.

This is based on the following works
PCI Utilities
Copyright (c) 1998--2020 Martin Mares <mj@ucw.cz>

pcimem.c
Copyright (C) 2010, Bill Farrow (bfarrow@beyondelectronics.us)


Installation
~~~~~~~~~~~~

## Quick Install (Recommended)

For end users who want a simple "install and forget" experience:

1. Build the self-extracting installer:
   ```bash
   make
   make installer
   ```

2. Run the installer on target system:
   ```bash
   sudo ./adnacom-hotplug-installer.sh
   ```

The installer will:
- Check root privileges
- Verify kernel parameters (warns if not configured)
- Install binary to `/usr/local/sbin/adnacom-hp`
- Install systemd service
- Enable and start the service automatically

## Manual Installation

If you prefer manual installation:

```bash
make
sudo make install
sudo systemctl daemon-reload
sudo systemctl enable adnacom-hotplug
sudo systemctl start adnacom-hotplug
```

## Kernel Parameters

**IMPORTANT**: The hotplug service requires specific kernel parameters to function correctly.

Add to `/etc/default/grub` in `GRUB_CMDLINE_LINUX_DEFAULT`:
```
pci=hpmmioprefsize=2MB,realloc,earlydump pcie_port_pm=off pcie_aspm=off
```

Then update GRUB and reboot:
```bash
sudo update-grub
sudo reboot
```

Verify parameters are active:
```bash
cat /proc/cmdline | grep -i pci
```


Usage
~~~~~

Check service status:
```bash
sudo systemctl status adnacom-hotplug
```

View logs:
```bash
sudo journalctl -u adnacom-hotplug -f
```

Run manually (foreground):
```bash
sudo ./adnacom-hp
```

