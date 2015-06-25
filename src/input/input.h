#ifndef __INPUT_H
#define __INPUT_H

#include <functional>
#include <stdlib.h>
#include "../encoder/encoder.h"
#include "../global.h"

class Input {
protected:
        Encoder* _encoder;
        int _id;
public:
        explicit Input() = default;
        virtual ~Input() = default;
        virtual void operator()(){}

        virtual void setEncoder(int id, Encoder* encoder){
            this->_encoder = encoder; 
            this->_id = id;
        }
};

#endif // __INPUT_H