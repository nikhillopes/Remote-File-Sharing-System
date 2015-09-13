/**
 * @nlopes_assignment1
 * @author  Nikhil Raphael Lopes <nlopes@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This contains the main function. Add further description here....
 */

#include <iostream>
#include <cstdlib>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <libgen.h>
#include <time.h>
#include "../include/global.h"



using namespace std;
#define BUF_SIZE 1024

int helpc()
{
	cout<<"You may use the following commands in Client mode"<<endl;
	cout<<" 1) CREATOR                            :- Displays the full name, UBIT name, and UB email address of the author of this application. "<<endl;
	cout<<" 2) MYIP                               :- Displays the IP address of this process. "<<endl;
	cout<<" 3) MYPORT                             :- Displays the port on which this process is listening for incoming connections. "<<endl;
	cout<<" 4) REGISTER <server IP> <port no>     :- This command is used by the client to register itself with the server and to get the IP and listening port numbers of all the peers currently registered with the server. "<<endl;
	cout<<" 5) CONNECT <destination> <port no>    :- This command establishes a new TCP connection to the specified <destination> at the specified < port no>. The <destination> can be either an IP address or a hostname. "<<endl;
	cout<<" 6) LIST                               :- Display a numbered list of all the connections this process is part of. "<<endl;
	cout<<" 7) TERMINATE <connection id>          :- This command will terminate the connection listed under the specified number when LIST is used to display all connections. "<<endl;
	cout<<" 8) UPLOAD <connection id> <file name> :- This command will upload the file specified to the host on the connection that has connection id specified. "<<endl;
	cout<<" 9) DOWNLOAD <connection id 1> <file1> :- This command will download the file specified to the your computer from the host that has connection id specified. "<<endl;
	cout<<"10) STATISTICS                         :- This command will display the the upload and download statistics. "<<endl;
	cout<<"11) EXIT                               :- This command exits the application. "<<endl;
	return 0;
}

int helps()
{
	cout<<"You may use the following commands in Server mode"<<endl;
	cout<<" 1) CREATOR                            :- Displays the full name, UBIT name, and UB email address of the author of this application. "<<endl;
	cout<<" 2) MYIP                               :- Displays the IP address of this process. "<<endl;
	cout<<" 3) MYPORT                             :- Displays the port on which this process is listening for incoming connections. "<<endl;
	cout<<" 4) LIST                               :- Display a numbered list of all the connections this process is part of. "<<endl;
	cout<<" 5) STATISTICS                         :- This command will display the the upload and download statistics. "<<endl;
	cout<<" 6) EXIT                               :- This command exits the application. "<<endl;
	return 0;
}

int creator()
{
	cout<<"Nikhil Raphael Lopes"<<endl;
	cout<<"UBIT Name : nlopes"<<endl;
	cout<<"Email     : nlopes@buffalo.edu"<<endl;
	cout<< "I have read and understood the course academic integrity policy located at http://www.cse.buffalo.edu/faculty/dimitrio/courses/cse4589_f14/index.html#integrity"<<endl;
	return 0;
}

void *get_in_addr(struct sockaddr *sa)
{
	return sa->sa_family == AF_INET? (void *) &(((struct sockaddr_in*)sa)->sin_addr): (void *) &(((struct sockaddr_in6*)sa)->sin6_addr);
}

string myip()   //Returns ip address of the local machine. got help from http://www.binarytides.com/get-local-ip-c-linux/
{
	string ip;
	const char* ipToConnect = "8.8.8.8";//ip address
	int dns_port = 53;                  //dns port
	struct sockaddr_in myipSockStruct;
	int myipSocket = socket ( AF_INET, SOCK_DGRAM, 0);
	if(myipSocket < 0)
	{
		cerr<<("Cannot Create Socket"); //no socket error
	}

	memset( &myipSockStruct, 0, sizeof(myipSockStruct) );
	myipSockStruct.sin_family = AF_INET;
	myipSockStruct.sin_addr.s_addr = inet_addr( ipToConnect );
	myipSockStruct.sin_port = htons( dns_port );

	int err = connect( myipSocket , (const struct sockaddr*) &myipSockStruct , sizeof(myipSockStruct) ); //connect making fake udp connection to google public dns server

	struct sockaddr_in name;
	socklen_t namelen = sizeof(name);
	err = getsockname(myipSocket, (struct sockaddr*) &name, &namelen);

	char buffer[100];
	const char* p = inet_ntop(AF_INET, &name.sin_addr, buffer, 100);  //function to convert into readable form
	ip=buffer;
	close(myipSocket); //close socket connection
	if(p != NULL)
	{
		//cout<<"Local ip is "+ip<<endl;
		return ip;
	}
	else
	{
		//Some error
		cerr<<"Could Not Find Local Address";
		return NULL;
	}
}

int sendall(int s, char *buf, int *len)    //From Beej
{
	int total = 0; 			// how many bytes we've sent
	int bytesleft = *len; 	// how many we have left to send
	int n;
	while(total < *len)
	{
		n = send(s, buf+total, bytesleft, 0);
		if (n == -1)
		{
			break;
		}
		total += n;
		bytesleft -= n;
	}
	*len = total; // return number actually sent here
	return n==-1?-1:0; // return -1 on failure, 0 on success
}

void displayServerList(char serverList[1024])
{
	for(int i=3;i<strlen(serverList);i++)
	{
		if(serverList[i]==' ')
		{
			cout<<"         ";
		}
		else if(serverList[i]==';')
		{
			cout<<endl;
		}
		else
		{
			cout<<serverList[i];
		}
	}
}

void displayStatList(char serverList[1024])
{
	for(int i=0;i<strlen(serverList);i++)
	{
		if(serverList[i]==' ')
		{
			cout<<"         ";
		}
		else if(serverList[i]==';')
		{
			cout<<endl;
		}
		else
		{
			cout<<serverList[i];
		}
	}
}
size_t getFileSize(const char* filePath)		//http://techoverflow.net/blog/2013/08/21/how-to-get-filesize-using-stat-in-c-cpp/
{
	struct stat st;
	if(stat(filePath, &st) != 0) {
		return 0;
	}
	return st.st_size;
}


//upload and download referenced from http://codereview.stackexchange.com/questions/43914/client-server-implementation-in-c-sending-data-files
//http://www.ccplusplus.com/2011/11/gettimeofday-example.html

int uploader(int sockNum, char *filePath, double *timeTaken, size_t *fileSize)
{
	char buff[BUF_SIZE]={0};
	struct timeval  start, end;
	size_t bytesRead = 0,bytesSent=0;
	size_t fSize, remainingR, remainingS;
	fSize=getFileSize(filePath);	//cout<<fSize;					//get FileSize to send to downloader
	*fileSize=fSize;
	remainingR=fSize;
	remainingS=fSize;
	FILE *fp = fopen(filePath,"rb");

	if(fp==NULL)
	{
		cerr<<"File Open Error";
		return 1;
	}

	snprintf ( buff, BUF_SIZE, "SIZE %lu ", fSize);   //send file size
	int nRead=sizeof(buff);
	sendall(sockNum, buff, &nRead);

	memset(buff, '0', sizeof(buff));
	gettimeofday(&start, NULL);
	cout<<"Starting Upload "<<endl;

	cout<<"Uploading..."<<endl;

	memset(buff, '0', sizeof(buff));
	while(remainingR > 0 && remainingS > 0)
	{
		int len;
		if (remainingR >= BUF_SIZE )
		{
			bytesRead = fread(buff, 1, BUF_SIZE , fp);
			len=bytesRead;
		}
		else if (remainingR<BUF_SIZE)
		{
			bytesRead = fread(buff, 1, BUF_SIZE-remainingR , fp);
			len=bytesRead;
		}
		int res=sendall(sockNum, buff, &len);
		if( res < 0)
		{
			cerr<<"ERROR: Failed to send file "<<endl;
			remainingR=0;
			remainingS=0;
			break;
		}
		remainingR=remainingR-bytesRead;
		remainingS=remainingS-len;
		memset(buff, '0', sizeof(buff));
	}
	/*
	 * There is something tricky going on with read ..
	 * Either there was error, or we reached end of file.
	 */
	if (remainingR==0 && remainingS==0)
	{

		if (ferror(fp))
		{
			cerr<<"Error Reading File. Transfer Failed"<<endl;
			return -1;
		}
		else
		{
			cout<<"End of File. Transfer Succeeded"<<endl;
			fclose(fp);
			gettimeofday(&end, NULL);
			*timeTaken=1000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000;
			return 0;
		}
	}
	else if (remainingR==0 && remainingS > 0)
		{
			if (ferror(fp))
			{
				cerr<<"Error Reading File. Transfer Failed"<<endl;
				return -1;
			}
			else
			{
				cout<<"Not everthing was sent"<<endl;
				fclose(fp);
				gettimeofday(&end, NULL);
				*timeTaken=1000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000;
				return 0;
			}
		}
}

int downloader(int sockNum, char *filePath, double *timeTaken, size_t *fileSize)
{

	struct timeval  start, end;
	size_t bytesReceived = 0,bytesWritten=0;
	char buff[BUF_SIZE];
	memset(buff, '0', sizeof(buff));
	FILE *fp = fopen(filePath,"wb");

	if(fp==NULL)
	{
		cerr<<"File Open Error";
		return 4;
	}

	size_t fSize, remaining;
	recv(sockNum, buff, BUF_SIZE,0);
	if(strstr(buff,"SIZE")!=NULL)
	{
		strtok(buff," ");
		fSize=strtol(strtok(NULL," "),0,10);
	}
	remaining=fSize;
	//cout<<fSize;
	memset(buff, '0', sizeof(buff));

	cout<<"Starting Download "<<endl;
	gettimeofday(&start, NULL);

	/* Receive data in chunks of BUF_SIZE bytes */


	cout<<"Downloading..."<<endl;

	while( remaining > 0)
	{
		bytesReceived = recv(sockNum, buff, BUF_SIZE, 0);
		bytesWritten = fwrite(buff,1,bytesReceived, fp);
		if(bytesWritten < bytesReceived)
		{
			cerr<<"File write failed."<<endl;
		}
		remaining=remaining-bytesWritten;
		memset(buff, '0', sizeof(buff));
	}

	if(bytesReceived < 0 || bytesReceived==0)
	{
		cerr<<"recv() failed"<<endl;
		return -1;
	}

	else
	{
		cout<<"Download Complete"<<endl;
		fclose(fp);
		gettimeofday(&end, NULL);
		*timeTaken=1000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000;
		*fileSize=fSize;
		cout<<"End of Download. Transfer Succeeded"<<endl;
		return 0;
	}
}
int downloader2(int sockNum1, int sockNum2, char *filePath1, char *filePath2, double *timeTaken1, double *timeTaken2, size_t *fileSize1, size_t *fileSize2)
{
	struct timeval start, end1, end2;
	size_t bytesReceived1 = 0,bytesWritten1=0;
	size_t bytesReceived2 = 0,bytesWritten2=0;

	size_t fSize1, remaining1;
	size_t fSize2, remaining2;
	bool status1=false, status2=false;

	char buff1[BUF_SIZE];
	char buff2[BUF_SIZE];
	memset(buff1, '0', sizeof(buff1));
	memset(buff2, '0', sizeof(buff2));
	FILE *fp1 = fopen(filePath1,"wb");
	FILE *fp2 = fopen(filePath2,"wb");

	if(fp1==NULL)
	{
		cerr<<"File1 Open Error";
		remaining1=0;
		status1=true;
	}
	if(fp2==NULL)
	{
		cerr<<"File2 Open Error";
		remaining2=0;
		status2=true;
	}

	cout<<"Files Opened"<<endl;
	recv(sockNum1, buff1, BUF_SIZE,0);
	if(strstr(buff1,"SIZE")!=NULL)
	{
		strtok(buff1," ");
		fSize1=strtol(strtok(NULL," "),0,10);
	}
	recv(sockNum2, buff2, BUF_SIZE,0);

	if(strstr(buff2,"SIZE")!=NULL)
	{
		strtok(buff2," ");
		fSize2=strtol(strtok(NULL," "),0,10);
	}

	remaining1=fSize1;
	remaining2=fSize2;

	memset(buff1, '0', sizeof(buff1));
	memset(buff2, '0', sizeof(buff2));

	cout<<"Starting Downloads "<<endl;

	gettimeofday(&start, NULL);

	/* Receive data parallely in chunks of BUF_SIZE bytes */


	cout<<"Downloading..."<<endl;

	while( remaining1> 0 || remaining2 > 0 || status1==false ||status2==false )
	{
		if (remaining1>0 && status1==false)
		{
			bytesReceived1 = recv(sockNum1, buff1, BUF_SIZE, 0);
			bytesWritten1 = fwrite(buff1,1,bytesReceived1, fp1);
			if(bytesWritten1 < bytesReceived1)
			{
				cerr<<"Download1 write failed."<<endl;
				status1=true;
			}
			remaining1=remaining1-bytesWritten1;		//cout<<"got1 "<<bytesReceived1<<" wrote1 "<<bytesWritten1<<" remaining1 "<<remaining1<<endl;
			memset(buff1, '0', sizeof(buff1));
		}
		else if(remaining1==0 && status1==false)
		{
			status1=true;
			cout<<"Download1 Complete"<<endl;
			fclose(fp1);
			gettimeofday(&end1, NULL);;
			*timeTaken1=1000 * (end1.tv_sec - start.tv_sec) + (end1.tv_usec - start.tv_usec) / 1000;
			*fileSize1=fSize1;
		}

		if (remaining2 > 0 && status2==false)
		{
			bytesReceived2 = recv(sockNum2, buff2, BUF_SIZE, 0);
			bytesWritten2 = fwrite(buff2,1,bytesReceived2, fp2);
			if(bytesWritten2 < bytesReceived2)
			{
				cerr<<"Download2 write failed."<<endl;
				status2=true;
			}
			remaining2=remaining2-bytesWritten2;		//cout<<"got2 "<<bytesReceived2<<" wrote2 "<<bytesWritten2<<" remaining2 "<<remaining2<<endl;
			memset(buff2, '0', sizeof(buff2));
		}
		else if(remaining2==0 && status2==false)
		{
			status2=true;
			cout<<"Download2 Complete"<<endl;
			fclose(fp2);
			gettimeofday(&end2, NULL);;
			*timeTaken2=1000 * (end2.tv_sec - start.tv_sec) + (end2.tv_usec - start.tv_usec) / 1000;
			*fileSize2=fSize2;
		}

	}

	if(bytesReceived1 < 0 || bytesReceived1==0 || bytesReceived2 < 0 || bytesReceived2==0)
	{
		cerr<<"Some recv() failed"<<endl;
		return -1;
	}

	else
	{
		cout<<"End of Downloads. All Succeeded"<<endl;
		return 0;
	}
}

int downloader3(int sockNum1, int sockNum2, int sockNum3, char *filePath1, char *filePath2, char *filePath3, double *timeTaken1, double *timeTaken2, double *timeTaken3, size_t *fileSize1, size_t *fileSize2, size_t *fileSize3)
{
	struct timeval start, end1, end2, end3;
	size_t bytesReceived1 = 0,bytesWritten1=0;
	size_t bytesReceived2 = 0,bytesWritten2=0;
	size_t bytesReceived3 = 0,bytesWritten3=0;

	size_t fSize1, remaining1;
	size_t fSize2, remaining2;
	size_t fSize3, remaining3;
	bool status1=false, status2=false, status3=false;

	char buff1[BUF_SIZE];
	char buff2[BUF_SIZE];
	char buff3[BUF_SIZE];

	memset(buff1, '0', sizeof(buff1));
	memset(buff2, '0', sizeof(buff2));
	memset(buff3, '0', sizeof(buff3));

	FILE *fp1 = fopen(filePath1,"wb");
	FILE *fp2 = fopen(filePath2,"wb");
	FILE *fp3 = fopen(filePath3,"wb");

	if(fp1==NULL)
	{
		cerr<<"File1 Open Error";
		remaining1=0;
		status1=true;
	}
	if(fp2==NULL)
	{
		cerr<<"File2 Open Error";
		remaining2=0;
		status2=true;
	}
	if(fp3==NULL)
	{
		cerr<<"File3 Open Error";
		remaining3=0;
		status3=true;
	}

	cout<<"Files Opened"<<endl;

	recv(sockNum1, buff1, BUF_SIZE,0);
	if(strstr(buff1,"SIZE")!=NULL)
	{
		strtok(buff1," ");
		fSize1=strtol(strtok(NULL," "),0,10);
	}

	recv(sockNum2, buff2, BUF_SIZE,0);
	if(strstr(buff2,"SIZE")!=NULL)
	{
		strtok(buff2," ");
		fSize2=strtol(strtok(NULL," "),0,10);
	}

	recv(sockNum3, buff3, BUF_SIZE,0);
	if(strstr(buff3,"SIZE")!=NULL)
	{
		strtok(buff3," ");
		fSize3=strtol(strtok(NULL," "),0,10);
	}

	remaining1=fSize1;
	remaining2=fSize2;
	remaining3=fSize3;

	memset(buff1, '0', sizeof(buff1));
	memset(buff2, '0', sizeof(buff2));
	memset(buff3, '0', sizeof(buff3));

	cout<<"Starting Downloads "<<endl;

	gettimeofday(&start, NULL);

	/* Receive data parallely in chunks of BUF_SIZE bytes */


	cout<<"Downloading..."<<endl;

	while( remaining1> 0 || remaining2 > 0 || remaining3 > 0|| status1==false || status2==false || status3==false )
	{
		if (remaining1>0 && status1==false)
		{
			bytesReceived1 = recv(sockNum1, buff1, BUF_SIZE, 0);
			bytesWritten1 = fwrite(buff1,1,bytesReceived1, fp1);
			if(bytesWritten1 < bytesReceived1)
			{
				cerr<<"Download1 write failed."<<endl;
				status1=true;
			}
			remaining1=remaining1-bytesWritten1;		//cout<<"got1 "<<bytesReceived1<<" wrote1 "<<bytesWritten1<<" remaining1 "<<remaining1<<endl;
			memset(buff1, '0', sizeof(buff1));
		}
		else if(remaining1==0 && status1==false)
		{
			status1=true;
			cout<<"Download1 Complete"<<endl;
			fclose(fp1);
			gettimeofday(&end1, NULL);;
			*timeTaken1=1000 * (end1.tv_sec - start.tv_sec) + (end1.tv_usec - start.tv_usec) / 1000;
			*fileSize1=fSize1;
		}

		if (remaining2 > 0 && status2==false)
		{
			bytesReceived2 = recv(sockNum2, buff2, BUF_SIZE, 0);
			bytesWritten2 = fwrite(buff2,1,bytesReceived2, fp2);
			if(bytesWritten2 < bytesReceived2)
			{
				cerr<<"Download2 write failed."<<endl;
				status2=true;
			}
			remaining2=remaining2-bytesWritten2;		//cout<<"got2 "<<bytesReceived2<<" wrote2 "<<bytesWritten2<<" remaining2 "<<remaining2<<endl;
			memset(buff2, '0', sizeof(buff2));
		}
		else if(remaining2==0 && status2==false)
		{
			status2=true;
			cout<<"Download2 Complete"<<endl;
			fclose(fp2);
			gettimeofday(&end2, NULL);;
			*timeTaken2=1000 * (end2.tv_sec - start.tv_sec) + (end2.tv_usec - start.tv_usec) / 1000;
			*fileSize2=fSize2;
		}

		if (remaining3 > 0 && status3==false)
		{
			bytesReceived3 = recv(sockNum3, buff3, BUF_SIZE, 0);
			bytesWritten3 = fwrite(buff3,1,bytesReceived3, fp3);
			if(bytesWritten3 < bytesReceived3)
			{
				cerr<<"Download3 write failed."<<endl;
				status3=true;
			}
			remaining3=remaining3-bytesWritten3;		//cout<<"got2 "<<bytesReceived2<<" wrote2 "<<bytesWritten2<<" remaining2 "<<remaining2<<endl;
			memset(buff3, '0', sizeof(buff3));
		}
		else if(remaining3==0 && status3==false)
		{
			status3=true;
			cout<<"Download3 Complete"<<endl;
			fclose(fp3);
			gettimeofday(&end3, NULL);;
			*timeTaken3=1000 * (end3.tv_sec - start.tv_sec) + (end3.tv_usec - start.tv_usec) / 1000;
			*fileSize3=fSize3;
		}
	}

	if(bytesReceived1 < 0 || bytesReceived1==0 || bytesReceived2 < 0 || bytesReceived2==0)
	{
		cerr<<"Some recv() failed"<<endl;
		return -1;
	}

	else
	{
		cout<<"End of Downloads. All Succeeded"<<endl;
		return 0;
	}
}

bool fileExists (char *filePath) 											//referenced from http://stackoverflow.com/questions/12774207/fastest-way-to-check-if-a-file-exist-using-standard-c-c11-c
{
	struct stat buffer;
	return (stat (filePath, &buffer) == 0);
}

bool dirExists(char *dirPath)												//http://linux.die.net/man/2/stat
{
	struct stat st;
	if(stat(dirPath,&st) == 0)
	{
		if(st.st_mode & S_IFDIR != 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}
struct nodes
{
	bool isServer;
	bool used;
	int host_id;
	char hostname[32];
	char ip_addr[32];
	int port_num;
};

struct stats
{
	char hostname[32];
	int uploads;
	double avgU;
	int downloads;
	double avgD;
	bool used;
	bool past;
};

struct download
{
	int dIdInt;
	char* dId;
	char * filePath;
	char * dirPath;
	double timeTaken;
	double speed;
	size_t fileSize;
};


int main(int argc, char **argv)
{
	if(argc!=3)
	{
		cerr<<"Invalid Startup Arguments. This application starts as a server (s) or a client (c) and requires port number between 1024 and 49151"<<endl;
		exit(-1);
	}
	//setbuf(stdout,NULL);																		  //get input immediately http://www.cplusplus.com/reference/cstdio/setbuf/
	char mode=argv[1][0];
	int portToListenOn=strtol(argv[2],NULL,10);
	string portToListenOnStr;
	stringstream out;
	out << portToListenOn;
	portToListenOnStr = out.str();
	char const* portToListenOnChar=portToListenOnStr.c_str();
	bool argCheckValue;
	char myipadd[32];
	strcpy(myipadd,myip().c_str());
	struct nodes nodeInfo[16];

	for(int q=0;q<16;q++)
	{
		nodeInfo[q].used=false;
		nodeInfo[q].isServer=false;
	}
	struct stats nodeStats[16];

	for(int q=0;q<16;q++)
	{
		nodeStats[q].used=false;
		nodeStats[q].past=false;
	}
	char serverList[1024];             //to store server ip list
	char statList[1024];
	char *serverIp;
	char *serverPort;
	bool isRegistered=false;
	int connectionCount=0;

	int             i, j, resultChecker, yes = 1;
	int             primarySocketD, maximumFileD, newSocketD;
	char            commandLine[100];
	struct addrinfo hints, *ai, *p, *servInfo;
	struct sockaddr_storage remoteaddr; // client address
	socklen_t addrlen;
	struct sockaddr_in addressToResolve;
	char buf[1024]; // buffer for client data
	int nbytes, len;
	char remoteIP[INET_ADDRSTRLEN];
	int tracker;
	char hostNameClient[32];
	gethostname(hostNameClient,sizeof(hostNameClient));

	fd_set   		masterSet, readSet;

	FD_ZERO(&masterSet);                                                                          // clear the master and temp sets
	FD_ZERO(&readSet);

	/*************************************************************/
	/* Filling up Struct Using getaddrinfo()				     */
	/* Getting a Socket 								   	     */
	/* Binding the Socket                                        */
	/* Got help from Beej                                        */
	/*************************************************************/

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;


	if(argc==3 && (mode=='c'||mode=='s') && (portToListenOn>1024 && portToListenOn<49151))
	{
		argCheckValue=true;
	}
	else
	{
		argCheckValue=false;
	}
	//cout<<portToListenOn<<endl;
	//cout<<portToListenOnChar<<endl;
	//cout<<mode<<endl;
	//cout<<argCheckValue<<endl;

	if(argCheckValue==false)
	{
		cerr<<"Invalid Startup Arguments. This application starts as a server (s) or a client (c) and requires port number between 1024 and 49151"<<endl;
		cerr<<"To run as a Server listening on port 4322; Usage:- './assignment1 s 4322'"<<endl;
		cerr<<"To run as a Client listening on port 4322; Usage:- './assignment1 c 4322'"<<endl;
		exit(1);
	};

	if(mode=='c')
	{
		cout<<"Running as a Client on Port "<<+portToListenOn<<endl;

	}
	else if(mode=='s')
	{
		cout<<"Running as a Server on Port "<<+portToListenOn<<endl;
		/**************************************/
		/* Fill-up Struct with info of Server  */
		/**************************************/
		nodeInfo[0].used=true;
		nodeInfo[0].host_id=1;																		//cout<<nodeInfo[0].nodeid<<endl;
		if(gethostname(nodeInfo[0].hostname,32)==-1)
		{
			cerr<<"gethostname() Failed"<<endl;
		}																						  	//cout<<nodeInfo[0].hostname<<endl;
		strcpy(nodeInfo[0].ip_addr,myipadd);											 			//cout<<nodeInfo[0].ipaddress<<endl;
		nodeInfo[0].port_num=portToListenOn;													   	//cout<<nodeInfo[0].portnumber<<endl;
		nodeInfo[0].isServer=true;
	}
	else
	{
		cout<<"Invalid Startup Conditions"<<endl;
		exit(1);
	};



	if ((resultChecker = getaddrinfo(NULL, portToListenOnChar, &hints, &ai)) != 0)
	{
		cerr<<"getaddrinfo() Failed. Exiting.";
		exit(-1);
	}

	for(p = ai; p != NULL; p = p->ai_next)
	{
		primarySocketD = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (primarySocketD < 0)
		{
			continue;
		}

		setsockopt(primarySocketD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));                // lose the pesky "address already in use" error message
		if (bind(primarySocketD, p->ai_addr, p->ai_addrlen) < 0)
		{
			close(primarySocketD);
			continue;
		}
		break;
	}                                                                                         //cout<<" Socket done "<<endl;

	if (p == NULL)
	{							// if we got here, it means we didn't get bound
		cerr<<"Failed to Bind(). Exiting.";
		exit(-1);
	}

	freeaddrinfo(ai); 																			   // all done with this

	if (listen(primarySocketD, 10) == -1)                                                        // listen
	{
		cerr<<"listen() Failed. Exiting.";
		exit(-1);
	}                                                                                              //cout<<"Listen Done"<<endl;

	FD_SET(primarySocketD, &masterSet);														   // add the listening socket descripter to the master set
	maximumFileD = primarySocketD;															   // keep track of the biggest file descriptor, so far, it's this one
	FD_SET(0, &masterSet);																	   // for getting input from user.............Got Help From http://stackoverflow.com/questions/10219340/using-stdin-with-select-in-c & beej

	//cout<<"Before Entering Main Loop"<<endl;

	for(;;)																						   // main loop
	{																						       //cout<<"Entering Main Loop"<<endl;

		readSet = masterSet; 																	       // copy master set to read set

		if (select(maximumFileD+1, &readSet, NULL, NULL, NULL) == -1)
		{
			cerr<<"select() Failed. Exiting.";
			exit(-1);
		}																					            //cout<<"After Select Check"<<endl;

		for(i = 0; i <= maximumFileD + 1; i++)													        // run through the existing readSet looking for data to read or user input to read
		{																					            //cout<<i<<endl;
			if(FD_ISSET(0,&readSet))
			{																							 //cout<<commandLine;
				if(fgets(commandLine,100,stdin)==NULL)                                                  //Got Help from http://beej.us/guide/bgc/output/html/multipage/gets.html
				{
					cerr<<"fgets() Failed. Exiting."<<endl;
					exit(-1);
				}		                                                                                //cout<<"After fgets()"<<endl;
				commandLine[strlen(commandLine) - 1] = '\0';
				if(strstr(commandLine,"CREATOR")!=NULL)
				{
					creator();
					break;
				}
				else if(strstr(commandLine,"HELP")!=NULL)
				{																			            //cout<<"Entering commandLine Compare"<<endl;
					if(mode=='s')
					{
						helps();
					}
					if(mode=='c')
					{
						helpc();
					}
					break;
				}
				else if(strstr(commandLine,"MYIP")!=NULL)
				{
					cout<<"IP Address:"+myip()<<endl;
					break;
				}
				else if(strstr(commandLine,"MYPORT")!=NULL)
				{
					cout<<"Port:"+portToListenOnStr<<endl;
					break;
				}
				else if(strstr(commandLine,"LIST")!=NULL)
				{
					if(mode=='s' && isRegistered==true)
					{
						for(int q=0; q<16 ;q++)
						{
							if (nodeInfo[q].used==true && nodeInfo[q].host_id!=1 && nodeInfo[q].isServer==false)
							{   //cout<<q<<endl;
								printf("%-5d%-35s%-20s%-8d\n", nodeInfo[q].host_id, nodeInfo[q].hostname, nodeInfo[q].ip_addr, nodeInfo[q].port_num);
							}

						}
					}
					else if(mode=='s' && isRegistered==false)
					{
						cerr<<"Not Registered Clients"<<endl;
					}
					else if(mode=='c' && isRegistered==true)
					{
						for(int q=0; q<16 ;q++)
						{
							if (nodeInfo[q].used==true && nodeInfo[q].isServer==true)
							{   //cout<<q<<endl;
								printf("%-5d%-35s%-20s%-8d\n", nodeInfo[q].isServer, nodeInfo[q].hostname, nodeInfo[q].ip_addr, nodeInfo[q].port_num);
							}
							else if (nodeInfo[q].used==true && nodeInfo[q].isServer==false)
							{   //cout<<q<<endl;
								printf("%-5d%-35s%-20s%-8d\n", nodeInfo[q].host_id, nodeInfo[q].hostname, nodeInfo[q].ip_addr, nodeInfo[q].port_num);
							}

						}
					}
					else if(mode=='c' && isRegistered==false)
					{
						cerr<<"Not Registered With Server"<<endl;
					}
					break;
				}
				else if(strstr(commandLine,"REGISTER")!=NULL)
				{
					if(mode=='c' && isRegistered==false)
					{
						strtok(commandLine," ");
						serverIp=strtok(NULL," ");              //cout<<serverIp<<endl;
						serverPort=strtok(NULL," ");            //cout<<serverPort<<endl;
						char msg[64]={0};
						gethostname(hostNameClient,sizeof(hostNameClient));
						strcat(msg,"REGISTER ");
						strcat(msg,portToListenOnChar);
						strcat(msg," ");
						strcat(msg,hostNameClient);				//cout<<msg<<endl;
						memset(&hints, 0, sizeof hints);
						hints.ai_family = AF_UNSPEC;
						hints.ai_socktype = SOCK_STREAM;
						if ((resultChecker = getaddrinfo(serverIp, serverPort, &hints, &servInfo)) != 0)
						{
							cerr<<"Error Registering. getaddrinfo() Failed. Exiting. "<<endl;
							cerr<<gai_strerror(resultChecker);
						}

						for(p = servInfo; p != NULL; p = p->ai_next)
						{
							if ((newSocketD = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1)
							{
								cerr<<"Register Failed. Exiting"<<endl;
								continue;
							}
							if (resultChecker=connect(newSocketD, p->ai_addr, p->ai_addrlen) == -1)
							{
								close(newSocketD);
								cerr<<"Register Failed. Exiting"<<endl;
								cerr<<gai_strerror(resultChecker)<<endl;
								continue;
							}
							break;
						}
						if (p == NULL)
						{
							cerr<<"Server Not Found";
						}
						len=sizeof(msg);
						if (sendall(newSocketD, msg, &len) == -1)
						{
							cerr<<"send Error. Please Retry";
						}
						else
						{
							isRegistered=true;
							/*Fill Server Info*/
							nodeInfo[0].used=true;
							nodeInfo[0].host_id=newSocketD;
							strcpy(nodeInfo[0].ip_addr,serverIp);
							nodeInfo[0].port_num=strtol(serverPort,0,10);
							nodeInfo[0].isServer=true;
							addrlen = sizeof addressToResolve;
							if (getpeername(newSocketD, (struct sockaddr*)&addressToResolve, &addrlen)!=0)
							{

								//cerr<<"Could Not Get Socket Info of Peer"<<endl;
								strcpy(nodeInfo[0].hostname,"SERVER");
							}
							else
							{
								addrlen = sizeof addressToResolve;
								if(getnameinfo((struct sockaddr*)&addressToResolve, addrlen, nodeInfo[0].hostname, sizeof nodeInfo[0].hostname, NULL, 0, NI_NAMEREQD)!=0)
								{
									//cerr<<"Could Not Resolve Hostname of Server"<<endl;
									strcpy(nodeInfo[0].hostname,"SERVER");
								}
							}
							cout<<"Registered On "<<nodeInfo[0].hostname<<":"<<nodeInfo[0].port_num<<endl<<"Added To List"<<endl<<"Waiting For Server List"<<endl;
							FD_SET(newSocketD, &masterSet);
							if (newSocketD > maximumFileD)
							{ 																						// keep track of the max
								maximumFileD = newSocketD;
							}

						}
						break;
					}
					else if(mode=='c' && isRegistered==true)
					{
						cerr<<"Already Registered. Don't Be Greedy.";
						break;
					}
					else
					{
						cerr<<"Not a valid command in Server mode. Please use HELP";
						break;
					}
				}
				else if(strstr(commandLine,"CONNECT")!=NULL)
				{
					if(mode=='c' && connectionCount<3 && isRegistered==true)
					{
						char parameterA[32];
						char *peerPort;
						char *peerIp=(char*)malloc(32);
						strtok(commandLine," ");
						strcpy(parameterA,strtok(NULL," "));                       //cout<<parameterA<<endl;
						peerPort=strtok(NULL," ");            			          //  cout<<peerPort<<endl;
						int peerPortInt=strtol(peerPort,0,10);

						char msg[64]={0};
						strcat(msg,"CONNECT ");
						strcat(msg,portToListenOnChar);
						strcat(msg," ");
						strcat(msg,hostNameClient);						             //cout<<msg<<endl;
						bool validConnection=true;

						if(isalpha(parameterA[6]) || isalpha(parameterA[1]))          //referenced from http://www.cplusplus.com/forum/articles/9742/
						{
							hostent * record = gethostbyname(parameterA);
							if(record == NULL)
							{
								cerr<<"Could Not Resolve Hostame of Peer"<<endl;
								break;
							}
							in_addr * address = (in_addr * )record->h_addr;
							peerIp = inet_ntoa(* address);				              //cout<<peerIp<<endl;
							strcpy(parameterA,peerIp);
						}
						else
						{
							strncpy(peerIp,parameterA,sizeof(parameterA));					//cout<<peerIp<<endl;
						}

						if(strstr(serverList,parameterA)==NULL && validConnection==true)
						{
							validConnection=false;
							cerr<<"Not a Valid Peer"<<endl;
						}

						else if((strcmp(parameterA,myipadd)==0) && peerPortInt==portToListenOn && validConnection==true )
						{
							validConnection=false;
							cerr<<"Self-Connection Forbidden"<<endl;
						}

						else if((strcmp(parameterA,nodeInfo[0].ip_addr)==0) && peerPortInt==nodeInfo[0].port_num && validConnection==true)
						{
							validConnection=false;
							cerr<<"Connection Not Allowed With Server."<<endl;
						}

						for (int q=1; q<16; q++)
						{
							if((strcmp(parameterA,nodeInfo[q].ip_addr)==0) && peerPortInt==nodeInfo[q].port_num && validConnection==true)
							{
								validConnection=false;
								cerr<<"Duplicate Connection Not Allowed."<<endl;
								break;
							}
						}

						if(validConnection==true)
						{
							//cout<<"Valid Peer"<<endl;

							memset(&hints, 0, sizeof hints);
							hints.ai_family = AF_UNSPEC;
							hints.ai_socktype = SOCK_STREAM;
							if ((resultChecker = getaddrinfo(peerIp, peerPort, &hints, &servInfo)) != 0)
							{
								cerr<<"Error Connecting. getaddrinfo() Failed. Exiting. "<<endl;
								cerr<<gai_strerror(resultChecker);
								break;
							}

							for(p = servInfo; p != NULL; p = p->ai_next)
							{
								if ((newSocketD = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1)
								{
									cerr<<"Connect Failed. Not a Valid Peer"<<endl;
									continue;
								}
								if (resultChecker=connect(newSocketD, p->ai_addr, p->ai_addrlen) == -1)
								{
									close(newSocketD);
									cerr<<"Connect Failed. Not a Valid Peer"<<endl;
									cerr<<gai_strerror(resultChecker)<<endl;
									continue;
								}
								break;
							}
							if (p == NULL)
							{
								cerr<<"Connect Failed. Not a Valid Peer";
							}
							len=sizeof(msg);
							if (sendall(newSocketD, msg, &len) == -1)
							{
								cerr<<"send Error. Please Retry";
							}
							else
							{
								connectionCount++;
								/*Fill Peer Info*/
								nodeInfo[newSocketD].used=true;
								nodeInfo[newSocketD].host_id=newSocketD;
								addrlen = sizeof addressToResolve;
								if (getpeername(newSocketD, (struct sockaddr*)&addressToResolve, &addrlen)!=0)
								{
									//cerr<<"Could Not Get Socket Info of Peer"<<endl;
									strcpy(nodeInfo[newSocketD].hostname,parameterA);
								}
								else
								{
									addrlen = sizeof addressToResolve;
									if(getnameinfo((struct sockaddr*)&addressToResolve, addrlen, nodeInfo[newSocketD].hostname, sizeof nodeInfo[newSocketD].hostname, NULL, 0, NI_NAMEREQD)!=0)
									{
										//cerr<<"Could Not Resolve Hostname of Peer"<<endl;
										strcpy(nodeInfo[newSocketD].hostname,parameterA);
									}
								}
								strcpy(nodeInfo[newSocketD].ip_addr,peerIp);
								nodeInfo[newSocketD].port_num=strtol(peerPort,0,10);

								nodeStats[newSocketD].used=true;
								strcpy(nodeStats[newSocketD].hostname,nodeInfo[newSocketD].hostname);
								nodeStats[newSocketD].uploads=0;
								nodeStats[newSocketD].avgU=0.0;
								nodeStats[newSocketD].downloads=0;
								nodeStats[newSocketD].avgD=0.0;

								FD_SET(newSocketD, &masterSet);
								if (newSocketD > maximumFileD)
								{ 																						// keep track of the max
									maximumFileD = newSocketD;
								}
								cout<<"Connected To "<<nodeInfo[newSocketD].hostname<<":"<<nodeInfo[newSocketD].port_num<<endl<<"Added To List."<<endl;
							}
						}
						break;
					}
					else if(mode=='c' && connectionCount<3 && isRegistered==false)
					{
						cerr<<"First Register With a Server."<<endl;
						break;
					}
					else if(mode=='c' && connectionCount>=3)
					{
						cerr<<"Limit of connection reached. Don't Be Greedy."<<endl;
						break;
					}
					else if(mode=='s')
					{
						cerr<<"Not a valid command in Server mode. Please use HELP"<<endl;
						break;
					}
				}
				else if(strstr(commandLine,"TERMINATE")!=NULL)
				{
					if (mode=='c')
					{
						strtok(commandLine," ");
						char* tId=strtok(NULL," ");
						int tIdInt=strtol(tId,0,10);
						if(connectionCount==0)
						{
							cerr<<"No Connected Peers"<<endl;
						}
						else if(nodeInfo[tIdInt].used==false )
						{
							cerr<<"Invalid Terminate Identifier"<<endl;
						}
						else if(nodeInfo[tIdInt].used==true && nodeInfo[tIdInt].isServer==true)
						{
							cerr<<"Cannot Terminate Connection With Server, Use Exit"<<endl;
						}
						else
						{
							char msg[64]={0};
							strcat(msg,"TERMINATE");
							len=sizeof(msg);
							cout<<"Terminating Connection With "<<nodeInfo[tIdInt].hostname<<endl;
							if (sendall(tIdInt, msg, &len) == -1)
							{
								cerr<<"send Error. Please Retry";
							}
							else
							{
								nodeInfo[tIdInt].used=false;
								nodeInfo[tIdInt].port_num=0;
								for(int q=15;q>=0;q--)
								{
									if(nodeStats[q].used==false && nodeStats[q].past==false)
									{
										nodeStats[q].used=true;
										nodeStats[q].past=true;
										strcpy(nodeStats[q].hostname,nodeInfo[tIdInt].hostname);
										nodeStats[q].avgD=nodeStats[tIdInt].avgD;
										nodeStats[q].avgU=nodeStats[tIdInt].avgU;
										nodeStats[q].uploads=nodeStats[tIdInt].uploads;
										nodeStats[q].downloads=nodeStats[tIdInt].downloads;
										nodeStats[tIdInt].used=false;
										break;
									}
								}
								close(tIdInt);
								FD_CLR(tIdInt, &masterSet);
								if (tIdInt==maximumFileD)
								{ 																						// keep track of the max
									maximumFileD--;
								}
							}
							cout<<"Connection Terminated"<<endl;
							connectionCount--;
						}
						break;
					}
					else if(mode=='s')
					{
						cerr<<"Not a valid command in Server mode. Please use HELP"<<endl;
						break;
					}
				}
				else if(strstr(commandLine,"EXIT")!=NULL)
				{
					if (mode=='c')
					{
						char msg[64]={0};
						strcat(msg,"EXIT");
						len=sizeof(msg);
						cout<<"Exiting..."<<endl;
						if (sendall(nodeInfo[0].host_id, msg, &len) == -1)
						{
							cerr<<"send Error. Please Retry";
						}
						else
						{
							char msg[64]={0};
							strcat(msg,"BYEBYE");
							len=sizeof(msg);
							for(int q=0; q<16 ; q++)
							{
								if (nodeInfo[q].used==true && q<=maximumFileD && nodeInfo[q].isServer==false)
								{
									if (sendall(nodeInfo[q].host_id, msg, &len) == -1)
									{
										cerr<<"send Error. Please Retry";
									}
									close(nodeInfo[q].host_id);
									FD_CLR(nodeInfo[q].host_id, &masterSet);
								}
							}
						}
						cout<<"All Connections Terminated"<<endl;
						exit(0);
						break;
					}
					else if(mode=='s')
					{
						bool canExit=true;
						for(int q=1;q<16;q++)
						{
							if(nodeInfo[q].used==true)
							{
								canExit=false;
							}
						}
						if(canExit)
						{
							cout<<"ByeBye"<<endl;
							exit(0);
						}
						else
						{
							cerr<<"Please Do Not Exit, Peers Still Connected"<<endl;
						}
					}
					break;
				}
				else if(strstr(commandLine,"UPLOAD")!=NULL)
				{
					if(mode=='c')
					{
						strtok(commandLine," ");
						char* uId=strtok(NULL," ");
						int uIdInt=strtol(uId,0,10);
						char *filePath=(char*)malloc(256);
						filePath=strtok(NULL," ");
						//cout<<"FP"<<filePath<<endl;
						if (nodeInfo[uIdInt].isServer==false && nodeInfo[uIdInt].used==true && fileExists(filePath)==true)
						{
							double timeTaken;
							size_t fileSize;
							double speed;
							char msg[256]={0};
							strcat(msg,"UPLOAD");
							strcat(msg," ");
							strcat(msg,filePath);
							len=sizeof(msg);		//cerr<<msg<<endl;

							if (sendall(uIdInt, msg, &len) == -1)
							{
								cerr<<"send Error. Please Retry";
							}
							else
							{

								if(uploader(uIdInt, filePath, &timeTaken, &fileSize)==0)
								{
									cout<<"Done"<<endl;
									speed=(fileSize/(1048576.00000))/(timeTaken/1000.00000);            // bits/sec
									cout<<"Tx: "<<hostNameClient<<" -> "<<nodeInfo[uIdInt].hostname<<" , File Size: "<<fileSize<<" Bytes, Time Taken: "<<timeTaken/1000<< "seconds, Tx Rate: "<<speed<<" MB/seconds"<<endl;
									if(nodeStats[uIdInt].uploads==0)
									{
										nodeStats[uIdInt].avgU=speed;
									}
									else
									{
										nodeStats[uIdInt].avgU=((nodeStats[uIdInt].avgU*nodeStats[uIdInt].uploads)+speed)/2;
									}
									nodeStats[uIdInt].uploads++;
								}
								else
								{
									cerr<<"Error"<<endl;
								}
							}
						}
						else if(fileExists(filePath)==false)
						{
							cerr<<"File does not exist"<<endl;
						}
						else if(nodeInfo[uIdInt].isServer==true)
						{
							cerr<<"Cannot Upload to Server"<<endl;
						}
						else if(nodeInfo[uIdInt].used==false)
						{
							cerr<<"Invalid Upload Id"<<endl;
						}
						break;
					}
					else if(mode=='s')
					{
						cerr<<"Not a valid command in Server mode. Please use HELP"<<endl;
						break;
					}
				}
				else if(strstr(commandLine,"DOWNLOAD")!=NULL)
				{
					if(mode=='c')
					{
						struct download dNode[4];
						int numOfD=0;
						for(int q=1;q<4;q++)
						{
							dNode[q].dIdInt=-1;
							dNode[q].filePath=(char*)malloc(256);
							dNode[q].dirPath=(char*)malloc(256);
						}
						uint pos=0;
						while (pos<strlen(commandLine))  //number of Downloads
						{
							char c=commandLine[pos];
							if (isspace(c))
							{
								numOfD++;
							}
							pos++;
						}
						numOfD=numOfD/2;
						//cout<<"no from string"<<numOfD<<endl;
						strtok(commandLine," ");
						char *pch=(char*)malloc(256);
						pch=strtok(NULL," ");
						pos=1;
						while(pch!=NULL && pos<=numOfD)
						{
							dNode[pos].dId=pch;
							dNode[pos].dIdInt=strtol(dNode[pos].dId,0,10);
							//cout<<pos<<dNode[pos].dIdInt<<endl;
							pch=strtok(NULL," ");
							dNode[pos].filePath=pch;
							//cout<<pos<<dNode[pos].filePath<<endl;
							char* temp=(char*)malloc(256);
							strcpy(temp,dNode[pos].filePath);
							dNode[pos].dirPath=dirname(temp);
							//cout<<pos<<dNode[pos].dirPath<<endl;
							pch=strtok(NULL, " ");
							pos++;
						}
						for(int q = 0; q < 4; q++)
						{
							for (int p = q + 1; p < 4; p++)
							{
								if(dNode[q].dIdInt<dNode[p].dIdInt)
								{
									dNode[0]=dNode[q];
									dNode[q]=dNode[p];
									dNode[p]=dNode[0];
								}
							}
						}


						for(int q=1;q<=numOfD;q++)
						{
							if(nodeInfo[dNode[q].dIdInt].used==false)
							{
								cout<<dNode[q].dIdInt<<" Is Invalid, Cannot Download"<<endl;
								numOfD--;
								dNode[q].dIdInt=-1;
							}
							else if(nodeInfo[dNode[q].dIdInt].isServer==true)
							{
								cout<<dNode[q].dIdInt<<" Is Server, Cannot Download"<<endl;
								numOfD--;
								dNode[q].dIdInt=-1;
							}
							else if(dNode[q].dIdInt==-1)
							{
								cout<<dNode[q].dIdInt<<" Is Invalid, Cannot Download"<<endl;
								numOfD--;
							}
						}
						for (int q=1;q<=numOfD;q++)
						{
							if(dirExists(dNode[q].dirPath)!=true)												//check if dir exists http://linux.die.net/man/2/stat
							{
								cout<<"Directory does not exists, creating directory"<<endl;
								if(mkdir(dNode[q].dirPath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)!=0)
								{
									cerr<<"Cannot create directory. Exiting"<<endl;
									exit(-1);
								}
							}
						}
						for(int q = 0; q < 4; q++)
						{
							for (int p = q + 1; p < 4; p++)
							{
								if(dNode[q].dIdInt<dNode[p].dIdInt)
								{
									dNode[0]=dNode[q];
									dNode[q]=dNode[p];
									dNode[p]=dNode[0];
								}
							}
						}

						if(numOfD==1)
						{
							char msg[256]={0};
							strcat(msg,"DOWNLOAD");
							strcat(msg," ");
							strcat(msg,dNode[1].filePath);
							len=sizeof(msg);		//cerr<<msg<<endl;

							if (sendall(dNode[1].dIdInt, msg, &len) == -1)
							{
								cerr<<"send Error. Please Retry";
							}
							else
							{

								if(downloader(dNode[1].dIdInt, dNode[1].filePath, &dNode[1].timeTaken, &dNode[1].fileSize)==0)
								{
									cout<<"Done"<<endl;
									if(dNode[1].timeTaken<=0)
									{
										dNode[1].timeTaken=1.0;
									}
									dNode[1].speed=(dNode[1].fileSize/1048576.00000)/(dNode[1].timeTaken/1000.00000);            // bits/sec
									cout<<"Rx: "<<hostNameClient<<" <- "<<nodeInfo[dNode[1].dIdInt].hostname<<" , File Size: "<<dNode[1].fileSize<<" Bytes, Time Taken: "<<dNode[1].timeTaken/1000<< "seconds, Rx Rate: "<<dNode[1].speed<<" MB/seconds"<<endl;
									nodeStats[dNode[1].dIdInt].downloads++;
									nodeStats[dNode[1].dIdInt].avgD=((nodeStats[dNode[1].dIdInt].avgD*nodeStats[dNode[1].dIdInt].downloads)+dNode[1].speed)/2;
								}
								else
								{
									cerr<<"Error"<<endl;
								}
							}
						}
						else if(numOfD==2)
						{
							char msg1[256]={0};
							strcat(msg1,"DOWNLOAD");
							strcat(msg1," ");
							strcat(msg1,dNode[1].filePath);
							int len1=sizeof(msg1);		cerr<<dNode[1].dIdInt<<" "<<msg1<<endl;
							char msg2[256]={0};
							strcat(msg2,"DOWNLOAD");
							strcat(msg2," ");
							strcat(msg2,dNode[2].filePath); cerr<<dNode[2].dIdInt<<" "<<msg2<<endl;
							int len2=sizeof(msg2);

							if (sendall(dNode[1].dIdInt, msg1, &len1) == -1)
							{
								cerr<<"send Error. Please Retry";
							}
							else if(sendall(dNode[2].dIdInt, msg2, &len2) == -1)
							{
								cerr<<"send Error. Please Retry";
							}
							else
							{

								if(downloader2(dNode[1].dIdInt, dNode[2].dIdInt, dNode[1].filePath, dNode[2].filePath, &dNode[1].timeTaken, &dNode[2].timeTaken, &dNode[1].fileSize, &dNode[2].fileSize)==0)
								{
									cout<<"Done"<<endl;
									if(dNode[1].timeTaken<=0)
									{
										dNode[1].timeTaken=1.0;
									}
									if(dNode[2].timeTaken<=0)
									{
										dNode[2].timeTaken=1.0;
									}
									dNode[1].speed=(dNode[1].fileSize/1048576.00000)/(dNode[1].timeTaken/1000);            // bits/sec
									cout<<"Rx: "<<hostNameClient<<" <- "<<nodeInfo[dNode[1].dIdInt].hostname<<" , File Size: "<<dNode[1].fileSize<<" Bytes, Time Taken: "<<dNode[1].timeTaken/1000<< "seconds, Rx Rate: "<<dNode[1].speed<<" MB/seconds"<<endl;
									if(nodeStats[dNode[1].dIdInt].downloads==0)
									{
										nodeStats[dNode[1].dIdInt].avgD=dNode[1].speed;
									}
									else
									{
										nodeStats[dNode[1].dIdInt].avgD=((nodeStats[dNode[1].dIdInt].avgD*nodeStats[dNode[1].dIdInt].downloads)+dNode[1].speed)/2;
									}
									nodeStats[dNode[1].dIdInt].downloads++;

									dNode[2].speed=(dNode[2].fileSize/1048576.00000)/(dNode[2].timeTaken/1000);            // bits/sec
									cout<<"Rx: "<<hostNameClient<<" <- "<<nodeInfo[dNode[2].dIdInt].hostname<<" , File Size: "<<dNode[2].fileSize<<" Bytes, Time Taken: "<<dNode[2].timeTaken/1000<< "seconds, Rx Rate: "<<dNode[2].speed<<" MB/seconds"<<endl;
									if(nodeStats[dNode[2].dIdInt].downloads==0)
									{
										nodeStats[dNode[2].dIdInt].avgD=dNode[2].speed;
									}else
									{
										nodeStats[dNode[2].dIdInt].avgD=((nodeStats[dNode[2].dIdInt].avgD*nodeStats[dNode[2].dIdInt].downloads)+dNode[2].speed)/2;
									}
									nodeStats[dNode[2].dIdInt].downloads++;
								}

								else
								{
									cerr<<"Error"<<endl;
								}
							}
						}
						else if(numOfD==3)
						{
							char msg1[256]={0};
							strcat(msg1,"DOWNLOAD");
							strcat(msg1," ");
							strcat(msg1,dNode[1].filePath);
							int len1=sizeof(msg1);		cerr<<dNode[1].dIdInt<<" "<<msg1<<endl;

							char msg2[256]={0};
							strcat(msg2,"DOWNLOAD");
							strcat(msg2," ");
							strcat(msg2,dNode[2].filePath); cerr<<dNode[2].dIdInt<<" "<<msg2<<endl;
							int len2=sizeof(msg2);

							char msg3[256]={0};
							strcat(msg3,"DOWNLOAD");
							strcat(msg3," ");
							strcat(msg3,dNode[3].filePath);
							int len3=sizeof(msg3);		cerr<<dNode[3].dIdInt<<" "<<msg3<<endl;


							if (sendall(dNode[1].dIdInt, msg1, &len1) == -1)
							{
								cerr<<"send Error. Please Retry";
							}
							else if(sendall(dNode[2].dIdInt, msg2, &len2) == -1)
							{
								cerr<<"send Error. Please Retry";
							}
							else if(sendall(dNode[3].dIdInt, msg3, &len3) == -1)
							{
								cerr<<"send Error. Please Retry";
							}
							else
							{

								if(downloader3(dNode[1].dIdInt, dNode[2].dIdInt, dNode[3].dIdInt, dNode[1].filePath, dNode[2].filePath, dNode[3].filePath, &dNode[1].timeTaken, &dNode[2].timeTaken, &dNode[3].timeTaken, &dNode[1].fileSize, &dNode[2].fileSize, &dNode[3].fileSize)==0)
								{
									cout<<"Done"<<endl;
									if(dNode[1].timeTaken<=0)
									{
										dNode[1].timeTaken=1.0;
									}
									if(dNode[2].timeTaken<=0)
									{
										dNode[2].timeTaken=1.0;
									}
									if(dNode[3].timeTaken<=0)
									{
										dNode[3].timeTaken=1.0;
									}
									dNode[1].speed=(dNode[1].fileSize/1048576.00000)/(dNode[1].timeTaken/1000);            // bits/sec
									cout<<"Rx: "<<hostNameClient<<" <- "<<nodeInfo[dNode[1].dIdInt].hostname<<" , File Size: "<<dNode[1].fileSize<<" Bytes, Time Taken: "<<dNode[1].timeTaken/1000<< "seconds, Rx Rate: "<<dNode[1].speed<<" MB/seconds"<<endl;
									if(nodeStats[dNode[1].dIdInt].downloads==0)
									{
										nodeStats[dNode[1].dIdInt].avgD=dNode[1].speed;
									}
									else
									{
										nodeStats[dNode[1].dIdInt].avgD=((nodeStats[dNode[1].dIdInt].avgD*nodeStats[dNode[1].dIdInt].downloads)+dNode[1].speed)/2;
									}
									nodeStats[dNode[1].dIdInt].downloads++;

									dNode[2].speed=(dNode[2].fileSize/1048576.00000)/(dNode[2].timeTaken/1000);            // bits/sec
									cout<<"Rx: "<<hostNameClient<<" <- "<<nodeInfo[dNode[2].dIdInt].hostname<<" , File Size: "<<dNode[2].fileSize<<" Bytes, Time Taken: "<<dNode[2].timeTaken/1000<< "seconds, Rx Rate: "<<dNode[2].speed<<" MB/seconds"<<endl;
									if(nodeStats[dNode[2].dIdInt].downloads==0)
									{
										nodeStats[dNode[2].dIdInt].avgD=dNode[2].speed;
									}else
									{
										nodeStats[dNode[2].dIdInt].avgD=((nodeStats[dNode[2].dIdInt].avgD*nodeStats[dNode[2].dIdInt].downloads)+dNode[2].speed)/2;
									}
									nodeStats[dNode[2].dIdInt].downloads++;

									dNode[3].speed=(dNode[3].fileSize/1048576.00000)/(dNode[3].timeTaken/1000);            // bits/sec
									cout<<"Rx: "<<hostNameClient<<" <- "<<nodeInfo[dNode[3].dIdInt].hostname<<" , File Size: "<<dNode[3].fileSize<<" Bytes, Time Taken: "<<dNode[3].timeTaken/1000<< "seconds, Rx Rate: "<<dNode[3].speed<<" MB/seconds"<<endl;
									if(nodeStats[dNode[3].dIdInt].downloads==0)
									{
										nodeStats[dNode[3].dIdInt].avgD=dNode[3].speed;
									}else
									{
										nodeStats[dNode[3].dIdInt].avgD=((nodeStats[dNode[3].dIdInt].avgD*nodeStats[dNode[3].dIdInt].downloads)+dNode[3].speed)/2;
									}
									nodeStats[dNode[3].dIdInt].downloads++;
								}
								else
								{
									cerr<<"Error"<<endl;
								}
							}
						}
						else
						{
							cerr<<"Nothing to Download"<<endl;
						}

					}
					else if(mode=='s')
					{
						cerr<<"Not a valid command in Server mode. Please use HELP"<<endl;
						break;
					}
				}
				else if(strstr(commandLine,"STATISTICS")!=NULL)
				{
					if(mode=='s' && isRegistered==true)
					{
						for(int q=0; q<16; q++)
						{
							if (nodeInfo[q].host_id!=1 && nodeInfo[q].used==true && nodeInfo[q].isServer==false)		// ask  everyone!
							{
								if (send(nodeInfo[q].host_id, "STATISTICS", sizeof("STATISTICS"),0) == -1)
								{
									cerr<<"Error Asking for Statistics Updates"<<endl;
								}
								else
								{
									recv(nodeInfo[q].host_id,buf,sizeof(buf),0);
									buf[strlen(buf)]='\0';
									strcpy(statList,buf);
									displayStatList(statList);
									cout<<endl;
									memset(buf, 0, 1024);
								}

							}
						}
					}
					else if(mode=='s' && isRegistered==false)
					{
						cerr<<"Not Registered Clients"<<endl;
					}
					else if(mode=='c' && isRegistered==true)
					{
						for(int q=0; q<16 ;q++)
						{
							if (nodeStats[q].used==true)
							{   //cout<<q<<endl;
								printf("%-20s%-20d%-20f%-20d%-20f\n", nodeStats[q].hostname, nodeStats[q].uploads, nodeStats[q].avgU, nodeStats[q].downloads, nodeStats[q].avgD);
							}

						}
					}
					else if(mode=='c' && isRegistered==false)
					{
						cerr<<"Not Registered With Server"<<endl;
					}
					break;
				}

				else
				{
					cerr<<"Command Not Recognized. Please use HELP."<<endl;
					break;
				}
				memset(commandLine, 0, 100);

			}

			else if(FD_ISSET(i,&readSet))
			{

				if (i == primarySocketD)                                   										// handle new connections
				{                                                                                          		//cout<<"In i=listeningSocketD"<<endl;
					addrlen = sizeof(remoteaddr);
					newSocketD = accept(primarySocketD,(struct sockaddr *)&remoteaddr,&addrlen);
					if (newSocketD == -1)
					{
						cerr<<"Accept Error";
					}
					else
					{
						FD_SET(newSocketD, &masterSet); 														// add to master set
						tracker=newSocketD;																		// for setting host_id
						if (newSocketD > maximumFileD)
						{ 																						// keep track of the max
							maximumFileD = newSocketD;
						}
						inet_ntop(remoteaddr.ss_family,get_in_addr((struct sockaddr*)&remoteaddr),remoteIP, INET_ADDRSTRLEN);
						if(mode=='s')
						{
							cout<<"New Registration Request... "<<endl;
						}
						else if(mode=='c')
						{
							cout<<"New Connection Request... "<<endl;
						}
					}
				}
				else
				{
					//cout<<"In Data Receiver"<<endl;
					if ((nbytes = recv(i, buf, sizeof buf, 0)) <= -1)
					{
						if (nbytes == 0)
						{
							cout<<"Socket "<<i<<" hung up"<<endl;
							nodeInfo[i].used=false;
							nodeInfo[i].port_num=0000;
						}
						else
						{
							cerr<<"recv(QQQ) failed"<<endl;
							nodeInfo[i].used=false;
							nodeInfo[i].port_num=0000;
						}

						close(i);
						FD_CLR(i, &masterSet);
					}
					else                 //commands and responses at other end
					{
						if(strstr(buf,"REGISTER")!=NULL && mode=='s')              //server action on receiveing register command
						{
							if (nodeInfo[tracker].used==false)
							{
								//cout<<tracker<<endl;
								buf[strlen(buf)]='\0';
								strtok(buf," ");
								nodeInfo[tracker].used=true;
								char *porttmp=strtok(NULL," ");
								nodeInfo[tracker].port_num=strtol(porttmp, NULL, 10);
								strcpy(nodeInfo[tracker].ip_addr,remoteIP);                                                      //cout<<nodeInfo[nodeCount].ipaddress<<endl;
								if (getpeername(tracker, (struct sockaddr*)&addressToResolve, &addrlen)!=0)
								{
									//cerr<<"Could Not Get Socket Info of Peer"<<endl;
									strcpy(nodeInfo[tracker].hostname,strtok(NULL," "));
								}
								else
								{
									addrlen = sizeof addressToResolve;
									if(getnameinfo((struct sockaddr*)&addressToResolve, addrlen, nodeInfo[tracker].hostname, sizeof nodeInfo[newSocketD].hostname, NULL, 0, NI_NAMEREQD)!=0)
									{
										//cerr<<"Could Not Resolve Hostname of Peer"<<endl;
										strcpy(nodeInfo[tracker].hostname,strtok(NULL," "));
									}
								}
								//cout<<nodeInfo[nodeCount].hostname<<endl;
								nodeInfo[tracker].host_id=tracker;

								//Now send the updated list to all clients//
								char msg[256]={0};
								strcat(msg,"LU ");  //means list update
								for(int q=0; q<16; q++)
								{
									if(nodeInfo[q].used==true && nodeInfo[q].isServer==false)
									{
										char tempnodeid[4];
										char tempportnumber[4];
										sprintf(tempnodeid,"%d",nodeInfo[q].host_id);
										sprintf(tempportnumber,"%d",nodeInfo[q].port_num);
										strcat(msg,tempnodeid);
										strcat(msg," ");
										strcat(msg,nodeInfo[q].hostname);
										strcat(msg," ");
										strcat(msg,nodeInfo[q].ip_addr);
										strcat(msg," ");
										strcat(msg,tempportnumber);
										strcat(msg,";");
									}
								}
								len=sizeof(msg);
								isRegistered=true;
								for(int q=0; q<16; q++)
								{
									if (/*nodeInfo[q].host_id!=i && */nodeInfo[q].host_id!=1 && nodeInfo[q].used==true && FD_ISSET(q,&masterSet))		// send to everyone!
									{
										//cout<<"sending to "<<q<<endl;
										if (sendall(q, msg, &len) == -1)
										{
											cerr<<"Error Sending List Updates"<<endl;
										}
										else
										{
											if(len<sizeof(msg))
											{
												cout<<"Only Sent "<<len<<" Bytes"<<endl;
											}
										}
									}
								}
								cout<<nodeInfo[tracker].hostname<<":"<<nodeInfo[tracker].port_num<<" Registered. "<<endl<<"Added To List. "<<endl<<"List Update Sent."<<endl;
								tracker=0;
								memset(buf, 0, 1024);
								break;
							}
						}
						else if(strstr(buf,"STATISTICS")!=NULL && mode=='c')         //got list update from server
						{
							char msg[256]={0};
							for(int q=0; q<16; q++)
							{
								if(nodeStats[q].used==true)
								{
									char tempU[20];
									char tempD[20];
									char tempAU[20];
									char tempAD[20];
									sprintf(tempU,"%d",nodeStats[q].uploads);
									sprintf(tempD,"%d",nodeStats[q].downloads);
									sprintf(tempAU,"%f",nodeStats[q].avgU);
									sprintf(tempAD,"%f",nodeStats[q].avgD);

									strcat(msg,hostNameClient);
									strcat(msg," ");
									strcat(msg,nodeStats[q].hostname);
									strcat(msg," ");
									strcat(msg,tempU);
									strcat(msg," ");
									strcat(msg,tempAU);
									strcat(msg," ");
									strcat(msg,tempD);
									strcat(msg," ");
									strcat(msg,tempAD);
									strcat(msg,";");
								}
							}
							int len=sizeof(msg);
							sendall(i,msg,&len);

							memset(buf, 0, 1024);
							break;

						}
						else if(strstr(buf,"LU")!=NULL && mode=='c')         //got list update from server
						{
							buf[strlen(buf)]='\0';
							strcpy(serverList,buf);
							cout<<"Got Updated Server-List"<<endl;
							displayServerList(serverList);
							cout<<endl;
							memset(buf, 0, 1024);
							break;

						}
						else if(strstr(buf,"CONNECT")!=NULL && mode=='c' && isRegistered==true)              //client action on recieveing connect command
						{
							if (connectionCount<3)
							{
								//cout<<tracker<<endl;
								buf[strlen(buf)]='\0';
								strtok(buf," ");
								nodeInfo[tracker].used=true;
								char *porttmp=strtok(NULL," ");
								nodeInfo[tracker].port_num=strtol(porttmp, NULL, 10);
								strcpy(nodeInfo[tracker].ip_addr,remoteIP);                                                      //cout<<nodeInfo[nodeCount].ipaddress<<endl;
								addrlen = sizeof addressToResolve;
								if (getpeername(tracker, (struct sockaddr*)&addressToResolve, &addrlen)!=0)
								{
									//cerr<<"Could Not Get Socket Info of Peer"<<endl;
									strcpy(nodeInfo[tracker].hostname,strtok(NULL," "));
								}
								else
								{
									addrlen = sizeof addressToResolve;
									if(getnameinfo((struct sockaddr*)&addressToResolve, addrlen, nodeInfo[tracker].hostname, sizeof nodeInfo[newSocketD].hostname, NULL, 0, NI_NAMEREQD)!=0)
									{
										//cerr<<"Could Not Resolve Hostname of Peer"<<endl;
										strcpy(nodeInfo[tracker].hostname,strtok(NULL," "));
									}
								}
								//cout<<nodeInfo[nodeCount].hostname<<endl;
								nodeInfo[tracker].host_id=tracker;
								connectionCount++;
								cout<<nodeInfo[tracker].hostname<<":"<<nodeInfo[tracker].port_num<<" Connected." <<endl<<"Added To List"<<endl;
								nodeStats[tracker].used=true;
								strcpy(nodeStats[tracker].hostname,nodeInfo[tracker].hostname);
								nodeStats[tracker].uploads=0;
								nodeStats[tracker].avgU=0.0;
								nodeStats[tracker].downloads=0;
								nodeStats[tracker].avgD=0.0;
								tracker=0;
								memset(buf, 0, 1024);
								break;
							}
							else
							{
								cout<<"Connection Request Refused, Maximum Connections Reached"<<endl;
								char msg[64]={0};
								strcat(msg,"REFUSE");
								len=sizeof(msg);
								if (sendall(tracker, msg, &len) == -1)
								{
									cerr<<"send Error. Please Retry";
								}
								close(tracker);
								FD_CLR(tracker,&masterSet);
								if (tracker==maximumFileD)
								{ 																						// keep track of the max
									maximumFileD--;
								}
								break;
							}
							memset(buf, 0, 1024);
							break;
						}
						else if(strstr(buf,"TERMINATE")!=NULL && mode=='c')         //got terminate req
						{
							if (nodeInfo[i].used==true && nodeInfo[i].host_id==i)
							{
								buf[strlen(buf)]='\0';
								cout<<nodeInfo[i].hostname<<" Sent Terminate Request"<<endl;
								nodeInfo[i].used=false;
								nodeStats[i].used=false;
								nodeInfo[i].port_num=0;
								close(i);
								FD_CLR(i, &masterSet);
								if (i==maximumFileD)
								{ 																						// keep track of the max
									maximumFileD--;
								}
								cout<<"Connection Terminated"<<endl;
								connectionCount--;
								for(int q=15;q>=0;q--)
								{
									if(nodeStats[q].used==false && nodeStats[q].past==false)
									{
										nodeStats[q].used=true;
										nodeStats[q].past=true;
										strcpy(nodeStats[q].hostname,nodeInfo[i].hostname);
										nodeStats[q].avgD=nodeStats[i].avgD;
										nodeStats[q].avgU=nodeStats[i].avgU;
										nodeStats[q].uploads=nodeStats[i].uploads;
										nodeStats[q].downloads=nodeStats[i].downloads;
										nodeStats[i].used=false;
										break;
									}
								}
							}
							memset(buf, 0, 1024);
							break;
						}
						else if(strstr(buf,"REFUSE")!=NULL && mode=='c')         //client refused connection
						{
							if (nodeInfo[i].used==true && nodeInfo[i].host_id==i)
							{
								buf[strlen(buf)]='\0';
								cout<<nodeInfo[i].hostname<<" Refused Connection"<<endl;
								nodeInfo[i].used=false;
								nodeStats[i].used=false;
								nodeInfo[i].port_num=0;
								close(i);
								FD_CLR(i, &masterSet);
								if (i==maximumFileD)
								{ 																						// keep track of the max
									maximumFileD--;
								}
								cout<<"Connection Terminated"<<endl;
								connectionCount--;
							}
							memset(buf, 0, 1024);
							break;
						}
						else if(strstr(buf,"EXIT")!=NULL && mode=='s')         //client exiting server action
						{
							if (nodeInfo[i].used==true)
							{
								buf[strlen(buf)]='\0';
								cout<<nodeInfo[i].hostname<<" Exited Swarm"<<endl;
								close(i);
								FD_CLR(i, &masterSet);
								if (i==maximumFileD)
								{ 																						// keep track of the max
									maximumFileD--;
								}
								nodeInfo[i].used=false;
								nodeInfo[i].port_num=0;
								//Now send the updated list to all clients//
								char msg[256]={0};
								strcat(msg,"LU ");  //means list update
								for(int q=0; q<10; q++)
								{
									if(nodeInfo[q].used==true)
									{
										char tempnodeid[4];
										char tempportnumber[4];
										sprintf(tempnodeid,"%d",nodeInfo[q].host_id);
										sprintf(tempportnumber,"%d",nodeInfo[q].port_num);
										strcat(msg,tempnodeid);
										strcat(msg," ");
										strcat(msg,nodeInfo[q].hostname);
										strcat(msg," ");
										strcat(msg,nodeInfo[q].ip_addr);
										strcat(msg," ");
										strcat(msg,tempportnumber);
										strcat(msg,";");
									}
								}
								len=sizeof(msg);
								isRegistered=true;
								for(int q=0; q<16; q++)
								{
									if (nodeInfo[q].host_id!=i && nodeInfo[q].isServer==false && nodeInfo[q].used==true && FD_ISSET(q,&masterSet))		// send to everyone!
									{
										//cout<<"sending to "<<q<<endl;
										if (sendall(q, msg, &len) == -1)
										{
											cerr<<"Error Sending List Updates"<<endl;
										}
										else
										{
											if(len<sizeof(msg))
											{
												cout<<"Only Sent "<<len<<" Bytes"<<endl;
											}
										}
									}
								}
								cout<<"List Update Sent"<<endl;
							}
							memset(buf, 0, 1024);
							break;
						}
						else if(strstr(buf,"BYEBYE")!=NULL && mode=='c')         //client exiting other clients action
						{
							if (nodeInfo[i].used==true && nodeInfo[i].host_id==i)
							{
								buf[strlen(buf)]='\0';
								cout<<nodeInfo[i].hostname<<" Exited the Swarm"<<endl;
								nodeInfo[i].used=false;
								nodeInfo[i].port_num=0;
								for(int q=15;q>=0;q--)
								{
									if(nodeStats[q].used==false && nodeStats[q].past==false)
									{
										nodeStats[q].used=true;
										nodeStats[q].past=true;
										strcpy(nodeStats[q].hostname,nodeInfo[i].hostname);
										nodeStats[q].avgD=nodeStats[i].avgD;
										nodeStats[q].avgU=nodeStats[i].avgU;
										nodeStats[q].uploads=nodeStats[i].uploads;
										nodeStats[q].downloads=nodeStats[i].downloads;
										nodeStats[i].used=false;
										break;
									}
								}
								close(i);
								FD_CLR(i, &masterSet);
								if (i==maximumFileD)
								{ 																						// keep track of the max
									maximumFileD--;
								}
								connectionCount--;
							}
							memset(buf, 0, 1024);
							break;
						}
						else if(strstr(buf,"UPLOAD")!=NULL && mode=='c')         //got upload command.....download file
						{
							if (nodeInfo[i].used==true && nodeInfo[i].host_id==i)
							{
								double timeTaken;
								size_t fileSize;
								double speed;
								strtok(buf," ");
								char* filePath=(char*)malloc(256);
								char* temp=(char*)malloc(256);
								temp=filePath;
								char* dirPath=(char*)malloc(256);
								filePath=strtok(NULL," ");
								//cout<<filePath<<endl;
								strcpy(temp,filePath);
								dirPath=dirname(temp);										//cout<<dirPath<<endl;
								cout<<nodeInfo[i].hostname<<" is Sending a File"<<endl;
								cout<<"File will be saved in directory "<<dirPath<<endl;
								if(dirExists(dirPath)!=true)												//check if dir exists http://linux.die.net/man/2/stat
								{
									cout<<"Directory does not exists, creating directory"<<endl;
									if(mkdir(dirPath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)!=0)
									{
										cerr<<"Cannot create directory. Exiting"<<endl;
										exit(-1);
									}
									if(fileExists(filePath)==true)
									{
										cerr<<"File Already Exists. File will be deleted"<<endl;
										if(remove(filePath)!=0)
										{
											cerr<<"Cannot Remove File. Exiting"<<endl;
											exit(-1);
										}
									}
								}
								cout<<"Ready to download..."<<endl;

								if(int x=downloader(i, filePath, &timeTaken, &fileSize)!=0)
								{
									cerr<<"Error"<<x<<endl;
								}
								else
								{
									cout<<"Done"<<endl;
									if(timeTaken<=0){timeTaken=1.0;}
									speed=(fileSize/1048576.00000)/(timeTaken/1000);            // bits/sec
									cout<<"Rx: "<<nodeInfo[i].hostname<<" -> "<<hostNameClient<<" , File Size: "<<fileSize<<" Bytes, Time Taken: "<<timeTaken/1000<< "seconds, Rx Rate: "<<speed<<" MB/seconds"<<endl;
									nodeStats[i].downloads++;
									nodeStats[i].avgD=((nodeStats[i].avgD*nodeStats[i].downloads)+speed)/2;
								}
							}
							memset(buf, 0, 1024);
							break;
						}
						else if(strstr(buf,"DOWNLOAD")!=NULL && mode=='c')         //got download command........Upload file
						{
							if (nodeInfo[i].used==true && nodeInfo[i].host_id==i)
							{
								double timeTaken;
								size_t fileSize;
								double speed;
								strtok(buf," ");
								char* filePath=(char*)malloc(256);
								filePath=strtok(NULL," ");
								cout<<nodeInfo[i].hostname<<" is Requesting "<<filePath<<endl;
								if(fileExists(filePath)==true)
								{
									cout<<"Ready to upload..."<<endl;

									if(int x=uploader(i, filePath, &timeTaken, &fileSize)!=0)
									{
										cerr<<"Error"<<x<<endl;
									}
									else
									{
										cout<<"Done"<<endl;
										if(timeTaken<=0){timeTaken=1.0;}
										speed=(fileSize/1048576.00000)/(timeTaken/1000);            // bits/sec
										cout<<"Tx: "<<nodeInfo[i].hostname<<" -> "<<hostNameClient<<" , File Size: "<<fileSize<<" Bytes, Time Taken: "<<timeTaken/1000<< "seconds, Tx Rate: "<<speed<<" MB/seconds"<<endl;
										nodeStats[i].uploads++;
										nodeStats[i].avgU=((nodeStats[i].avgU*nodeStats[i].uploads)+speed)/2;
									}
								}
								else
								{
									cerr<<"File Not Found"<<endl;
								}
							}
							memset(buf, 0, 1024);
							break;
						}
						else
						{
							memset(buf, 0, 1024);
							break;
						}

					}
				}
			}
		}
	}
	cout<<"exit";
	return 0;
}
