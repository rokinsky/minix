#include <errno.h>
#include <inttypes.h>
#include <lib.h>
#include <minix/com.h>
#include <unistd.h>
#include <minix/syslib.h>

int distort_time(pid_t pid, uint8_t scale)
{
  message m = { .m1_i1 = pid, .m1_i2 = scale };

  // m->m_type = PM_DISTORT_TIME;
  // int status = ipc_sendrec(PM_PROC_NR, &m);
  // if (status != 0) return(status);
  // return(msgptr->m_type);

  return (_taskcall(PM_PROC_NR, PM_DISTORT_TIME, &m));

  // return (_syscall(PM_PROC_NR, PM_DISTORT_TIME, &m)) == 0 ? 0 : errno;
}
