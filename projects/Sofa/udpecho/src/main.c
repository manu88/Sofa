#include "helpers.h"
#include "runtime.h"
#include <Sofa.h>
#include <Thread.h>
#include "net.h"
#include <arpa/inet.h>
#include <netinet/in.h>

int fOut = -1;

int main(int argc, char *argv[])
{   
    
    RuntimeInit2(argc, argv);
    SFPrintf("[%i] started\n", SFGetPid());

    argc -=2;
    argv = &argv[2];

    if(argc < 2)
    {
        SFPrintf("usage: udpecho port\n");
        return 1;
    }

    int inport = atoi(argv[1]);
    SFPrintf("[UDP ECHO] in port %i\n", inport);


    NetInit();

    	// create a UDP socket, creation returns -1 on failure
	int sock;
	if ((sock = NetSocket(PF_INET, SOCK_DGRAM, 0)) < 0) 
    {
		SFPrintf("could not create socket\n");
		return 1;
	}

    // socket address used for the server
	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;

	// htons: host to network short: transforms a value in host byte
	// ordering format to a short value in network byte ordering format
	server_address.sin_port = htons(inport);

	// htons: host to network long: same as htons but to long
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);


    // bind it to listen to the incoming connections on the created server
	// address, will return -1 on error
	if ((NetBind(sock, (struct sockaddr *)&server_address,
	          sizeof(server_address))) < 0) 
	{
		SFPrintf("could not bind socket\n");
		return 1;
	}

    // socket address used to store client address
	struct sockaddr_in client_address;
	int client_address_len = 0;

	// run indefinitely
	while (1)
    {
		char buffer[500];

		// read content into buffer from an incoming client
		int len = NetRecvFrom(sock, buffer, sizeof(buffer), 0,
		                   (struct sockaddr *)&client_address,
		                   &client_address_len);

        buffer[len] = '\0';
		SFPrintf("received %i: '%s' from client %s:%u\n", len, buffer,
		       inet_ntoa(client_address.sin_addr), client_address.sin_port);

		ssize_t retSend = NetSendTo(sock, buffer, len, 0, (struct sockaddr *)&client_address,
		       sizeof(client_address));

		SFPrintf("Send to returned %zi\n", retSend);

    }

    return 0;
}

