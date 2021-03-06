#ifndef __INPUT_H
#define __INPUT_H

#include <functional>
#include <stdlib.h>
#include "../observer/inputobserver.h"
#include "../global.h"

class Input {
protected:
        InputObserver* _observer;
        int _id;

        bool stopped = false;

public:
        explicit Input() = default;
        virtual ~Input() = default;
        virtual void operator()(){}

        void setInputObserver(int id, InputObserver* observer){
            this->_observer = observer; 
            this->_id = id;
        }
        virtual void stop() { stopped = true; }
        virtual void start() { stopped = false; }
        virtual void setInputOffset(int x, int y){}
        virtual void setInputOffsetX(int x1, int x2){}
        virtual void setInputOffsetX(int x){}

};

#endif // __INPUT_H