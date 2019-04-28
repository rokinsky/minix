#include <inttypes.h>
#include <lib.h>
#include <unistd.h>

static int _distortcall(message *m)
{
  m->m_type = PM_DISTORT_TIME;

  int status = ipc_sendrec(PM_PROC_NR, m);
  if (status != 0) m->m_type = status;

  return m->m_type < 0 ? -m->m_type : m->m_type;
}

int distort_time(pid_t pid, uint8_t scale)
{
  message m = { .m1_i1 = pid, .m1_i2 = scale };

  return _distortcall(&m);
}
