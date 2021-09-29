#include "dependencies.h"

struct hold_rotations
{// used to hold data for each thread (rotated number)
     uint32_t number;
     int slot_number;
};

struct Memory *shm_ptr;

uint32_t rotate(uint32_t number, int bits_rotated)
{// take a number and rotate it by b bits
     if (bits_rotated == 0) return number;
     return (number >> bits_rotated) | (number << (32 - bits_rotated));
}

uint32_t trial_division(uint32_t number)
{
     uint32_t f = 2;
     while (number > 1)
     {
          if (number % f == 0)
          {
               printf("%u\n", f);
               number /= f;
          }
          else f++;
     }
}

int main(void)
{
     key_t shm_key;
     int shm_id;

     // create shared memory
     shm_key = ftok(".", 'x');
     shm_id = shmget(shm_key, sizeof(struct Memory), 0666);
     if (shm_id < 0) { printf("Server could not get memory\n"), exit(1); }

     // attach to shared memory
     shm_ptr = (struct Memory *) shmat(shm_id, NULL, 0);
     if ((int) shm_ptr == -1) { printf("Server could not attach to shared memory\n"), exit(1); }

     // read data from shared memory
     uint32_t number = shm_ptr -> number;
     
     // start 32 threads for this number
     pthread_t thread_id[32];
     
     // create 32 structs for each rotated number
     struct hold_rotations thread_data[32];

     // rotate each number and assign to thread data
     for (int i = 0; i < 32; i++) 
     {
          thread_data[i].number = rotate(number, i);
          trial_division(thread_data[i].number);
     }

     // trial division for each of the 32 numbers

     



     return 0;
}