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
using namespace std;

#define CPORT 23661
#define AWSPORT 24661
#define BACKLOG 10

long fname_process(string finput, int* numbers, int n);

int min(int* nums, int n);
int max(int* nums, int n);
int sum(int* nums, int n);
int sos(int* nums, int n);

void sendtoAWS(int result, char* fname);

int main(){

	struct sockaddr_in server, client, *serverinfo;
	struct sockaddr_storage their_addr;
	int sockfd, newfd;

	//create UDP socket to connect with client (taken from TheSecurityTube Youtube tutorial)
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		
	if (sockfd == -1){
		close(sockfd);
		perror("Socket failed");
		exit(-1);
	}

	memset(&server, 0, sizeof server);
	server.sin_family = AF_INET;
	server.sin_port = htons(CPORT);
	//bzero(&server.sin_zero,0);

	socklen_t len = sizeof(struct sockaddr_in);

	cout << "The Server C is up and running using UDP on port " << CPORT << endl;


	if (bind(sockfd, (struct sockaddr*) &server, len) == -1){
		close(sockfd);
		perror("Bind failed");
		exit(-1);

	}

	while (1){
		//cout << "Listening..." << endl;
		
		//listen for and wait for client to connect
		listen(sockfd, BACKLOG);

		int recn;
		int n;

		//receive UDP packets
		recvfrom(sockfd, &recn, sizeof(recn) , 0, (struct sockaddr*) &client, &len);
		n = ntohl(recn);

		int* numbers = new int[n];
		recvfrom(sockfd, numbers, n*sizeof(int), 0, (struct sockaddr*) &client, &len);

		//print numbers received
		/*for (int i=0; i<n;i++){
			cout << numbers[i] << endl;
		}*/

		cout << "The Server C has received " << n << " numbers." << endl;

		//receive the function to perform
		char fname[4];
		recvfrom(sockfd, fname, 4, 0, (struct sockaddr*) &client, &len);

		int result;
		result = fname_process(fname, numbers, n);

		cout << "The Server C has successfully finished the reduction "<< fname<<": " << result << endl;

		sendtoAWS(result, fname);

		cout << "The Server C has successfully finished sending the reduction value to AWS server" << endl;
	}

	return 0;
}

void sendtoAWS(int result, char* fname){

	struct sockaddr_in server, client, *serverinfo;
	struct sockaddr_storage their_addr;
	struct hostent *hp;
	int sockfd, newfd;

	//create UDP socket to connect with client
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		
	if (sockfd == -1){
		close(sockfd);
		perror("Socket failed");
		exit(-1);
	}

	memset(&server, 0, sizeof server);
	server.sin_family = AF_INET;
	server.sin_port = htons(AWSPORT);
	//bzero(&server.sin_zero,0);
	hp = gethostbyname("24661");

	if (hp==0){
		perror("Host error");
		exit(-1);
	}

	memcpy((char*)hp->h_addr, (char*)&server.sin_addr, hp->h_length);


	socklen_t len = sizeof(struct sockaddr_in);

	/*if ((connect(sockfd, (struct sockaddr*) &server, len)) == -1){

		perror("Connect failed");
		exit(-1);
	}*/

	//send signal to identify serverC
	int final = htonl(result);
	char servename[2]="C";
	sendto(sockfd, servename, sizeof(servename), 0, (struct sockaddr*) &server, len);

	sendto(sockfd, (const char*) &final, sizeof(final), 0, (struct sockaddr*) &server, len);
	sendto(sockfd, fname, sizeof(fname), 0, (struct sockaddr*) &server, len);
}


long fname_process(string finput, int* numbers, int n){
	char fname[4];
	int result;


	if (finput.compare("SUM")==0){
		strcpy(fname, "SUM");
		result = sum(numbers, n);
	}
	else if (finput.compare("MIN")==0){
		strcpy(fname,"MIN");
		result = min(numbers, n);
	}
	else if (finput.compare("MAX")==0){
		strcpy(fname, "MAX");
		result = max(numbers, n);
	}
	else if (finput.compare("SOS")==0){
		strcpy(fname, "SOS");
		result = sos(numbers, n);
	}
	else{
		perror("Incorrect function name");
		exit(1);
	}

	return result;
}

int min(int* nums, int n){

	int minimum = 10000;
	for (int i=0; i<n-1; i++){
		if (nums[i] < minimum){
			minimum = nums[i];
		}
	}

	return minimum;

}
int max(int* nums, int n){
	int maximum = -10000;

	for (int i = 0; i<n; i++){
		if (nums[i] > maximum){
			maximum = nums[i];
		}
	}
	return maximum;

}
int sum(int* nums, int n){
	int summation = 0;

	for (int i = 0; i<n; i++){
		
		summation += nums[i];
	}

	return summation;

}
int sos(int* nums, int n){
	int sumsqr = 0;

	for (int i = 0; i<n; i++){
		
		sumsqr += (nums[i]*nums[i]);
	}

	return sumsqr;
}