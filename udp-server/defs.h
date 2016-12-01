#ifndef DEFS_H
#define DEFS_H

// C assert //
#include <assert.h>

#ifdef RECD_DEBUG
#define ASSERT_MACRO(EXPR) assert(EXPR)
#else
#define ASSERT_MACRO(EXPR)
#endif

namespace  iz {

struct interface_t
{
    void    (*init)(void);
    void    (*copy)(const void*, void*, int);
    int     (*put_ndata)(void*, int);
    int     (*put_data)(void*);
    void*   (*get_data)(void);
    void    (*deinit)(void);
    int     (*main_proxy)(int, char**);
    interface_t* (*getSelf)(void);
    void*   this_interface;
    interface_t* nextPlugin;
};


class InterfaceList
{
public:
    InterfaceList();
    ~InterfaceList();
    void put(interface_t *iface);
    interface_t *getFront();
    interface_t *getBack();

private:
    void clear();
    interface_t* head, *tail;
};


} // iz
#endif // DEFS_H
