#include "dependencies.h"

void input_output();
void progress();
void delete(int length);
void display(int progress, int length, int slot);
void receive();
void user_input();
int send(uint32_t number);

struct Memory *shm_ptr; // shared memory
time_t thread_time[SIZE];
char runtime_input[11];

// scan for user input while receving data
void *user_input_thread(void * data) { fgets(runtime_input, sizeof(runtime_input), stdin); strtok(runtime_input, "\n"); }

int main(void)
{
    key_t shm_key;
    int shm_id;

    // create shared memory key
    shm_key = ftok(".", 'x');
    shm_id = shmget(shm_key, sizeof(struct Memory), IPC_CREAT | 0666);
    if (shm_id < 0) { printf("Client could not get memory\n"); exit(1); }

    // attach to shared memory
    shm_ptr = (struct Memory *) shmat(shm_id, NULL, 0);
    if ((long) shm_ptr == -1) { printf("Client could not attach to shared memory\n"); exit(1); }

    // set up flags
    shm_ptr -> c_flag = 0;
    for (int i = 0; i < SIZE; i++) shm_ptr -> s_flag[i] = 2;
    for (int i = 0; i < SIZE; i++) shm_ptr -> progress[i] = -1;

    // display user options
    printf("The server will start factorising numbers once 10 numbers have been sent\n");
    printf("If the server has finished factorising a query, you may enter another query\n");
    printf("Enter q at anytime to quit\n");

    input_output(); // run program

    shmdt((void *) shm_ptr); // detach memory and exit
    shmctl(shm_id, IPC_RMID, NULL);

    return 0;
}

void input_output()
{
    char input[11];
    int slot_number = 0;

    while (1)
    {// loop until all requests handles

        //progress();
        receive();

        if (slot_number < 9)
        {// prompt user for 10 32 bit integer

            // get number from user, convert to 32 bit ul
            printf("Enter a 32 bit number: ");
            fgets(input, sizeof(input), stdin);
            strtok(input, "\n");

            // user entered a number
            uint32_t number = strtoul(input, NULL, 10); 

            slot_number = send(number);

            // there is an available slot, send and start clock
            shm_ptr -> progress[slot_number] = 0;
            time(&(thread_time[slot_number]));
            
            memset(input, 0, sizeof(input)); // clear input
        }

        else user_input(); // only check for thread activity after 10 queries received
        
    }
}

void progress()
{
    int percent_complete, active = 0;
    printf("Progress: ");

    for (int i = 0; i < 10; i++)
    {// loop through 10 queries

        if (shm_ptr -> progress[i] != -1)
        {// check if they're active
            
            active++;
            percent_complete = (int) round(shm_ptr -> progress[i] / 32.0 * 100); // get overall %
            percent_complete = round(percent_complete / 5) * 5; // round to nearest 5 %
            display(percent_complete, 10, i + 1);
            printf(" ");
        } 
    }
    delete(10 + (26 * active));
}

void delete(int length)
{// delete the bar to stop overflow and re-display

    for (int i = 0; i < length; i++) printf("\b");
    for (int i = 0; i < length; i++) printf(" ");
    for (int i = 0; i < length; i++) printf("\b");
}

void display(int progress, int length, int slot)
{// draw the bar to screen

    int full = progress / 10;
    printf("Slot %d: %d%% ", slot, progress);

    for (int i = 0; i < full; i++) printf("▓");
    for (int i = 0; i < length - full; i++) printf("_");
    printf("|");
    fflush(0);
}

void receive()
{
    for (int i = 0; i < SIZE; i++)
    {// receive server data

        if (shm_ptr -> s_flag[i] == 1)
        {// factors can be read

            printf("Query %d: Factor: %u\n", i + 1, shm_ptr -> slot[i]);
            shm_ptr -> s_flag[i] = 0;
        }

        if (shm_ptr -> progress[i] > 31)
        {// factorisation is complete

            shm_ptr -> progress[i] = -1;
            
            time_t elapsed; // get and print time taken for this query
            time(&elapsed);
            double time_taken = difftime(elapsed, thread_time[i]);

            printf("Query %d is finished and took %.f seconds\n", i + 1, time_taken);
        }
    }
}

void user_input()
{// check for user input

    pthread_t check_input;
    int slot_number = 0;

    pthread_create(&check_input, NULL, user_input_thread, NULL);

    if (strncmp(runtime_input, "q", 1) == 0)
    {// detach and exit
        shm_ptr -> c_flag = 9; // let server know to quit
        shmdt((void *) shm_ptr); 
        exit(1); 
    } 

    else
    {// a number was entered

        uint32_t number = strtoul(runtime_input, NULL, 10);
        if (number > 0)
        {// check if available slot

            slot_number = send(number);
            if (slot_number < 11)
            {// there is an available slot
            
                shm_ptr -> progress[slot_number] = 0;
                time(&(thread_time[slot_number]));
            }                    

            else printf("Server is busy\n");
        }
    }

    memset(runtime_input, 0, sizeof(runtime_input));
}

int send(uint32_t number)
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