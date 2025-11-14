#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/seccomp.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <ctype.h>

// my to upper function
void uppercase(char * buffer);

int main(){
    char buffer[1024];  // buffer to receive the messages
    
    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {  // fgets use the buffer to store my stdin(file descriptor)

        uppercase(buffer);
    
        printf("Message: %s\n", buffer);    // print the message
        fflush(stdout);
    
        if (gethostname(buffer, 1024) == 0){    // gets hostname to print
            uppercase(buffer);
            printf("Hostname: %s\n", buffer);
        } else {
            perror("gethostname");
        }
        
        fflush(stdout);
        pid_t pid = getpid();
        if (pid){
            printf("PID: %d\n", (int) pid); // prints the PID for my current printing process
        }
        
        fflush(stdout);
        sleep(3);
        
    }
    
    return 0;
}

void uppercase(char *buffer){
    size_t len = strlen(buffer);
    for (size_t i = 0; i < len; i++){
        buffer[i] = toupper((unsigned char)buffer[i]);  // uses the built-in toupper function from C
    }
}
