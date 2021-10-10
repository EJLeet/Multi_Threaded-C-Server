#include "dependencies.h"

void create_threads();
uint32_t rotate(uint32_t number, int bits_rotated);
void trial_division(void *data);
int msleep(long msec);

struct hold_rotations { uint32_t number; int slot_number; }; // used to hold data for each thread (rotated number)
struct Memory *shm_ptr; // shared memory

sem_t mutex[SIZE]; // semaphore access for factorising threads on specific slot

int main(void)
{
    key_t shm_key;
    int shm_id;

    // create shared memory
    shm_key = ftok(".", 'x');
    shm_id = shmget(shm_key, sizeof(struct Memory), 0666);
    if (shm_id < 0) { printf("Server could not get memory\n"); exit(1); }

    // attach to shared memory
    shm_ptr = (struct Memory *) shmat(shm_id, NULL, 0);
    if ((long) shm_ptr == -1) { printf("Server could not attach to shared memory\n"); exit(1); }
    
    printf("Waiting for client to send input\n");

    create_threads();

    shmdt((void *) shm_ptr); // detach and exit

    return 0;
}

void create_threads()
{// setup threads and communication data/flags between server and client
    
    for (int i = 0; i < SIZE; i++) sem_init(&(mutex[i]), 0, 1); // ensure threads do not read and write at the same time

    while (1)
    {// loop until theres no more numbers queued

        if (shm_ptr -> c_flag == 1)
        {// check to see if client has sent a number

            uint32_t number = shm_ptr -> number; // read data from shared memory

            // assign a slot number to client
            uint32_t slot_number = SIZE + 1; // returns out of bounds if no slot available
            for (uint32_t i = 0; i < SIZE; i++)
            {// look for available threads

                if (shm_ptr -> progress[i] == -1)
                {// if there is a thread available, assign it

                        slot_number = i;
                        shm_ptr -> s_flag[i] = 0;
                        break;
                }
            }

            shm_ptr -> number = slot_number;

            shm_ptr -> c_flag = 0;

            pthread_t thread_id[32]; // start 32 threads for this number
            
            struct hold_rotations thread_data[32]; // create 32 structs for each rotated number

            for (int i = 0; i < 32; i++) 
            {// rotate each number and assign to thread data
            
                thread_data[i].number = rotate(number, i);
                thread_data[i].slot_number = slot_number;
                pthread_create(&(thread_id[i]), NULL, (void *) trial_division, (void *) &(thread_data[i]));
            }
        }

        if (shm_ptr -> c_flag == 9)
        {// detach memory and exit
            shmdt((void *) shm_ptr);
            exit(1);
        }
    }  
}

uint32_t rotate(uint32_t number, int bits_rotated)
{// take a number and rotate it by b bits
    if (bits_rotated == 0) return number;
    return (number >> bits_rotated) | (number << (32 - bits_rotated));
}

void trial_division(void *data)
{
    uint32_t number = ((struct hold_rotations *)data) -> number, factor = 2, prev_factor = 0;
    int slot_number = ((struct hold_rotations *)data) -> slot_number;

    while (number > 1)
    {// Calculate factor

        if (number % factor == 0)
        {// is a factor

            if (prev_factor != factor)
            {// dont print duplicates inside same thread
            
                sem_wait(&(mutex[slot_number]));
                
                while (shm_ptr -> s_flag[slot_number] != 0); // ensure client is ready
                
                // send factor
                shm_ptr -> slot[slot_number] = factor;
                shm_ptr -> s_flag[slot_number] = 1;
                prev_factor = factor;

                msleep(10); // 10 millisecond delay

                sem_post(&(mutex[slot_number])); // semaphore signal
            }

            number /= factor; // decrease f
        }

        else factor++; // is not a factor move to next f
    }

    // thread has finished
    sem_wait(&(mutex[slot_number]));
    shm_ptr -> progress[slot_number]++;
    printf("Query: %d   Thread: %d completed\n", slot_number + 1, shm_ptr -> progress[slot_number]);

    sem_post(&(mutex[slot_number]));

    pthread_exit(NULL);
}

int msleep(long msec)
{// function will use nanosleep() to sleep for passed number of milliseconds
    struct timespec ts;
    int res;

    if (msec < 0) { errno = EINVAL; return -1; }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do res = nanosleep(&ts, &ts);
    while (res && errno == EINTR);

    return res;
}