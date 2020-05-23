#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void error(const char *msg) {
    perror(msg);
    exit(1);
}
int cnt = 0;
int main() {
    FILE *fp = fopen("table.txt","w");     //open a file to store the address list of peers
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;             
    int n,i=0;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);   // creating sockets
    if (sockfd < 0)
         error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = 10006;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;   // initlizing socket parameters 
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)  // binding the socket to given parameters
        error("ERROR on binding");
    listen(sockfd, 5);                  // wait in listen state
    clilen = sizeof(cli_addr);
    //Below code is modified to handle multiple clients using fork
    //------------------------------------------------------------------
    int pid;
    while (1)    //server is running 
    {
         newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);    // accept a connection from peer
         //printf (" new socket %d and %d\t old socket %d\n",i, newsockfd, sockfd); 
         if (newsockfd < 0)
                error("ERROR on accept");
         //fork new process
         ++cnt;

         ++i; 
	 char str[INET_ADDRSTRLEN]; 
         inet_ntop(AF_INET, &(cli_addr.sin_addr), str, INET_ADDRSTRLEN);   //get the ip address of the peer
	 //printf("%s\n", str);	
         pid = fork();                     // create a child process to handle the peer request and parent process again waits for new peers
         if (pid < 0) 
         {
              error("ERROR in new process creation");
         }
         
         if (pid == 0) 
         {
            int j=i-1;
            char sw[100];
            FILE *fp = fopen("table.txt","a+");    // open file in append mode
            
            bzero(buffer, 256);
            //n = read(newsockfd, buffer, 255);
	    recv(newsockfd,buffer,255,0);          // get the peer id sent by the client     
            //printf("%s",buffer);
            strcpy(sw,"/ip4/");
            strcat(sw,str);
            strcat(sw,"/tcp/4001/ipfs/");
            strcat(sw,buffer);                            //create a swarm address by including the client ip and peer id
            fprintf(fp,"%s\n", sw);                       //update the file by entering the client's swarm addresss
            printf("Update in peerlist.txt\n");
            fclose(fp);
            char buffer2[200];
	    FILE *f = fopen ("table.txt", "r+");       // open file in read mode in order to send the list of swarm address to current client
            fscanf(f,"%s\n", buffer2);                 // read swarm address line by line 
	    	//write(newsockfd,buffer2,strlen(buffer2 ));
	    send(newsockfd,buffer2,strlen(buffer2),0);       //send the swarm address to the client         
            //printf("%lu\n",strlen(buffer2));
	    while( strlen(buffer2) > 4)    // repeat the process below unill end of file
            {
                    
                    bzero(buffer2, 256);	
		    fscanf(f,"%s\n", buffer2);   // again read the next line
		    sleep(1);	                 // wait for one seconds so that client can read previous message 
		    send(newsockfd,buffer2,strlen(buffer2),0);  // send the swarm address to the client 
                    //printf("%lu\n",strlen(buffer2));
	           //write(newsockfd,buffer2,strlen(buffer2)); 		
            }  
            bzero(buffer2, 256);
            strcpy(buffer2,"end");    
	    sleep(1);	 
	    send(newsockfd,buffer2,strlen(buffer2),0);		// send the end message to client 
		//write(newsockfd,buffer2,strlen(buffer2)); 
	    fclose(f);
	    close(newsockfd);  //close socket
          }
         
          
               
          
    }
    //-------------------------------------------------------------------
   return 0;
}
