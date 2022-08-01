/**********************************************************************************************/
/*  This program is part of the Barcelona OpenMP Tasks Suite                                  */
/*  Copyright (C) 2009 Barcelona Supercomputing Center - Centro Nacional de Supercomputacion  */
/*  Copyright (C) 2009 Universitat Politecnica de Catalunya                                   */
/*                                                                                            */
/*  This program is free software; you can redistribute it and/or modify                      */
/*  it under the terms of the GNU General Public License as published by                      */
/*  the Free Software Foundation; either version 2 of the License, or                         */
/*  (at your option) any later version.                                                       */
/*                                                                                            */
/*  This program is distributed in the hope that it will be useful,                           */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of                            */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                             */
/*  GNU General Public License for more details.                                              */
/*                                                                                            */
/*  You should have received a copy of the GNU General Public License                         */
/*  along with this program; if not, write to the Free Software                               */
/*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA            */
/**********************************************************************************************/

/*
 * Original code from the Cilk project (by Keith Randall)
 * 
 * Copyright (c) 2000 Massachusetts Institute of Technology
 * Copyright (c) 2000 Matteo Frigo
 */

#include <atomic>
#include <oneapi/tbb.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <alloca.h>
#include "bots.h"
#include "arena.h"


/* Checking information */

static int solutions[] = {
        1,
        0,
        0,
        2,
        10, /* 5 */
        4,
        40,
        92,
        352,
        724, /* 10 */
        2680,
        14200,
        73712,
        365596,
};
#define MAX_SOLUTIONS sizeof(solutions)/sizeof(int)

/* XXX: Using std::atomic instead of omp atomic */
std::atomic<int> total_count;

/*
 * <a> contains array of <n> queen positions.  Returns 1
 * if none of the queens conflict, and returns 0 otherwise.
 */
int ok(int n, char *a)
{
     int i, j;
     char p, q;

     for (i = 0; i < n; i++) {
	  p = a[i];

	  for (j = i + 1; j < n; j++) {
	       q = a[j];
	       if (q == p || q == p - (j - i) || q == p + (j - i))
		    return 0;
	  }
     }
     return 1;
}

void nqueens_ser (int n, int j, char *a, std::atomic<int> *solutions)
{
	std::atomic<int> res;
	int i;

	if (n == j) {
		/* good solution, count it */
		*solutions = 1;
		return;
	}

	*solutions = 0;

     	/* try each possible position for queen <j> */
	for (i = 0; i < n; i++) {
		{
	  		/* allocate a temporary array and copy <a> into it */
	  		a[j] = (char) i;
	  		if (ok(j + 1, a)) {
	       			nqueens_ser(n, j + 1, a,&res);
				*solutions += res;
			}
		}
	}
}

#if defined(MANUAL_CUTOFF)

void nqueens(int n, int j, char *a, std::atomic<int> *solutions, int depth)
{
	std::atomic<int> *csols;
	bots_debug_with_location_info("entered nqueens()");
	int i;


	if (n == j) {
		/* good solution, count it */
		*solutions = 1;
		return;
	}


	*solutions = 0;
	csols = (std::atomic<int> *)alloca(n*sizeof(int));
	memset(csols,0,n*sizeof(int));

	oneapi::tbb::task_group g;
     	/* try each possible position for queen <j> */
	for (i = 0; i < n; i++) {
		if ( depth < bots_cutoff_value ) {
			g.run([=, &csols] {
	  			/* allocate a temporary array and copy <a> into it */
	  			char * b = (char *)alloca(n * sizeof(char));
	  			memcpy(b, a, j * sizeof(char));
	  			b[j] = (char) i;
	  			if (ok(j + 1, b))
	       				nqueens(n, j + 1, b,&csols[i],depth+1);
			});
		} else {
  			a[j] = (char) i;
  			if (ok(j + 1, a))
       				nqueens_ser(n, j + 1, a,&csols[i]);
		}
	}

	g.wait();
	for ( i = 0; i < n; i++) *solutions += csols[i];
}


#else 

void nqueens(int n, int j, char *a, std::atomic<int> *solutions, int depth)
{
	bots_debug_with_location_info("entered nqueens(), *solutions = %d\n",
			solutions->load());
	std::atomic<int> *csols;
	int i;


	if (n == j) {
		/* good solution, count it */
		*solutions = 1;
	bots_debug_with_location_info("good solution, count it, *solutions = %d\n",
			solutions->load());
		return;
	}


	*solutions = 0;
	csols = (std::atomic<int> *)alloca(n*sizeof(int));
	memset(csols,0,n*sizeof(int));

	oneapi::tbb::task_group g;
     	/* try each possible position for queen <j> */
	for (i = 0; i < n; i++) {
		g.run([=, &csols] {
	  		/* allocate a temporary array and copy <a> into it */
	  		char * b = (char *)alloca(n * sizeof(char));
	  		memcpy(b, a, j * sizeof(char));
	  		b[j] = (char) i;
	  		if (ok(j + 1, b))
       				nqueens(n, j + 1, b,&csols[i],depth); //FIXME: depth or depth+1 ???
		});
	}

	g.wait();
	for ( i = 0; i < n; i++) *solutions += csols[i];
	bots_debug_with_location_info("leaving nqueens, *solutions = %d\n",
			solutions->load());
}

#endif


extern "C" void
init_queens()
{
	init_arenaptr();
}

extern "C" void
cleanup_queens()
{
	fini_arenaptr();
}

extern "C" void find_queens (int size)
{
	total_count=0;

        bots_message("Computing N-Queens algorithm (n=%d) ", size);
	arenaptr->execute([=] {
		{
			char *a;

			a = (char *)alloca(size * sizeof(char));
			nqueens(size, 0, a, &total_count,0);
		}
	});
	bots_message(" completed!\n");
}


extern "C" int verify_queens (int size)
{
	if ( size > MAX_SOLUTIONS ) return BOTS_RESULT_NA;
	if ( total_count == solutions[size-1]) return BOTS_RESULT_SUCCESSFUL;
	return BOTS_RESULT_UNSUCCESSFUL;
}
