#ifndef __ENCODEROBSERVER_H
#define __ENCODEROBSERVER_H

#include <functional>
#include <string>
#include <thread>
#include <netinet/in.h>
#include <stdint.h>

class EncoderObserver {
public:
	virtual void onEncoderDataReceived(uint8_t type, uint8_t* data, int size) = 0;
};

#endif // __ENCODEROBSERVER_H
