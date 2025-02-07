project(veritable-lasagna-test CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG eff443c6ef5eb6ab598bfaae27f9427fdb4f6af7)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(VL_TESTS "memory" "buffer" "linearpool" "hashtable" "arena" "linked_list" "set" "stack" "queue" "random" "fixedpool")

if(${VL_SHARED_TESTS})
    set(VL_TEST_LIB ${VL_LIBRARY_SHARED})
else ()
    set(VL_TEST_LIB ${VL_LIBRARY})
endif ()

enable_testing()

include(GoogleTest)
message(STATUS "Configuring tests for...")
foreach (DEF IN LISTS VL_TESTS)
    set(VL_CURRENT_TEST vlasagna_test_${DEF})
    message(STATUS "    ${VL_CURRENT_TEST}")

    add_executable(${VL_CURRENT_TEST} "${DEF}.cpp")

    target_link_libraries(${VL_CURRENT_TEST} ${VL_TEST_LIB} GTest::gtest_main)
    target_include_directories(${VL_CURRENT_TEST} PRIVATE "${VL_INCLUDE}" "${VL_CONFIG_HEADER_INCLUDE}")
    gtest_discover_tests(${VL_CURRENT_TEST})
endforeach ()

set(VL_TEST_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)
