#!/bin/sh

#include lib
. ./test-myfs-lib

# start test
echo "mount and umount"

# load module
insmod "$module_file"

# create mount point
mkdir -p "$myfs_mnt"

# format partition
./mkfs.myfs "$dev"

# mount filesystem
mount -t myfs "$dev" "$myfs_mnt"

# show registered filesystems
cat /proc/filesystems | grep myfs

# show mounted filesystems
cat /proc/mounts | grep myfs

# list all filesystem files
ls -la "$myfs_mnt"
:<<END
# umount filesystem
umount "$myfs_mnt"

# unload module
rmmod myfs
END
