#!/bin/sh

set -ex

#include lib
. ./test-myfs-lib

echo "Setup before test : create virtual device"

# create virtual device
truncate --size=100M "$vdev"
sudo losetup "$dev" "$vdev"
