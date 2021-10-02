#include "dependencies.h"

time_t thread_time[SIZE];
char runtime_input[11];

void *user_input_thread(void * data)
{// scan for user input while receving data

     fgets(runtime_input, sizeof(runtime_input), stdin);
     strtok(runtime_input, "\n");
}

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

     char input;
     pthread_t check_input;
     int slot_number = 0;

     while (1)
     {// loop to get numbers from user or receive data from server
          // check for user input
          pthread_create(&check_input, NULL, user_input_thread, NULL);

          if (strncmp(runtime_input, "q", 1) == 0) break;

          else
          {// a number was entered

               uint32_t number = strtoul(runtime_input, NULL, 10);
               if (number > 0)
               {// check if available slot

                    slot_number = send_query(shm_ptr, number);
                    if (slot_number < 11)
                    {// there is an available slot
                    
                         shm_ptr -> complete_threads[slot_number] = 0;
                         time(&(thread_time[slot_number]));
                    }                    

                    else printf("Server is busy\n");
               }
          }

          memset(runtime_input, 0, sizeof(runtime_input));

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

     printf("%d numbers sent to server\n", slot_number);
     
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

          printf("%d numbers sent to server\n", slot_number + 1);
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

     // display user options
     printf("The server will start factorising numbers once 10 numbers have been sent\n");
     printf("If the server has finished factorising a query, you may enter another query\n");
     printf("Enter q at anytime to quit\n");

     get_input(shm_ptr); // get numbers from user

     shmdt((void *) shm_ptr); // detach memory and exit
     shmctl(shm_id, IPC_RMID, NULL);

     return 0;
}