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

#define APORT 21661
#define BPORT 22661
#define CPORT 23661
#define AWSPORT 24661
#define TCPPORT 25661
#define BACKLOG 10


void sendtoA(int* numbers, int n, char* fname);
void sendtoB(int* numbers, int n, char* fname);
void sendtoC(int* numbers, int n, char* fname);
void send_client(int final, int port, int addr);
int recfromBack();

int process_calc(char* funcname, int num1, int num2, int num3);

int main(){
	

	struct sockaddr_in server, client, *serverinfo;
	struct sockaddr_storage their_addr;
	int sockfd, newfd;
	int clientport, clientaddr;

	//create TCP socket to connect with client (taken from TheSecurityTube Youtube tutorial)
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
		
	if (sockfd == -1){
		close(sockfd);
		perror("Socket failed");
		exit(-1);
	}

	memset(&server, 0, sizeof server);
	server.sin_family = AF_INET;
	server.sin_port = htons(TCPPORT);
	//bzero(&server.sin_zero,0);

	socklen_t len = sizeof(struct sockaddr_in);

	cout << "The AWS is up and running." << endl;
	
	//be able to reuse the socket address (taken from stack overflow)
	int optval = 1;
	if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(int)) == -1) {
     
        perror("setsockopt") ;
        exit(-1) ;
    }

	if (bind(sockfd, (struct sockaddr*) &server, len) == -1){
		close(sockfd);
		perror("Bind failed");
		exit(-1);

	}

	//listen for and wait for client to connect and accept
	listen(sockfd, BACKLOG);

	//while(1){
		newfd = accept(sockfd, (struct sockaddr *)&client, &len);

		if (newfd == -1){

			perror("Accept failed");
			exit(-1);
		}
		
		clientport = client.sin_port;
		clientaddr = client.sin_addr.s_addr;
		//cout << "Client port number is: " << client.sin_port << endl;

		char fname[4];
		//receive the function name
		int r = recv(newfd, fname, sizeof(fname), 0);
		if (r == -1){
			perror("Receive failed");
			exit(-1);
		}
		
		//cout << "Received fname: " << fname << endl;
		
		//receive the number of values in the file
		int recn;
		int n;
		recv(newfd, &recn, sizeof(recn), 0);
		n = ntohl(recn);

		//receive array of numbers to distribute
		int* numbers = new int [n];
		int buffer = 12000;
		int* temp[12000];
		
		int i =0;
		int Xfinal = 0;

		/*while(1){
			
			int bytes = recv(newfd, temp, buffer,0);
			if (bytes == -1){
				perror("Receive failed: ");
				exit(1);
			}
			//receive until delimiter tells the end of numbers
			if (temp[i] == '\0'){
				break;
			}
			i++;
		}*/
		int bytes = recv(newfd, temp, buffer,0);
			if (bytes == -1){
				perror("Receive failed: ");
				exit(1);
			}
		numbers = (int*) temp;

		//print out numbers received
		/*for (int i=0;i<n;i++){
			cout << numbers[i] << endl;
		}*/

		cout << "The AWS has received " << n << " numbers from the client using TCP over port " << TCPPORT << endl;

		//test print 1st numbers received from client		
		//cout << numbers[0] << " " << numbers[1] << endl;
		
		//distribute the numbers across the 3 servers
		sendtoA(numbers, n, fname);
		sendtoB(numbers, n, fname);
		sendtoC(numbers, n, fname);

		Xfinal = recfromBack();

		cout << "The AWS has successfully finished the reduction " << fname <<": "<< Xfinal << endl;

		int finnum;
		finnum = htonl(Xfinal);

		//send back to client
		//send_client(Xfinal, clientport, clientaddr);
		send(newfd, (const char*) &finnum, sizeof(finnum), 0);

		cout << "The AWS has successfully finished sending the reduction value to client" << endl;
		
		shutdown(newfd,0);
		close(newfd);
	//}	

	return 0;
}


void sendtoA(int* numbers, int n, char* fname){

	int* Anumbers = new int[n/3];
	for (int i = 0; i< n/3; i++){

		Anumbers[i] = numbers[i];
	}
	

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
	server.sin_port = htons(APORT);
	//bzero(&server.sin_zero,0);
	hp = gethostbyname("21661");

	if (hp==0){
		perror("Host error");
		exit(-1);
	}

	memcpy((char*)hp->h_addr, (char*)&server.sin_addr, hp->h_length);
	//cout << server.sin_addr.s_addr << endl;

	socklen_t len = sizeof(struct sockaddr_in);

	/*if ((connect(sockfd, (struct sockaddr*) &server, len)) == -1){

		perror("Connect failed");
		exit(-1);
	}
	*/
	int count = htonl(n/3);

	//send # of numbers to be received by ServerA
	int z =sendto(sockfd, (const char*) &count, sizeof(count), 0, (struct sockaddr*) &server, len);
	if (z<0){
		perror("sendto failed: ");
		exit(-1);
	}
	//send the numbers to ServerA
	sendto(sockfd, Anumbers, (n/3)*sizeof(int), 0, (struct sockaddr*) &server, len);

	//send the function 
	sendto(sockfd, fname, sizeof(fname),0, (struct sockaddr*) &server, len);


	cout << "The AWS sent "<< n/3 <<" numbers to Backend Server A." << endl;
	
	/*int Aresult;
	int numA;
	char funcname[4];
	recvfrom(sockfd, &Aresult, sizeof(Aresult),0, (struct sockaddr*)&client, &len);
	numA = ntohl(Aresult);

	recvfrom(sockfd, funcname, sizeof(funcname), 0, (struct sockaddr*) &client, &len);

	cout << "The AWS received reduction result of "<< funcname<< " from Backend " 
		"Server A using UDP over port "<< AWSPORT << " and it is: " << numA << endl;*/

}

void sendtoB(int* numbers, int n, char* fname){

	int* Bnumbers = new int[n/3];
	for (int i = 0; i<n/3; i++){

		Bnumbers[i] = numbers[i+(n/3)];
	}

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
	server.sin_port = htons(BPORT);
	//bzero(&server.sin_zero,0);
	hp = gethostbyname("22661");

	if (hp==0){
		perror("Host error");
		exit(-1);
	}

	memcpy((char*)hp->h_addr, (char*)&server.sin_addr, hp->h_length);

	socklen_t len = sizeof(struct sockaddr_in);

	int count = htonl(n/3);

	//send # of numbers to be received by ServerB
	sendto(sockfd, (const char*) &count, sizeof(count), 0, (struct sockaddr*) &server, len);

	//send the numbers to ServerB
	sendto(sockfd, Bnumbers, (n/3)*sizeof(int), 0, (struct sockaddr*) &server, len);


	//send the function 
	sendto(sockfd, fname, sizeof(fname),0, (struct sockaddr*) &server, len);

	cout << "The AWS sent "<< n/3 <<" numbers to Backend Server B." << endl;
}

void sendtoC(int* numbers, int n, char* fname){
	int* Cnumbers = new int[n/3];
	for (int i = 0; i<n/3; i++){

		Cnumbers[i] = numbers[i+(2*n/3)];
	}

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
	server.sin_port = htons(CPORT);
	//bzero(&server.sin_zero,0);
	hp = gethostbyname("23661");

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

	int count = htonl(n/3);

	//send # of numbers to be received by ServerA
	sendto(sockfd, (const char*) &count, sizeof(count), 0, (struct sockaddr*) &server, len);

	//send the numbers to ServerA
	sendto(sockfd, Cnumbers, (n/3)*sizeof(int), 0, (struct sockaddr*) &server, len);


	//send the function 
	sendto(sockfd, fname, sizeof(fname),0, (struct sockaddr*) &server, len);

	cout << "The AWS sent "<< n/3 <<" numbers to Backend Server C." << endl;

}

int recfromBack(){

	struct sockaddr_in server1, Aserver, Bserver, Cserver, *serverinfo;
	struct sockaddr_storage their_addr;
	int sockfd1;

	int Aresult, Bresult, Cresult;
	int numA, numB, numC;
	int final;

	char funcname[4];

	//create UDP socket to receive from backend servers
	sockfd1 = socket(AF_INET, SOCK_DGRAM, 0);
		
	if (sockfd1 == -1){
		close(sockfd1);
		perror("Socket failed");
		exit(-1);
	}

	memset(&server1, 0, sizeof server1);

	memset(&Aserver, 0, sizeof Aserver);
	memset(&Bserver, 0, sizeof Bserver);
	memset(&Cserver, 0, sizeof Cserver);
	server1.sin_family = AF_INET;
	server1.sin_port = htons(AWSPORT);
	//bzero(&server1.sin_zero,0);

	socklen_t len = sizeof(struct sockaddr_in);
	
	if (bind(sockfd1, (struct sockaddr*) &server1, len) == -1){
		close(sockfd1);
		perror("Bind failed");
		exit(-1);

	}
	
	//listen for and wait for backend servers to send their results
	listen(sockfd1, BACKLOG);
	bool A = false, B = false, C = false;
	char serv[2];

	while(!(A && B && C)){
		
		recvfrom(sockfd1, serv, 2, 0, (struct sockaddr*)serverinfo, &len);
		//string serv = buff;
		cout << "RECEIVED FROM: " << serv << endl;
	

		if (strcmp(serv, "A") == 0){
			recvfrom(sockfd1, &Aresult, sizeof(Aresult),0, (struct sockaddr*)&Aserver, &len);
			numA = ntohl(Aresult);

			recvfrom(sockfd1, funcname, sizeof(funcname), 0, (struct sockaddr*) &Aserver, &len);

			cout << "The AWS received reduction result of "<< funcname<< " from Backend " 
				"Server A using UDP over port "<< AWSPORT << " and it is: " << numA << endl;
			
			A = true;
		}

		else if (strcmp(serv, "B")==0){
			recvfrom(sockfd1, &Bresult, sizeof(Bresult),0, (struct sockaddr*)&Bserver, &len);
			numB = ntohl(Bresult);


			recvfrom(sockfd1, funcname, 4, 0, (struct sockaddr*) &Bserver, &len);

			cout << "The AWS received reduction result of "<< funcname<< " from Backend " 
				"Server B using UDP over port "<< AWSPORT << " and it is: " << numB << endl;	

			B = true;	
		}

		else if (strcmp(serv, "C")==0){
			recvfrom(sockfd1, &Cresult, sizeof(Cresult),0, (struct sockaddr*)&Cserver, &len);
			numC = ntohl(Cresult);

			recvfrom(sockfd1, funcname, 4, 0, (struct sockaddr*) &Cserver, &len);

			cout << "The AWS received reduction result of "<< funcname<< " from Backend " 
				"Server C using UDP over port "<< AWSPORT << " and it is: " << numC << endl;

			C = true;	
		}

	
	}

	//calculate final value from each of the servers	
	final = process_calc(funcname, numA, numB, numC);

	return final;
}

int process_calc(char* funcname, int numA, int numB, int numC){

	int Xfinal;

	if (strcmp(funcname,"SUM")==0){
		Xfinal = numA + numB + numC;
	}
	else if (strcmp(funcname,"MIN")==0){
		int temp;

		if (numA < numB){
			
			temp = numA;
			if (temp > numC){
				temp = numC;
			}
			
		}
		else{
			temp = numB;
			if (numC < temp){
				temp = numC;
			}
			
		}
		Xfinal = temp;
	}

	else if (strcmp(funcname, "MAX")==0){
		int temp;

		if (numA > numB){
			
			temp = numA;
			if (temp < numC){
				temp = numC;
			}
			
		}
		else{
			temp = numB;
			if (numC > temp){
				temp = numC;
			}
			
		}

		Xfinal = temp;
	}
	else if (strcmp(funcname, "SOS")==0){
		Xfinal = numA + numB + numC;
	}
	else{
		perror("Incorrect function name");
		exit(1);
	}
	return Xfinal;
}

void send_client(int final, int port, int addr){

	struct sockaddr_in client;

	int sockfd, newfd;
	memset(&client, 0, sizeof client);


	//cout << "Creating socket...." << endl;

	//Create TCP Socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd == -1){
		close(sockfd);
		perror("Socket failed");
		exit(-1);

	}		

	//code taken from youtube video
	client.sin_family = AF_INET;
	client.sin_port = htons(port);
	client.sin_addr.s_addr = addr;
	//bzero(&client.sin_zero,0);

	socklen_t len = sizeof(struct sockaddr_in);

	//cout << "CONNECTING..." << endl;
	if ((connect(sockfd, (struct sockaddr*) &client, len)) == -1){

		perror("Connect failed");
		exit(-1);
	}

	int finnum;
	finnum = htonl(final);
	send(sockfd, (const char*) &finnum, sizeof(final),0);
}