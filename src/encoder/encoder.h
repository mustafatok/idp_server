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

        virtual int width(){ return _width; }
        virtual int height(){ return _height; }       
        // TODO : They are just valid for multifile. Edit this!
        virtual int lWidth(){ return _lWidth; }
        virtual int lHeight(){ return _lHeight; }
        virtual int rWidth(){ return _rWidth; }
        virtual int rHeight(){ return _rHeight; }

protected:
        int _width;
        int _height;
        int _csp = 0;
        int _id; 

        // TODO : They are just valid for multifile. Edit this!
        int _lWidth, _lHeight, _rWidth, _rHeight;
        EncoderObserver* _observer;
};


#endif // __ENCODER_H
