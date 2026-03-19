# ComponentHelper.cmake - Common utilities for VL components

#
# vl_add_component - Creates a library target for a VL component
#
#  \param COMPONENT_NAME Name of the component (e.g., "Core", "Math")
#  \param SOURCES_VAR Variable containing the source files for this component
#  \param INCLUDE_DIR Include directory for this component's public headers
#  \param OUTPUT_NAME Base name for the output library file
#
function(vl_add_component COMPONENT_NAME SOURCES_VAR INCLUDE_DIR OUTPUT_NAME)
    set(TARGET_NAME ${COMPONENT_NAME})
    string(TOLOWER ${COMPONENT_NAME} component_dir)

    # Create the library target
    add_library(${TARGET_NAME} ${${SOURCES_VAR}})

    # Configure include directories
    target_include_directories(${TARGET_NAME}
            PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${INCLUDE_DIR}/>
            $<BUILD_INTERFACE:${VL_CONFIG_HEADER_INCLUDE}>
            
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
            PRIVATE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${INCLUDE_DIR}/vl>
    )

    # Set standard library properties
    set_target_properties(${TARGET_NAME} PROPERTIES
            OUTPUT_NAME   ${OUTPUT_NAME}
            DEBUG_POSTFIX "d"
            C_STANDARD    11
            C_STANDARD_REQUIRED ON
    )

    # Apply common compile options and system libraries
    target_compile_features(${TARGET_NAME} PUBLIC c_std_11)
    target_compile_options(${TARGET_NAME} PUBLIC "${VL_COMPILE_OPTIONS}")
    target_link_libraries(${TARGET_NAME} PUBLIC "${VL_SYSTEM_LIBS}")

    # Handle shared library specifics
    if(BUILD_SHARED_LIBS)
        target_compile_definitions(${TARGET_NAME}
                PRIVATE VL_BUILDING_SHARED
                INTERFACE VL_USING_SHARED
        )

        set_target_properties(${TARGET_NAME} PROPERTIES
                C_VISIBILITY_PRESET hidden
                VISIBILITY_INLINES_HIDDEN ON
                POSITION_INDEPENDENT_CODE ON
        )
    endif()

    # Create alias for this project
    add_library(VLasagna::${COMPONENT_NAME} ALIAS ${TARGET_NAME})

    # Add to global install targets list
    set(VL_INSTALL_TARGETS ${VL_INSTALL_TARGETS} ${TARGET_NAME} PARENT_SCOPE)

    # Register component for testing
    set(VL_COMPONENTS ${VL_COMPONENTS} ${COMPONENT_NAME} PARENT_SCOPE)
endfunction()

#
# vl_install_component - Installs a component with standard configuration
#
#  \param COMPONENT_NAME Name of the component
#  \param INCLUDE_DIR Include directory to install
#
function(vl_install_component COMPONENT_NAME INCLUDE_DIR)
    # Normalize the include directory path
    get_filename_component(NORMALIZED_INCLUDE_DIR
            "${CMAKE_CURRENT_SOURCE_DIR}/${INCLUDE_DIR}"
            ABSOLUTE
    )

    # Install regular header files
    install(DIRECTORY ${NORMALIZED_INCLUDE_DIR}/
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    # Install configured headers for this component
    foreach(header_entry IN LISTS VL_CONFIGURED_HEADERS)
        string(REPLACE ":" ";" header_parts ${header_entry})
        list(GET header_parts 0 header_component)
        list(GET header_parts 1 source_path)
        list(GET header_parts 2 install_path)

        if(header_component STREQUAL ${COMPONENT_NAME})
            # Normalize the installation path
            get_filename_component(install_dir ${install_path} DIRECTORY)
            get_filename_component(normalized_install_dir
                    "${CMAKE_INSTALL_INCLUDEDIR}/${install_dir}"
                    ABSOLUTE
            )

            install(FILES ${source_path}
                    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${install_dir}
            )
        endif()
    endforeach()
endfunction()

# Add a function to install global configured headers
function(vl_install_global_configured_headers)
    foreach(header_entry IN LISTS VL_CONFIGURED_HEADERS)
        string(REPLACE ":" ";" header_parts ${header_entry})
        list(GET header_parts 0 header_component)
        list(GET header_parts 1 source_path)
        list(GET header_parts 2 install_path)

        if(header_component STREQUAL "Global")
            get_filename_component(install_dir ${install_path} DIRECTORY)
            install(FILES ${source_path}
                    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${install_dir}
            )
        endif()
    endforeach()
endfunction()

#
# vl_configure_component_tests - Configure all tests for a component
#
# This function automatically:
# - Creates test library for linked tests (if any)
# - Configures all standard and linked tests
# - Uses lowercase component name for directory structure
# - Leverages CMake's PUBLIC dependency management
#
# Usage: vl_configure_component_tests(ComponentName
#          TESTS test1 test2 test3
#          LINKED_TESTS linked_test1 linked_test2)
#
#  \param COMPONENT_NAME Name of the component being tested
#  \param TESTS List of standard test names for this component
#  \param LINKED_TESTS List of linked test names for this component (optional)
#
function(vl_configure_component_tests COMPONENT_NAME)
    # Parse arguments
    cmake_parse_arguments(VL_TEST "" "" "TESTS;LINKED_TESTS" ${ARGN})

    # Validate that we have the component name and target exists
    if(NOT COMPONENT_NAME)
        message(FATAL_ERROR "vl_configure_component_tests: COMPONENT_NAME is required")
    endif()

    if(NOT TARGET VLasagna::${COMPONENT_NAME})
        message(WARNING "vl_configure_component_tests: Component ${COMPONENT_NAME} target does not exist, skipping tests")
        return()
    endif()

    # Determine component directory (lowercase)
    string(TOLOWER ${COMPONENT_NAME} component_dir)

    message(STATUS "  ${COMPONENT_NAME} component:")

    # Build test library for linked tests if needed
    if(VL_TEST_LINKED_TESTS)
        set(VL_LINKED_TESTS_SRC)
        foreach(test_name IN LISTS VL_TEST_LINKED_TESTS)
            list(APPEND VL_LINKED_TESTS_SRC "${component_dir}/linked/${test_name}.c")
        endforeach()

        set(VL_TEST_LIB vl_${COMPONENT_NAME}_test_lib)
        add_library(${VL_TEST_LIB} ${VL_LINKED_TESTS_SRC})

        # When building shared libraries, explicitly mark test library functions as exports
        if(BUILD_SHARED_LIBS)
            target_compile_definitions(${VL_TEST_LIB}
                    PRIVATE VL_BUILDING_TEST_SHARED
                    INTERFACE VL_USING_TEST_SHARED
            )

            set_target_properties(${VL_TEST_LIB} PROPERTIES
                    C_VISIBILITY_PRESET hidden
                    VISIBILITY_INLINES_HIDDEN ON
                    POSITION_INDEPENDENT_CODE ON
            )
        endif()

        set_target_properties(${VL_TEST_LIB} PROPERTIES
                C_STANDARD 11
                C_STANDARD_REQUIRED ON
        )

        target_include_directories(${VL_TEST_LIB} PRIVATE "${VL_CONFIG_HEADER_INCLUDE}")
        target_link_libraries(${VL_TEST_LIB} PUBLIC VLasagna::${COMPONENT_NAME})
    endif()

    # Configure standard tests
    if(VL_TEST_TESTS)
        foreach(test_name IN LISTS VL_TEST_TESTS)
            vl_add_component_test_internal(${COMPONENT_NAME} ${test_name} ${component_dir})
        endforeach()
    endif()

    # Configure linked tests
    if(VL_TEST_LINKED_TESTS)
        foreach(test_name IN LISTS VL_TEST_LINKED_TESTS)
            vl_add_component_linked_test_internal(${COMPONENT_NAME} ${test_name} ${component_dir})
        endforeach()
    endif()
endfunction()

#
# vl_add_component_test_internal - Internal function to add a standard component test
#
#  \param COMPONENT_NAME Name of the component being tested
#  \param TEST_NAME Name of the test
#  \param COMPONENT_DIR Directory containing the test files
#
function(vl_add_component_test_internal COMPONENT_NAME TEST_NAME COMPONENT_DIR)
    set(target_name vl_test_${COMPONENT_NAME}_${TEST_NAME})
    string(TOLOWER ${target_name} target_name)

    set(test_source_file "${COMPONENT_DIR}/${TEST_NAME}.cpp")

    message(STATUS "    ${target_name} (${test_source_file})")

    add_executable(${target_name} "${test_source_file}")
    set_target_properties(${target_name} PROPERTIES
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED ON
    )

    if(BUILD_SHARED_LIBS)
        set_target_properties(${target_name} PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        )

        # CMake will automatically handle copying dependencies thanks to PUBLIC linkage
        vl_copy_component_dependencies(${target_name} VLasagna::${COMPONENT_NAME})
    endif()

    # Link against the component - CMake's PUBLIC dependency management handles transitive deps
    target_link_libraries(${target_name} PRIVATE VLasagna::${COMPONENT_NAME} GTest::gtest_main)
    target_include_directories(${target_name} PRIVATE "${VL_CONFIG_HEADER_INCLUDE}")
    gtest_discover_tests(${target_name} DISCOVERY_MODE PRE_TEST)

    # Apply common test configuration
    vl_apply_test_configuration(${target_name})
endfunction()

#
# vl_add_component_linked_test_internal - Internal function to add a linked component test
#
#  \param COMPONENT_NAME Name of the component being tested
#  \param TEST_NAME Name of the test
#  \param COMPONENT_DIR Directory containing the test files
#

function(vl_add_component_linked_test_internal COMPONENT_NAME TEST_NAME COMPONENT_DIR)
    set(target_name vl_test_${COMPONENT_NAME}_${TEST_NAME})
    string(TOLOWER ${target_name} target_name)

    set(test_source_file "${COMPONENT_DIR}/${TEST_NAME}.cpp")

    message(STATUS "    ${target_name} (Linked to vl_${COMPONENT_NAME}_test_lib) (${test_source_file})")

    add_executable(${target_name} "${test_source_file}")
    set_target_properties(${target_name} PROPERTIES
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED ON
    )

    add_dependencies(${target_name}
            vl_${COMPONENT_NAME}_test_lib
            VLasagna::${COMPONENT_NAME})


    target_link_libraries(${target_name} PRIVATE
            vl_${COMPONENT_NAME}_test_lib
            VLasagna::${COMPONENT_NAME}
            GTest::gtest_main
    )

    if(BUILD_SHARED_LIBS)
        set_target_properties(${target_name} PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        )

        # Copy dependencies of test library as well as the tested component.
        vl_copy_component_dependencies(${target_name} vl_${COMPONENT_NAME}_test_lib)
        vl_copy_component_dependencies(${target_name} VLasagna::${COMPONENT_NAME})
    endif()

    target_include_directories(${target_name} PRIVATE "${VL_CONFIG_HEADER_INCLUDE}")
    gtest_discover_tests(${target_name} DISCOVERY_MODE PRE_TEST)

    vl_apply_test_configuration(${target_name})
endfunction()

#
# vl_copy_component_dependencies - Copy shared library dependencies for a test target
#
#  \param TARGET_NAME Name of the test target
#  \param COMPONENT_TARGET Name of the component target (e.g., VLasagna::Core)
#
function(vl_copy_component_dependencies TARGET_NAME COMPONENT_TARGET)
    # Get all dependencies of the component (transitively)
    get_target_property(linked_libs ${COMPONENT_TARGET} INTERFACE_LINK_LIBRARIES)

    # Copy the main component library
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:${COMPONENT_TARGET}>
            $<TARGET_FILE_DIR:${TARGET_NAME}>
    )

    # Copy any VLasagna component dependencies
    if(linked_libs)
        foreach(lib ${linked_libs})
            if(lib MATCHES "^VLasagna::")
                if(TARGET ${lib})
                    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                            $<TARGET_FILE:${lib}>
                            $<TARGET_FILE_DIR:${TARGET_NAME}>
                    )
                endif()
            endif()
        endforeach()
    endif()
endfunction()

#
# vl_apply_test_configuration - Apply common test configuration (dependencies, sanitizers, coverage)
#
#  \param TARGET_NAME Name of the test target
#
function(vl_apply_test_configuration TARGET_NAME)
    add_dependencies(Tests ${TARGET_NAME})

    if(VL_COVERAGE)
        add_dependencies(coverage_report ${TARGET_NAME})
    endif()

    if(DEFINED VL_SANITIZER_FLAGS)
        target_compile_options(${TARGET_NAME} PRIVATE ${VL_SANITIZER_FLAGS})
        target_link_options(${TARGET_NAME} PRIVATE ${VL_SANITIZER_FLAGS})
    endif()
endfunction()
