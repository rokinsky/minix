/*===========================================================================*
 * This file takes care of system call that deal with distort time.
 *
 * The entry points into this file is
 *   do_distort_time: perform the DISTORT_TIME system call
 *===========================================================================*/

#include <stdbool.h>
#include "pm.h"
#include "mproc.h"

/* Brackets are never redundant here, states are below. */
#define DT_CHECK(flag, state) (((flag) & (state)) == (state))

#define DT_NORMAL     0 /* Everything is as usual. */
#define DT_DISTORTED  1 /* Someone has distorted, but there is no benchmark. */
#define DT_BENCHMARK  2 /* Benchmark is already there. */

/* From whom in the family tree got distorted. */
#define DT_ANTECEDENT 4
#define DT_DESCENDANT 8

/* Unnecessary structure, but it's comfortable. */
typedef struct {
  int id;
  pid_t pid;
  int parent_id;
} process;

extern void get_time_perception(mess_pm_lc_time* time, clock_t rt, time_t bt)
{
  clock_t res = rt, bm = mp->mp_dt_benchmark;

  uint8_t flag = mp->mp_dt_flag;
  float scale = mp->mp_dt_scale;

  if (DT_CHECK(flag, DT_DISTORTED) && scale != 1) {
    if (!DT_CHECK(flag, DT_BENCHMARK)) {
      /* Set the starting point. */
      mp->mp_dt_flag |= DT_BENCHMARK;
      mp->mp_dt_benchmark = rt;
    } else if (scale == 0) {
      /* Time is frozen. */
      res = bm;
    } else {
      /* Let's distort! */
      bool is_antecedent = DT_CHECK(flag, DT_ANTECEDENT); 
      scale = is_antecedent ? scale : (1 / scale);
      res = bm + ((rt - bm) * scale);
    }
  }

  /* Beauty is not important. */
  time->sec = bt + (res / system_hz);
  time->nsec = (uint32_t) ((res % system_hz) * 1000000000ULL / system_hz);
}

extern void reset_time_perception()
{
  for (int i = 0; i < NR_PROCS; i++)
    mproc[i].mp_dt_flag ^= DT_BENCHMARK;
}

static bool is_ancestor(process candidate, process descendant)
{
  struct mproc proc = mproc[descendant.parent_id]; 

  /* Fact: init is the ancestor of every other process in the system. */
  if (candidate.pid == 1)
    return 1;

  /* Until we meet with init. It could have been a recursion, is not it? */
  while (proc.mp_pid != 1) {
    if (candidate.pid == proc.mp_pid)
      return true;
    proc = mproc[proc.mp_parent];
  }

  return false;
}

static int lookup_mproc(process* p)
{
  for (int i = 0; i < NR_PROCS; i++ ) {
    if (p->pid == mproc[i].mp_pid) {
      p->id = i;
      p->parent_id = mproc[i].mp_parent;
      return 0;
    }
  }
  return -1;
}

int do_distort_time()
{
  process caller = { .pid = mp->mp_pid };
  process target = { .pid = m_in.m1_i1 };
  uint8_t scale = m_in.m1_i2;

  lookup_mproc(caller);

  if (lookup_mproc(target) < 0)
    return EINVAL; /* The target not found. */
  else if (caller.id == target.id)
    return EPERM; /* The caller cannot distort itself. */

  /* Check caller's "family position", only one can be true. */
  bool is_antecedent = is_ancestor(caller, target);
  bool is_descendant = is_ancestor(target, caller);

  if (!is_descendant && !is_antecedent)
    return EPERM; /* The target is not from caller's family. */

  /* Finally... */
  struct mproc* proc = &mproc[target.id];
  proc->mp_dt_scale = scale;
  proc->mp_dt_flag = DT_DISTORTED;
  proc->mp_dt_flag |= is_antecedent ? DT_ANTECEDENT : DT_DESCENDANT;

  return OK;
}
