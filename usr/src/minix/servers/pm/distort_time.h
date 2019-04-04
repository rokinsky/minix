#ifndef DISTORT_TIME_H
#define DISTORT_TIME_H

#include <minix/com.h>
#include <sys/time.h>

void get_time_perception(mess_pm_lc_time* time, clock_t rt, time_t bt);

void reset_time_perception();

#endif // DISTORT_TIME_H