#!/bin/sh

#include lib
. ./test-myfs-lib

# start test
echo "rename and move"

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

# create new directory
mkdir dir1 && echo "OK. Directory1 created." || echo "NOT OK. Directory1 creation failed."
mkdir dir2 && echo "OK. Directory2 created." || echo "NOT OK. Directory2 creation failed."

# unmount filesystem
cd ..
umount "$myfs_mnt"

# mount filesystem
mount -t myfs "$dev" "$myfs_mnt"

# check whether directory is still there
ls "$myfs_mnt" | grep dir1 && echo "OK. Deriectory1 exists." || echo "NOT OK. Directory1 does not exist."
ls "$myfs_mnt" | grep dir2 && echo "OK. Deriectory2 exists." || echo "NOT OK. Directory2 does not exist."

# rename directory
mv "$myfs_mnt"/dir2 "$myfs_mnt"/subdir && echo "OK. Change dir2 to subdir." || echo "NOT OK. dir2 does not changed."

# unmount filesystem
umount /mnt/myfs

# mount filesystem
mount -t myfs "$dev" "$myfs_mnt"

# check whether directory is still there
ls "$myfs_mnt" | grep subdir && echo "OK. Deriectory1 exists." || echo "NOT OK. Directory1 does not exist."

# change to directory
cd "$myfs_mnt" 

# create new file
touch file.txt && echo "OK. File created." || echo "NOT OK. File creation failed."

# unmount filesystem
cd ..
umount "$myfs_mnt"

# mount filesystem
mount -t myfs "$dev" "$myfs_mnt"

# check whether file is still there
ls "$myfs_mnt" | grep file.txt && echo "OK. File exists " || echo "NOT OK. File does not exist."

# move file
mv "$myfs_mnt"/file.txt "$myfs_mnt"/subdir && echo "OK. File moved." || echo "NOT OK. Failed to move file."

# unmount filesystem
cd ..
umount "$myfs_mnt"

# mount filesystem
mount -t myfs "$dev" "$myfs_mnt"

# check whether directory and file are still there
ls "$myfs_mnt" | grep subdir && echo "OK. Subdirectory exists " || echo "NOT OK. Subdirectory does not exist."
ls "$myfs_mnt" | grep file.txt && echo "OK. File exists " || echo "NOT OK. File does not exist."

# unmount filesystem
umount "$myfs_mnt"

# unload module
rmmod myfs
