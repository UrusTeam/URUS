#pragma once

#include "FastFunctor.h"

// macros to hide the details of delegate functions using fastfunctor
#define AP_HAL_CLASSPROC(classptr, func) fastfunctor::MakeDelegate(classptr, func)
#define AP_HAL_MEMBERPROC(func) AP_HAL_CLASSPROC(this, func)

#define DELEGATE_FUNCTION(rettype, ...) fastfunctor::FastFunctor0<rettype, ## __VA_ARGS__>

#define FUNCTOR_TYPEDEF(name, rettype, ...) \
    typedef DELEGATE_FUNCTION(rettype, ## __VA_ARGS__) name

#define FUNCTOR_DECLARE(name, rettype, ...) \
    DELEGATE_FUNCTION(rettype, ## __VA_ARGS__) name

#define FUNCTOR_BIND(obj, func, rettype, ...) \
    AP_HAL_CLASSPROC(obj, func)

#define FUNCTOR_BIND_MEMBER(func, rettype, ...) \
    AP_HAL_MEMBERPROC(func)
