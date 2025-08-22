#include <stdio.h> // for printf
#include <unistd.h> // for any linux utilities
#include <signal.h> // for signal
#include <sys/wait.h> // for wait() and fork
#include <string.h> // for memset
#include <stdlib.h> // for exit
#include <sys/types.h> // for kill
#include <stdbool.h> // for bool

#define LENGTH 8 // The fixed length bitstring

char recvdString[9]; // the buffer for the child to store the received string into
volatile sig_atomic_t recvdIndex = 0;
bool canSend = false; // synchronization mechanism for notifying the parent that the child is ready to receive
volatile sig_atomic_t canReceive = true;

void sigHandle1(int sigid) { 
    // TODO : Implement this function for when child receives a 1
    if (recvdIndex < LENGTH)
        recvdString[recvdIndex++] = '1';
    kill(getppid(), SIGUSR2); 
    // printf("[Child] Received 1 at index %d\n", recvdIndex - 1); // Debugging line to show when a 1 is received
}

void sigHandle0(int sigid) {
    // TODO : Implement this function for when child receives a 0
    if (recvdIndex < LENGTH)
        recvdString[recvdIndex++] = '0';
    kill(getppid(), SIGUSR2); 
    // printf("[Child] Received 0 at index %d\n", recvdIndex - 1); // Debugging line to show when a 0 is received
}

void synchronizeParent (int sigid) { // helper function to ensure the parent only sends once the child is ready to receive
    canSend = true;
}

void parent_can_send(int sigid) {
    canReceive = true;
}

int main () {

    signal(SIGUSR1,synchronizeParent); // to ensure parent is able to understand when child is telling it that its ready to receive
    int cpid = fork(); // fork
    if (cpid < 0) {
        perror("Fork failed");
        _exit(0);
    }
    // printf("HERE\n");
    if (cpid == 0) {
        signal(SIGUSR1, sigHandle1);
        signal(SIGUSR2, sigHandle0);
        kill(getppid(),SIGUSR1); 
        while(recvdIndex != LENGTH) pause(); // Wait till we receive all the bits from parent
        recvdString[8] = '\0'; // Null terminates the string so that it prints in an expected manner
        printf("[Child] Received bitstring is\t%s\n",recvdString); // Do not edit, prints the received bitstring
        exit(0);
    }
    else {

        printf("Please input a %d-bit bitstring:\t",LENGTH);

        char tmp[256]; // buffer to store bitstring
        fgets(tmp, LENGTH + 1, stdin); // Take input from user

        if (strlen(tmp) != 8) {
            printf("Error : Input string not of length %d\n",LENGTH);
            kill(cpid,9);
            wait(NULL);
            exit(1);
        }

        for (int i = 0; i < LENGTH; i++) {
            if (!(tmp[i] == '1' || tmp[i] == '0')) {
                printf("Error : Input string not a bitstring at index %d char %c\n",i,tmp[i]);
                kill(cpid,9);
                wait(NULL);
                exit(1);
            }
        }

        printf("[Parent] Input bitstring is \t%s\n",tmp);

        while (!canSend) {;} // Wait until the child is ready to receive
        signal(SIGUSR2, parent_can_send); 
        for (int i = 0; i < LENGTH; i++) {

            while(!canReceive) {;} 
            // printf("[Parent] Sending %c to child\n",tmp[i]);
            if (tmp[i] == '1') {
                // TODO : Add mechanism to send 1 to child
                kill(cpid, SIGUSR1);
                canReceive = false;
            }
            else {
                // TODO : Add mechanism to send 0 to child
                kill(cpid, SIGUSR2);
                canReceive = false;
            }
        }
        // printf("[Parent] Sent bitstring to child\n");
        wait(NULL); // reap the child
    }
}
