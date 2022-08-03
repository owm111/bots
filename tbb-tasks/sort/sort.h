#ifndef _SORT_H
#define _SORT_H

typedef long ELM;

void seqquick(ELM *low, ELM *high); 
void seqmerge(ELM *low1, ELM *high1, ELM *low2, ELM *high2, ELM *lowdest);
ELM *binsplit(ELM val, ELM *low, ELM *high); 
void cilkmerge(ELM *low1, ELM *high1, ELM *low2, ELM *high2, ELM *lowdest);
void cilkmerge_par(ELM *low1, ELM *high1, ELM *low2, ELM *high2, ELM *lowdest);
void cilksort(ELM *low, ELM *tmp, long size);
void cilksort_par(ELM *low, ELM *tmp, long size);
void scramble_array( ELM *array ); 
void fill_array( ELM *array ); 
void sort ( void ); 

extern "C" void sort_par (void);
extern "C" void sort_init (void);
extern "C" int sort_verify (void);
extern "C" void par_init();

#endif /* _SORT_H */
