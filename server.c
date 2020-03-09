#include "comm.h"

void handle_request(int fd){
char oper, acc;
uint32_t val;
oper = shmptr->op;
acc = shmptr->account;

if(!(acc >= 'A' && acc <= 'H'))
  shmptr->op = 'I';
else
   switch(oper){
      
     case 'C': lseek(fd,(acc - 'A')*4,SEEK_SET);
               if(read(fd, &val, 4) < 0){
                  perror("read error\n");
                  exit(1);
                }
               shmptr->value = val;
               shmptr->op = 'D';
               break;
     case 'A': lseek(fd,(acc - 'A')*4,SEEK_SET);
               if(read(fd, &val, 4) < 0){
                 perror("read error\n");
                  exit(1);
                }

               lseek(fd, -4, SEEK_CUR);
               val += shmptr->value;
               if(write(fd, &val, 4) < 0){
                 perror("write error\n");
                  exit(1);
                }

               shmptr->value = val;

               shmptr->op = 'D';
               break;
     case 'S': lseek(fd,(acc - 'A')*4,SEEK_SET);
               if(read(fd, &val, 4) < 0){
                 perror("read error\n");
                  exit(1);
                }

               lseek(fd, -4, SEEK_CUR);
               if(shmptr->value <= val){
                 val -= shmptr->value;
                 if(write(fd, &val, 4) < 0){
                 perror("write error\n");
                  exit(1);
                }

                 shmptr->value = val;
                 shmptr->op = 'D';
                 break;
               } else {
                 shmptr->op = 'I';
                 break;
               }
 
     default: shmptr->op = 'I';
   }
} 
int main(int argc, char* argv[]){
 
 key_t key; int fd,i;
 uint32_t init_value = 0;
 unsigned short semvals[] = {1, 0, 0};
 union semun arg;

 if(argc != 2){
  printf("Use: %s filename \n", argv[0]);
  exit(1);
 }
 //open accounts file
 if((fd = open(argv[1],O_RDWR)) < 0)
 {  //try creating
    if((fd = open(argv[1], O_CREAT|O_EXCL|O_RDWR, 0666)) < 0){
         perror("Can not open, nor create the file!\n");
         exit(1);
      }
    else { //Accounts file created. Initialize it.
      lseek(fd,0,SEEK_SET);
      for(i=0; i<8; i++){
         if(write(fd,&init_value,4) < 0){
                 perror("write error\n");
                  exit(1);
                }
        }
     }
}
 /*From now on we have an open accounts file with file descriptor fd. */
 //Create and initialize shared memory
 if ((key = ftok("/tmp", 'a')) == (key_t) -1) {
    perror("IPC error: ftok"); exit(1);
}
 shmid = shmget(key,sizeof(struct sh_buffer),IPC_CREAT|0666);
 if(shmid < 0){
   perror("shmget error(server)\n");
   exit(1);
  }
 shmptr = (struct sh_buffer*) shmat(shmid,NULL,0);
 if(shmptr == (void *) -1){
  perror("shmat error(server)\n");
  exit(1);
 }
 shmptr->op = 'N';
 shmptr->account = 'N';
 shmptr->value = init_value;

 //Create and initialize semaphores
 semid = semget(key,3,IPC_CREAT|0666);
 if(semid < 0){
   perror("semget error(server)\n");
   exit(1);
  }
 //Initialize the semaphores
 arg.array = semvals;
 if((semctl(semid, 0, SETALL, arg.array)) == -1)
 {
   perror("semctl error(server)\n");
   exit(1);
  }

 /*Start listening for client requests */

 while(1){
  P(semid,REQUEST);
  handle_request(fd);
  V(semid,RESPONSE);
 }

 return 0;
}
