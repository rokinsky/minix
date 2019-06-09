#!/usr/pkg/bin/bash

# Color codes
RESET="\033[0m"
RED="\033[0;31m"
GREEN="\033[01;32m"

# Prepare
/sbin/mknod /dev/adler c 20 0;
service up /service/adler -dev /dev/adler;

# Test helper
NUM=1
function try {
	GOT=$(head -c 8 /dev/adler | xargs echo)
    if [ "$GOT" = "$1" ];
    then
    	echo -e $GREEN"TEST "$NUM" PASSED"$RESET
    else
    	echo -e $RED"TEST "$NUM" FAILED"$RESET" GOT $GOT EXPECTED $1"
    fi
    NUM=$((NUM+1))
}

function tryTooSmallSize {
	GOT=$(dd if=/dev/adler bs=$1 count=1 2>&1 | grep "Invalid")
	if [ "$GOT" = "dd: Read error: Invalid argument" ];
	then
		echo -e $GREEN"SMALL READ SIZE TEST "$NUM" PASSED"$RESET
	else
		echo -e $RED"SMALL READ SIZE TEST "$NUM" FAILED"$RESET" READ SHOULD FAIL WITH EINVAL"
	fi
	NUM=$((NUM+1))
}

function tryTooBigSize {
	GOT=$(dd if=/dev/adler bs=$1 count=1 2>/dev/null)
	if [ "$GOT" = "00000001" ];
	then
		echo -e $GREEN"BIG READ SIZE TEST "$NUM" PASSED"$RESET
	else
		echo -e $RED"BIG READ SIZE TEST "$NUM" FAILED"$RESET" READ SHOULD FAIL WITH EINVAL"
	fi
	NUM=$((NUM+1))
}

# No data
try 00000001

# Hash "Hello\n"
echo "Hello" > /dev/adler
try 078b01ff

# After read the hash should reset
try 00000001

# Updating the service preserves the hash
echo "Hello" > /dev/adler
service update /service/adler -dev /dev/adler;
try 078b01ff

# Longer string
echo -n "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vestibulum tempus." > /dev/adler;
try 2ab01c33

# Fragmentation of string
echo -n "Lorem ipsum dolor sit amet, consect" > /dev/adler;
echo -n "etur adipiscing elit. Vestibulum tempus." > /dev/adler;
try 2ab01c33

# Write, update, write, read
echo -n "Lorem ipsum dolor sit amet, consect" > /dev/adler;
service update /service/adler -dev /dev/adler;
echo -n "etur adipiscing elit. Vestibulum tempus." > /dev/adler;
try 2ab01c33

# 1 MB file
cat /root/task6/to_hash1 > /dev/adler
try ee873cfb

# The hash was reset after read
try 00000001

# 2 MB random binary data
# This test can detect the usage of char instead of unsigned char
# inside the write buffer
cat /root/task6/to_hash2 > /dev/adler
try 3a47dd5f

# Check read size
for i in {1..7}
do
   tryTooSmallSize $i
done

for i in {8..20}
do
   tryTooBigSize $i
done

# Cleanup
rm /dev/adler
service down adler
