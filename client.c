#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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

  int fdready = 0;
  fd_set inputfds;
  FD_ZERO(&inputfds);

  struct timeval inputTimeOut;
  memset(&inputTimeOut, 0, sizeof(struct timeval));
  inputTimeOut.tv_sec = 1;
  inputTimeOut.tv_usec = 0;

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

  printf("chat loop\n");
	while (1)
  {
    FD_ZERO(&inputfds);
    FD_SET(0, &inputfds);
    FD_SET(sockfd, &inputfds);

    inputTimeOut.tv_sec = 0;
    inputTimeOut.tv_usec = 0;

    if ((fdready = select(sockfd + 1, &inputfds, NULL, NULL, &inputTimeOut)) > 0)
    {
      memset(buf, 0, BUFSIZE);
      if (FD_ISSET(0, &inputfds))
      {
        read(0, buf, BUFSIZE);

        if (strcmp(buf, "disconnect\n") == 0)
          closeChat(sockfd, hostname);

        if (write(sockfd, buf, strlen(buf)) < 0)
        {
          perror("ERROR writing to socket");
          exit(0);
        }
      }
      else if (FD_ISSET(sockfd, &inputfds))
      {
        read(sockfd, buf, BUFSIZE);

        printf("%s",buf);
      }
    }

    if (fdready < 0)
    {
      perror("ERROR reading from socket");
      exit(0);
    }
  }

  closeChat(sockfd, hostname);
}
