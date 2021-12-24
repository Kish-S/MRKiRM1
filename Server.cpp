// UDP server that use blocking sockets

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "conio.h"
#include <fcntl.h>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define SERVER_PORT 27015	// Port number of server that will be used for communication with clients
#define BUFFER_SIZE 512		// Size of buffer that will be used for sending and receiving messages to clients

// Checks if ip address belongs to IPv4 address family
bool is_ipV4_address(sockaddr_in6 address);

int main()
{
    // Server address 
     sockaddr_in6  serverAddress; 
	 sockaddr_in   ipv4serverAddress;
	// Buffer we will use to send and receive clients' messages
    char dataBuffer[BUFFER_SIZE];

	// WSADATA data structure that is to receive details of the Windows Sockets implementation
    WSADATA wsaData;

	// Initialize windows sockets library for this process
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
    {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        return 1;
    }

    // Initialize serverAddress structure used by bind function
	memset((char*)&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin6_family = AF_INET6; 			// set server address protocol family
    serverAddress.sin6_addr = in6addr_any;			// use all available addresses of server
    serverAddress.sin6_port = htons(SERVER_PORT);	// Set server port
	serverAddress.sin6_flowinfo = 0;				// flow info



													// Initialize memory for address structure
	memset((char*)&ipv4serverAddress, 0, sizeof(ipv4serverAddress));

	// Initialize address structure of server
	ipv4serverAddress.sin_family = AF_INET;								// IPv4 address famly
	ipv4serverAddress.sin_addr.s_addr = INADDR_ANY;	// Set server IP address using string
	ipv4serverAddress.sin_port = htons(SERVER_PORT);					// Set server port


    // Create a socket 
    SOCKET serverSocket = socket(AF_INET6,      // IPv6 address famly
								 SOCK_DGRAM,   // datagram socket
								 IPPROTO_UDP); // UDP
	SOCKET ipv4Socket = socket(AF_INET,      // IPv4 address famly
								SOCK_DGRAM,   // Datagram socket
								IPPROTO_UDP); // UDP protocol


	// Check if socket creation succeeded
    if (serverSocket == INVALID_SOCKET)
    {
        printf("Creating socket failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
	unsigned long mode = 1;
	ioctlsocket(serverSocket, FIONBIO, &mode); /* Change the socket into non-blocking state	*/
	ioctlsocket(ipv4Socket, FIONBIO, &mode); /* Change the socket into non-blocking state	*/

	// Disable receiving only IPv6 packets. We want to receive both IPv4 and IPv6 packets.
	int iResult;
    // Bind server address structure (type, port number and local address) to socket
    iResult = bind(serverSocket,(SOCKADDR *)&serverAddress, sizeof(serverAddress));

	// Check if socket is succesfully binded to server datas
    if (iResult == SOCKET_ERROR)
    {
        printf("Socket bind failed with error: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

	// Bind server address structure (type, port number and local address) to socket
	iResult = bind(ipv4Socket, (SOCKADDR *)&ipv4serverAddress, sizeof(ipv4serverAddress));

	// Check if socket is succesfully binded to server datas
	if (iResult == SOCKET_ERROR)
	{
		printf("Socket bind failed with error: %d\n", WSAGetLastError());
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}


	printf("Simple UDP server waiting client messages.\n");

    // Main server loop
    while(1)
    {
        // Declare and initialize client address that will be set from recvfrom
        sockaddr_in6 clientAddress;
		sockaddr_in  client2Address;
		memset(&clientAddress, 0, sizeof(clientAddress));
		memset(&client2Address, 0, sizeof(client2Address));

		// Set whole buffer to zero
        memset(dataBuffer, 0, BUFFER_SIZE);

		// size of client address
		int sockAddrLen = sizeof(clientAddress);

		// Receive client message
        iResult = recvfrom(serverSocket,						// Own socket
			               dataBuffer,							// Buffer that will be used for receiving message
						   BUFFER_SIZE,							// Maximal size of buffer
						   0,									// No flags
						   (struct sockaddr *)&clientAddress,	// Client information from received message (ip address and port)
						   &sockAddrLen);						// Size of sockadd_in structure
		
		if (iResult > 0)
		{
			char ipAddress[INET6_ADDRSTRLEN]; // INET6_ADDRSTRLEN 65 spaces for hexadecimal notation of IPv6
							
			inet_ntop(clientAddress.sin6_family, &clientAddress.sin6_addr, ipAddress, sizeof(ipAddress));

			unsigned short clientPort = ntohs(clientAddress.sin6_port);

			printf("IPv6 Client connected from ip: %s, port: %d, sent: %s.\n", ipAddress, clientPort, dataBuffer);
		}


		iResult = recvfrom(ipv4Socket,						// Own socket
			dataBuffer,							// Buffer that will be used for receiving message
			BUFFER_SIZE,							// Maximal size of buffer
			0,									// No flags
			(struct sockaddr *)&client2Address,	// Client information from received message (ip address and port)
			&sockAddrLen);						// Size of sockadd_in structure

		if (iResult > 0)
		{
			char ipAddress2[INET_ADDRSTRLEN];
			inet_ntop(client2Address.sin_family, &client2Address.sin_addr, ipAddress2, sizeof(ipAddress2));
			unsigned short client2Port = ntohs(client2Address.sin_port);
			printf("IPv4 Client connected from ip: %s, port: %d, sent: %s.\n", ipAddress2, client2Port, dataBuffer);
		}

		
    }

    // Close server application
    iResult = closesocket(serverSocket);
    if (iResult == SOCKET_ERROR)
    {
        printf("closesocket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
        return 1;
    }
	
	printf("Server successfully shut down.\n");
	
	// Close Winsock library
	WSACleanup();
	_getch();
	return 0;
}

bool is_ipV4_address(sockaddr_in6 address)
{
	char *check = (char*)&address.sin6_addr.u;

	for (int i = 0; i < 10; i++)
		if(check[i] != 0)
			return false;
		
	if(check[10] != -1 || check[11] != -1)
		return false;

	return true;
}
