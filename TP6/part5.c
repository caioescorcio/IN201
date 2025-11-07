#include <stdio.h>
#include <fcntl.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/seccomp.h>
#include <sys/prctl.h>
#include <sys/syscall.h>

// for pipe() and fork() explanation, view pipe_fork.png
// pipe() create 2 unidirectional comunication processess

int main(){
    
    int p1[2];  
    int p2[2];
    char buffer[1024];
    char exec[1024];
    int error;

    if(pipe(p1) == -1) {    // create the new pipe() (2 file descriptors, for reading and writing) and verify them 
        printf("Error while creating pipe(p1)\n");
        syscall(SYS_exit, 0);
    }    

    if(pipe(p2) == -1) {    
        close(p1[0]);
        close(p1[1]);
        printf("Error while creating pipe(p2)\n");
        syscall(SYS_exit, 0);
    }    

    printf("Success while creating pipes:\n");
    printf("  p1 (parent -> child):  read=%d  write=%d\n", p1[0], p1[1]);
    printf("  p2 (child -> parent):  read=%d  write=%d\n\n", p2[0], p2[1]);

    /*
        When pipe we create separate write and read fields ("temporary files") for our process: 
        pipe(file_descriptor[2]) creates the following data:
            file_descriptor[0] = readable area
            file_descriptor[1] = writable area 
    */

    /*
        The fork() creates a clone of the current process, using the same memory instances as the original one.
        For the TP we're going to use the parent process to read the data from p1[0] and to write it on the p2[1]
        while the child process reads data from p2[0] and writes data in the p1[1] 
    */

    // parent 
    if(fork() == 0){ 
        close(p1[1]);   // close the respective unused data buses
        close(p2[0]); 

        // it 

        while(1){
            read(p1[0], buffer, 1023);              // read from p1[0] and allocates on buffer
            sscanf(buffer, "e%[^\n]", exec);        // reads buffer till \n
            error = system(exec);                   // execute command
            sprintf(buffer, "\nError: %d\n", error);
            if (error == -1)
                write(p2[1], "Error while executing\n", 22);
            else
                write(p2[1], "Success\n", 8);       // writes back success
        }
    }

    // child
    else {
        close(p1[0]);
        close(p2[1]); 
        prctl(PR_SET_SECCOMP, SECCOMP_MODE_STRICT); // applies restriction

        const char * commands[] = {      // commands to be executed
            "els",
            "epwd",
            "ewhoami",
            "eexit"
        };

        for (int i = 0; i < 4; i++) {
            printf("[v] command sent: %s\n\n", commands[i]+1);
            write(p1[1], commands[i], strlen(commands[i]) + 1);
            write(p1[1], "\n", 1);

            read(p2[0], buffer, sizeof(buffer) - 1);
            printf("\n[^] response: %s", buffer);

        }
        
        syscall(SYS_exit, 0);

    };

    // close the data buses
    close(p1[0]); close(p1[1]);
    close(p2[0]); close(p2[1]);
    syscall(SYS_exit, 0);

}
