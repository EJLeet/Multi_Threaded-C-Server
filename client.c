#include "dependencies.h"

int main(void)
{
     key_t shm_key;
     int shm_id;
     struct Memory *shm_ptr;
     char query[11];

     // create shared memory key
     shm_key = ftok(".", 'x');
     shm_id = shmget(shm_key, sizeof(struct Memory), IPC_CREAT | 0666);
     if (shm_id < 0) { printf("Client could not get memory\n"), exit(1); }

     // attach to shared memory
     shm_ptr = (struct Memory *) shmat(shm_id, NULL, 0);
     if ((int) shm_ptr == -1) { printf("Client could not attach to shared memory\n"), exit(1); }

     // query for 32 bit integers
     printf("Enter a 32 bit integer: ");
     fgets(query, sizeof(query), stdin);

     // convert 32 bit integer to unsigned long and send to shared memory
     shm_ptr -> number = strtoul(query, NULL, 10);
     



     return 0;
}