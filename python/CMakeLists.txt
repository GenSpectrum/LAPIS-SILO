find_package(Python3 REQUIRED COMPONENTS Interpreter Development.Module)

# Find Cython
execute_process(
        COMMAND "${Python3_EXECUTABLE}" -c "import Cython; print(Cython.__version__)"
        OUTPUT_VARIABLE CYTHON_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE CYTHON_NOT_FOUND
)

if(CYTHON_NOT_FOUND)
    message(FATAL_ERROR "Cython not found. Install with: pip install Cython")
endif()

message(STATUS "Found Cython version: ${CYTHON_VERSION}")

# Cython modules to build
set(CYTHON_MODULES database)

# Build each Cython module
foreach(module ${CYTHON_MODULES})
    set(pyx_file "${CMAKE_SOURCE_DIR}/python/silodb/${module}.pyx")
    set(cpp_file "${CMAKE_BINARY_DIR}/python/silodb/${module}.cpp")

    # Run Cython to generate C++ file
    add_custom_command(
            OUTPUT ${cpp_file}
            COMMAND ${Python3_EXECUTABLE} -m cython
            --cplus
            -3
            -I "${CMAKE_SOURCE_DIR}/python/silodb"
            -o ${cpp_file}
            ${pyx_file}
            DEPENDS ${pyx_file}
            COMMENT "Cythonizing ${module}.pyx"
    )

    # Create Python extension module
    Python3_add_library(${module} MODULE ${cpp_file})

    # Set include directories
    target_include_directories(${module} PRIVATE
            ${CMAKE_SOURCE_DIR}/src
            ${CMAKE_SOURCE_DIR}/python/silodb
    )

    # Link against the portable core directly: the bindings only touch
    # silo::Database (preprocessing, append, query, save/load), all of which
    # live in silolib. They need none of the Poco HTTP API layer, so linking
    # silolib keeps Poco out of the Python extension. Linking the OBJECT
    # library directly pulls in its objects and usage requirements.
    target_link_libraries(${module} PRIVATE silolib)

    # Set output name and location
    set_target_properties(${module} PROPERTIES
            PREFIX ""
            OUTPUT_NAME "${module}"
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/python/silodb"
    )

    # Install to Python package directory
    # Note: CMAKE_INSTALL_PREFIX is set by setup.py to point to the silodb directory
    install(TARGETS ${module}
            LIBRARY DESTINATION .
    )
endforeach()

# Install Python package files
install(FILES "${CMAKE_SOURCE_DIR}/python/silodb/__init__.py"
        DESTINATION .
)
