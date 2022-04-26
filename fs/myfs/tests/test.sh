#!/bin/sh

echo "\nSTART TEST\n"
./test-myfs-start.sh

echo "\nTEST #0\n"
./test-myfs-0.sh

echo "\nTEST #1\n"
./test-myfs-1.sh

echo "\nTEST #2\n"
./test-myfs-2.sh

echo "\nTEST #3\n"
./test-myfs-3.sh

echo "\nTEST #4\n"
./test-myfs-4.sh

echo "\nTEST #5\n"
./test-myfs-5.sh

echo "\nSTART END\n"
./test-myfs-end.sh
