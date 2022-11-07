/*
Server.cpp
COMP 4320 Project 1
Authors: 
Shane McNabb srm0081
Connor Sanders cas0203
Kirpa Chahal ksc0037
*/





#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <string>
#define BUF_SIZE 512



char readFile() 
{
   std::ifstream myfile;
   myfile.open("TestFile.txt");
   std::string myline;
   
   if ( myfile.is_open() ) {
       while ( myfile )
       {
           std::getline (myfile, myline);
           std::cout << myline << '\n';
       }
   }
   else {
       std::cout << "Couldn't open file\n";
   }
   return 0;
}

int checkSum(char* packetData)
{
    int count=0;
    int i = 0;
    while(packetData[i] != '\0')
    {
        count += packetData[i];
        i++;
    }
    return count;
}


void processAndSend(int clisockfd, char *fileName, struct sockaddr_in client)
{
    char buffer[BUF_SIZE];
    char packetData[BUF_SIZE - 14];
    int index = 0;
    FILE *f = fopen(fileName, "r");
    int count = 0;

    while(fgets(packetData, sizeof(packetData), f) != NULL)
    {
        char fullPackage[BUF_SIZE];
        int sum = checkSum(packetData);
        
        //Formatting the message (index)|(checksum)|(data)\n
        sprintf(fullPackage, "%01d_%01d,%s", index, sum, packetData);
        std::cout << "Sending package " << fullPackage[0] << ": " << fullPackage;
        sendto(clisockfd, fullPackage, BUF_SIZE, 2048, (const struct sockaddr *)&client, sizeof(client));

        index++;
    }
}

int main( int argc, char *argv[] ) {
   //connecting to client
   int sockfd, newsockfd, portno, clilen;
   char buffer[BUF_SIZE];
   struct sockaddr_in serv_addr, cli_addr;
   int n;
   char *file = "TestFile.txt";


   /* First call to socket() function */
   sockfd = socket(AF_INET, SOCK_STREAM, 0);

   if (sockfd < 0) {
       perror("ERROR opening socket");
       exit(1);
   }

   /* Initialize socket structure */
   bzero((char *) &serv_addr, sizeof(serv_addr));
   portno = 5001;

   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = INADDR_ANY;
   serv_addr.sin_port = htons(portno);

   /* Now bind the host address using bind() call.*/
   if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
       perror("ERROR on binding");
       exit(1);
   }

   /* Now start listening for the clients, here process will
      * go in sleep mode and will wait for the incoming connection
   */

   listen(sockfd,5);
   clilen = sizeof(cli_addr);

   /* Accept actual connection from the client */
   newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, reinterpret_cast<socklen_t *>(&clilen));

   if (newsockfd < 0) {
       perror("ERROR on accept");
       exit(1);
   }

/* If connection is established then start communicating */
   bzero(buffer,BUF_SIZE);
   
   //read command from client
   n = read(newsockfd,buffer,512);
    printf(buffer);
   if (n < 0) {
       perror("ERROR reading from socket");
       exit(1);
   }
    
    
    //write response to client that code is starting
    n = write(newsockfd,"Getting TestFile\n",17);
        if (n < 0) {
            perror("ERROR writing to socket");
            exit(1);}
        
    //begin processing file into packets, adding headers, and sending to client
    processAndSend(newsockfd, file, serv_addr);
        
    //write end of file?
    bzero(buffer,BUF_SIZE);
    n = write(newsockfd, "\0", BUF_SIZE);
   

/* Write a response to the client */
    n = write(newsockfd,"I got to the end",18);
    close(newsockfd);
    return n;
}