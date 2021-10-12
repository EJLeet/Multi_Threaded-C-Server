#include "dependencies.h"

void create_threads();
uint32_t rotate(uint32_t number, int bits_rotated);
void trial_division(void *data);
void test_mode();
void print_test(void *data);

struct hold_rotations
{
    uint32_t number;
    int slot_number;
};                      // used to hold data for each thread (rotated number)
struct Memory *shm_ptr; // shared memory

pthread_mutex_t mutex[SIZE]; // setup a mutex for each slot

int main(void)
{
    key_t shm_key;
    int shm_id;

    // create shared memory
    shm_key = ftok(".", 'x');
    shm_id = shmget(shm_key, sizeof(struct Memory), 0666);
    if (shm_id < 0) { printf("Server could not get memory\n"); exit(1); }

    // attach to shared memory
    shm_ptr = (struct Memory *)shmat(shm_id, NULL, 0);
    if ((long)shm_ptr == -1) { printf("Server could not attach to shared memory\n"); exit(1); }

    printf("Waiting for client to send input\n");

    create_threads();

    shmdt((void *)shm_ptr); // detach and exit

    return 0;
}

void create_threads()
{ // setup threads and communication data/flags between server and client

    for (int i = 0; i < SIZE; i++) pthread_mutex_init(&mutex[i], NULL); // ensure threads do not read and write at the same time

    while (1)
    { // loop until theres no more numbers queued

        if (shm_ptr->c_flag == 1)
        { // check to see if client has sent a number

            uint32_t number = shm_ptr->number; // read data from shared memory

            if (number == 0) test_mode();

            else
            { // assign a slot number to client

                uint32_t slot_number = SIZE + 1; // returns out of bounds if no slot available

                for (uint32_t i = 0; i < SIZE; i++)
                { // look for available threads

                    if (shm_ptr->progress[i] == -1)
                    { // if there is a thread available, assign it

                        slot_number = i;
                        shm_ptr->s_flag[i] = 0;
                        break;
                    }
                }

                shm_ptr->number = slot_number;

                shm_ptr->c_flag = 0;

                pthread_t thread_id[32]; // start 32 threads for this number

                struct hold_rotations thread_data[32]; // create 32 structs for each rotated number

                for (int i = 0; i < 32; i++)
                { // rotate each number and assign to thread data

                    thread_data[i].number = rotate(number, i);
                    thread_data[i].slot_number = slot_number;
                    pthread_create(&(thread_id[i]), NULL, (void *)trial_division, (void *)&(thread_data[i]));
                }
            }
        }

        // quit
        if (shm_ptr->c_flag == 9) { printf("Client has left - Server will terminate\n"); exit(1); }
    }
}

uint32_t rotate(uint32_t number, int bits_rotated)
{ // take a number and rotate it by b bits

    if (bits_rotated == 0) return number;
    return (number >> bits_rotated) | (number << (32 - bits_rotated));
}

void trial_division(void *data)
{ // perform null previous factor trial division

    printf("Running Trial Division\n");
    uint32_t number = ((struct hold_rotations *)data)->number, factor = 2, prev_factor = 0;
    int slot_number = ((struct hold_rotations *)data)->slot_number;

    while (number > 1)
    { // Calculate factor

        if (number % factor == 0)
        { // is a factor

            if (prev_factor != factor)
            { // dont print duplicates inside same thread

                pthread_mutex_lock(&mutex[slot_number]);

                while (shm_ptr->s_flag[slot_number] != 0) ; // ensure client is ready

                // send factor
                shm_ptr->slot[slot_number] = factor;
                shm_ptr->s_flag[slot_number] = 1;
                prev_factor = factor;

                usleep(10000); // 10 millisecond delay

                pthread_mutex_unlock(&mutex[slot_number]);
            }

            number /= factor; // decrease f
        }

        else factor++; // is not a factor move to next f
    }

    // thread has finished
    pthread_mutex_lock(&mutex[slot_number]);
    shm_ptr->progress[slot_number]++;
    pthread_mutex_unlock(&mutex[slot_number]);
    pthread_exit(NULL);
}

void test_mode()
{ // simulate test mode

    printf("Running TESTMODE\n");

    for (int query = 0; query < 3; query++)
    { // simulate 3 queries

        uint32_t slot_number = SIZE + 1; // returns out of bounds if no slot available

        for (uint32_t j = 0; j < SIZE; j++)
        { // look for available threads

            if (shm_ptr->progress[j] == -1)
            { // if there is a thread available, assign it

                slot_number = j;
                shm_ptr->s_flag[j] = 0;
                break;
            }
        }

        shm_ptr->number = slot_number;

        shm_ptr->c_flag = 8; // set test mode flag

        pthread_t thread_id[10];               // create 10 threads for this query
        struct hold_rotations thread_data[10]; // create 10 structs to hold each thread data

        for (int i = 0; i < SIZE; i++)
        {
            for (int j = 0; j < SIZE; j++)
            { // assign numbers 0-9 to query

                thread_data[j].number = i * 10 + j;
                thread_data[j].slot_number = slot_number;
                pthread_create(&(thread_id[j]), NULL, (void *)print_test, (void *)&(thread_data[j]));
            }
        }
    }

    // revert flags and slots for normal mode
    for (int i = 0; i < SIZE; i++) shm_ptr->progress[i] = -1;
    shm_ptr->c_flag = 0;
}

void print_test(void *data)
{ // print the test case data

    uint32_t number = ((struct hold_rotations *)data)->number;
    int slot_number = ((struct hold_rotations *)data)->slot_number;
    srand(time(NULL)); // set seed for random delay

    pthread_mutex_lock(&mutex[slot_number]);

    while (shm_ptr->s_flag[slot_number] != 0) ; // ensure client is ready

    // send number
    shm_ptr->slot[slot_number] = number;
    shm_ptr->s_flag[slot_number] = 1;

    usleep((rand() % 90 + 10) * 1000); //random delay between 10 and 100ms

    pthread_mutex_unlock(&mutex[slot_number]);

    // thread has finished
    pthread_mutex_lock(&mutex[slot_number]);
    shm_ptr->progress[slot_number]++;

    pthread_mutex_unlock(&mutex[slot_number]);

    pthread_exit(NULL);
}