#!/bin/sh

#include lib
. ./test-myfs-lib

# start test
echo "open and create"

# load module
insmod "$module_file"

# create mount point
mkdir -p "$myfs_mnt"

# format partition
./mkfs.myfs "$dev"

# mount filesystem
mount -t myfs "$dev" "$myfs_mnt"

# change to myfs root folder & list all filesystem files
cd "$myfs_mnt"
ls -la

# create new file
touch b.txt && echo "OK. File created." || echo "NOT OK. File creation failed."

# unmount filesystem
cd ..
umount "$myfs_mnt"

# mount filesystem
mount -t myfs "$dev" "$myfs_mnt"

# check whether b.txt is still there
ls "$myfs_mnt" | grep b.txt && echo "OK. File b.txt exists " || echo "NOT OK. File b.txt does not exist."

# unmount filesystem
umount "$myfs_mnt"

# unload module
rmmod myfs
