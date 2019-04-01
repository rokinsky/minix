#include <lib.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <minix/rs.h>

int distort_time(pid_t pid, uint8_t scale)
{
	message m;
	m.m1_i1 = getpid();
	m.m1_i2 = pid;
	m.m1_i3 = scale;

	return (_syscall(PM_PROC_NR, PM_DISTORT_TIME, &m));
}
