#include <inttypes.h>
#include <lib.h>
#include <unistd.h>

int distort_time(pid_t pid, uint8_t scale)
{
  message m = { .m1_i1 = pid, .m1_i2 = scale, .m_type = PM_DISTORT_TIME };

  /*
   * I get undefined reference to `_taskcall'
   * if I try to use `_taskcall` from <minix/syslib.h>.
   */
  int status = ipc_sendrec(PM_PROC_NR, &m);
  if (status != 0) return status;
  return -m.m_type;
}
