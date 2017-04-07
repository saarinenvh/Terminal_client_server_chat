#include <stdio.h> 
#include <string.h>   //strlen 
#include <stdlib.h> 
#include <errno.h> 
#include <unistd.h>   //close 
#include <arpa/inet.h>    //close 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros 
#include <time.h>
    
#define TRUE   1 
#define FALSE  0 
#define PORT 8888 
    
struct user {
	char nick[21];
	int sd;  
};

int main(int argc , char *argv[])  
{  
    int opt = TRUE;  
    char nick_buffer[22], msg[1046], buffer[1025], welcome_message[1000]; 
    int master_socket, addrlen, new_socket, max_clients = 15, activity, msgread;
    int max_sd, sd;
    int n, i;
    struct user users[15];  
    struct sockaddr_in address;
  	time_t rawtime;
  	struct tm * timeinfo;


    fd_set readfds;  			// Set of socket descriptors
    
    
    if (argc != 2) {
    	fprintf(stderr,"usage: %s <CHATIN NIMI>\n", argv[0]);
    	exit(0);
    }
    
  	
  	// WELCOME MESSAGE TO SERVER
    char *msg_a = "\n\t\tWELCOME TO CHAT SERVER - ";
    strcpy(welcome_message, msg_a);
    strcat(welcome_message, argv[1]);
    strcat(welcome_message, " - Today is ");
    char tmn_start[80];
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
	sprintf(tmn_start, "%d.%d.%d %d:%d:%d \n",timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
	strcat(welcome_message, tmn_start);

	// Initialize all users in user array
    for (i = 0; i < max_clients; i++) {
    	users[i].sd = 0;
    	strcpy(users[i].nick, " ");
    }
        
    // Creates a master socket
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)  {  
        perror("Failed to create socket");  
        exit(EXIT_FAILURE);  
    }  
    
    // Option allows master socket for multiple connections
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 ) {  
        perror("Failed to set options in master socket");  
        exit(EXIT_FAILURE);  
    }  
    
    // Address
    address.sin_family = AF_INET;  
    address.sin_addr.s_addr = INADDR_ANY;  
    address.sin_port = htons( PORT );  
        
    // Binding master socket to port
    if ( bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0 )  {  
        perror("Binding to port failed");  
        exit(EXIT_FAILURE);  
    }  
            
    // Listening connections
    if (listen(master_socket, 3) < 0) {  
        perror("listen");  
        exit(EXIT_FAILURE);  
    }  
    
	printf("Listener works on port: %d \n", PORT);  
    addrlen = sizeof(address);  
    puts("Ready for connections...");  
        
    while(TRUE) {  
    
        //Edit socket set
        FD_ZERO(&readfds);   
        FD_SET(master_socket, &readfds);  
        max_sd = master_socket;  
            
        //Add users to set 
        for ( i = 0 ; i < max_clients ; i++) {  
            sd = users[i].sd;
            if(sd > 0) {            
                FD_SET( sd , &readfds);  
                }
                
            if(sd > max_sd) {
                max_sd = sd;  
            }
        }  
    
		// Activity check from sockets
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);  
      
        if ((activity < 0) && (errno!=EINTR)) {  
            printf("select error");  
        }  
            

        if (FD_ISSET(master_socket, &readfds)) {  
            if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {  
                perror("accept");  
                exit(EXIT_FAILURE);  
            } 
		send(new_socket, "CONNECTED\n", 10, 0);

				    fd_set ackfds;
				    FD_ZERO(&ackfds);
				    FD_SET(new_socket, &ackfds);

				    struct timeval ackTimeOut;
				    memset(&ackTimeOut, 0, sizeof(struct timeval));
				    ackTimeOut.tv_sec = 1;
				    ackTimeOut.tv_usec = 0;
				    char ackBuffer[14];
				    memset(ackBuffer, 0, 14);

				    if (select(new_socket + 1, &ackfds, NULL, NULL, &ackTimeOut) > 0)
				    {
				      if (read(new_socket, ackBuffer, 14) < 0)
				      {
				        break;
				      }

				      if (strcmp(ackBuffer, "ACKNOWLEDGED\n") == 0)
				      {
				     
             
            
            // Print socket information to server
            printf("New connection , socket fd is %d , ip is : %s\n" , new_socket , inet_ntoa(address.sin_addr)); 
            
            
            // Check users nick for welcome text
       		char *nick_msg = "\nPlease enter your nickname: ";
       		char *success = "SUCCESS";
            char msg_b[350] = "\nUsers online: ";
            
            for ( i = 0; i < max_clients; i++ ) {
            	if ( users[i].sd != 0 )  {
            		strcat(msg_b, users[i].nick);
            		strcat(msg_b, " ");
            	} 
            }
            strcat(msg_b, "\n");
            
            // Send messages to new client
            send(new_socket, welcome_message, strlen(welcome_message), 0);
            send(new_socket, msg_b, strlen(msg_b),0);
            send(new_socket, nick_msg, strlen(nick_msg),0);
            send(new_socket, success, strlen(success), 0);
            printf("INIT MESSAGE SENT\n");

                
            //add new socket to array of users
            for (i = 0; i < max_clients; i++) {  
                // If position in array is empty
                if( users[i].sd == 0 ) {  

                    users[i].sd = new_socket;
                    n = read( users[i].sd, nick_buffer, 20);
                    nick_buffer[n] = '\0';
                    
                    // SEND JOIN MESSAGE TO CHANNEL
                    char msg_join[50] = "*** New user joined:  ";
                    strcat(msg_join, nick_buffer);
                    for (int x = 0; x < max_clients; x++) {
                    	if ( users[x].sd != sd) {
	                    	send(users[x].sd , msg_join , strlen(msg_join) , 0 );	                    		
                    	}
                    }
                    
                    // ADD NAME INFO
                    nick_buffer[n - 1] = '\0';
                    strcpy(users[i].nick, nick_buffer); 
 
                    printf("New socket added list into index %d\n" , i);  
                    break;  
                }  
            }  
            }
            }
        }  
            
        // IO - operations
        for (i = 0; i < max_clients; i++)  
        {  
            sd = users[i].sd;
                
            if (FD_ISSET( sd , &readfds)) {  
            
            	// Closing socket and message read
                if ((msgread = read( sd , buffer, 1024)) == 0)  
                {  
                    // Somebody disconnected 
                    getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);  
                    printf("Host disconnected , ip %s , port %d \n" , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));  
                        
                    // close socket
                    close(sd);  
                    users[i].sd = 0;
                    
                    // SEND QUIT MESSAGE TO CHANNEL
                    char quit_msg[50] = "*** User disconnected: ";
                    strcat (quit_msg, users[i].nick);
                    strcat (quit_msg, "\n");
                    for (int x = 0; x < max_clients; x++) {
                    	if ( users[x].sd != sd) {
	                    	send(users[x].sd , quit_msg , strlen(quit_msg) , 0 );	                    		
                    	}
                    }
                    
                    strcpy(users[i].nick, " ");
                    
                } else {  
                
                    buffer[msgread] = '\0';
                    
                    for (int x = 0; x < max_clients; x++) {
                    	
                    	// TIMESTAMP
                    	char tmn[20];
					  	time ( &rawtime );
					  	timeinfo = localtime ( &rawtime );
						sprintf(tmn, "[%d:%d:%d] ",timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
						strcpy(msg,tmn);
						
						// Add nick
                    	strcat(msg, users[i].nick);
                    	strcat(msg, ": ");
                    	
                    	// Add buffer
                    	strcat(msg, buffer);
                    	
                    	if ( users[x].sd != sd) {
	                    	send(users[x].sd , msg , strlen(msg) , 0 );	                    		
                    	}
                    } 
                }  
            }  
        }  
    }  
        
    return 0;  
} 
