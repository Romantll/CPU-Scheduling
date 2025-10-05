#include <string.h>
#include "pslibrary.h"

#define READY 0
#define RUNNING 1
#define WAITING 2
#define DONE 3


//Timeline strings output character
static inline char state_char(int state){
	if (state == READY) return 'r';
	if (state == RUNNING) return 'R';
   	if (state == WAITING) return 'w';
	return '\0';
}	

typedef struct{
	int cpu, io, cpu2;
	int stage;
	int state;
}P;


static void initP(P *p, int x, int y, int z){
	p->cpu1 = x;
	p->io = y;
	p->cpu2 = z;
	p->stage = 0;
	p-> state = READY;
}

static int next_cpu_left(const P *p){
	if (p->stage == 0) return p->cpu1;
	if(p->stage == 2) return p->cpu2;
	return 0;
}


static void finish_cpu_done(P *p, char *s, int i){
	if (p->state != RUNNING) return;

	if (p->stage == 0 && p->cpu1 ==0){
		if (p->io == 0){
			p->stage = 3;
			p->state = DONE;
			s[i] = '\0';
		}else{
			p->stage = 1;
			p->state = WAITING;
		}
	}else if (p->stage == 2 && p-> cpu2 == 0){
		p->stage =3;
		p->state = DONE;
		s[i] = '\0';
	}
}

static void finish_io_done(P* p){
	if(p->state == WAITING && p->stage == 1 && p->io == 0){
		p->stage = 2;
		p->state = READY;
	}
}

/* ------------------------------------------------------------------
 * Selection helpers (assign_fcfs/sjf/psjf/rr)
 * For clarity and cleanliness, the per-tick selection policy
 * is factored out of each scheduler's main loop. All schedulers
 * share the same tick order:
 * 	1. Finish CPU 2. Finish IO 3. Choose runner 4. emit 5. decrement
 * 	Only the selection rule differs.
 * 	Factoring it out reduces duplication and bugs. Overhead is trivial.
 * 	------------------------------------------------------------------*/

static void assign_fcfs(P *a, P *b){

	if(a->state != RUNNING && b->state != RUNNING){
		if(a->state == READY && a->stage ! = 3){
			a->state = RUNNING;
		}else if(b->state == READY && b->stage !=3){
				b->state = RUNNING;
		}
	}
}



