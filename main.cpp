/*Daniel Cheng - dyc8av
Written April 27, 2015
This is an implementation of a FTP server. THe minimum commands
USER, QUIT, PORT, TYPE, MODE, STRU, RETR, STOR, NOOP are
implemented in addition to LIST. This code will be able to
interact with any off-the-shelf FTP client.*/

#include <iostream>
#include <stdio.h>   
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
using namespace std;

int main(int argc, char* argv[]) {
	// check if port number is provided
	if(argv[1] == NULL) {
		cout << "No parameter provided" << endl;
		exit(0);
	}

	// convert port number to int
	int listenPort = atoi(argv[1]);

	// create a TCP socket
	long int serverSocketFD;
	serverSocketFD = socket(AF_INET, SOCK_STREAM, 0);
	if(serverSocketFD == -1) {
		perror("Error creating TCP socket");
		exit(0);
	}

	// bind the socket to a local address:port and set variables
	struct sockaddr_in serverSocketAddress;
	memset(&serverSocketAddress, 0, sizeof(serverSocketAddress));

	serverSocketAddress.sin_family = AF_INET;
	serverSocketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverSocketAddress.sin_port = htons(listenPort);

	int bindtest = bind(serverSocketFD, (struct sockaddr *) &serverSocketAddress, sizeof(serverSocketAddress));
	if(bindtest == -1) {
		perror("Error binding");
		exit(0);
	}

	// listen for connections
	int listentest = listen(serverSocketFD, 0);
	if(listentest == -1) {
		perror("Error listening");
		exit(0);
	}

	// accept a connection
	long int clientSocketFD;
	struct sockaddr_in clientSocketAddress;
	socklen_t clientAddressSize = sizeof(clientSocketAddress);
	clientSocketFD = accept(serverSocketFD, (struct sockaddr *) &clientSocketAddress, &clientAddressSize);
	if(clientSocketFD == -1) {
		perror("Error accepting");
		exit(0);
	}

	// prompt client that server is ready
	cout << "Server has started..." << endl;
	char msg[] = "220 awaiting input\r\n";
	int writetest = write(clientSocketFD, msg, sizeof(msg));
	if(writetest == -1) {
		perror("Error writing"); 
		exit(0);
	}

	char buffer[100];
	bool type = false;
	string ip = "";
	int newport = 0;

	// interact with the client continuously
	while(true) {
		// read the message that the client sends from the 
		// control connection
		memset(buffer, 0, sizeof(buffer));
		int readtest = read(clientSocketFD, buffer, sizeof(buffer)-1);
		if(readtest == -1) {
			perror("Error reading");
			exit(0);
		}

		// parse the message read into buffer
		// cmdbuff - command
		// varbuff - variables (capitalized)
		// pathbuff - same as variables, but lowercase. Used in path names
		char cmdbuff[5];
		memcpy(cmdbuff,&buffer,4);
		cmdbuff[4] = '\0';
		char varbuff[90];
		char pathbuff[90];
		memcpy(varbuff,&buffer[5],90);
		memcpy(pathbuff,&buffer[5],90);
		for(int i = 0; i < sizeof(varbuff); i++) {
			varbuff[i] = toupper(varbuff[i]);
		}

		/*----- implementations of FTP commands -----*/

		// allow the client to log in
		if(strcmp(cmdbuff,"USER") == 0) {
			char tempmsg[] = "230 user logged in, proceed.\r\n";
			int tempwritetest = write(clientSocketFD, tempmsg, sizeof(tempmsg));
			if(tempwritetest == -1) {
				perror("Error writing"); 
				exit(0);
			}
		}

		// allow the client to quit the connection
		if(strcmp(cmdbuff,"QUIT") == 0) {
			char tempmsg[] = "221 Goodbye.\r\n";
			int tempwritetest = write(clientSocketFD, tempmsg, sizeof(tempmsg));
			exit(0);
			if(tempwritetest == -1) {
				perror("Error writing"); 
				exit(0);
			}
		}

		// recieve the PORt command and parse the variables
		// into ip addres and port number according to the specifications
		// PORT a1,a2,a3,a4,p1,p2, where ip = a1.a2.a3.a4, port = p1*256+p2
		if(strcmp(cmdbuff,"PORT") == 0) {
			char tempmsg[] = "200 Command okay.\r\n";
			int tempwritetest = write(clientSocketFD, tempmsg, sizeof(tempmsg));
			if(tempwritetest == -1) {
				perror("Error writing"); 
				exit(0);
			}
			
			// tokenize the parameters passed
			char * tokenbuff[6];
			char * token;
			token = strtok(varbuff,",");
			for(int i = 0; i < 6; i++) {
				tokenbuff[i] = token;
				token = strtok(NULL, ",");
			}
			// calculate the port number
			newport = atoi(tokenbuff[4]) * 256 + atoi(tokenbuff[5]);
			// construct the string representing ip
			for(int i = 0; i < 4; i++) {
				string temps = tokenbuff[i];
				ip = ip + temps + ".";
			}
			ip = ip.substr(0,ip.length()-1);
		}

		// not implemented, but return "215" for client to proceed
		if(strcmp(cmdbuff,"SYST") == 0) {
			char tempmsg[] = "215 command not implemented\r\n";
			int tempwritetest = write(clientSocketFD, tempmsg, sizeof(tempmsg));
			if(tempwritetest == -1) {
				perror("Error writing"); 
				exit(0);
			}
		}

		// only supports binary image mode, will enforce for
		// RETR and STOR. Global boolean "type" will keep track
		if(strcmp(cmdbuff,"TYPE") == 0) {
			if(strncmp(varbuff,"I",1) == 0) {
				char tempmsg[] = "200 Command okay.\r\n";
				int tempwritetest = write(clientSocketFD, tempmsg, sizeof(tempmsg));
				type = true;
				if(tempwritetest == -1) {
					perror("Error writing"); 
					exit(0);
				}
			}
			else {
				char tempmsg[] = "200 Command okay.\r\n";
				int tempwritetest = write(clientSocketFD, tempmsg, sizeof(tempmsg));
				type = false;
				if(tempwritetest == -1) {
					perror("Error writing"); 
					exit(0);
				}
			}
		}

		// implements MODE
		if(strcmp(cmdbuff,"MODE") == 0) {
			char tempmsg[] = "200 Command okay.\r\n";
			int tempwritetest = write(clientSocketFD, tempmsg, sizeof(tempmsg));
			if(tempwritetest == -1) {
				perror("Error writing"); 
				exit(0);
			}
		}

		// only supports FILE structures
		if(strcmp(cmdbuff,"STRU") == 0) {
			if(strncmp(varbuff,"F",1) == 0) {
				char tempmsg[] = "200 Command okay.\r\n";
				int tempwritetest = write(clientSocketFD, tempmsg, sizeof(tempmsg));
				if(tempwritetest == -1) {
					perror("Error writing"); 
					exit(0);
				}
			}
			else {
				char tempmsg[] = "504 Command not implemented for that parameter.\r\n";
				int tempwritetest = write(clientSocketFD, tempmsg, sizeof(tempmsg));
				if(tempwritetest == -1) {
					perror("Error writing"); 
					exit(0);
				}
			}
		}

		// will retrieve files from the remote file system to
		// the local file system
		if(strcmp(cmdbuff,"RETR") == 0) {
			if(type) { // check if binary image mode enabled
				// parse the input to get the correct path
				for(int i = 0; i < 90; i++) {
					if(pathbuff[i] == '\n') {
						pathbuff[i] = '\0';
						break;
					}
				}

				int countpath = -1;
				for(int i = 0; i < 90; i++) {
					if(pathbuff[i] == '\0') {
						break;
					}
					countpath++;
				}
				// correctly set the null terminating byte at end of path
				pathbuff[countpath] = '\0';
				// end parsing

				FILE * file;
				long size;
				
				// attempt to open the file at specified path
				if((file = fopen(pathbuff,"r")) != NULL) {
					// create the data connection
					struct sockaddr_in stSockAddr;
					int Res;
					long int SocketFD = socket(PF_INET, SOCK_STREAM, 0);

					if (-1 == SocketFD)
					{
						perror("cannot create socket");
						exit(EXIT_FAILURE);
					}

					memset(&stSockAddr, 0, sizeof(stSockAddr));
					// set appropiate ip and port returned from PORT
					stSockAddr.sin_family = AF_INET;
					stSockAddr.sin_port = htons(newport);
					Res = inet_pton(AF_INET, (const char *)ip.c_str(), &stSockAddr.sin_addr);
					// connect to client
					if (-1 == connect(SocketFD, (struct sockaddr *)&stSockAddr, sizeof(stSockAddr)))
					{
						perror("connect failed");
						close(SocketFD);
						exit(EXIT_FAILURE);
					}

					//notify client that data connection is beginning
					char tempmsg[] = "150 opening data connection, beginning transfer\r\n";
					int tempwritetest = write(clientSocketFD, tempmsg, sizeof(tempmsg));
					if(tempwritetest == -1) {
						perror("Error writing"); 
						exit(0);
					}

					// create a buffer that can fit the file
					fseek(file, 0, SEEK_END);
					size = ftell(file);
					fseek(file, 0, SEEK_SET);
					char getbuf[size];
					memset(getbuf,0,sizeof(getbuf));
					// read contents of file into the buffer
					fread(getbuf,1,size,file);
					fclose(file);
					// write the buffer to the data connection
					int tempgetbufwrite = write(SocketFD, getbuf, sizeof(getbuf));
					if(tempgetbufwrite == -1) {
						perror("Error writing"); 
						exit(0);
					}

					// notify client that data transmission is complete
					char tempmsg2[] = "226 transfer complete, closing data connection\r\n";
					int tempwritetest2 = write(clientSocketFD, tempmsg2, sizeof(tempmsg2));
					if(tempwritetest2 == -1) {
						perror("Error writing"); 
						exit(0);
					}

					// close the data connection
					(void) shutdown(SocketFD, SHUT_RDWR);
					close(SocketFD);

				}
				else {
					// ierror file can not be opened
					perror ("");
					char tempmsg6[] = "450 Requested file action not taken.\r\n";
					int tempwritetest6 = write(clientSocketFD, tempmsg6, sizeof(tempmsg6));
					if(tempwritetest6 == -1) {
						perror("Error writing"); 
						exit(0);
					}
				}
			}
			else {
				// error if binary image mode not enabled
				char err[] = "451 Requested action aborted: local error in processing\r\n";
				int tempwritetest7 = write(clientSocketFD, err, sizeof(err));
				if(tempwritetest7 == -1) {
					perror("Error writing"); 
					exit(0);
				}
			}
		}

		// allow the client to transfer files from local file
		// system to remote file system
		if(strcmp(cmdbuff,"STOR") == 0) {
			if(type) { // check if binary image mode enabled
				// parse the path
				for(int i = 0; i < 90; i++) {
					if(pathbuff[i] == '\n') {
						pathbuff[i] = '\0';
						break;
					}
				}

				int countpath = -1;
				for(int i = 0; i < 90; i++) {
					if(pathbuff[i] == '\0') {
						break;
					}
					countpath++;
				}
				// correctly set the null terminating byte at end of path
				pathbuff[countpath] = '\0';
				// end parsing

				FILE * file;

				// attempt to open file at that path
				if((file = fopen(pathbuff,"w")) != NULL) {
					// begin creating data connection
					struct sockaddr_in stSockAddr;
					int Res;
					long int SocketFD = socket(PF_INET, SOCK_STREAM, 0);

					if (-1 == SocketFD)
					{
						perror("cannot create socket");
						exit(EXIT_FAILURE);
					}

					memset(&stSockAddr, 0, sizeof(stSockAddr));
					// set appropiate variables, such as ip and port
					// returned from PORT
					stSockAddr.sin_family = AF_INET;
					stSockAddr.sin_port = htons(newport);
					Res = inet_pton(AF_INET, (const char *)ip.c_str(), &stSockAddr.sin_addr);
					// connect to client (data)
					if (-1 == connect(SocketFD, (struct sockaddr *)&stSockAddr, sizeof(stSockAddr)))
					{
						perror("connect failed");
						close(SocketFD);
						exit(EXIT_FAILURE);
					}

					// notify client that connection is beginning
					char tempmsg[] = "150 opening data connection, beginning transfer\r\n";
					int tempwritetest = write(clientSocketFD, tempmsg, sizeof(tempmsg));
					if(tempwritetest == -1) {
						perror("Error writing"); 
						exit(0);
					}

					// read from the client into buffer and write the buffer
					// to a file until entire file is transferred
					char readbuf[1024];
					memset(readbuf, 0, sizeof(readbuf));
					while(read(SocketFD, readbuf, sizeof(readbuf)) > 0) {
						int readcount = 0;
						for(int i = 0; i < sizeof(readbuf); i++) {
							if(readbuf[i] != 0) {
								readcount++;
							}
						}

						fwrite(readbuf,1,readcount,file);
						memset(readbuf, 0, sizeof(readbuf));
					}
					fclose(file);

					// notify client that data transmission is complete
					char tempmsg2[] = "226 transfer complete, closing data connection\r\n";
					int tempwritetest2 = write(clientSocketFD, tempmsg2, sizeof(tempmsg2));
					if(tempwritetest2 == -1) {
						perror("Error writing"); 
						exit(0);
					}

					// close the data connection
					(void) shutdown(SocketFD, SHUT_RDWR);
					close(SocketFD);

				}
				else {
					// error if file can't be opened
					perror ("");
					char tempmsg6[] = "450 Requested file action not taken.\r\n";
					int tempwritetest6 = write(clientSocketFD, tempmsg6, sizeof(tempmsg6));
					if(tempwritetest6 == -1) {
						perror("Error writing"); 
						exit(0);
					}
				}
			}
			else {
				// error if binary image mode not enabled
				char err[] = "451 Requested action aborted: local error in processing\r\n";
				int tempwritetest7 = write(clientSocketFD, err, sizeof(err));
				if(tempwritetest7 == -1) {
					perror("Error writing"); 
					exit(0);
				}
			}
		}

		// will send a response, acts like a ping
		if(strcmp(cmdbuff,"NOOP") == 0) {
			char tempmsg[] = "200 Command okay.\r\n";
			int tempwritetest = write(clientSocketFD, tempmsg, sizeof(tempmsg));
			if(tempwritetest == -1) {
				perror("Error writing"); 
				exit(0);
			}
		}

		// allow the client to retrieve directory listings
		if(strcmp(cmdbuff,"LIST") == 0) {
			/*The next section of code is used for creating the
			correct path to open the directory. The current working
			directory is stored in cbuff, and has a null terminating byte
			correctly found at the end. The parameter that is input
			after "ls" is stored in pathbuff, however, it includes
			a \n new line character. We wish to strip the \n
			combine pathbuff and cbuff correctly with the null 
			terminating byte at the end of the string for 
			opendir() to work successfully.*/

			/*--------- begin parsing and variable declaration----------*/
			char cbuff[100];
			memset(cbuff,0,sizeof(cbuff));
			DIR * dir;
			struct dirent *ent;
			getcwd(cbuff,100);

			// strip off \n in pathbuff
			for(int i = 0; i < 90; i++) {
				if(pathbuff[i] == '\n') {
					pathbuff[i] = '\0';
					break;
				}
			}

			// find the location to insert the null termating byte
			// in cbuff
			int count = 0;
			for(int i = 0; i < 100; i++) {
				if(cbuff[i] == '\0') {
					break;
				}
				count++;
			}

			// find the location to insert the null termating byte
			// in pathbuff
			int countpath = 0;
			for(int i = 0; i < 90; i++) {
				if(pathbuff[i] == '\0') {
					break;
				}
				countpath++;
			}

			// set the null byte to end of cmdbuff always
			int nullindex = count;

			// if ls has a parameter, add in path syntax and 
			// calculate the null byte offset of cbuff and
			// pathbuff.
			if(countpath != 0) {
				cbuff[count] = '/';
				nullindex = count + countpath;
			}

			// append pathbuff to cbuff
			strcat(cbuff,pathbuff);

			// correctly place the null termating byte in cbuff
			cbuff[nullindex] = '\0';

			/*--------- end parsing path----------*/

			// attempt to open the directory at specified path
			if((dir = opendir(cbuff)) != NULL) {
				// begin data connection creation
				struct sockaddr_in stSockAddr;
				int Res;
				long int SocketFD = socket(PF_INET, SOCK_STREAM, 0);

				if (-1 == SocketFD)
				{
					perror("cannot create socket");
					exit(EXIT_FAILURE);
				}

				memset(&stSockAddr, 0, sizeof(stSockAddr));
				// set variables, such as ip and port
				// returned from PORT
				stSockAddr.sin_family = AF_INET;
				stSockAddr.sin_port = htons(newport);
				Res = inet_pton(AF_INET, (const char *)ip.c_str(), &stSockAddr.sin_addr);
				// create data connection
				if (-1 == connect(SocketFD, (struct sockaddr *)&stSockAddr, sizeof(stSockAddr)))
				{
					perror("connect failed");
					close(SocketFD);
					exit(EXIT_FAILURE);
				}

				// notify client that data transmission is beginning
				char tempmsg[] = "150 opening data connection, beginning transfer\r\n";
				int tempwritetest = write(clientSocketFD, tempmsg, sizeof(tempmsg));
				if(tempwritetest == -1) {
					perror("Error writing"); 
					exit(0);
				}

				// write directory entries to client over data connection
				while ((ent = readdir (dir)) != NULL) {
					string s = ent->d_name;
					char tempmsgdata[s.size()];
					for(int i = 0; i < s.size(); i++) {
						tempmsgdata[i] = s.at(i);
					}
					int tempwritetestdata = write(SocketFD, tempmsgdata, sizeof(tempmsgdata));
					if(tempwritetestdata == -1) {
						perror("Error writing"); 
						exit(0);
					}
					char space[] = "\r\n";
					int spacedata = write(SocketFD, space, 2);
					if(spacedata == -1) {
						perror("Error writing"); 
						exit(0);
					}
				}
				closedir (dir);

				// notify client that data transmission is complete
				char tempmsg2[] = "226 transfer complete, closing data connection\r\n";
				int tempwritetest2 = write(clientSocketFD, tempmsg2, sizeof(tempmsg2));
				if(tempwritetest2 == -1) {
					perror("Error writing"); 
					exit(0);
				}

				// close the data connection
				(void) shutdown(SocketFD, SHUT_RDWR);
				close(SocketFD);
			}
			else {
  			/* could not open directory */
				perror ("");
				char tempmsg6[] = "450 Requested file action not taken. Please check path formatting\r\n";
				int tempwritetest6 = write(clientSocketFD, tempmsg6, sizeof(tempmsg6));
				if(tempwritetest6 == -1) {
					perror("Error writing"); 
					exit(0);
				}
			}
		}
	}
	return 0;
}