
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char * argv[])   // get the ip address of bootstarp server from client in command line
{

    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    if(argc < 2)
    {
        printf("bootstarp server address misssing\n");
    }

    char buffer[256];
    char recvline[200];
    char sendline[10]="h";
    portno = 10006;                         //create and initilize socket parameters
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);   // set the ip address of bootstrap node
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);                                                 // initilize the socket
    
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
         error("ERROR connecting");
    //printf("Please enter Your Swarm Address: ");
    bzero(buffer,256);
    FILE *f = popen("ipfs id | awk 'FNR == 2 {print $2}'| awk '{print substr($0,2,length()-3);}'","r");  // fetech the peer ID of the system by executing shell command
         
    fscanf(f,"%s", buffer);   /* read output from command */
     
    fclose(f);
    
    //fgets(buffer,255,stdin);
	    //n = write(sockfd,buffer,strlen(buffer));
    send(sockfd,buffer,strlen(buffer),0);  		// send the peer ID to the bootstrap node    
    bzero(buffer,256);
           
    FILE *fp = fopen("swarmpeers.txt", "w+");   //open the file to store the swarm addresses of other peers
    recv(sockfd,buffer,255,0);                  // receive the address of swarm peer from server  
		//n = read(sockfd, buffer, 255);
    fprintf(fp,"%s\n",buffer);                   //write the address into file 
    //printf("%lu\n",strlen(buffer));
    //printf("%s\n",buffer);		
    while(strlen(buffer) > 4)       // repeat the process below until end messgae is recevide
    {   
            //printf("helloo");

            bzero(buffer,256);          
            //n = read(sockfd, buffer, 255);
	    recv(sockfd,buffer,255,0);   // again receive the next swarm address from server
	    if(strlen(buffer) > 5) 		
	    fprintf(fp,"%s\n",buffer);  // writing to the file
	     //printf("%s\n",buffer); 
            //printf("%lu\n",strlen(buffer));	
             //bzero(buffer,256);
            //send(sockfd,sendline,strlen(sendline),0);
	    
   }
   fclose(fp); //close file
   close(sockfd); //close socket
    return 0;
}
