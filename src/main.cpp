#include <fstream>
#include <iostream>
#include <thread>

#include "input/input.h"
#include "input/camera.h"
#include "input/file.h"
#include "input/multifile.h"
#include "encoder/encoder.h"
#include "output/network.h"
#include "tools/timer.h"

using namespace std;

// CameraInput videoInput("Allied Vision Technologies-50-0536872642");
// FileInput videoInput("/home/mustafa/Downloads/STEREOTEST.avi");
// MultiFileInput videoInput("file", "/LEFT.mp4", "/RIGHT.mp4");
MultiFileInput videoInput("camera", "Allied Vision Technologies-50-0536874357", "Allied Vision Technologies-50-0536872642");

H264Encoder encoder;

thread *encoderThread = nullptr;

void onNewConnection(struct sockaddr_in*, int) {
	cout << "onNewConnection called" << endl;
	if (encoderThread == nullptr) {
		encoderThread = new thread([&](){
				videoInput();
		});
	}
}

void onCloseConnection(struct sockaddr_in*, int) {
	if (encoderThread != nullptr) {
		cout << "onCloseConnection called" << endl;
		videoInput.stop();
		encoderThread->join();
		delete encoderThread;
		encoderThread = nullptr;
	}
}

int main(int argc, char* argv[])
{

	UdpSocket socket;
	encoder.setDataCallback(&socket, &UdpSocket::send);
	videoInput.setSizeCallback(&encoder, &H264Encoder::setSize);
	videoInput.setFrameCallback(&encoder, &H264Encoder::pushFrame);
	videoInput.setStatusCallback(&encoder, &H264Encoder::printStats);
	videoInput.setColorspaceCallback(&encoder, &H264Encoder::setColorspace);
	socket.setConnectionCallback(onNewConnection);
	socket.setCloseConnectionCallback(onCloseConnection);

	if (!socket.initServer(2525)) {
		cerr << "Could not initiate udp socket." << endl;
		return -1;
	}

	string input;
	for(;;) {

		cin >> input;
		if (input == "exit") {
				break;
		}
	}
	
	// TODO: cleanup

	return 0;
}
