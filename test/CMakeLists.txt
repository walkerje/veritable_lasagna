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
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

FetchContent_MakeAvailable(googletest)

enable_testing()
include(GoogleTest)

add_custom_target(Tests
        COMMAND ${CMAKE_CTEST_COMMAND}
        --output-on-failure
        --test-dir "${CMAKE_BINARY_DIR}/test"
        -C $<CONFIG>
        USES_TERMINAL
        COMMENT "Running all tests via ctest"
)

# Configure sanitizers if enabled
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    if(VL_ENABLE_ASAN)
        message(STATUS "Enabling AddressSanitizer")
        set(VL_SANITIZER_FLAGS "${VL_SANITIZER_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
    endif()

    if(VL_ENABLE_UBSAN)
        message(STATUS "Enabling UndefinedBehaviorSanitizer")
        set(VL_SANITIZER_FLAGS "${VL_SANITIZER_FLAGS} -fsanitize=undefined")
    endif()

    if(VL_ENABLE_TSAN)
        message(STATUS "Enabling ThreadSanitizer")
        set(VL_SANITIZER_FLAGS "${VL_SANITIZER_FLAGS} -fsanitize=thread")
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

# -------------------------------------------------------------------------------
# Component Test Configuration
# -------------------------------------------------------------------------------

message(STATUS "Configuring tests for...")

# Configure Core component tests
vl_configure_component_tests(Core
#        TESTS

        LINKED_TESTS
        "socket" "atomic" "async_pool" "async_queue"
        "log" "memory" "algo" "linked_list"
        "hashtable" "buffer" "arena" "set"
        "stack" "queue" "random" "pool"
        "msgpack" "filesys"
)