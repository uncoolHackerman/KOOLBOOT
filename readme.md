# Changelogs

## KOOLBOOT v0.0.01 (17/12/2022)

- added the BIOS Parameter Block and Extended Boot Record data to the boot sector, which makes KOOLBOOT compatible with FAT12 formatted disks
- KOOLBOOT will now use the BIOS to get the geometry of the boot device
- KOOLBOOT now loads a second stage bootloader stored in ::/koolboot.bin

## KOOLBOOT v0.0.00 (17/12/2022)

- added support for booting from the BIOS
- added puts function
