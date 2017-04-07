#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUFSIZE 1024

void closeChat(int sockfd, char *hostname)
{
  printf("Disconnected from %s\n", hostname);
  close(sockfd);
  exit(0);
}


int main(int argc, char **argv)
{
    int sockfd = -1;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    struct addrinfo hints, *res, *ressave;
    char *hostname;
    char buf[BUFSIZE];


    /* check command line arguments */
    if (argc != 3)
    {
       fprintf(stderr,"usage: %s <hostname> <port>\n", argv[0]);
       exit(0);
    }
    hostname = argv[1];

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(hostname, argv[2], &hints, &res) != 0)
    {
        fprintf(stderr,"ERROR, no such host as %s\n", hostname);
        exit(0);
    }
    ressave = res;

    do
    {
        sockfd = socket(res->ai_family, res->ai_socktype, 0);
        if (sockfd < 0)
            continue;

        if (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0)
            break;

        close(sockfd);  /* ignore this one */
    } while ( (res = res->ai_next) != NULL);

    freeaddrinfo(ressave);

    if (sockfd < 0)
    {
      perror("ERROR opening socket");
      exit(0);
    }

	while (1)
  {
    /* print the server's reply */
    memset(buf, 0, BUFSIZE);

    if (read(sockfd, buf, BUFSIZE) < 0)
    {
        perror("ERROR reading from socket");
        exit(0);
    }

    printf("%s",buf);

	    /* get message line from the user */
    memset(buf, 0, BUFSIZE);
    fgets(buf, BUFSIZE, stdin);

    if (strcmp(buf, "disconnect\n") == 0)
        closeChat(sockfd, hostname);

    /* send the message line to the server */

    if (write(sockfd, buf, strlen(buf)) < 0)
    {
      perror("ERROR writing to socket");
      exit(0);
    }
  }

  closeChat(sockfd, hostname);
}
