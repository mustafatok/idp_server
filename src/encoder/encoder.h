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

        void setEncoderObserver(int id, EncoderObserver* observer){
            this->_id = id;
            this->_observer = observer; 
        }
        virtual void postProcess(uint8_t type, uint8_t* data, int size){
            _observer->onEncodedDataReceived(_id, type, data, size);
        }
        virtual void stop(){}

protected:
        int _width;
        int _height;
        int _csp = 0;
        int _id; 

        EncoderObserver* _observer;
};


#endif // __ENCODER_H
