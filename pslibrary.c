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
	int cpu1, io, cpu2;
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
		if(a->state == READY && a->stage != 3){
			a->state = RUNNING;
		}else if(b->state == READY && b->stage !=3){
				b->state = RUNNING;
		}
	}
}


//SJF selection
static void assign_sjf(P *a, P *b){
	if(a->state != RUNNING && b->state != RUNNING) {
		int ra = (a->state == READY && a->stage != 3);
		int rb = (b->state == READY && b->stage !=3);
		if (ra && rb) {
			int la = next_cpu_left(a), lb = next_cpu_left(b);
			if(la <= lb) a->state = RUNNING; else b->state = RUNNING;
		}else if (ra) {a->state = RUNNING;}
		else if (rb) {b->state = RUNNING;}
	}
}


//PSJF selection
static void assign_psjf(P *a, P *b){
	int ra = (a->state == READY && a->stage != 3);
	int rb = (b->state == READY && b->stage !=3);
	
	//If both processes not ready do nothing
	if(!ra && !rb) {return;}

	int la = (a->state == RUNNING ? next_cpu_left(a) : (ra ? next_cpu_left(a) : 0x7fffffff));
	int lb = (b->state == RUNNING ? next_cpu_left(b) : (rb ? next_cpu_left(b) : 0x7fffffff));

	if (a->state == RUNNING) a->state = READY;
	if (b->state == RUNNING) b->state = READY;

	if (la <= lb) {a->state = RUNNING; }
	else		  {b->state = RUNNING; }
}

//RR selection
static void assign_rr(P *a, P *b, int *current, int *qleft, int q){
	if(*current == 0){
		if(a->state == READY && a->stage != 3){
			*current = 1;
			a->state = RUNNING;
			*qleft = q;
		}else if (b->state == READY && b->stage != 3){
			*current = 2;
			b->state = RUNNING;
			*qleft = q;
			return;
		}
	}
}

static void emit_chars(P *p, char *s, int i){
	if(p->state != DONE) s[i] = state_char(p->state);
}

//Decrements counters based on process state
static void dec_counter(P *p){
	if(p->state == RUNNING){
		if (p->stage == 0 && p->cpu1 > 0) p->cpu1--;
		else if (p->stage == 2 && p->cpu2 > 0) p->cpu2--;
	}else if (p->state == WAITING){
		if(p->stage == 1 && p->io > 0) p->io--;
	}
}

//Fist-Come-First-Served
void fcfs(char *s1, char *s2, int x1, int y1, int z1, int x2, int y2, int z2){
	P a,b; initP(&a,x1,y1,z1); initP(&b,x2,y2,z2);
	int i = 0; s1[0]=s2[0]='\0';

	for(; a.stage!=3 || b.stage!=3; i++){
		// Close CPU bursts that just reached 0
		finish_cpu_done(&a, s1, i);
		finish_cpu_done(&b, s2, i);

		//Close IO if done
		finish_io_done(&a);
		finish_io_done(&b);

		//selection
		assign_fcfs(&a,&b);

		emit_chars(&a,s1,i);
		emit_chars(&b,s2,i);

		dec_counter(&a);
		dec_counter(&b);
	}
}

//Non-preemptive Shortest job first
void sjf(char *s1, char *s2, int x1, int y1, int z1, int x2, int y2, int z2){
	P a,b; initP(&a,x1,y1,z1); initP(&b,x2,y2,z2);
	int i=0; s1[0]=s2[0]='\0';

	for(; a.stage!=3 || b.stage!=3; i++){
		finish_cpu_done(&a, s1, i);
		finish_cpu_done(&b, s2, i);

		finish_io_done(&a);
		finish_io_done(&b);

		assign_sjf(&a,&b);

		emit_chars(&a,s1,i);
		emit_chars(&b,s2,i);

		dec_counter(&a);
		dec_counter(&b);

	}
}

//Preemptive shortest job first
void psjf(char *s1, char *s2, int x1, int y1, int z1, int x2, int y2, int z2){
	P a,b; initP(&a,x1,y1,z1); initP(&b,x2,y2,z2);
	int i=0; s1[0]=s2[0]='\0';

	for (; a.stage!=3 || b.stage!=3; i++) {
		finish_cpu_done(&a,s1,i);
		finish_cpu_done(&b,s2,i);

		finish_io_done(&a);
		finish_io_done(&b);


		assign_psjf(&a,&b);
															
	    emit_chars(&a,s1,i);
        emit_chars(&b,s2,i);
															
		dec_counter(&a);
		dec_counter(&b);
	}
}

//Round Robin
void rr(char *s1, char *s2, int q, int x1, int y1, int z1, int x2, int y2, int z2) {
	P a,b; initP(&a,x1,y1,z1); initP(&b,x2,y2,z2);
	int i=0, current=0, qleft=0;
	s1[0]=s2[0]='\0';

	for (; a.stage!=3 || b.stage!=3; i++) {
		finish_cpu_done(&a,s1,i);
		finish_cpu_done(&b,s2,i);

		if (current==1 && a.stage==3) current=0;
		if (current==2 && b.stage==3) current=0;
		if (current==1 && a.state==WAITING) current=0;
		if (current==2 && b.state==WAITING) current=0;

		finish_io_done(&a);
		finish_io_done(&b);

		if (current==1 && a.state==RUNNING){
			if(qleft<=0) {a.state = READY; current=0;}
		}else if (current==2 && b.state == RUNNING){
			if (qleft <= 0) {b.state = READY; current = 0;}
		}
						        
		if (current == 0) assign_rr(&a, &b, &current, &qleft, q);

		emit_chars(&a, s1, i);
		emit_chars(&b, s2, i);

		if (a.state == RUNNING) { dec_counter(&a); if (current == 1) qleft--; }
		else dec_counter(&a);


		if (b.state == RUNNING) { dec_counter(&b); if (current == 2) qleft--;}
		else dec_counter(&b);
	}
}


