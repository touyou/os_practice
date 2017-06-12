#!/usr/bin/zsh

sudo kvm -drive file=rootfs-2014.img -m 256 -smp 2 -kernel mylinux/arch/x86/boot/bzImage -append "root=/dev/sda1" -nographic
