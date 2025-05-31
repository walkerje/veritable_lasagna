project(veritable-lasagna-test CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG eff443c6ef5eb6ab598bfaae27f9427fdb4f6af7)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# These tests will compile just fine as C++ with C linkage, and can be tested "straight forward".
set(VL_TESTS "memory" "buffer" "hashtable" "arena" "linked_list" "set" "stack" "queue" "random" "pool" "msgpack")

# While these tests MUST be compiled as C and executed through linkage.
# The C test definition and C++ test executor will have the same file name, excluding extension.
# The C test definition must be in the "linked" subdirectory, while the bootstrap is in the regular test directory.
set(VL_LINKED_TESTS "atomic" "async_pool" "async_queue")

enable_testing()

include(GoogleTest)
message(STATUS "Configuring tests for...")
foreach (DEF IN LISTS VL_TESTS)
    set(VL_CURRENT_TEST vl_test_${DEF})
    message(STATUS "    ${VL_CURRENT_TEST}")

    add_executable(${VL_CURRENT_TEST} "${DEF}.cpp")

    if(BUILD_SHARED_LIBS)
        set_target_properties(${VL_CURRENT_TEST} PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        )

        add_custom_command(TARGET ${VL_CURRENT_TEST} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:${VL_CORE_TARGET}>
                $<TARGET_FILE_DIR:${VL_CURRENT_TEST}>
        )
    endif()


    target_link_libraries(${VL_CURRENT_TEST} ${VL_CORE_TARGET} GTest::gtest_main)
    target_include_directories(${VL_CURRENT_TEST} PRIVATE "${VL_INCLUDE}" "${VL_CONFIG_HEADER_INCLUDE}")
    gtest_discover_tests(${VL_CURRENT_TEST})
endforeach ()

foreach(DEF IN LISTS VL_LINKED_TESTS)
    set(VL_LINKED_TESTS_SRC ${VL_LINKED_TESTS_SRC} "linked/${DEF}.c")
endforeach ()

set(VL_TEST_LIB vl_test_lib)

add_library(${VL_TEST_LIB} ${VL_LINKED_TESTS_SRC})
target_include_directories(${VL_TEST_LIB} PRIVATE "${VL_INCLUDE}" "${VL_CONFIG_HEADER_INCLUDE}")
target_link_libraries(${VL_TEST_LIB} ${VL_CORE_TARGET})

foreach(DEF IN LISTS VL_LINKED_TESTS)
    set(VL_CURRENT_TEST vl_test_${DEF})
    message(STATUS "    ${VL_CURRENT_TEST} (Linked to ${VL_TEST_LIB})")

    add_executable(${VL_CURRENT_TEST} "${DEF}.cpp")

    if(BUILD_SHARED_LIBS)
        set_target_properties(${VL_CURRENT_TEST} PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        )

        add_custom_command(TARGET ${VL_CURRENT_TEST} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:${VL_CORE_TARGET}>
                $<TARGET_FILE_DIR:${VL_CURRENT_TEST}>
        )
    endif()

    target_include_directories(${VL_CURRENT_TEST} PRIVATE "${VL_INCLUDE}" "${VL_CONFIG_HEADER_INCLUDE}")
    target_link_libraries(${VL_CURRENT_TEST} PRIVATE VLasagna::Core ${VL_TEST_LIB} GTest::gtest_main)
    gtest_discover_tests(${VL_CURRENT_TEST})
endforeach ()