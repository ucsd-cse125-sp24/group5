#pragma once

#include <vector>
#include <memory>
#include "Handler.h"

namespace bge {
    class Event {
        virtual void subscribe(Handler handler) {}
        virtual void notify(){}
        
        std::vector<std::shared_ptr<Handler>> subscribers;
    };

}