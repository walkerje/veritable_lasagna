#include "atomic.h"
#include <vl/vl_atomic.h>

vl_bool_t vlTESTAtomicCompile(){
    vl_atomic_bool_t atomicFlag;
    vlAtomicStore(&atomicFlag, VL_TRUE);
    return VL_TRUE;
}