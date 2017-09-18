#include <RPio.h>
#include <midi.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#define THRDS 2
#define COLN 8
#define ROWN 2
#define SETN 6 //7 final
//chord types
#define MAJ	0	
#define MIN 	1
#define DIM	2
#define AUG	3

void *keypress(void *arg);
void *output(void *arg);

char setname(char);
int exit_key=0;

int on =0;
int key=0;
int cur=0;
int row=0;
int octu = 4;
int inv=0;
int chordtype = MAJ;


	
char prmt='d';
int colpin[COLN] = {27, 4, 16, 12, 7, 10, 23, 18};
int rowpin[ROWN] = {17, 22};
int settingpin[SETN] = {0, 1, 2, 3, 9  11};
	
pthread_mutex_t exit_mu, key_mu, oct_mu;

int main (void) {
	int therr; 
	int t,f=0;
        void *status;
	pthread_t thread[THRDS];
	pthread_attr_t attr;

	GpioInit();
	midi_uart_init();

//collumns
	for (f=0;f<COLN;f++) 
         SETFUNC(colpin[f],OUTPUT);

	 SETFUNC(19,OUTPUT);

	for (f=0;f<COLN;f++) 
         CLRPIN(colpin[f]);

         CLRPIN(19);

//rows
	for (f=0;f<ROWN;f++) {	
	 SETFUNC(rowpin[f],INPUT);
	 SETEVENT(rowpin[f],IO_RIS);
	 SETEVENT(rowpin[f],IO_FAL);
	}
	for (f=0;f<SETN;f++)	
	 SETFUNC(settingpin[f],INPUT);


	printf("press q to quit press a/z to raise/lower oct\n");

        pthread_mutex_init(&exit_mu, NULL);
        pthread_mutex_init(&key_mu, NULL);
        pthread_mutex_init(&oct_mu, NULL);

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	if(therr = pthread_create(thread, NULL, keypress,  NULL)){
          printf("ERROR CODE FOR THREAD 0 = %d\n",therr);
	  exit(-1);
	}

	if(therr = pthread_create(thread+1,NULL, output,  NULL)){
          printf("ERROR CODE FOR THREAD 1 = %d\n",therr);
	  exit(-1);
	}

	pthread_attr_destroy(&attr);

        while ((prmt = getchar()) != 'q'){
	  if(prmt == 'a') {
	    if(octu<8) {
	     pthread_mutex_lock(&oct_mu);
	     octu++;
	     pthread_mutex_unlock(&oct_mu);
            }
           }
	  if(prmt == 'z') {
	    if(octu>0) {
	     pthread_mutex_lock(&oct_mu);
	     octu--;
	     pthread_mutex_unlock(&oct_mu);
            }	
           }
	 }
	
        pthread_mutex_lock(&exit_mu);
	exit_key=1;
	pthread_mutex_unlock(&exit_mu);
	while(t<THRDS) {
	   pthread_join(thread[t], &status);
	   t++;
	}
	pthread_mutex_destroy(&exit_mu);
        pthread_mutex_destroy(&key_mu);
        pthread_mutex_destroy(&oct_mu);
	pthread_exit(NULL);
	return 0;
}

void *keypress(void *arg) {
	int last = -1;
	int r=0;
        while(exit_key == 0) {	
       	  SETPIN(colpin[cur]);
	  if(last>-1);
	  CLRPIN(colpin[last]);
	  pthread_mutex_lock(&key_mu);
          for(r=0;r<ROWN;r++) {
	   if(CHKEVN(rowpin[r])) { 
             if(CHKPIN(rowpin[r])) {
		on = 1;
	        row = r;
	        key = (int) (8*row+cur-2);
	        chordtype = MAJ; 
                inv = 0;
		for(r=0;r<SETN;r++) {
		  if(!CHKPIN(settingpin[r])) {
                     inv = (r+1)%3;
		     chordtype = (r+1)/3;
		     break;
		   }
		}
		CLREVN(rowpin[r]);
		while(!(CHKEVN(rowpin[0])||CHKEVN(rowpin[1])));
	     }
	     else {
	        on = 0;
		CLREVN(rowpin[r]);
             }
            }
	   }
          last = cur;
  	  cur++;
	  if(cur>(COLN-1)) cur = 0;
	  pthread_mutex_unlock(&key_mu);
	  usleep(499);
	}
	pthread_exit((void*) 0);
}
void *output(void *arg) {
	int last=-1;
	char note,chan,oct,vel=0;
	char triad[3] = {0x00, 0x00, 0x00};
	char noteName,sharp;
        while(exit_key == 0) {
	 if(last != on) {
	  last = on;
	  if (on==1) {
	   SETPIN(19);
           vel = 64;
	   note = (char)((key-1)%12);
           oct = (char)((key-1)/12 + octu);
	   triad[0] = NOTE(note,oct);
	   triad[1] = triad[0]+4;
	   triad[2] = triad[1]+3;
	   if(inv>0) {
	      triad[0] += 0x0c;
	      if(inv>1) triad[1] += 0x0c;
	   }
	   if(chordtype>MAJ && chordtype<AUG){
		triad[1] -= 1;
	  	if(chordtype>2) triad[2] -= 1;
	   }
	   else if(chordtype == AUG) triad[2] +=1; 
           
	   
           if (note<5) {
	    if (note%2 == 1) sharp = '#';
	    else sharp = ' ';
	   }
           else{
	    if (note%2 == 0) sharp = '#';
	    else sharp = ' ';
	   }
	   noteName = setname(note);
	  }
	  else {
	   CLRPIN(19);
	   vel = 0;
	  }
          midi_write(NOTE_ON,(char)0,triad[0], vel);
	  usleep(1000);
	  midi_write(NOTE_ON,(char)0,triad[1], vel);
	  usleep(1000);
	  midi_write(NOTE_ON,(char)0,triad[2], vel);
	  usleep(1000);
         }	
printf("key = %2d note = %c %c oct = %d vel = %2d inv = %d chord = %d \r", key, noteName, sharp, oct, vel, inv, chordtype);
	}
	pthread_exit((void*) 0);
}

char setname(char n) {
	switch(n) {
	 case 0:
	 case 1:
	  return 'C';
	 case 2:
	 case 3:
	  return 'D';
	 case 4:
	  return 'E';
	 case 5:
	 case 6:
	  return 'F';
	 case 7:
	 case 8:
	  return 'G';
	 case 9:
	 case 10:
	  return 'A';
	 case 11:
	  return 'B';
	}
}
