#ifndef RECORDERIFACE_H
#define RECORDERIFACE_H

namespace iz {

/// plugin reflection
/// \brief The RecIface struct
///
struct RecIface
{
    void    (*init)(void);
    void    (*copy)(const void* src, void* dst, int len);
    int     (*put_ndata)(void*, int);
    int     (*put_data)(void*);
    void*   (*get_data)(void);
    void    (*deinit)(void);
    int     (*main_proxy)(int, char**); // if we need to call lib in main
    void*   this_interface;
    RecIface* next;
};

} // iz

#endif // RECORDERIFACE_H
