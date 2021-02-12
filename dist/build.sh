#!/bin/sh


IMG_NAME=sofa.img
MOUNT_PATH=mountPt

dd if=/dev/zero of=$IMG_NAME bs=400M count=1
mkfs.fat $IMG_NAME
syslinux $IMG_NAME
mkdir -p $MOUNT_PATH

echo "----> mount stuff"
losetup -fP $IMG_NAME
mount -o loop /dev/loop0 $MOUNT_PATH

echo "----> copy files"

cp /usr/lib/syslinux/modules/bios/mboot.c32 $MOUNT_PATH
cp /usr/lib/syslinux/modules/bios/libcom32.c32 $MOUNT_PATH
cp ../build/kernel-x86_64-pc99 $MOUNT_PATH/kernel
cp ../build/kernel_task-image-x86_64-pc99 $MOUNT_PATH/rootserver
cat > $MOUNT_PATH/syslinux.cfg <<EOF
SERIAL 0 115200
DEFAULT seL4test
LABEL seL4test
kernel mboot.c32
append kernel --- rootserver
EOF


#correct all file rights
#sudo chmod -R +r $MOUNT_PATH

# Do what you want with the mounted image :) 

#
echo "----> unmount stuff"

sudo umount $MOUNT_PATH
rm -r $MOUNT_PATH
sudo losetup -d /dev/loop0

sudo chmod a+rwX sofa.img
echo "----> done"