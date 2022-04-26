#!/bin/sh

#include lib
. ./test-myfs-lib

# start test
echo "delete"

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
touch test.txt && echo "OK. File created." || echo "NOT OK. File creation failed."

# unmount filesystem
cd ..
umount "$myfs_mnt"

# mount filesystem
mount -t myfs "$dev" "$myfs_mnt"

# check whether dir is still there
ls "$myfs_mnt" | grep test.txt && echo "OK. File test.txt exists " || echo "NOT OK. File test.txt does not exist."

# change to myfs root folder
cd "$myfs_mnt"

# delete file
rm test.txt && echo "OK. File test.txt removed." || echo "NOT OK. File test.txt deletion failed."

# unmount filesystem
cd ..
umount "$myfs_mnt"

# mount filesystem
mount -t myfs "$dev" "$myfs_mnt"

# check whether file is still there
ls "$myfs_mnt" | grep test.txt && echo "NOT OK. File test.txt still exists." || echo "OK. File test.txt does not exists."

# unmount filesystem
umount "$myfs_mnt"

# unload module
rmmod myfs
