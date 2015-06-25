#ifndef __ENCODER_H
#define __ENCODER_H
#include <stdint.h>
#include "../output/network.h"

class Encoder {
public:
        virtual void setSize(int id, int width, int height) {
            this->_width = width;
            this->_height = height;
        }
        virtual void setColorspace(int id, int csp) {
            this->_csp = csp;
        }
        virtual void pushFrame(int id, uint8_t** framePlanes, int* framePlaneSizes, int planes) = 0;
        virtual void pushFrame(int id, uint8_t** lframePlanes, int* lframePlaneSizes, int lplanes, uint8_t** rframePlanes, int* rframePlaneSizes, int rplanes) = 0;
        virtual void printStats(int id, int code){}

        void setSocket(UdpSocket* socket){
            this->_socket = socket; 
        }

protected:
        int _width;
        int _height;
        int _csp = 0;

        UdpSocket* _socket;
};


#endif // __ENCODER_H
