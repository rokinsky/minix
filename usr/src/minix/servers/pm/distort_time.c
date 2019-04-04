/* This file takes care of those system calls that deal with time.
 *
 * The entry points into this file are
 *   do_distort_time: perform the DISTORT_TIME system call
 */

#include <stdbool.h>
#include "mproc.h"
#include "pm.h"

#define DT_NORMAL     0
#define DT_DISTORTED  1
#define DT_BENCHMARK  2

#define DT_ANTECEDENT 4
#define DT_DESCENDANT 8

#define DT_CHECK(flag,state) (((flag) & (state)) == (state))

/*===========================================================================*
 *				do_distort_time				*
 *===========================================================================*/
typedef struct {
  int id;
  pid_t pid;
  int parent_id;
} process;

static void lookup_mproc(process* p)
{
  p->id = -1;
  for (int i = 0; i < NR_PROCS; i++ ) {
    if (p->pid == mproc[i].mp_pid) {
      p->id = i;
      p->parent_id = mproc[i].mp_parent;
    }
  }
}

static void find_mprocs(process* caller, process* target)
{
  lookup_mproc(caller);
  lookup_mproc(target);
}

static void reset_time_perception()
{
  for (int i = 0; i < NR_PROCS; i++)
    mproc[i].mp_dt_flag = DT_NORMAL;
}

static void get_time_perception(mess_pm_lc_time* time, clock_t rt, time_t bt)
{
  pid_t pid = mp->mp_pid;
  struct timespec now = { 
    .tv_sec = bt + (rt / system_hz),
    .tv_nsec = (uint32_t) ((rt % system_hz) * 1000000000ULL / system_hz),
  };
  struct timespec res = now, bm = mp->mp_dt_benchmark;

  uint8_t flag = mp->mp_dt_flag;
  float scale = (float) mp->mp_dt_scale;

  if (DT_CHECK(flag, DT_DISTORTED) && scale != 1) {
    if (!DT_CHECK(flag, DT_BENCHMARK) {
      mp->mp_dt_flag |= DT_BENCHMARK;
      mp->mp_dt_benchmark = now;
    } else if (scale == 0) {
      res = bm;
    } else {
      bool is_antecedent = DT_CHECK(flag, DT_ANTECEDENT); 
      scale = is_antecedent ? (float) 1 / scale : scale;
      res.tv_sec = bm.tv_sec + (now.tv_sec - bm.tv_sec) * scale;
      res.tv_nsec = bm.tv_nsec + (now.tv_nsec - bm.tv_nsec) * scale;
    }
  }

  time->sec = res.tv_sec;
  time->nsec = res.tv_nsec;
}

static int is_ancestor(process candidate, process descendant)
{
  struct mproc proc = mproc[descendant.parent_id]; 

  /* init is the ancestor of every other process in the system */
  if (candidate.pid == 1)
    return 1;

  while (proc.mp_pid != 1) {
    if (candidate.pid == proc.mp_pid)
      return 1;
    proc = mproc[proc.mp_parent];
  }

  return 0;
}

int do_distort_time()
{
  process caller = { .pid = m_in.m1_i1 };
  process target = { .pid = m_in.m1_i2 };
  uint8_t scale = m_in.m1_i3;

  find_mprocs(&caller, &target);
  if (target.id < 0) 
    return EINVAL;
  else if (caller.id == target.id) 
    return EPERM;

  /* check caller's "family position", only one can be true */
  int is_cantecedent = is_ancestor(caller, target);
  int is_cdescendant = is_ancestor(target, caller);

  if (!is_cdescendant && !is_cantecedent)
    return EPERM;

  struct mproc* proc = &mproc[target.id];
  proc->mp_td_scale = scale;
  proc->mp_td_flag = DT_DISTORTED;
  proc->mp_td_flag |= is_cantecedent ? DT_DESCENDANT : DT_ANTECEDENT;

  return OK;
}
