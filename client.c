#include "comm.h"
char buff[20];

void write_request(char oper, char acc, uint32_t val){
 shmptr->op = oper;
 shmptr->account = acc;
 shmptr->value = val;
 
 printf("Client: Operation %c \n", oper);
}

void read_response(){
 char oper = shmptr->op;

 if(oper == 'I'){ //invalid
  printf("Invalid operation. Client exits.\n");
  V(semid,CLIENT);
  exit(1);
 } else { //oper is 'D', i.e. done
  printf("The current value of account %c is %u.\n", shmptr->account, shmptr->value);
  }

 } 

 int main(int argc, char* argv[]){

 key_t key; 
 int i;
 int16_t val;

 if(argc != 2){
  printf("Use: %s account(A-H) \n", argv[0]);
  exit(1);
 }

 if((strlen(argv[1]) != 1) || !(argv[1][0] >= 'A' && argv[1][0] <= 'H')){
  printf("Use: %s account(A-H) \n", argv[0]);
  exit(1);
 }
  
 /*get semaphores and shared memory */
 
 if ((key = ftok("/tmp", 'a')) == (key_t) -1) {
    perror("IPC error: ftok"); exit(1);
}
 shmid = shmget(key,sizeof(struct sh_buffer),0666);
 if(shmid < 0){
   perror("shmget error(client)\n");
   exit(1);
  }
 shmptr = (struct sh_buffer*) shmat(shmid,NULL,0);
 if(shmptr == (void *) -1){
  perror("shmat error(client)\n");
  exit(1);
 }

  semid = semget(key,0,0666);
 if(semid < 0){
   perror("semget error(client)\n");
   exit(1);
  }

/*Begin with the requests */

P(semid, CLIENT);

write_request('C',argv[1][0],0);
V(semid,REQUEST);
P(semid,RESPONSE);
read_response();

//read a line
printf("Type a positive number for addition or a negative for subtraction:\n");
i = 0;
while(read(0,&buff[i],1) == 1){

 if(buff[i] == '\n')
   break;
 
  i++;
} 
val = (int16_t) atoi(buff);

if(val < 0){
 val = -val;
 write_request('S', argv[1][0], (uint32_t) val); 
}
else
 write_request('A', argv[1][0], (uint32_t) val);  

V(semid,REQUEST);
P(semid,RESPONSE);
read_response();

V(semid,CLIENT);

//exit(0);

return 0;
}
