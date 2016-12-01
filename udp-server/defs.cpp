#include "defs.h"

namespace iz {

InterfaceList::InterfaceList()
    : head(nullptr),
      tail(nullptr)
{
}

InterfaceList::~InterfaceList()
{
    //clear();
}

void InterfaceList::put(interface_t* iface)
{
    if (head == nullptr) {
        head = iface->getSelf();
        tail = head;
    } else {
        interface_t* newone = iface->getSelf();
        tail->nextPlugin = newone;
        tail = newone;
    }
}

interface_t* InterfaceList::getFront()
{
    interface_t* tmp = head;
    return tmp;
}

interface_t* InterfaceList::getBack()
{
    interface_t* tmp = tail;
    return tmp;
}

void InterfaceList::clear()
{
    while (head != nullptr) {
        interface_t* tmp  = head;
        head = head->nextPlugin;
        delete tmp;
    }
}

}
