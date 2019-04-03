/* This file takes care of those system calls that deal with time.
 *
 * The entry points into this file are
 *   do_getres:		perform the CLOCK_GETRES system call
 *   do_gettime:	perform the CLOCK_GETTIME system call
 *   do_settime:	perform the CLOCK_SETTIME system call
 *   do_time:		perform the GETTIMEOFDAY system call
 *   do_stime:		perform the STIME system call
 *   do_distort_time: perform the DISTORT_TIME system call
 */

#include "pm.h"
#include <minix/callnr.h>
#include <minix/com.h>
#include <signal.h>
#include <sys/time.h>
#include "mproc.h"

/*===========================================================================*
 *				do_gettime				     *
 *===========================================================================*/
int do_gettime()
{
  clock_t ticks, realtime, clock;
  time_t boottime;
  int s;

  if ( (s=getuptime(&ticks, &realtime, &boottime)) != OK)
  	panic("do_time couldn't get uptime: %d", s);

  switch (m_in.m_lc_pm_time.clk_id) {
	case CLOCK_REALTIME:
		clock = realtime;
		break;
	case CLOCK_MONOTONIC:
		clock = ticks;
		break;
	default:
		return EINVAL; /* invalid/unsupported clock_id */
  }

  mp->mp_reply.m_pm_lc_time.sec = boottime + (clock / system_hz);
  mp->mp_reply.m_pm_lc_time.nsec =
	(uint32_t) ((clock % system_hz) * 1000000000ULL / system_hz);

  return(OK);
}

/*===========================================================================*
 *				do_getres				     *
 *===========================================================================*/
int do_getres()
{
  switch (m_in.m_lc_pm_time.clk_id) {
	case CLOCK_REALTIME:
	case CLOCK_MONOTONIC:
		/* tv_sec is always 0 since system_hz is an int */
		mp->mp_reply.m_pm_lc_time.sec = 0;
		mp->mp_reply.m_pm_lc_time.nsec = 1000000000 / system_hz;
		return(OK);
	default:
		return EINVAL; /* invalid/unsupported clock_id */
  }
}

/*===========================================================================*
 *				do_settime				     *
 *===========================================================================*/
static void reset_time_perception();

int do_settime()
{
  int s;

  reset_time_perception();

  if (mp->mp_effuid != SUPER_USER) { 
      return(EPERM);
  }

  switch (m_in.m_lc_pm_time.clk_id) {
	case CLOCK_REALTIME:
		s = sys_settime(m_in.m_lc_pm_time.now, m_in.m_lc_pm_time.clk_id,
			m_in.m_lc_pm_time.sec, m_in.m_lc_pm_time.nsec);
		return(s);
	case CLOCK_MONOTONIC: /* monotonic cannot be changed */
	default:
		return EINVAL; /* invalid/unsupported clock_id */
  }
}

/*===========================================================================*
 *				do_time					     *
 *===========================================================================*/
static void get_time_perception(mess_pm_lc_time* time, clock_t rt, time_t bt);

int do_time()
{
/* Perform the time(tp) system call. This returns the time in seconds since 
 * 1.1.1970.  MINIX is an astrophysically naive system that assumes the earth 
 * rotates at a constant rate and that such things as leap seconds do not 
 * exist.
 */
  clock_t ticks, realtime;
  time_t boottime;
  int s;

  if ( (s=getuptime(&ticks, &realtime, &boottime)) != OK)
  	panic("do_time couldn't get uptime: %d", s);

  get_time_perception(&(mp->mp_reply.m_pm_lc_time), realtime, boottime);
  return(OK);
}

/*===========================================================================*
 *				do_stime				     *
 *===========================================================================*/
int do_stime()
{
/* Perform the stime(tp) system call. Retrieve the system's uptime (ticks 
 * since boot) and pass the new time in seconds at system boot to the kernel.
 */
  clock_t uptime, realtime;
  time_t boottime;
  int s;

  if (mp->mp_effuid != SUPER_USER) { 
      return(EPERM);
  }
  if ( (s=getuptime(&uptime, &realtime, &boottime)) != OK) 
      panic("do_stime couldn't get uptime: %d", s);
  boottime = m_in.m_lc_pm_time.sec - (realtime/system_hz);

  s= sys_stime(boottime);		/* Tell kernel about boottime */
  if (s != OK)
	panic("pm: sys_stime failed: %d", s);

  return(OK);
}

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
  for (int i = 0; i < NR_PROCS; i++) {
    struct mproc* proc = &mproc[i];
    proc->mp_time_is_distorted = 0;
    proc->mp_time_scale = 0;
  }
}

static void get_time_perception(mess_pm_lc_time* time, clock_t rt, time_t bt)
{
  pid_t pid = mp->mp_pid;
  struct timespec now = { 
    .tv_sec = bt + (rt / system_hz),
    .tv_nsec = (uint32_t) ((rt % system_hz) * 1000000000ULL / system_hz),
  };

  if (mp->mp_time_is_distorted > 0) {

  } else {
    time->sec = now.tv_sec;
    time->nsec = now.tv_nsec;
  }
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

  printf("distort_time: caller - (%d), (id %d, par %d).\n", caller.pid, caller.id, caller.parent_id);
  printf("distort_time: target - (%d), (id %d, par %d).\n", target.pid, target.id, target.parent_id);

  int is_antecedent = is_ancestor(caller, target);
  int is_descendant = is_ancestor(target, caller);

  if (!is_descendant && !is_antecedent)
    return EPERM;

  float* p_scale = &mproc[target.id].mp_time_scale;
  *p_scale = scale > 0 ? (is_antecedent ? scale : (float) 1 / scale) : 0;

  mproc[target.id].mp_time_is_distorted = *p_scale != 1;

  return OK;
}
