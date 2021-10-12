Multi-Thread Client & Server
2803ICT - Assignment 2

AUTHOR

    Ethan Leet
    s5223103


ABOUT

    This program creates a server and client who communicate through shared memory. The user connected to the client program will enter 32 bit integers (queries) which will be sent to the server to be factorised. Factorised numbers will be returned back to the client when they are available. The user can enter '0' to enter the test mode, test mode will only run if there are no outstanding queries from ther user. The server will report the time taken to complete each query. The client will display query progress bars if there is a delay in server response.


Compiling

    To compile this program the following files need to be in the same directory:

    makefile
    client.c
    server.c
    dependencies.c

    Next, run 'make -B' to force a new build of the program.


Run Time

    Client
        The client needs to be executed first. It requires no command line arguements. After the client is active, the user may begin entering factors (or 0 for test mode). During run time, the user may continuously enter more factors, or press q to quit. A maximum of 10 queries may be outstanding from the user at any given time.

    Server
        The server needs to be executed after the client is active. No command line arguments are required for the server. There is no input allowed on the server side. It will continuously find and return factors until there are no more numbers or the user has entered quit.
    




