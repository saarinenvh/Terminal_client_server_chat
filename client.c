#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <signal.h>

#define BUFSIZE 1024

void closeChat(int sockfd, char *hostname)
{
  printf("Disconnected from %s\n", hostname);
  close(sockfd);
  exit(0);
}

void handleSIGPIPE(int signum)
{
  printf("Connection to server was lost!\n");
  exit(signum);
}


int main(int argc, char **argv)
{
  int sockfd = -1;
  struct sockaddr_in serveraddr;
  struct hostent *server;
  struct addrinfo hints, *res, *ressave;
  char *hostname;
  char buf[BUFSIZE];

  int connected = 0;
  int fdready = 0;
  fd_set inputfds;
  FD_ZERO(&inputfds);

  signal(SIGPIPE, handleSIGPIPE);

  struct timeval inputTimeOut;
  memset(&inputTimeOut, 0, sizeof(struct timeval));

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
    fprintf(stderr,"Error, no such host as %s\n", hostname);
    exit(0);
  }
  ressave = res;

  do
  {
    sockfd = socket(res->ai_family, SOCK_STREAM, 0);
    if (sockfd < 0)
      continue;

    if (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0)
      break;

    close(sockfd);  /* ignore this one */
  } while ( (res = res->ai_next) != NULL);

  freeaddrinfo(ressave);

  if (sockfd < 0)
  {
    perror("Error opening socket");
    exit(0);
  }

  inputTimeOut.tv_sec = 5;
  inputTimeOut.tv_usec = 0;
  FD_SET(sockfd, &inputfds);

  if (select(sockfd + 1, &inputfds, NULL, NULL, &inputTimeOut) > 0)
  {
    memset(buf, 0, BUFSIZE);
    if (read(sockfd, buf, BUFSIZE) < 0)
    {
      perror("Error reading from socket");
      exit(0);
    }

    if (strcmp(buf, "CONNECTED\n") != 0)
    {
      fprintf(stderr, "Cannot connect to %s\n", hostname);
      exit(0);
    }

    if (send(sockfd, "ACKNOWLEDGED\n", 14, 0) < 0)
    {
      perror("Error acknowledging server");
      exit(0);
    }
  }
  else
  {
    fprintf(stderr, "Cannot connect to %s\n", hostname);
    exit(0);
  }

  if (setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &(int){1}, sizeof(int)) < 0)
  {
    perror("Error modifying socket");
    exit(0);
  }

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
      if (FD_ISSET(0, &inputfds) && connected)
      {
        read(0, buf, BUFSIZE);

        if (strcmp(buf, "disconnect\n") == 0)
          closeChat(sockfd, hostname);

        if (send(sockfd, buf, strlen(buf), 0) < 0)
        {
          perror("Error writing to socket");
          exit(0);
        }
      }
      else if (FD_ISSET(sockfd, &inputfds))
      {
        if (read(sockfd, buf, BUFSIZE) < 0)
        {
          perror("Error reading from socket");
          exit(0);
        }

        if (strstr(buf, "SUCCESS") != NULL)
        {
          strcpy(strstr(buf, "SUCCESS"), "\0");
          connected = 1;
        }
        printf("%s",buf);
        fflush(stdout);
      }
    }

    if (fdready < 0)
    {
      perror("Error selecting socket");
      exit(0);
    }
  }

  closeChat(sockfd, hostname);
}
