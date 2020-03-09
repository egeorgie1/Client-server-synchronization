#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <unistd.h>

#define CLIENT 0
#define REQUEST 1
#define RESPONSE 2

struct sh_buffer{
  char op; //as request: 'C'-check, 'A'-add, 'S'-subtract
           //as initialization: 'N' - no op. yet
           //as response: 'D'- operation done, 'I'-invalid operation

  char account;
  uint32_t value;
} *shmptr;
 
union semun{
  int val;
  struct semid_ds *buf;
  unsigned short *array;
};
  
int semid;
int shmid;

/* P operation on semaphore */
void P(int semid, int semnum) {
 
 struct sembuf psem = {0, -1, 0};

 psem.sem_num = semnum;
 
 if(semop(semid, &psem, 1) == -1) {
  perror("semop error \n");
  exit(1);
 }
}

/* V operation on semaphore */
void V(int semid, int semnum) {
 
 struct sembuf vsem = {0, 1, 0};

 vsem.sem_num = semnum;
 
 if(semop(semid, &vsem, 1) == -1) {
  perror("semop error \n");
  exit(1);
 }
}

