#include "dependencies.h"

void query(struct Memory *shm_ptr)
{    
     char user_input[11], input;

     while (1)
     {// loop to get numbers from user or receive data from server
          
          
     }

}

int main(void)
{
     key_t shm_key;
     int shm_id;
     struct Memory *shm_ptr;

     // create shared memory key
     shm_key = ftok(".", 'x');
     shm_id = shmget(shm_key, sizeof(struct Memory), IPC_CREAT | 0666);
     if (shm_id < 0) { printf("Client could not get memory\n"), exit(1); }

     // attach to shared memory
     shm_ptr = (struct Memory *) shmat(shm_id, NULL, 0);
     if ((long) shm_ptr == -1) { printf("Client could not attach to shared memory\n"), exit(1); }

     // set up flags
     shm_ptr -> c_flag = 0;
     for (int i = 0; i < SIZE; i++) shm_ptr -> s_flag[i] = 2;
     for (int i = 0; i < SIZE; i++) shm_ptr -> complete_threads[i] = -1;

     query(shm_ptr); // loop and pass numbers to server

     shmdt((void *) shm_ptr); // detach memory and exit
     shmctl(shm_id, IPC_RMID, NULL);

     return 0;
}