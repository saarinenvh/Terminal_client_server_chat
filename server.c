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
    fd_set readfds;  			// Set of socket descriptors
    
    if (argc != 2) {
    	fprintf(stderr,"usage: %s <CHATIN NIMI>\n", argv[0]);
    	exit(0);
    }
    
  	
  	// WELCOME MESSAGE TO SERVER
    char *msg_a = "WELCOME TO CHAT SERVER ";
    strcpy(welcome_message, msg_a);
    strcat(welcome_message, argv[1]);

   
    

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
        
    while(TRUE)  
    {  
        //Edit socket set
        FD_ZERO(&readfds);   
        FD_SET(master_socket, &readfds);  
        max_sd = master_socket;  
            
        //add child sockets to set 
        for ( i = 0 ; i < max_clients ; i++) {  
            sd = users[i].sd;
            if(sd > 0) {            
                FD_SET( sd , &readfds);  
                }
                
            if(sd > max_sd) {
                max_sd = sd;  
            }
        }  
    
		// Waiting for any activity in sockets
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);  
      
        if ((activity < 0) && (errno!=EINTR)) {  
            printf("select error");  
        }  
            

        if (FD_ISSET(master_socket, &readfds))  
        {  
            if ((new_socket = accept(master_socket, 
                    (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)  
            {  
                perror("accept");  
                exit(EXIT_FAILURE);  
            }  
            
            //inform user of socket number - used in send and receive commands 
            printf("New connection , socket fd is %d , ip is : %s\n" , new_socket , inet_ntoa(address.sin_addr)); 
            char msg_b[350] = "\nKanavalla käyttäjiä: ";
            
            for ( i = 0; i < max_clients; i++ ) {
            	if ( users[i].sd != 0 )  {
            		strcat(msg_b, users[i].nick);
            		strcat(msg_b, " ");
            	} 
            }
            
       		char *nick_msg = "\nPlease enter your nickname: ";
       		char *success = "SUCCESS";
            //send new connection greeting message 
            send(new_socket, welcome_message, strlen(welcome_message), 0);
            send(new_socket, msg_b, strlen(msg_b),0);
            send(new_socket, nick_msg, strlen(nick_msg),0);
            send(new_socket, success, strlen(success), 0);
            puts("Welcome message sent successfully"); 

                
            //add new socket to array of sockets 
            for (i = 0; i < max_clients; i++)  
            {  
                //if position is empty 
                if( users[i].sd == 0 )  
                {  

                    users[i].sd = new_socket;
                    n = read( users[i].sd, nick_buffer, 20);
                    char *join_msg = nick_buffer;
                    nick_buffer[n-1] = ':';
                    nick_buffer[n] = ' ';
                    nick_buffer[n + 1] = '\0';
                    strcpy(users[i].nick, nick_buffer); 

                    for (int x = 0; x < max_clients; x++) {
                    	if ( users[x].sd != sd) {
	                    	send(users[x].sd , join_msg , strlen(join_msg) , 0 );	                    		
                    	}
                    	}
                    
                               	
                    printf("Adding to list of sockets as %d\n" , i);  
                    break;  
                }  
            }  
        }  
            
        //else its some IO operation on some other socket
        for (i = 0; i < max_clients; i++)  
        {  
            sd = users[i].sd;

                
            if (FD_ISSET( sd , &readfds))  
            {  
                //Check if it was for closing , and also read the 
                //incoming message 
                if ((msgread = read( sd , buffer, 1024)) == 0)  
                {  
                    //Somebody disconnected , get his details and print 
                    getpeername(sd , (struct sockaddr*)&address , \
                        (socklen_t*)&addrlen);  
                    printf("Host disconnected , ip %s , port %d \n" , 
                          inet_ntoa(address.sin_addr) , ntohs(address.sin_port));  
                        
                    //Close the socket and mark as 0 in list for reuse 
                    close( sd );  
                    users[i].sd = 0;
                    strcpy(users[i].nick, " ");
                }  
                    
                //Send message to all clients
                else
                {  
                    //set the string terminating NULL byte on the end 
                    //of the data read 
                    buffer[msgread] = '\0';
                    for (int x = 0; x < max_clients; x++) {
                    	strcpy(msg, users[i].nick);
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
