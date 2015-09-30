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

MultiFileInput *videoInput;
// MultiFileInput videoInput("camera", "Allied Vision Technologies-50-0536874357", "Allied Vision Technologies-50-0536872642");

MultiH264Encoder *encoder = nullptr;
UdpSocket output;
int mode = -1;
thread *encoderThread = nullptr;

void init(){
	output.initClientParameters(mode, encoder->lWidth(), encoder->lHeight(), encoder->rWidth(), encoder->rHeight());
	if (encoderThread == nullptr) {
		encoderThread = new thread([&](){
			(*videoInput)();
		});
	}
}
void onNewConnection(struct sockaddr_in*, int) {
	cout << "onNewConnection called" << endl;
	init();
}

void onCloseConnection(struct sockaddr_in*, int) {
	if (encoderThread != nullptr) {
		cout << "onCloseConnection called" << endl;
		videoInput->stop();
		encoder->stop();
		encoderThread->join();
		delete encoderThread;
		encoderThread = nullptr;
	}
}

int main(int argc, char* argv[])
{
	bool camera = false;
	for(int i = 1; i < argc; ++i) {
		std::string value(argv[i]);
		if (value == "--camera") {
			camera = true;
		}
	}

	string input;
	for(;;) {
		cout << "1 - MODE_BEST" << endl                    
			 <<	"2 - MODE_VERTICALCONCAT - 2000" << endl
			 <<	"3 - MODE_LEFTRESIZED - 200 - 800" << endl
			 <<	"4 - MODE_RIGHTRESIZED - 800 - 200" << endl
			 <<	"5 - MODE_LEFTBLURRED - 400 - 600" << endl
			 <<	"6 - MODE_RIGHTBLURRED - 600 - 400" << endl
			 <<	"7 - MODE_INTERLEAVING - 2000 - 2000" << endl;

		 
		cin >> input;
		if (input == "exit") {
				break;
		}
		int lbitRate = 5000;
		int rbitRate = 5000;

		if (input == "1") {
			mode = (int) MODE_VERTICALCONCAT;
			cout << "MODE_BEST - 5000" << endl;
		}else if (input == "2") {
			cout << "MODE_VERTICALCONCAT - 2000" << endl;
			mode = (int) MODE_VERTICALCONCAT;
			lbitRate = 2000;
		}else if (input == "3") {
			cout << "MODE_LEFTRESIZED - 200 - 800" << endl;
			mode = (int) MODE_LEFTRESIZED;
			lbitRate = 200;
			rbitRate = 800;
		}else if (input == "4") {
			cout << "MODE_RIGHTRESIZED - 800 - 200" << endl;
			mode = (int) MODE_RIGHTRESIZED;
			lbitRate = 800;
			rbitRate = 200;
		}else if (input == "5") {
			cout << "MODE_LEFTBLURRED - 400 - 600" << endl;
			mode = (int) MODE_LEFTBLURRED;
			lbitRate = 400;
			rbitRate = 600;
		}else if (input == "6") {
			cout << "MODE_RIGHTBLURRED - 600 - 400" << endl;
			mode = (int) MODE_RIGHTBLURRED;
			lbitRate = 600;
			rbitRate = 400;
		}else if (input == "7") {
			cout << "MODE_INTERLEAVING - 2000 - 2000" << endl;
			mode = (int) MODE_INTERLEAVING;
			lbitRate = 2000;
			rbitRate = 2000;
		}
		if(mode == -1) continue;

		bool flag = false;
		if(encoder == nullptr){
			output.setConnectionCallback(onNewConnection);
			output.setCloseConnectionCallback(onCloseConnection);

			if (!output.initServer(2525)) {
				cerr << "Could not initiate udp socket." << endl;
				return -1;
			}
		}else{
			videoInput->stop();
			encoder->stop();
			encoderThread->join();
			delete encoderThread;
			delete encoder;
			delete videoInput;
			flag = true;
			encoderThread = nullptr;
			 

		}
		if(camera){
			videoInput = new MultiFileInput("camera", "Allied Vision Technologies-50-0536874357", "Allied Vision Technologies-50-0536872642");
		}else{
			videoInput = new MultiFileInput("file", "/LEFT.mp4", "/RIGHT.mp4");
		}

		if(videoInput->type() == "camera"){
			encoder = new MultiH264Encoder(mode, 960, 544);
			encoder->setFps(25);
		}else{
			encoder = new MultiH264Encoder(mode, 960, 544);
			encoder->setFps(25);
		}
		
		encoder->setBitRate(lbitRate, rbitRate);
		encoder->setEncoderObserver(0, &output);
		videoInput->setInputObserver(0, encoder);

		if(flag)
			init();
		// videoInput->start();

	}
	
	// TODO: cleanup

	return 0;
}
