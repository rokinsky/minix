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

extern clock_t get_time_perception(clock_t realtime)
{
  uint8_t flag = mp->mp_dt_flag;
  float scale = mp->mp_dt_scale;
  clock_t benchmark = mp->mp_dt_benchmark;

  if (!DT_CHECK(flag, DT_DISTORTED) || scale == 1) {
    /* Nothing happens. */
    return realtime;
  } else if (!DT_CHECK(flag, DT_BENCHMARK)) {
    /* Set the starting point. */
    mp->mp_dt_flag |= DT_BENCHMARK;
    mp->mp_dt_benchmark = realtime;
    return realtime;
  } else if (scale == 0) {
    /* Time is frozen. */
    return benchmark;
  } else {
    /* Let's distort! */
    bool is_antecedent = DT_CHECK(flag, DT_ANTECEDENT); 
    scale = is_antecedent ? scale : 1 / scale;
    return benchmark + (realtime - benchmark) * scale;
  }
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
    return true;

  /* Check until it meet with init. */
  while (proc.mp_pid != 1) {
    if (candidate.pid == proc.mp_pid)
      return true;
    proc = mproc[proc.mp_parent];
  }

  return false;
}

static bool lookup_mproc(process* p)
{
  for (int i = 0; i < NR_PROCS; i++ ) {
    if (p->pid == mproc[i].mp_pid) {
      p->id = i;
      p->parent_id = mproc[i].mp_parent;
      return true;
    }
  }
  return false;
}

int do_distort_time()
{
  process caller = { .pid = mp->mp_pid };
  process target = { .pid = m_in.m1_i1 };
  uint8_t scale = m_in.m1_i2;

  lookup_mproc(&caller);

  if (!lookup_mproc(&target))
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
