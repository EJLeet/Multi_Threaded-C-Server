#include "dependencies.h"

int main(int argc, char *argv[])
{
     key_t          ShmKEY;
     int            ShmID;
     struct Memory  *ShmPTR;
     
     if (argc != 5) {
          printf("Use: %s #1 #2 #3 #4\n", argv[0]);
          exit(1);
     }
     
     ShmKEY = ftok(".", 'x');
     ShmID = shmget(ShmKEY, sizeof(struct Memory), IPC_CREAT | 0666);
     if (ShmID < 0) {
          printf("*** shmget error (client) ***\n");
          exit(1);
     }
     printf("client has received a shared memory of four integers...\n");
     
     ShmPTR = (struct Memory *) shmat(ShmID, NULL, 0);
     if ((long) ShmPTR == -1) {
          printf("*** shmat error (client) ***\n");
          exit(1);
     }
     printf("client has attached the shared memory...\n");
     
     ShmPTR->status  = NOT_READY;
     ShmPTR->data[0] = atoi(argv[1]);
     ShmPTR->data[1] = atoi(argv[2]);
     ShmPTR->data[2] = atoi(argv[3]);
     ShmPTR->data[3] = atoi(argv[4]);
     printf("client has filled %d %d %d %d to shared memory...\n",
            ShmPTR->data[0], ShmPTR->data[1], 
            ShmPTR->data[2], ShmPTR->data[3]);
     ShmPTR->status = FILLED;
     
     printf("Please start the server in another window...\n");
                 
     while (ShmPTR->status != TAKEN)
          sleep(1);
          
     printf("client has detected the completion of its server...\n");
     shmdt((void *) ShmPTR);
     printf("client has detached its shared memory...\n");
     shmctl(ShmID, IPC_RMID, NULL);
     printf("client has removed its shared memory...\n");
     printf("client exits...\n");
     return 0;
}
