#include <lib.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <minix/rs.h>

int get_pm_endpt(endpoint_t *pt)
{
	return minix_rs_lookup("pm", pt);
}

int distort_time(pid_t pid, uint8_t scale)
{
	endpoint_t pm_pt;
	message m;

	if (get_pm_endpt(&pm_pt) != 0)
	{
		errno = ENOSYS;
		return -1;
	}
	return (_syscall(pm_pt, PM_DISTORT_TIME, &m));
}
