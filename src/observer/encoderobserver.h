#ifndef __ENCODEROBSERVER_H
#define __ENCODEROBSERVER_H

#include <stdint.h>

class EncoderObserver {
public:
	virtual void onEncodedDataReceived(int id, uint8_t type, uint8_t* data, int size) = 0;
};

#endif // __ENCODEROBSERVER_H
