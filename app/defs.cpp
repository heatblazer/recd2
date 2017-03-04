#include "defs.h"

namespace recd {

InterfaceList::InterfaceList()
    : head(nullptr),
      tail(nullptr)
{
}

InterfaceList::~InterfaceList()
{
    clear();
}

void InterfaceList::put(interface_t* iface)
{
    if (head == nullptr) {
        head = iface;
        tail = head;
    } else {
        interface_t* newone = iface;
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

/// clear all
/// careful: they may not be used,
/// first perform a deinit logic to
/// finish your tasks
/// \brief InterfaceList::clear
///
void InterfaceList::clear()
{
    while (head != nullptr) {
        interface_t* tmp  = head;

        head = head->nextPlugin;
        // BUGFIX!
        // shall not be called on that pointer
        if (tmp == nullptr) {
            delete tmp;
        }
    }
}

} // recd
