#ifndef __ENCODER_H
#define __ENCODER_H
#include <stdint.h>
#include "../observer/inputobserver.h"
#include "../observer/encoderobserver.h"

class Encoder{
public:
        void setSize(int width, int height) {
            this->_width = width;
            this->_height = height;
        }
        void setColorspace(int csp) {
            this->_csp = csp;
        }

        void setEncoderObserver(EncoderObserver* observer){
            this->_observer = observer; 
        }
        virtual void postProcess(uint8_t type, uint8_t* data, int size){
            _observer->onEncoderDataReceived(type, data, size);
        }
        virtual void stop(){}

protected:
        int _width;
        int _height;
        int _csp = 0;

        EncoderObserver* _observer;
};


#endif // __ENCODER_H
