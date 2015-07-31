#include <fstream>
#include <iostream>
#include <thread>

#include "input/input.h"
#include "input/camera.h"
#include "input/file.h"
#include "input/multifile.h"
#include "encoder/encoder.h"
#include "encoder/multiencoder.h"
#include "output/network.h"
#include "tools/timer.h"

using namespace std;

// CameraInput videoInput("Allied Vision Technologies-50-0536872642");
// FileInput videoInput("/home/mustafa/Downloads/STEREOTEST.avi");
MultiFileInput videoInput("file", "/LEFT.mp4", "/RIGHT.mp4");
// MultiFileInput videoInput("camera", "Allied Vision Technologies-50-0536874357", "Allied Vision Technologies-50-0536872642");

// H264Encoder encoder;

MultiH264Encoder *encoder;
UdpSocket output;
int mode = -1;
thread *encoderThread = nullptr;

void onNewConnection(struct sockaddr_in*, int) {
	cout << "onNewConnection called" << endl;
	output.send(PROTOCOL_TYPE_MODE_INIT, nullptr, mode);
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
		encoder->stop();
		encoderThread->join();
		delete encoderThread;
		encoderThread = nullptr;
	}
}

int main(int argc, char* argv[])
{
	cout << "1 - MODE_VERTICALCONCAT" << endl                    
		 <<	"2 - MODE_LEFTRESIZED" << endl
		 << "3 - MODE_RIGHTRESIZED" << endl 
		 << "4 - MODE_LEFTBLURRED" << endl
		 << "5 - MODE_RIGHTBLURRED" << endl
		 << "6 - MODE_INTERLEAVING" << endl;

	string input;
	for(;;) {

		cin >> input;
		if (input == "exit") {
				break;
		}

		if (input == "1") {
			mode = (int) MODE_VERTICALCONCAT;
			cout << "MODE_VERTICALCONCAT" << endl;
		}else if (input == "2") {
			mode = (int) MODE_LEFTRESIZED;
			cout << "MODE_LEFTRESIZED" << endl;
		}else if (input == "3") {
			mode = (int) MODE_RIGHTRESIZED;
			cout << "MODE_RIGHTRESIZED" << endl;
		}else if (input == "4") {
			mode = (int) MODE_LEFTBLURRED;
			cout << "MODE_LEFTBLURRED" << endl;
		}else if (input == "5") {
			mode = (int) MODE_RIGHTBLURRED;
			cout << "MODE_RIGHTBLURRED" << endl;
		}else if (input == "6") {
			mode = (int) MODE_INTERLEAVING;
			cout << "MODE_INTERLEAVING" << endl;
		}
		if(mode != -1){
			encoder = new MultiH264Encoder(mode);
		}

		encoder->setEncoderObserver(0, &output);
		videoInput.setInputObserver(0, encoder);

		output.setConnectionCallback(onNewConnection);
		output.setCloseConnectionCallback(onCloseConnection);

		if (!output.initServer(2525)) {
			cerr << "Could not initiate udp socket." << endl;
			return -1;
		}

	}
	
	// TODO: cleanup

	return 0;
}
