#!/bin/sh

#include lib
. ./test-myfs-lib

# start test
echo "mkdir and rmdir"

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

# create new directories
dir="$myfs_mnt"
for var in a b c d
do
	dir="$dir"/$var
	mkdir $dir && echo "OK. Directory ($dir) created." || echo "NOT OK. Directory ($dir) creation failed." 
done

# create new filesystem
dir="$myfs_mnt"
for var in 1 2 3 4 5 6 7 8 9
do
	file="$dir"/a/$var 
	touch $file && echo "OK. File ($file) created." || echo "NOT OK. File ($file) creation failed."
done

# unmount filesystem
cd ..
umount "$myfs_mnt"

# mount filesystem
mount -t myfs "$dev" "$myfs_mnt"
# check whether dir is still there
dir="$myfs_mnt"
for var in a b c d
do
	dir="$dir"/$var
	ls $dir && echo "OK. Directory ($dir) exists." || echo "NOT OK. Directory ($dir) does not exits." 
done

# change to myfs root folder
cd "$myfs_mnt"

# delete directory
rmdir $dir && echo "OK. Directory ($dir) has been removed." || echo "NOT OK. Directory ($dir) deletion failed."

# unmount filesystem
cd ..
umount "$myfs_mnt"

# mount filesystem
mount -t myfs "$dev" "$myfs_mnt"

# check whether dir is still there
ls $dir && echo "NOT OK. Directory ($dir) still exist." || echo "OK. Directory ($dir) does not exist."

# unmount filesystem
umount "$myfs_mnt"

# unload module
rmmod myfs
