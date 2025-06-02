project(veritable-lasagna-test CXX C)

# Test build options
option(VL_ENABLE_COVERAGE "Enable code coverage reporting" OFF)
option(VL_ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(VL_ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
option(VL_ENABLE_TSAN "Enable ThreadSanitizer" OFF)

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

# Configure sanitizers if enabled
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    if(VL_ENABLE_ASAN)
        message(STATUS "Enabling AddressSanitizer")
        set(VL_SANITIZER_FLAGS "${SANITIZER_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
    endif()

    if(VL_ENABLE_UBSAN)
        message(STATUS "Enabling UndefinedBehaviorSanitizer")
        set(VL_SANITIZER_FLAGS "${SANITIZER_FLAGS} -fsanitize=undefined")
    endif()

    if(VL_ENABLE_TSAN)
        message(STATUS "Enabling ThreadSanitizer")
        set(VL_SANITIZER_FLAGS "${SANITIZER_FLAGS} -fsanitize=thread")
    endif()
endif()

# Configure code coverage if enabled
if(VL_ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    message(STATUS "Enabling code coverage reporting")
    set(VL_COVERAGE ON)
    set(COVERAGE_COMPILER_FLAGS "-g -O0 --coverage -fprofile-arcs -ftest-coverage")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COVERAGE_COMPILER_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COVERAGE_COMPILER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")

    # Add custom target for generating coverage report
    find_program(GCOVR_PATH gcovr REQUIRED)
    add_custom_target(coverage
        COMMAND ${GCOVR_PATH} --xml-pretty --exclude-unreachable-branches --print-summary
                --root ${CMAKE_SOURCE_DIR} --output=${CMAKE_BINARY_DIR}/coverage.xml
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating code coverage report"
    )

    # Add target that depends on running all tests before generating coverage
    add_custom_target(coverage_report
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target coverage
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/test
        COMMENT "Running tests and generating coverage report"
    )
endif()

# Function to configure a standard C++ test
function(add_vl_test test_name)
    set(target_name vl_test_${test_name})
    message(STATUS "    ${target_name}")

    add_executable(${target_name} "${test_name}.cpp")
    set_target_properties(${target_name} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )

    if(BUILD_SHARED_LIBS)
        set_target_properties(${target_name} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        )

        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:${VL_CORE_TARGET}>
            $<TARGET_FILE_DIR:${target_name}>
        )
    endif()

    target_link_libraries(${target_name} PRIVATE VLasagna::Core GTest::gtest_main)
    target_include_directories(${target_name} PRIVATE "${VL_INCLUDE}" "${VL_CONFIG_HEADER_INCLUDE}")
    gtest_discover_tests(${target_name})

    if(VL_COVERAGE)
        add_dependencies(coverage_report ${target_name})
    endif ()

    if(DEFINED VL_SANITIZER_FLAGS)
        target_compile_options(${target_name} PRIVATE ${VL_SANITIZER_FLAGS})
        target_link_options(${target_name} PRIVATE ${VL_SANITIZER_FLAGS})
    endif ()
endfunction()

# Function to configure a test with C implementation and C++ test harness
function(add_vl_linked_test test_name)
    set(target_name vl_test_${test_name})
    message(STATUS "    ${target_name} (Linked to ${VL_TEST_LIB})")

    add_executable(${target_name} "${test_name}.cpp")
    set_target_properties(${target_name} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )

    if(BUILD_SHARED_LIBS)
        set_target_properties(${target_name} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        )

        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:${VL_CORE_TARGET}>
            $<TARGET_FILE_DIR:${target_name}>
        )
    endif()

    target_include_directories(${target_name} PRIVATE "${VL_INCLUDE}" "${VL_CONFIG_HEADER_INCLUDE}")
    target_link_libraries(${target_name} PRIVATE VLasagna::Core ${VL_TEST_LIB} GTest::gtest_main)
    gtest_discover_tests(${target_name})

    if(VL_COVERAGE)
        add_dependencies(coverage_report ${target_name})
    endif ()

    if(DEFINED VL_SANITIZER_FLAGS)
        target_compile_options(${target_name} PRIVATE ${VL_SANITIZER_FLAGS})
        target_link_options(${target_name} PRIVATE ${VL_SANITIZER_FLAGS})
    endif ()
endfunction()

# Create the C test library
set(VL_LINKED_TESTS_SRC)
foreach(test_name IN LISTS VL_LINKED_TESTS)
    list(APPEND VL_LINKED_TESTS_SRC "linked/${test_name}.c")
endforeach()

set(VL_TEST_LIB vl_test_lib)

add_library(${VL_TEST_LIB} STATIC ${VL_LINKED_TESTS_SRC})
set_target_properties(${VL_TEST_LIB} PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED ON
)
target_include_directories(${VL_TEST_LIB} PRIVATE "${VL_INCLUDE}" "${VL_CONFIG_HEADER_INCLUDE}")
target_link_libraries(${VL_TEST_LIB} PRIVATE VLasagna::Core)

message(STATUS "Configuring tests for...")
foreach(test_name IN LISTS VL_TESTS)
    add_vl_test(${test_name})
endforeach()

foreach(test_name IN LISTS VL_LINKED_TESTS)
    add_vl_linked_test(${test_name})
endforeach()
