#!/bin/bash

cp usr/src/minix/drivers/adler/adler.c test_data/
# Cleanup
ssh minix "rm -rf ~/task6"
ssh minix "rm -rf /usr/src/minix/drivers/adler"

# Copy solution and test data to VM
scp -r $(dirname $0)/test_data minix:~/task6

# Apply changes
ssh minix "cp ~/task6/system.conf /etc/system.conf"
ssh minix "mkdir /usr/src/minix/drivers/adler;"
ssh minix "cp ~/task6/adler.c /usr/src/minix/drivers/adler"
ssh minix "cp ~/task6/Makefile /usr/src/minix/drivers/adler"
ssh minix "cd /usr/src/minix/drivers/adler;
		make clean;
        make;
        make install;

        service up /service/adler;
        service update /service/adler;
        service down adler;"

ssh minix "~/task6/run_test.sh"
