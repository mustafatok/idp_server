#include "network.h"

#include <iostream>

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include "../global.h"

using namespace std;

#define PACKET_SIZE	50000

UdpSocket::UdpSocket()
{
	// readCallback = [](uint8_t, uint8_t*, int) {
	// 		// dummy
	// };
}

UdpSocket::~UdpSocket()
{
	close();
}


bool UdpSocket::initServer(uint16_t port)
{
	// create socket
	if ((_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		cerr << "Could not initialize udp socket." << endl;
		return false;
	}

	// initialize local interfaces and port
	memset(static_cast<void*>(&localAddress), 0, sizeof(struct sockaddr_in));
	localAddress.sin_family = AF_INET;
	localAddress.sin_port = htons(port);
	localAddress.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(_socket, reinterpret_cast<struct sockaddr*>(&localAddress), sizeof(struct sockaddr_in)) < 0) {
		cerr << "Bind to socket failed!" << endl;
		return false;
	}

	readerThread = new thread([&](){
		(*this)();
	});

	return true;
}

void UdpSocket::close()
{
	if (_socket != -1) {
		int sd = _socket;
		_socket = -1;
		shutdown(sd, SHUT_RDWR);
		readerThread->join();
		delete readerThread;
	}
}

void UdpSocket::operator()(){
	struct sockaddr_in incomming;
	struct sockaddr* in = reinterpret_cast<struct sockaddr*>(&incomming);
	socklen_t inlen = sizeof(struct sockaddr_in);

	// todo catch EINTR signal
	while (_socket != -1) {
		ssize_t result = 0;

		if (!headerValid) {
			uint8_t header[HEADER_SIZE];

			result = recvfrom(_socket, header, HEADER_SIZE, MSG_WAITALL, in, &inlen);
			if (result == 0) { // orderly shutdown
				break;
			} else if (result != HEADER_SIZE) {
				cerr << "Reading protocol-header failed!, Error: "  << strerror(errno) << endl;
				continue;
			}

			headerValid = true;
			payloadType = header[0];
			uint8_t* sizePtr = &header[1];
			payloadSize = *(reinterpret_cast<uint32_t*>(sizePtr));

			if (payloadSize > 0) {
				payload = new uint8_t[payloadSize + 8]; // +8 because of decoder, otherwise we would have to copy everything!!
			} else {
				payload = nullptr;
			}

			headerValid = true;
		} else {
			if (payloadSize > 0) {
				result = recvfrom(_socket, payload + payloadPosition, payloadSize - payloadPosition, MSG_WAITALL, in, &inlen);
				if (result == 0) {
						break;
				} else if (result < 0) {
						cerr << "Reading payload failed! " << strerror(errno)<< ", recvsize: "<< result  << endl;
						continue;
				} else if (result + payloadPosition == payloadSize) {
						cerr << "Reading payload succeeded, size: " << result << endl;
						payloadPosition = 0;
				} else {
						payloadPosition += result;
						continue;
				}
			} else if (payloadType == PROTOCOL_TYPE_INIT) {
				cout << "PROTOCOL_TYPE_INIT" << endl;
				remoteAddress = incomming;
				remoteAddress.sin_port = htons(7070);
				connectionCallback(&incomming, inlen);
			} else if (payloadType == PROTOCOL_TYPE_CLOSE) {
				closeConnectionCallback(&incomming, inlen);
			}

			headerValid = false;
			// readCallback(payloadType, payload, payloadSize);
		}
	}
}

void UdpSocket::send(uint8_t* data, int size)
{
	int res = 0;
	if ((res = sendto(_socket, data, size, 0, reinterpret_cast<struct sockaddr*>(&remoteAddress), sizeof(struct sockaddr_in))) == -1) {
		cerr << "Could not send package with size: " << size << endl;
	}
}

void UdpSocket::send(uint8_t type, uint8_t* data, int size)
{

	uint8_t tmpData[2 + size];
	tmpData[0] = type;
    memcpy(&(tmpData[1]), data,size);
    tmpData[size + 1] = '\n';
  
	if (data != nullptr && size > 0) {
		if ( size > PACKET_SIZE ) {
			return;
		}
		if ( size > 0)
			send (&(tmpData[0]) , size + 2);
	}
}

void UdpSocket::initClientParameters(int32_t mode, int32_t lWidth, int32_t lHeight, int32_t rWidth, int32_t rHeight){
	int size = sizeof(int32_t) * 5;
	uint8_t* data;
	data = new uint8_t[size];
	int32_t* tmp;
	tmp = reinterpret_cast<int32_t*>(data);
	*tmp = mode;
	*(tmp + 1) = lWidth;
	*(tmp + 2) = lHeight;
	*(tmp + 3) = rWidth;
	*(tmp + 4) = rHeight;
	this->send(PROTOCOL_TYPE_CLIENT_INIT, &(data[0]), size);
}
