# SD Card Formatting for BeagleBone Black

---

## 1. Requirements

- SD card (min 4 GB, recommended 8+ GB, up to 32GB)
- Card reader
- Linux machine or Linux Virtual Machine

---

## ⚠️ Virtual Machine Notice

If you are using a **Linux Virtual Machine** (VirtualBox, VMware, etc.), the VM **cannot directly detect an SD card** inserted into your computer's built-in card reader.

You **must** use an **external USB card reader** and pass it through to the VM:

- Plug in the USB card reader with the SD card inserted
- In your VM settings, enable USB passthrough for the card reader device
- The VM will then detect it as a USB storage device (e.g. `/dev/sdb`)

---

## 2. Insert SD Card and Find the Device

```bash
lsblk -d -o NAME,ROTA,TRAN.MODEL
```

Example output: `/dev/sdb`, can be '/dev/sdc'

> ⚠️ **Be careful** not to select your system disk!

---

## 3. Unmount the SD Card

Before wiping, make sure all partitions are unmounted:

```bash
sudo umount /dev/sdb*
```

> This is safe to run even if nothing is mounted — it will just show a warning and continue.

---

## 4. Wipe Existing Partitions

```bash
sudo wipefs -a /dev/sdb
```

---

## 5. Create Partitions (MBR Table)

Run:

```bash
sudo fdisk /dev/sdb
```

Inside `fdisk`, enter the following commands in order:

| Step | Command | Description |
|------|---------|-------------|
| 1 | `o` | Create new MBR partition table |
| 2 | `n` | New partition |
| | `p` | Primary |
| | `1` | Partition number |
| | 8192 | Default start sector |
| | `+256M` | Boot partition size |
| 3 | `t` | Change partition type |
| | `c` | W95 FAT32 (LBA) |
| 4 | `a` | Set boot flag on partition 1 |
| 5 | `n` | New partition (root) |
| | `p` | Primary |
| | `2` | Partition number |
| | 532480 | Default start sector |
| | Enter | Use remaining disk space |
| 6 | `w` | Write changes and exit |

---

## 6. Format the Partitions

```bash
sudo mkfs.vfat -F 32 /dev/sdb1
sudo mkfs.ext4 /dev/sdb2
```

---

## 7. Mount the Partitions

```bash
mkdir -p /mnt/bbb-boot /mnt/bbb-root

sudo mount /dev/sdb1 /mnt/bbb-boot
sudo mount /dev/sdb2 /mnt/bbb-root
```

---

## 8. Sync and Unmount

After copying any files to the SD card, always sync and unmount cleanly before removing it:

```bash
sync
sudo umount /dev/sdb*
```

> `sync` flushes all pending writes to disk — skipping it can cause corrupted data on the SD card.
