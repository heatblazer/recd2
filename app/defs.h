#ifndef DEFS_H
#define DEFS_H

// C assert //
#include <assert.h>

#ifdef iz_DEBUG
#define ASSERT_MACRO(EXPR) assert(EXPR)
#else
#define ASSERT_MACRO(EXPR)
#endif

namespace  recd {

struct interface_t
{
    void    (*init)(void);
    void    (*copy)(const void*, void*, int);
    int     (*put_ndata)(void*, int);
    int     (*put_data)(void*);
    void*   (*get_data)(void);
    void    (*deinit)(void);
    int     (*main_proxy)(int, char**);
    void    (*setName)(const char*);
    const char* (*getName)(void);
    interface_t* (*getSelf)(void);
    char   name[256];
    interface_t* nextPlugin;
};

/// originaly made to be a template,
/// but I don`t need it to be so...
/// kiss rule :)
/// \brief The InterfaceList class
///
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


} // recd
#endif // DEFS_H
