#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

#define PORT 25661
#define BACKLOG 10

int receivefinal();

int main(int argc, char* argv[]){

	if (argc != 2){
		perror("Input the function name.");
		exit(-1);
	}
	string finput(argv[1]);
	char fname[4];
	if (finput.compare("sum")==0){
		strcpy(fname, "SUM");
	}
	else if (finput.compare("min")==0){
		strcpy(fname,"MIN");
	}
	else if (finput.compare("max")==0){
		strcpy(fname, "MAX");
	}
	else if (finput.compare("sos")==0){
		strcpy(fname, "SOS");
	}
	else{
		perror("Incorrect function name");
	}


	//cout << fname << endl;

	struct sockaddr_in server, client;

	int sockfd, newfd;
	memset(&server, 0, sizeof server);

	//cout << "Creating socket...." << endl;

	//Create TCP Socket (beej's tutorial)
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd == -1){
		close(sockfd);
		perror("Socket failed");
		exit(-1);

	}

	int optval = 1;
	if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(int)) == -1) {
     
        perror("setsockopt") ;
        exit(-1) ;
    }

	//(code taken from TheSecurityTube Youtube tutorial)
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	server.sin_addr.s_addr = INADDR_ANY;
	//bzero(&server.sin_zero,0);

	socklen_t len = sizeof(struct sockaddr_in);

	//cout << "CONNECTING..." << endl;
	if ((connect(sockfd, (struct sockaddr*) &server, len)) == -1){

		perror("Connect failed");
		exit(-1);
	}

	cout << "The client is up and running." << endl;

	FILE* pfile;
	string temp;

	int count = 0;

	pfile = fopen("nums.csv", "r");
	fseek(pfile, 0L, SEEK_END);
	int filesize = ftell(pfile);
	int* numbers = new int[1000];
	char* buffer[12000];

	ifstream myfile;
	myfile.open("nums.csv");

	if (myfile.is_open()){
		while (myfile.good()){
			getline(myfile,temp);
			numbers[count] = atoi(temp.c_str());
			count++;
		}
		//numbers[count+1] = '\0';
		myfile.close();
		count--;
	}
	else{
		cout << "Can't open file" << endl;
		return 1;
	}

	send(sockfd, fname, sizeof(fname),0);
	cout << "The client has sent the reduction type " << fname << " to AWS." << endl;

	int n = htonl(count);
	send(sockfd, (const char*) &n, sizeof(count), 0);

	//test print out 1st numbers read from file
	//cout << numbers[0] << ", " << numbers[1] << endl;

	//send numbers read from file to AWS
	send(sockfd, numbers, count*sizeof(int), 0);

	cout << "The client has sent " << count << " numbers to AWS." << endl;

	long Xfinal; 
	//Xfinal = receivefinal();
	//receive the final result from AWS
	int recn;
	
	listen(sockfd, BACKLOG);
	recv(sockfd, &recn, sizeof(recn), 0);
	Xfinal = ntohl(recn);

	cout << "The client has received reduction "<< fname << ": "<< Xfinal << endl;

	free(numbers); //clear the buffer memory
	close(sockfd);
	return 0;
}

int receivefinal(){
	struct sockaddr_in server, client, *serverinfo;
	struct sockaddr_storage their_addr;
	int sockfd, newfd;
	int clientport, clientaddr;

	//create TCP socket to connect with server
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
		
	if (sockfd == -1){
		close(sockfd);
		perror("Socket failed");
		exit(-1);
	}

	memset(&server, 0, sizeof server);
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	//bzero(&server.sin_zero,0);

	socklen_t len = sizeof(struct sockaddr_in);

	cout << "The AWS is up and running." << endl;
	
	if (bind(sockfd, (struct sockaddr*) &server, len) == -1){
		close(sockfd);
		perror("Bind failed");
		exit(-1);

	}
	
	//listen for and wait for server to connect and accept
	listen(sockfd, BACKLOG);
	newfd = accept(sockfd, (struct sockaddr *)&client, &len);

	if (newfd == -1){

		perror("Accept failed");
		exit(-1);
	}

	//receive the final result from AWS
	int recn;
	int n;
	recv(sockfd, &recn, sizeof(recn), 0);
	n = ntohl(recn);

	return n;
}