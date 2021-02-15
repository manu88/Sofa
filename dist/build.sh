#!/bin/sh


IMG_NAME=sofa.img
MOUNT_PATH=mountPt

dd if=/dev/zero of=$IMG_NAME bs=400M count=1
mkfs.ext2 $IMG_NAME
mkdir -p $MOUNT_PATH

echo "----> mount stuff"
losetup -fP $IMG_NAME

LOOP_ID=$(losetup -f --show -P $IMG_NAME)
[ $? -eq 0 ] || exit 1

echo $LOOP_ID

mount -o loop $LOOP_ID $MOUNT_PATH
#extlinux --install $MOUNT_PATH
echo "----> copy files"
mkdir -p $MOUNT_PATH/boot/grub
cp /usr/lib/syslinux/modules/bios/mboot.c32 $MOUNT_PATH
cp /usr/lib/syslinux/modules/bios/libcom32.c32 $MOUNT_PATH
cp /usr/lib/syslinux/modules/bios/vesamenu.c32 $MOUNT_PATH
cp /usr/lib/syslinux/modules/bios/libutil.c32 $MOUNT_PATH
cp ../build/images/kernel-x86_64-pc99 $MOUNT_PATH/kernel
cp ../build/images/kernel_task-image-x86_64-pc99 $MOUNT_PATH/rootserver




#correct all file rights
#sudo chmod -R +r $MOUNT_PATH

# Do what you want with the mounted image :) 



cp -R root/* $MOUNT_PATH

exit
echo "----> unmount stuff"

sudo umount $MOUNT_PATH
rm -r $MOUNT_PATH
sudo losetup -d $LOOP_ID

sudo chmod a+rwX sofa.img
echo "----> done"
