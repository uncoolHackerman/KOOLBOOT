# Changelogs (dates in DD/MM/YYYY)

## KOOLBOOT v0.0.01 (17/12/2022 - 19/12/2922)

- added the BIOS Parameter Block and Extended Boot Record data to the boot sector, which makes KOOLBOOT compatible with FAT12 formatted disks
- KOOLBOOT will now use the BIOS to get the geometry of the boot device
- KOOLBOOT now loads a second stage bootloader stored in ::/koolboot.bin
- added support for FAT16 formatted, unpartitioned hard disks
- fixed some bugs that were inherited from COOLBOOT
- fixed some bugs that weren't inherited from COOLBOOT

## KOOLBOOT v0.0.00 (17/12/2022)

- added support for booting from the BIOS
- added puts function
