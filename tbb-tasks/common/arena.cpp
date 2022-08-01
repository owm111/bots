#include "arena.h"
#include "bots.h"

#include <cstdlib>
#include <sched.h>
#include <sys/sysinfo.h>

/* This observer won't work so well if an arena is reused */
class pinning_observer : public oneapi::tbb::task_scheduler_observer {
	const unsigned nprocs;
	const unsigned shift = 0;
	unsigned counter = 0;
public:
	pinning_observer(oneapi::tbb::task_arena &a, bool cores_only = false)
		: oneapi::tbb::task_scheduler_observer(a),
		shift(cores_only ? 1 : 0), nprocs(get_nprocs())
	{
		observe(true);
	}
	void on_scheduler_entry(bool worker) override {
		cpu_set_t *mask = CPU_ALLOC(nprocs);
		auto mask_size = CPU_ALLOC_SIZE(nprocs);
		int cpu_number = (counter++ << shift) % nprocs;
		CPU_ZERO_S(mask_size, mask);
		CPU_SET_S(cpu_number, mask_size, mask);
		if (sched_setaffinity(0, mask_size, mask)) {
			perror("sched_setaffinity");
			exit(EXIT_FAILURE);
		}
		bots_debug("Scheduled a thread on CPU %d\n", cpu_number);
	}
	void on_scheduler_exit(bool worker) override {
		bots_debug("Scheduler exited (worker = %d)\n", worker);
	}
};

oneapi::tbb::task_arena *arenaptr;
oneapi::tbb::task_scheduler_observer *observerptr;

void
init_arenaptr()
{
	int default_conc = get_nprocs();

	char *places_a = std::getenv("OMP_PLACES");
	bool cores_only = false;
	if (places_a == NULL || places_a[0] == 't') { /* threads */
		/* leave the default */
	} else { /* assume cores otherwise */
		cores_only = true;
		default_conc >>= 1;
	}

	char *num_threads_a = std::getenv("OMP_NUM_THREADS");
	int max_concurrency;
	if (num_threads_a) {
		int i = std::atoi(num_threads_a);
		if (i < 1)
			i = 1;
		max_concurrency = i;
	} else {
		max_concurrency = default_conc;
	}

	arenaptr = new oneapi::tbb::task_arena(max_concurrency);

	char *proc_bind_a = std::getenv("OMP_PROC_BIND");
	oneapi::tbb::task_scheduler_observer *observer;
	if (proc_bind_a && proc_bind_a[0] == 'c') { /* close */
		bots_debug("Constructed pinning observer\n");
		observerptr = new pinning_observer(*arenaptr, cores_only);
	} else { /* ignore other settings */
		/* Don't care */
		bots_debug("default observer\n");
		observerptr = new oneapi::tbb::task_scheduler_observer(*arenaptr);
	}
}

void
fini_arenaptr()
{
	bots_debug("Calling cleanup code\n");
	delete observerptr;
	delete arenaptr;
}
