# boot.s 17/12/2022 - 18/12/2022
# basic boot sector for KOOLBOOT
# Written by Gabriel Jickells

.code16                         # the BIOS will boot this code in 16-bit real mode

# BIOS Parameter Block (BPB)
jmp start                      # skip the FAT data
nop
BPB_OEM_Label: .ascii "KOOLBOOT"
BPB_BytesPerSector: .word 512   # there are in fact 512 bytes in a sector
BPB_SectorsPerCluster: .byte 2  # 1kb clusters
BPB_ReservedSectors: .word 1    # essentially the amount of boot sectors there are
BPB_FAT_Count: .byte 1          # normally there are 2 FATs for verification but the second is not used by KOOLBOOT so having 1 saves 4.5Kib on disk
BPB_RootDirEntries: .word 224   # value obtained from mkfs.fat
BPB_TotalSectors: .word 2880    # 1.44MiB disk
BPB_MediaType: .byte 0xF0       # I don't know either
BPB_SectorsPerFAT: .word 9      # value obtained from mkfs.fat
BPB_SectorsPerTrack: .word 18   # what the BIOS would give us for a 1.44MiB disk
BPB_HeadCount: .word 2          # needed for the LBA to CHS conversion
BPB_HiddenSectors: .long 0      # LBA of the start of the partition. Only needed for hard drives
BPB_LargeSectors: .long 0       # only needed if there are more than 65535 sectors on the disk
# Extended Boot Record (EBR)
EBR_DriveNumber: .byte 0        # store dl here
EBR_Reserved: .byte 0           # Windows NT flags. This is not Windows NT so this should always be zero
EBR_Signature: .byte 0x29       # must be either 0x28 or 0x29
EBR_SerialNumber: .long 0x20F78A99  # not important, got this from a random number generator
EBR_VolumeID: .ascii "K BOOT BEST"
EBR_SystemID: .ascii "FAT12   " # documentation says not to trust this value but you might as well

start:
    xor %ax, %ax                # reset ax to 0
    mov %ax, %ds                # reset the data segment register
    mov %ax, %es                # reset the extra segment registe
    mov %ax, %ss                # reset the stack segment register
    mov $0x7c00, %sp            # set the stack to begin at the start of the program (stack grows downwards)
    mov %sp, %bp                # so we know where the start of the stack is
    ljmp $0, $main              # reset the code segment register. ljmp code_seg, offset

# Functions

# ARGUMENTS
# si = char* str (pointer to the string that needs to be printed)
puts:
    pusha                       # push all of the general purpose registers to the stack
    mov $0x0E, %ah              # write character in TTY mode
    mov $0, %bh                 # write on page 0
    1:                          # while(*str)
        lodsb                   # move [ds:si] into al and increment si
        test %al, %al           # check for end of string
        jz 1f                   # break if it is the end of the string
        int $0x10               # BIOS video services
        jmp 1b
    1:                          # end of the loop
        popa                    # restore the general purpose registers
        ret                     # return from the function

# ARGUMENTS
# ax = LBA address
# OUTPUT
# ch = low 8 bits of cylinder number
# cl = sector number (bits 0 - 5) and high 2 bits of cylinder number (bits 6 - 7)
# dh = head number
LBA2CHS:
    push %ax                    # save the LBA value
    push %dx                    # save the boot drive value in dl
    xor %dx, %dx                # dx must be zero before performing the div instruction
    divw (BPB_SectorsPerTrack)  # all of the conversion equations start with some form of (lba / SectorsPerTrack)
    inc %dx                     # dx = remainder so dx + 1 = Sector number
    mov %dl, %cl                # cl = sector number
    xor %dx, %dx
    divw (BPB_HeadCount)        # both the cylinder equation and the head equation use some form of ((lba / SectorsPerTrack) / HeadCount)
    shl $6, %ah                 # ah = high two bits of cylinder number
    or %ah, %cl                 # sector number and high 2 bits of cylinder number are stored in the same register
    mov %al, %ch                # ch = low 8 bits of cylinder number
    mov %dl, %al                # save the head number in al
    pop %dx                     # restore the boot drive
    mov %al, %dh                # dh = head number
    pop %ax                     # restore the LBA value
    ret

# ARGUMENTS
# ax = LBA
# cl = amount of sectors to be read
# dl = drive
# es:bx = bufferOut
ReadSectors:
    pusha
    push %cx                    # save the amount of sectors that need to be read
    call LBA2CHS                # int $0x13 expects an LBA Address
    pop %ax                     # restore the amount of sectors to be read into al (which is where int $0x13 expects them to be)
    mov $5, %di                 # di = amount of attempts left
    1:
        test %di, %di           # check if the function has run out of tries
        jz Reboot               # we should not continue any further if we can't read from the disk
        dec %di                 # use one of the attempts
        mov $0x02, %ah          # read sectors from the disk
        stc                     # set the carry flag because some BIOSes don't set it properly
        int $0x13               # BIOS disk services
        jnc 1f                  # jump to the end of the function if there are no errors
        mov $0x00, %ah          # reset disk system
        int $0x13
        jmp 1b                  # if there are errors then retry
    1:                          # end of the function
        popa
        ret

Reboot:
    ljmp $0xF000, $0xFFF0       # start of BIOS

# ARGUMENTS
# ax = cluster
# OUTPUT
# ax = LBA
Cluster2LBA:
    sub $2, %ax
    mulb (BPB_SectorsPerCluster)
    addw (g_DataSectionLBA), %ax
    ret

# Data

Intro_MSG: .asciz "KOOLBOOT v0.0.01\n\r"
g_DataSectionLBA: .word 0
g_File: .ascii "KOOLBOOTBIN"
#_File: .ascii "123456789AB"

main:
    # print the welcome message to the screen
    mov $Intro_MSG, %si
    call puts
    # get the drive parameters
    movb %dl, (EBR_DriveNumber) # save the drive number to free up dl for use by the bootloader
    xor %di, %di                # set es:di to 0:0 to avoid BIOS bugs
    mov $0x08, %ah              # get drive parameters
    int $0x13                   # BIOS disk services
    and $0x3F, %cx              # cx = sectors per track, we don't need to use the cylinder number
    movw %cx, (BPB_SectorsPerTrack)
    inc %dh                     # dh = Head Count (Maximum Head Number + 1)
    mov %dh, %dl
    xor %dh, %dh                # dx = Head Count
    movw %dx, (BPB_HeadCount)
    xor %di, %di
    mov %di, %es
    ReadRootDirectory:
        movw (BPB_RootDirEntries), %ax
        mov $32, %cx            # sizeof(DirectoryEntry) = 32
        mul %cx                 # ax = RootDirectorySize
        add $511, %ax           # BytesPerSector - 1 = 511 (for rounding purposes, we should always round up)
        xor %dx, %dx
        divw (BPB_BytesPerSector) # ax = RootDirectorySectors
        mov %ax, %cx              # cx = RootDirectorySectors
        movw (BPB_SectorsPerFAT), %ax
        mulb (BPB_FAT_Count)
        addb (BPB_ReservedSectors), %al # ax = RootDirectoryLBA
        movb (EBR_DriveNumber), %dl # dl = drive number
        mov $Buffer, %bx        # es:bx = 0x0000:buffer
        call ReadSectors
        add %cl, %al            # ax = DataSectionLBA
        movw %ax, (g_DataSectionLBA)
    FindFile:
        mov $Buffer, %si        # si = g_RootDirectory[0]
    FindFileLoop:
        push %si                # save the current index in the RootDirectory
        mov $11, %cx            # compare 11 bytes/characters
        mov $g_File, %di        # di = name of file we are looking for
        repe cmpsb              # while(*(ds:si) == *(es:di) && cx >= 0) {si++; di++; cx--;}
        pop %si
        je ReadFAT
        add $32, %si            # go to the next entry in the root directory
        jmp FindFileLoop        # will go into an infinite loop if the file isn't in the root directory
    ReadFAT:
        add $26, %si            # get the first cluster of the file
        mov (%si), %ax          # ax = g_File.FirstClusterLow
        push %ax
        movw (BPB_ReservedSectors), %ax # FAT starts right after the boot sector(s)
        mov $Buffer, %bx        # load the FAT into the buffer
        movb (EBR_DriveNumber), %dl
        movw (BPB_SectorsPerFAT), %cx
        call ReadSectors
    ReadFile:
        pop %ax                 # ax = g_File.FirstClusterLow
        .equ STAGE2SEGMENT, 0x0000
        .equ STAGE2OFFSET, 0x0500    # where the stage2.bin file will be loaded
        mov $STAGE2SEGMENT, %bx # load the stage2 address segment into es
        mov %bx, %es
        mov $STAGE2OFFSET, %bx  # load the stage2 address offset into bx
    ReadFileLoop:
        push %ax
        call Cluster2LBA        # LBA = Cluster2LBA(CurrentCluster)
        movw (BPB_SectorsPerCluster), %cx   # count = Sectors per cluster
        movb (EBR_DriveNumber), %dl # drive = boot drive
        call ReadSectors
        mov (BPB_BytesPerSector), %ax   # work out the next bx value
        xor %ch, %ch
        mul %cx
        add %ax, %bx            #will overflow after 64KiB
        pop %ax                 # ax = CurrentCluster
        mov $3, %cx             # FAT_Index = (CurrentCluster * 3) / 2
        mul %cx
        mov $2, %cx
        xor %dx, %dx
        div %cx                 # ax = FatIndex
        mov $Buffer, %si        # si = &g_FAT[0]
        add %ax, %si            # si = &g_FAT[FatIndex]
        movw (%si), %ax         # ax = g_FAT[FatIndex]
        test %dl, %dl           # if there is a remainder then it means that the Current Cluster was even
        jz ClusterEven
        ClusterOdd:
            shr $4, %ax
            jmp LoopCTRL
        ClusterEven:
            and $0x0FFF, %ax
        LoopCTRL:
            cmp $0x0FF8, %ax    # check for end of file
            jl ReadFileLoop
        Finish:
            mov (EBR_DriveNumber), %dl
            xor %ax, %ax        # set up the segment registers and stack for stage 2
            mov %ax, %ds
            mov %ax, %es
            mov %ax, %fs
            mov %ax, %gs
            mov %ax, %ss
            mov $0xFFF0, %sp    # set up stack
            mov %sp, %bp
            ljmp $STAGE2SEGMENT, $STAGE2OFFSET  # go to stage 2

.org 510                        # skip to the offset of the BIOS booting number
.word 0xAA55                    # define the BIOS booting number
Buffer:
