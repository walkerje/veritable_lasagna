project(veritable-lasagna-test CXX C)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 11)

include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG eff443c6ef5eb6ab598bfaae27f9427fdb4f6af7)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# These tests will compile just fine with C linkage, and can be tested "straight forward".
set(VL_TESTS "memory" "buffer" "linearpool" "hashtable" "arena" "linked_list" "set" "stack" "queue" "random" "fixedpool")

# While these tests MUST be compiled as C and executed through static linkage.
# The C test definition and C++ test executor will have the same file name, excluding extension.
# The C test definition must be in the "linked" subdirectory, while the bootstrap is in the regular test directory.
set(VL_LINKED_TESTS "atomic")

enable_testing()

include(GoogleTest)
message(STATUS "Configuring tests for...")
foreach (DEF IN LISTS VL_TESTS)
    set(VL_CURRENT_TEST vlasagna_test_${DEF})
    message(STATUS "    ${VL_CURRENT_TEST}")

    add_executable(${VL_CURRENT_TEST} "${DEF}.cpp")

    target_link_libraries(${VL_CURRENT_TEST} ${VL_STATIC_TARGET} GTest::gtest_main)
    target_include_directories(${VL_CURRENT_TEST} PRIVATE "${VL_INCLUDE}" "${VL_CONFIG_HEADER_INCLUDE}")
    gtest_discover_tests(${VL_CURRENT_TEST})
endforeach ()

foreach(DEF IN LISTS VL_LINKED_TESTS)
    set(VL_CURRENT_TEST vlasagna_test_${DEF})
    set(VL_CURRENT_TEST_LIB ${VL_CURRENT_TEST}_lib)
    message(STATUS "    ${VL_CURRENT_TEST} (Linked to ${VL_CURRENT_TEST_LIB})")

    add_library(${VL_CURRENT_TEST_LIB} STATIC "linked/${DEF}.c")
    target_include_directories(${VL_CURRENT_TEST_LIB} PRIVATE "${VL_INCLUDE}" "${VL_CONFIG_HEADER_INCLUDE}")
    target_link_libraries(${VL_CURRENT_TEST_LIB} ${VL_STATIC_TARGET})

    add_executable(${VL_CURRENT_TEST} "${DEF}.cpp")
    target_include_directories(${VL_CURRENT_TEST} PRIVATE "${VL_INCLUDE}" "${VL_CONFIG_HEADER_INCLUDE}")
    target_link_libraries(${VL_CURRENT_TEST} ${VL_STATIC_TARGET} ${VL_CURRENT_TEST_LIB} GTest::gtest_main)
    gtest_discover_tests(${VL_CURRENT_TEST})
endforeach ()

set(VL_TEST_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)
