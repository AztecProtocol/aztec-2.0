# copyright 2019 Spilsbury Holdings
#
# usage: barretenberg_module(module_name [dependencies ...])
#
# Scans for all .cpp files in a subdirectory, and creates a library named <module_name>.
# Scans for all .test.cpp files in a subdirectory, and creates a gtest binary named <module name>_tests.
# Scans for all .bench.cpp files in a subdirectory, and creates a benchmark binary named <module name>_bench.
#
# We have to get a bit complicated here, due to the fact CMake will not parallelise the building of object files
# between dependent targets, due to the potential of post-build code generation steps etc.
# To work around this, we create "object libraries" containing the object files.
# Then we declare executables/libraries that are to be built from these object files.
# These assets will only be linked as their dependencies complete, but we can parallelise the compilation at least.

function(barretenberg_module MODULE_NAME)
    file(GLOB_RECURSE SOURCE_FILES *.cpp)
    file(GLOB_RECURSE HEADER_FILES *.hpp)
    list(FILTER SOURCE_FILES EXCLUDE REGEX ".*\.(test).cpp$")

    if(SOURCE_FILES)
        add_library(
            ${MODULE_NAME}_objects
            OBJECT
            ${SOURCE_FILES}
        )

        add_library(
            ${MODULE_NAME}
            STATIC
            $<TARGET_OBJECTS:${MODULE_NAME}_objects>
        )

        target_link_libraries(
            ${MODULE_NAME}
            PUBLIC
            ${ARGN}
        )

        set(MODULE_LINK_NAME ${MODULE_NAME})
    endif()


endfunction()