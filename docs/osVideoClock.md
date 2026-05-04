# #11 - Enable BBB Internet Connection, Clock and Video Reproduction


## 1. OS Installation

### 1.1 Image Selection

Debian image was selected from the BeagleBoard.org page. The chosen image was:

```
am335x-debian-12.13-base-v5.10-ti-armhf-2026-04-23-4gb.img.xz
```

- **OS:** Debian 12.13 (Bookworm)
- **Kernel:** 5.10.168-ti-r83 (Texas Instruments optimized kernel for AM335x)
- **Architecture:** ARMv7l (32-bit ARM)


### 1.2 Flashing the Image


```bash
lsblk -d -o NAME,SIZE,ROTA,TRAN,MODEL


sudo umount /dev/sda1

xzcat am335x-debian-12.13-base-v5.10-ti-armhf-2026-04-23-4gb.img.xz | sudo dd of=/dev/sda bs=4M status=progress conv=fsync


sync
```

`dd` writes data to sd card replacing what was there before

### 1.3 Pre-boot Configuration

Before first boot, the FAT32 boot partition was mounted and `sysconf.txt` was edited:

```bash
sudo mount /dev/sda1 /mnt/sdboot
sudo nano /mnt/sdboot/sysconf.txt
```

The following values were configured:

```
user_name=debian
user_password=temppwd
hostname=beaglebone
timezone=Europe/Belgrade
```

### 1.4 Booting from SD Card

To boot BBB:

1. Insert the microSD card 
2. Hold the **S2** button
3. Apply power while holding S2
4. Hold for ~5 seconds then release
5. The 4 blue LEDs sweep — confirming SD boot

### 1.5 Verifying the Installation

SSH connection was established over USB Gadget Ethernet:

```bash
ssh debian@192.168.7.2
```

---

## 2. Internet Connection Setup

### 2.1 The Problem

The BBB has no WiFi and was connected to the laptop only via USB. Internet had to be shared from the laptop to the BBB over the USB.

### 2.2 Laptop-side Configuration

Run on the **laptop** to enable NAT (Network Address Translation) internet sharing:

```bash
# Enable IP forwarding
echo 1 | sudo tee /proc/sys/net/ipv4/ip_forward

# Share WiFi internet to BBB USB interface
sudo iptables -t nat -A POSTROUTING -o wlp0s20f3 -j MASQUERADE
sudo iptables -A FORWARD -i enxc8df84b59c75 -j ACCEPT
sudo iptables -A FORWARD -o enxc8df84b59c75 -j ACCEPT
```

> **Note:** `wlp0s20f3` is the laptop WiFi interface. `enxc8df84b59c75` is the BBB USB network interface — this name is derived from the BBB's MAC address

### 2.3 BBB-side Configuration

Run on the **BBB** to set the default gateway and DNS:

```bash
sudo ip route add default via 192.168.7.1
echo "nameserver 8.8.8.8" | sudo tee /etc/resolv.conf
```

### 2.4 Verification

```bash
ping -c 3 8.8.8.8    # Test routing
ping -c 3 google.com  # Test DNS
```
---

## 3. System Update and Time Synchronization

### 3.1 System Update

```bash
sudo apt update && sudo apt upgrade -y
```

### 3.2 NTP Time Synchronization

NTP (Network Time Protocol) was installed to automatically sync the clock on every boot:

```bash
sudo apt install ntp -y
sudo systemctl enable ntpsec
sudo systemctl start ntpsec
```

Set the correct timezone:
```bash
sudo timedatectl set-timezone Europe/Belgrade
```

---

## 4. Display Setup (HDMI)

### 4.1 Resolution Configuration

The default resolution was 1280x1024. It was reduced to 1024x768 by editing `/boot/uEnv.txt`:

```bash
sudo nano /boot/uEnv.txt
```

The active `cmdline` was replaced with the HDMI resolution override line:

```
cmdline=fsck.repair=yes earlycon coherent_pool=1M net.ifnames=0 lpj=1990656 rng_core.default_quality=100 video=HDMI-A-1:1024x768@60e
```

### 4.2 Desktop Environment

A lightweight XFCE4 desktop was installed:

```bash
sudo apt install xfce4 xfce4-terminal lightdm --no-install-recommends -y
sudo apt install xserver-xorg-video-fbdev xinit -y
```

Start the desktop:
```bash
startx
```

---

## 5. Digital Clock

### 5.1 Installation

```bash
sudo apt install python3-pygame -y
```

### 5.2 Clock Script

File: `/home/debian/clock.py`

---

## 6. Video Playback

### 6.1 Installation

`mpv` is video player we are using:

```bash
sudo apt install mpv -y
```

### 6.2 Transferring the Video File

Video was copied from the laptop to the BBB:

```bash
scp /home/lenovo/Downloads/video.mp4 debian@192.168.7.2:/home/debian/video.mp4
```


---

## 7. Clock and Video Side by Side

Both are launched simultaneously from startup.sh:

```bash
chmod +x /home/debian/startup.sh
./startup.sh
```

File: `startup.sh`

```sh
#!/bin/bash

#!/bin/bash

export DISPLAY=:0

cleanup() {
    echo "Gasim sve"
    kill $CLOCK_PID 2>/dev/null
    exit 0
}


trap cleanup SIGINT SIGTERM


python3 /home/debian/clock.py &
CLOCK_PID=$!

DISPLAY=:0 mpv --geometry=512x400+0+184 --no-border --loop --vf=scale=144:72 /home/debian/beagle.mp4


```




