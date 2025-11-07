#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/seccomp.h>
#include <sys/prctl.h>
#include <sys/syscall.h>


// for the 3rd question, we use: echo "ecat /etc/passwd > ./attack_result.txt" | ./tp6.exe

int main(){
        
    prctl(PR_SET_SECCOMP, SECCOMP_MODE_STRICT);     // we killed the execution of the program if i try to access other files
    
    // calloc = alocate zeros
    char * buffer = calloc (1024, sizeof(char));    
    char * exec = calloc (1023, sizeof(char));
    int error;
    int a, b, result;
    char op;

    read(0, buffer, 1024);                        // read from standard (0), allocate in buffer, 50 chars
    sscanf(&buffer[0], "%c", &op);              // scan from buffer, %d and allocate in number (filter the chars)  
    if(op == 'e'){
        sscanf(buffer, "e%[^\n]", exec);    // reads buffer till \n
        error = system(exec);               // execute command
        sprintf(buffer, "\nError: %d\n", error);
    } else{
        sscanf(buffer, "%d,%d", &a,&b);
        if (op == '-'){ 
            result = b-a;
        }
        else{ 
            result = b+a;
        }
        sprintf(buffer, "a = %d\nb = %d\nresult = %d\n", a, b, result);   // buffer <= string, with number as argument
    }   
 
    write(1, buffer, strlen(buffer));                       // write in standard (1) from buffer 50 chars

    free(buffer);   // because of calloc
    free(exec);
    
    syscall(SYS_exit, 0);

}
