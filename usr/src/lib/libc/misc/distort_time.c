#include <errno.h>
#include <inttypes.h>
#include <lib.h>
#include <minix/com.h>
#include <unistd.h>

int distort_time(pid_t pid, uint8_t scale)
{
  message m;
  m.m1_i1 = pid;
  m.m1_i2 = scale;

  return (_syscall(PM_PROC_NR, PM_DISTORT_TIME, &m)) == 0 ? 0 : errno;
}
