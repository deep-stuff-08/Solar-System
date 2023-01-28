#include"include/planetinfo.h"
#include <arpa/inet.h>
#include <assert.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/file.h>

void getValuesFor(FILE* f, const char* str, char (*arr)[10]) {
	int cu = 0;

	while(str[cu] != '\0') {
		if(fgetc(f) == str[cu]) {
			cu++;
		} else {
			cu = 0;
		}
	}
	for(int j = 1; j < 9; j++) {
		int buffer1pos = 0;
		char ch;
		while((ch = fgetc(f)) != '<') {
			arr[j][buffer1pos] = ch;
			buffer1pos++;
		}
		arr[j][buffer1pos] = '\0';
		char breaker[] = "/center></td><td><center>";
		char *currentPos = breaker; 
		
		while(*currentPos != '\0') {
			if(fgetc(f) == *currentPos) {
				currentPos++;
			} else {
				currentPos = breaker;
			}
		}
	}
}

void initPlanetInfo() {
	char buffer[BUFSIZ];
	enum CONSTEXPR { MAX_REQUEST_LEN = 1024};
	char request[MAX_REQUEST_LEN];
	char request_template[] = "GET /hbase/Solar/soldata.html HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n";
	struct protoent *protoent;
	char hostname[] = "hyperphysics.phy-astr.gsu.edu";
	in_addr_t in_addr;
	int request_len;
	int socket_file_descriptor;
	ssize_t nbytes_total, nbytes_last;
	struct hostent *hostent;
	struct sockaddr_in sockaddr_in;
	unsigned short server_port = 80;

	request_len = snprintf(request, MAX_REQUEST_LEN, request_template, hostname);
	if (request_len >= MAX_REQUEST_LEN) {
		fprintf(stderr, "request length large: %d\n", request_len);
		exit(EXIT_FAILURE);
	}

	protoent = getprotobyname("tcp");
	if (protoent == NULL) {
		perror("getprotobyname");
		exit(EXIT_FAILURE);
	}
	socket_file_descriptor = socket(AF_INET, SOCK_STREAM, protoent->p_proto);
	if (socket_file_descriptor == -1) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	hostent = gethostbyname(hostname);
	if (hostent == NULL) {
		fprintf(stderr, "error: gethostbyname(\"%s\")\n", hostname);
		exit(EXIT_FAILURE);
	}
	in_addr = inet_addr(inet_ntoa(*(struct in_addr*)*(hostent->h_addr_list)));
	if (in_addr == (in_addr_t)-1) {
		fprintf(stderr, "error: inet_addr(\"%s\")\n", *(hostent->h_addr_list));
		exit(EXIT_FAILURE);
	}
	sockaddr_in.sin_addr.s_addr = in_addr;
	sockaddr_in.sin_family = AF_INET;
	sockaddr_in.sin_port = htons(server_port);

	if (connect(socket_file_descriptor, (struct sockaddr*)&sockaddr_in, sizeof(sockaddr_in)) == -1) {
		perror("connect");
		exit(EXIT_FAILURE);
	}

	nbytes_total = 0;
	while (nbytes_total < request_len) {
		nbytes_last = write(socket_file_descriptor, request + nbytes_total, request_len - nbytes_total);
		if (nbytes_last == -1) {
			perror("write");
			exit(EXIT_FAILURE);
		}
		nbytes_total += nbytes_last;
	}

	FILE* fw = fopen("solar.tmp", "w");
	
	while ((nbytes_total = read(socket_file_descriptor, buffer, BUFSIZ)) > 0) {
		fputs(buffer, fw);
	}
	if (nbytes_total == -1) {
		perror("read");
		exit(EXIT_FAILURE);
	}

	close(socket_file_descriptor);
	fclose(fw);
}

void getDistanceFromSun(char (*arr)[10]) {
	FILE* f = fopen("solar.tmp", "r");
	if(f == NULL) {
		printf("File Not Found");
	}
	strcpy(arr[0], "0.00");
	getValuesFor(f, "Mean distance from sun<br> (10<sup>6</sup> km)</center></td><td><center>", arr);
}

void getRevolution(char (*arr)[10]) {
	FILE* f = fopen("solar.tmp", "r");
	if(f == NULL) {
		printf("File Not Found");
	}
	strcpy(arr[0], "0.00");
	getValuesFor(f, "Period of revolution, years</center></td><td><center>", arr);

}

void getOrbitalSpeed(char (*arr)[10]) {
	FILE* f = fopen("solar.tmp", "r");
	if(f == NULL) {
		printf("File Not Found");
	}
	strcpy(arr[0], "0.00");
	getValuesFor(f, "Orbital speed, km/s</center></td><td><center>", arr);
}

void getMass(char (*arr)[10]) {
	FILE* f = fopen("solar.tmp", "r");
	if(f == NULL) {
		printf("File Not Found");
	}
	strcpy(arr[0], "333,000");
	getValuesFor(f, "Mass(earth=1)</center></td><td><center>", arr);
}

void getDensity(char (*arr)[10]) {
	FILE* f = fopen("solar.tmp", "r");
	if(f == NULL) {
		printf("File Not Found");
	}
	strcpy(arr[0], "1.408");
	getValuesFor(f, "Density (water = 1)</center></td><td><center>", arr);
}

void getEscapeVelocity(char (*arr)[10]) {
	FILE* f = fopen("solar.tmp", "r");
	if(f == NULL) {
		printf("File Not Found");
	}
	strcpy(arr[0], "617.7");
	getValuesFor(f, "Escape velocity, equator, km/s</center></td><td><center>", arr);
}

void getSatallite(char (*arr)[10]) {
	FILE* f = fopen("solar.tmp", "r");
	if(f == NULL) {
		printf("File Not Found");
	}
	strcpy(arr[0], "N/A");
	getValuesFor(f, "Satellites</center></td><td><center>", arr);
}
