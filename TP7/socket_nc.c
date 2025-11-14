#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <strings.h>

void error(char *msg) {
    perror(msg);
    exit(1);
}

// socket for my netcat function
int main(void){
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);   // tries to open socket 
        if (sockfd < 0) error("ERROR opening socket");
    struct sockaddr_in serv_addr, cli_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));

    int portno = 8888;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    serv_addr.sin_family = AF_INET;
    
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    listen(sockfd,5);
    int clilen = sizeof(cli_addr);
    int newsockfd ;
    while (1) {

        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0)
            error("ERROR on accept");

        pid_t pid = fork(); // fork the current process to execute in multiple PIDs

        if (pid < 0)
            error("ERROR on fork");

        if (pid == 0) {
            close(sockfd);

            if (dup2(newsockfd, STDIN_FILENO) < 0)  // duplicate my stdin file descriptor to my socket 
                error("dup2 stdin");
            if (dup2(newsockfd, STDOUT_FILENO) < 0) // duplicate my stdout file descriptor to my socket
                error("dup2 stdout");

            close(newsockfd);   // closes my previous socket file as the file descriptor is now my remote stdin/stdout
            execlp("./tp7.exe", "./tp7.exe", (char *)NULL); // execute my ./tp7.exe with the std stream from my socket and sends to my socket the output
            error("execlp failed");
        }

        else {
            close(newsockfd);
        }
    }

    close(sockfd);
    
    return 0;
}