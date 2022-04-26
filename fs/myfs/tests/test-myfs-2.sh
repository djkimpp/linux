#!/bin/sh

#include lib
. ./test-myfs-lib

# start test
echo "read and write"

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

# create new file
touch c.txt && echo "OK. File created." || echo "NOT OK. File creation failed."
echo "Hello~!" > c.txt

# unmount filesystem
cd ..
umount "$myfs_mnt"

# mount filesystem
mount -t myfs "$dev" "$myfs_mnt"

# check whether c.txt is still there
ls "$myfs_mnt" | grep c.txt && echo "OK. File b.txt exists " || echo "NOT OK. File b.txt does not exist."

# read c.txt file
cat "$myfs_mnt"/c.txt

# unmount filesystem
umount "$myfs_mnt"

# unload module
rmmod myfs
