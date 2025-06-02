include(CheckTypeSize)

# Primitive type resolution.
# This section is dense and awkward.
# Its purpose is to select explicit sizes purely from built-in language types.
# This offers less dependence on the C standard, allowing the configuration of all integer and floating-point types,
# with their own expectations (explicit IEEE-754 format) and documentation (128-bit floats exist on some platforms).

set(VL_PRIMITIVE_INTEGER_TYPES char short int long "long long")
set(VL_PRIMITIVE_INTEGER_TYPES_NAMES CHAR SHORT INT LONG LONG_LONG)

set(VL_ITER 0)
set(VL_INT_STATUS OFF)
foreach (VL_TYPE_ITER ${VL_PRIMITIVE_INTEGER_TYPES})
    list(GET VL_PRIMITIVE_INTEGER_TYPES_NAMES ${VL_ITER} VL_CURRENT_NAME)
    check_type_size("${VL_TYPE_ITER}" VL_SIZEOF_${VL_CURRENT_NAME} BUILTIN_TYPES_ONLY LANGUAGE C)
    MATH(EXPR VL_ITER "${VL_ITER} + 1")
endforeach ()

macro(vl_define_int_primitive size)
    set(VL_ITER 0)
    set(VL_INT_STATUS OFF)
    MATH(EXPR VL_INT_BITS "${size} * 8")
    foreach (VL_TYPE_ITER ${VL_PRIMITIVE_INTEGER_TYPES})
        list(GET VL_PRIMITIVE_INTEGER_TYPES_NAMES ${VL_ITER} VL_CURRENT_NAME)
        if(${VL_SIZEOF_${VL_CURRENT_NAME}} EQUAL ${size})
            set(VL_I${VL_INT_BITS}_T "${VL_TYPE_ITER}")
            set(VL_U${VL_INT_BITS}_T "unsigned ${VL_TYPE_ITER}")
            set(VL_INT_STATUS ON)
            break ()
        endif ()
        MATH(EXPR VL_ITER "${VL_ITER} + 1")
    endforeach ()

    if(NOT ${VL_INT_STATUS})
        message(WARNING "Failed to find integer type on target platform for ${VL_INT_BITS}-bit width.")
    endif ()
endmacro()

# Try to define 1, 2, 4, and 8 byte wide integer types.
vl_define_int_primitive(1)
vl_define_int_primitive(2)
vl_define_int_primitive(4)
vl_define_int_primitive(8)

# Define VL_INT and VL_UINT, starting with 32-bit types and working down.
if(DEFINED VL_I32_T)
    set(VL_INT_T  "${VL_I32_T}")
    set(VL_UINT_T "${VL_U32_T}")
    set(VL_SIZEOF_STANDARD_INT 4)
elseif (DEFINED VL_I16_T)
    set(VL_INT_T  "${VL_I16_T}")
    set(VL_UINT_T "${VL_U16_T}")
    set(VL_SIZEOF_STANDARD_INT 2)
else ()
    set(VL_INT  "${VL_I8_T}")
    set(VL_UINT "${VL_U8_T}")
    set(VL_SIZEOF_STANDARD_INT 1)
    message(WARNING "VL_INT resolved to 8-bit integers!")
endif ()

if(DEFINED VL_I8_T)
    set(VL_ISMALL_T "${VL_I8_T}")
    set(VL_USMALL_T "${VL_U8_T}")
elseif(DEFINED VL_I16_T)
    set(VL_ISMALL_T "${VL_I16_T}")
    set(VL_USMALL_T "${VL_U16_T}")
else()
    message(FATAL_ERROR "Failed to determine smallest integer types.")
endif ()

if(DEFINED VL_I64_T)
    set(VL_ILARGE_T "${VL_I64_T}")
    set(VL_ULARGE_T "${VL_U64_T}")
elseif(DEFINED VL_I32_T)
    set(VL_ILARGE_T "${VL_I32_T}")
    set(VL_ULARGE_T "${VL_U32_T}")
else ()
    message(FATAL_ERROR "Failed to determine largest integer types.")
endif ()

set(VL_IPTR_T "${VL_ILARGE_T}")
set(VL_UPTR_T "${VL_ULARGE_T}")

# Now define floating point primitives.
set(VL_PRIMITIVE_FLOAT_TYPES float double "long double")

check_type_size("float"             VL_SIZEOF_FLOAT         BUILTIN_TYPES_ONLY LANGUAGE C)
check_type_size("double"            VL_SIZEOF_DOUBLE        BUILTIN_TYPES_ONLY LANGUAGE C)
check_type_size("long double"       VL_SIZEOF_LONG_DOUBLE   BUILTIN_TYPES_ONLY LANGUAGE C)

if(${VL_SIZEOF_FLOAT} EQUAL 4)
    set(VL_F32_T "float")
elseif (${VL_SIZEOF_DOUBLE} EQUAL 4)
    set(VL_F32_T "double")
else ()
    message(FATAL_ERROR "Failed to find a 32-bit floating point type.")
endif ()

if (${VL_SIZEOF_DOUBLE} EQUAL 8)
    set(VL_F64_T "double")
elseif (${VL_SIZEOF_LONG_DOUBLE} EQUAL 8)
    set(VL_F64_T "long double")
else ()
    message(WARNING "Failed to find 64-bit floating point type.")
endif ()

if(${VL_SIZEOF_LONG_DOUBLE} GREATER 8)
    set(VL_FHIGHP_T "long double")
    # Some modern Intel platforms support 16-byte (128-bit) floating point types, offering even more precision.
    message(STATUS "Found high-precision floating point type spanning ${VL_SIZEOF_LONG_DOUBLE} bytes for VL_FHIGHP_T.")
elseif (DEFINED VL_F64_T)
    set(VL_FHIGHP_T "double")
elseif (DEFINED VL_F32_T)
    set(VL_FHIGHP_T "float")
endif ()

set(VL_STRUCTURE_OFFSET_T       "${VL_ULARGE_T}")
set(VL_STRUCTURE_INDEX_T        "${VL_UINT_T}")
set(VL_MEMORY_T                 "${VL_USMALL_T}")
set(VL_MEMORY_SIZE_T            "${VL_ULARGE_T}")

string(REPEAT "FF" ${VL_SIZEOF_STANDARD_INT} VL_STRUCTURE_INDEX_MAX)
set(VL_STRUCTURE_INDEX_MAX "0x${VL_STRUCTURE_INDEX_MAX}")