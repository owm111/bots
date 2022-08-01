/* Common code for initializing an arena and observer based on OMP_* variables.
 *
 * Call init_arenaptr(), use arenaptr->execute(...) to run parallel code, and
 * finally call fini_arenaptr().
 *
 * Assumes CPUs 2n and 2n+1 are siblings (forall n : N < nprocs / 2).
 * This can * be verified by checking the output following command: */
//	sort -nu /sys/devices/system/cpu/cpu*/topology/thread_siblings_list

#pragma once

#include <oneapi/tbb.h>

/* These are global, but shouldn't be constructed until init_queens */
extern oneapi::tbb::task_arena *arenaptr;
extern oneapi::tbb::task_scheduler_observer *observerptr;

void init_arenaptr();
void fini_arenaptr();
