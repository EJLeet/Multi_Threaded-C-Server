#include "dependencies.h"

time_t thread_time[SIZE];

int send_query(struct Memory *shm_ptr, uint32_t number)
{
     // wait until server can accept
     while (shm_ptr -> c_flag != 0);

     // send number and change flag
     shm_ptr -> number = number;
     shm_ptr -> c_flag = 1;

     // wait for server response
     while (shm_ptr -> c_flag != 0);

     // return slot number
     return shm_ptr -> number;
}

void receive_factors(struct Memory *shm_ptr)
{// display factors sent form server

     while (1)
     {// loop to get numbers from user or receive data from server
               
          for (int i = 0; i < SIZE; i++)
          {// receive server data

               if (shm_ptr -> s_flag[i] == 1)
               {// factors can be read

                    printf("Query %d: Factor: %u\n", i + 1, shm_ptr -> slot[i]);
                    shm_ptr -> s_flag[i] = 0;
               }

               if (shm_ptr -> complete_threads[i] > 31)
               {// factorisation is complete

                    shm_ptr -> complete_threads[i] = -1;
                    
                    time_t elapsed; // get and print time taken for this query
                    time(&elapsed);
                    double time_taken = difftime(elapsed, thread_time[i]);
                    printf("Query %d is finished and took %.f seconds\n", i + 1, time_taken);
               }
          }     
     }
}

void get_input(struct Memory *shm_ptr)
{// get a 32 bit number from the user
     char user_input[11];
     int slot_number = 0;

     while(++slot_number < 10)
     {// loop until client has entered 10 numbers

          // get number from user, convert to 32 bit ul
          printf("Enter a 32 bit number: ");
          fgets(user_input, sizeof(user_input), stdin);
          strtok(user_input, "\n");

          // user entered a number
          uint32_t number = strtoul(user_input, NULL, 10); 

          // send query to server and receive slot number
          slot_number = send_query(shm_ptr, number);

          // there is an available slot, send and start clock
          shm_ptr -> complete_threads[slot_number] = 0;
          time(&(thread_time[slot_number]));
          
          memset(user_input, 0, sizeof(user_input)); // clear input
     }

     receive_factors(shm_ptr);
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

     get_input(shm_ptr); // get numbers from user

     shmdt((void *) shm_ptr); // detach memory and exit
     shmctl(shm_id, IPC_RMID, NULL);

     return 0;
}