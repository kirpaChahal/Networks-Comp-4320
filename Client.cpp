/*
Client.cpp
COMP 4320 Project 1
Authors: 
Shane McNabb srm0081
Connor Sanders cas0203
Kirpa Chahal ksc0037
*/

#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <netdb.h>
#include <netinet/in.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <time.h>
#define BUF_SIZE 512



bool checkTheSum(char* packetData, int checksum)
{
    int count = 0; 
    int i = 0;
    while(packetData[i] != '\0')
    {
        count += packetData[i];
        i++;
    }
    return checksum == count;
}



void damagePacket(int bytes, char *token)
{  
   for(int i = 0; i < bytes; i++)
   {
       int index = rand() % strlen(token);
       token[index] = ((rand() % 20) + 20);
   }
   return;
}

//gremlin

int gremlin(double damageProb, double lossProb, char * token)
{
    int damageNumber = rand() % 100;
    int lossNumber = rand() % 100;
    bool damageCalc = damageNumber < (damageProb * 100); 
    bool lossCalc = lossNumber < (lossProb * 100);
    
    if(lossCalc)
    {
        return 1;
    }
    if(damageCalc)
    {
        //damage the packet
        int dmgBits = rand() % 100;
        if
            (dmgBits < 10) damagePacket(3, token);
        else if(
            dmgBits < 20) damagePacket(2, token);
        else 
            damagePacket(1, token);

        return 0;
    }
    else
    {
        return 0;
    }
    
}



void readAndAssemble(int servfd, struct sockaddr_in serv_addr, socklen_t servSize, double damageProb, double lossProb)
{
    std::ofstream file("clientOutput.txt");
    char fullPackage[BUF_SIZE];
    bzero(fullPackage, 511);
    const char s[2] = "_";
    const char r[2]= ",";
    int checksum, index;
    int counter = 0;
    char *token;
    while(true)
    {
        //receives from server until server sends \0
        recvfrom(servfd, fullPackage, BUF_SIZE, MSG_WAITALL, (struct sockaddr *)&serv_addr, &servSize);
        if(fullPackage[0] == '\0')
        {
            break;
        }        

        //separate out buffer into the header parts and the data
        token = strtok(fullPackage, s);
        index = atoi(token);
        token = strtok(NULL, r);
        checksum = atoi(token);
        token = strtok(NULL, s);

        if (gremlin(damageProb, lossProb, token) == 1)
        {
            std::cout << "Packet " << counter << " is Missing!\n";
            counter ++;
            continue;
        }
        
        if(index != counter)
        {
            std::cout << "Packet " << counter << " is Missing!\n";
            counter ++;
        }

        bool correct = checkTheSum(token, checksum);
        if (!correct)
        {
            std::cout << "Packet " << index << " is corrupted\n";
        }

        //write contents to file
        file << token;
        counter++;

    }
    file.close();

}


int main(int argc, char *argv[]) 
{
    double damageProb = 0.2;
    double lossProb = 0.2; //default values
    
    std::cout << "Enter Damage Probability";
    std::cin >> damageProb;
    std::cout << "Enter Loss Probability";
    std::cin >> lossProb;



    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    socklen_t servSize = sizeof(serv_addr);
    srand (time(NULL));

 
    char buffer[512];
    char userbuffer[12];

    portno = 5001;

   /* Create a socket point */
   sockfd = socket(AF_INET, SOCK_STREAM, 0);

   if (sockfd < 0) 
   {
       perror("ERROR opening socket");
       exit(1);
   }

    

   bzero((char *) &serv_addr, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
   serv_addr.sin_port = htons(portno);

   /* Now connect to the server */
   if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
       perror("ERROR connecting");
       exit(1);
   }

   /* Now ask for a message from the user, this message
      * will be read by server
   */

   printf("enter command: ");
   bzero(buffer,512);
   fgets(buffer,512,stdin);
   /* Send message to the server */
   n = write(sockfd, userbuffer, strlen(buffer));

   if (n < 0) {
       perror("ERROR writing to socket");
       exit(1);
   }

   /* Now read server response */
    bzero(buffer,BUF_SIZE);
    read(sockfd, buffer, BUF_SIZE);
    printf(buffer);
    bzero(buffer, BUF_SIZE);
    
    readAndAssemble(sockfd, serv_addr, servSize, damageProb, lossProb);

   return 0;
}