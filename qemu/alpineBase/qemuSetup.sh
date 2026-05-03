#!/bin/bash

# First time installation
qemu-system-arm -M virt \
  -cpu cortex-a15 \
  -m 512M \
  -kernel vmlinuz-virt \
  -initrd initramfs-virt \
  -drive file=alpine-virt-3.23.4-armv7.iso,id=cdrom,if=none,format=raw \
  -device virtio-blk-device,drive=cdrom \
  -drive file=mydisk.qcow2,if=none,id=hd0 \
  -device virtio-blk-device,drive=hd0 \
  -append "console=ttyAMA0" \
  -nographic 


# Regular run 
qemu-system-arm -M virt \
  -cpu cortex-a15 \
  -m 512M \
  -kernel vmlinuz-virt \
  -initrd initramfs-virt \
  -drive file=mydisk.qcow2,if=none,id=hd0 \
  -device virtio-blk-device,drive=hd0 \
  -append "console=ttyAMA0 root=/dev/vda3 rootfstype=ext4" \
  -netdev user,id=net0,hostfwd=tcp::2222-:22 \
  -device virtio-net-device,netdev=net0 \
  -device nec-usb-xhci \
  -device usb-kbd \
  -device usb-mouse \
  -device virtio-gpu-pci,xres=1920,yres=1080 \
  -display sdl \
  -serial stdio


# Additional commands inside Alpine VM (if connecting via ssh not working):
# ip addr add 10.0.2.15/24 dev eth0
# ip link set eth0 up

# Connecting to VM: ssh -p 2222 root@localhost
# Copying file to VM: scp -P 2222 <file_to_copy> root@localhost:/root/