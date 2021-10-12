#include "dependencies.h"

void input_output();
void progress();
void delete (int length);
void display(int progress, int length, int slot);
void receive();
void user_input();
int send(uint32_t number);
void *user_input_thread(void *data);

struct Memory *shm_ptr;   // shared memory
time_t thread_time[SIZE]; // used for thread times
clock_t timer;            // used for 500ms progress bars
bool testing = false;

int main(void)
{
    key_t shm_key;
    int shm_id;

    // create shared memory key
    shm_key = ftok(".", 'x');
    shm_id = shmget(shm_key, sizeof(struct Memory), IPC_CREAT | 0666);
    if (shm_id < 0) { printf("Client could not get memory\n"); exit(1); }

    // attach to shared memory
    shm_ptr = (struct Memory *)shmat(shm_id, NULL, 0);
    if ((long)shm_ptr == -1) { printf("Client could not attach to shared memory\n"); exit(1); }

    // set up flags
    shm_ptr->c_flag = 0;
    for (int i = 0; i < SIZE; i++) shm_ptr->s_flag[i] = 2;
    for (int i = 0; i < SIZE; i++) shm_ptr->progress[i] = -1;

    // display user options
    printf("If the server has finished factorising a query, you may enter another\n");
    printf("Enter q at anytime to quit\n");
    printf("Enter 0 when the server is innactive to run test mode\n");

    input_output(); // run program

    shmdt((void *)shm_ptr); // detach memory and exit
    shmctl(shm_id, IPC_RMID, NULL);

    return 0;
}

void input_output()
{ // main run function for program, handles all input and output requests

    pthread_t check_input;
    printf("Enter a 32 bit number: ");

    while (1)
    { // loop until all requests handles

        clock_t elapsed = clock() - timer;
        int msec = elapsed * 1000 / CLOCKS_PER_SEC;

        // display progress bars if 500ms delay and there are active queries and not test mode
        for (int i = 0; i < SIZE; i++) if (shm_ptr->progress[i] != -1 && msec > 500 && !testing) progress();

        receive();
        pthread_create(&check_input, NULL, user_input_thread, NULL);
    }
}

void progress()
{ // displays progress by calling display and delete

    int percent_complete, active = 0;
    printf("Progress: ");

    for (int i = 0; i < SIZE; i++)
    { // loop through 10 queries

        if (shm_ptr->progress[i] != -1)
        { // check if they're active

            active++;
            percent_complete = (int)shm_ptr->progress[i] / 32.0 * 100; // get overall %
            percent_complete = round(percent_complete / 5) * 5;        // round to nearest 5 %
            display(percent_complete, 10, i + 1);
            printf(" ");
        }
    }

    delete (10 + (26 * active));
}

void delete (int length)
{ // delete the bar to stop overflow and re-display

    for (int i = 0; i < length; i++) printf("\b");
    for (int i = 0; i < length; i++) printf(" ");
    for (int i = 0; i < length; i++) printf("\b");
}

void display(int progress, int length, int slot)
{ // draw the bar to screen

    int full = progress / 10;
    printf("Slot %d: %d%% ", slot, progress);

    for (int i = 0; i < full; i++) printf("▓");
    for (int i = 0; i < length - full; i++) printf("_");

    printf("|");
}

void receive()
{ // get factors from server

    if (testing)
    { // test mode

        for (int i = 0; i < SIZE; i++)
        { // receive server data

            if (shm_ptr->s_flag[i] == 1)
            { // factors can be read

                printf("TEST-MODE Factor: %u\n", shm_ptr->slot[i]);
                shm_ptr->s_flag[i] = 0;
            }
        }
    }

    else
    { // normal mode - timers for progress bars

        for (int i = 0; i < SIZE; i++)
        { // receive server data

            if (shm_ptr->s_flag[i] == 1)
            { // factors can be read

                printf("Query %d: Factor: %u\n", i + 1, shm_ptr->slot[i]);
                shm_ptr->s_flag[i] = 0;
                timer = clock(); // start 500ms timer
            }

            if (shm_ptr->progress[i] > 31)
            { // factorisation is complete

                shm_ptr->progress[i] = -1;

                // get and print time taken for this query
                time_t elapsed;
                time(&elapsed);
                double time_taken = difftime(elapsed, thread_time[i]);

                printf("Query %d is finished and took %.f seconds\n", i + 1, time_taken);
                timer = clock(); // start 500ms timer
            }
        }
    }
}

void *user_input_thread(void *data)
{ // scan for user input while receving data

    char runtime_input[100]; // input for thread
    int slot_number = 0;

    fgets(runtime_input, sizeof(runtime_input), stdin);
    strtok(runtime_input, "\n");

    // error checks
    for (int i = 0; i < strlen(runtime_input); i++) if (isalpha(runtime_input[i]) &&
                                                        strlen(runtime_input) > 1 ||
                                                        strlen(runtime_input) > 10) { printf("Invalid Input\n"); input_output(); }

    if (strncmp(runtime_input, "q", 1) == 0)
    { // quit and advise server to do same

        printf("Exiting\n");
        shm_ptr->c_flag = 9;
        exit(1);
    }

    else
    { // number was entered

        uint32_t number = strtoul(runtime_input, NULL, 10);
        if (number > 0)
        { // check if available slot

            testing = false;

            slot_number = send(number);
            if (slot_number < 11)
            { // there is an available slot

                shm_ptr->progress[slot_number] = 0;
                time(&(thread_time[slot_number]));
            }

            else
                printf("Server is busy\n");
        }

        else if (number == 0)
        { // test mode

            bool active = false;
            for (int i = 0; i < SIZE; i++)
            { // check if server is active

                if (shm_ptr->progress[i] != -1)
                {

                    printf("Can not enter test mode - Server is busy!\n");
                    active = true;
                    break;
                }
            }

            if (!active)
            { // run test mode

                testing = true;
                printf("TEST-MODE\n");
                shm_ptr->number = 0;
                shm_ptr->c_flag = 1;
            }
        }
    }
    memset(runtime_input, 0, sizeof(runtime_input));
}

int send(uint32_t number)
{
    // wait until server can accept
    while (shm_ptr->c_flag != 0) ;

    // send number and change flag
    shm_ptr->number = number;
    shm_ptr->c_flag = 1;

    // wait for server response
    while (shm_ptr->c_flag != 0);

    // return slot number
    return shm_ptr->number;
}