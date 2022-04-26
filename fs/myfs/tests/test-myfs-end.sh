#!/bin/sh

set -ex

#include lib
. ./test-myfs-lib

echo "Clear all files for test"

# create virtual device
sudo losetup -d "$dev"
sudo rm -rf "$myfs_mnt"
sudo rm -f "$vdev"
