########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

list(APPEND fast_float_COMPONENT_NAMES FastFloat::fast_float)
list(REMOVE_DUPLICATES fast_float_COMPONENT_NAMES)
if(DEFINED fast_float_FIND_DEPENDENCY_NAMES)
  list(APPEND fast_float_FIND_DEPENDENCY_NAMES )
  list(REMOVE_DUPLICATES fast_float_FIND_DEPENDENCY_NAMES)
else()
  set(fast_float_FIND_DEPENDENCY_NAMES )
endif()

########### VARIABLES #######################################################################
#############################################################################################
set(fast_float_PACKAGE_FOLDER_RELEASE "/root/.conan2/p/fast_7004c9a1e0b13/p")
set(fast_float_BUILD_MODULES_PATHS_RELEASE )


set(fast_float_INCLUDE_DIRS_RELEASE "${fast_float_PACKAGE_FOLDER_RELEASE}/include")
set(fast_float_RES_DIRS_RELEASE )
set(fast_float_DEFINITIONS_RELEASE )
set(fast_float_SHARED_LINK_FLAGS_RELEASE )
set(fast_float_EXE_LINK_FLAGS_RELEASE )
set(fast_float_OBJECTS_RELEASE )
set(fast_float_COMPILE_DEFINITIONS_RELEASE )
set(fast_float_COMPILE_OPTIONS_C_RELEASE )
set(fast_float_COMPILE_OPTIONS_CXX_RELEASE )
set(fast_float_LIB_DIRS_RELEASE )
set(fast_float_BIN_DIRS_RELEASE )
set(fast_float_LIBRARY_TYPE_RELEASE UNKNOWN)
set(fast_float_IS_HOST_WINDOWS_RELEASE 0)
set(fast_float_LIBS_RELEASE )
set(fast_float_SYSTEM_LIBS_RELEASE )
set(fast_float_FRAMEWORK_DIRS_RELEASE )
set(fast_float_FRAMEWORKS_RELEASE )
set(fast_float_BUILD_DIRS_RELEASE )
set(fast_float_NO_SONAME_MODE_RELEASE FALSE)


# COMPOUND VARIABLES
set(fast_float_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${fast_float_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${fast_float_COMPILE_OPTIONS_C_RELEASE}>")
set(fast_float_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${fast_float_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${fast_float_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${fast_float_EXE_LINK_FLAGS_RELEASE}>")


set(fast_float_COMPONENTS_RELEASE FastFloat::fast_float)
########### COMPONENT FastFloat::fast_float VARIABLES ############################################

set(fast_float_FastFloat_fast_float_INCLUDE_DIRS_RELEASE "${fast_float_PACKAGE_FOLDER_RELEASE}/include")
set(fast_float_FastFloat_fast_float_LIB_DIRS_RELEASE )
set(fast_float_FastFloat_fast_float_BIN_DIRS_RELEASE )
set(fast_float_FastFloat_fast_float_LIBRARY_TYPE_RELEASE UNKNOWN)
set(fast_float_FastFloat_fast_float_IS_HOST_WINDOWS_RELEASE 0)
set(fast_float_FastFloat_fast_float_RES_DIRS_RELEASE )
set(fast_float_FastFloat_fast_float_DEFINITIONS_RELEASE )
set(fast_float_FastFloat_fast_float_OBJECTS_RELEASE )
set(fast_float_FastFloat_fast_float_COMPILE_DEFINITIONS_RELEASE )
set(fast_float_FastFloat_fast_float_COMPILE_OPTIONS_C_RELEASE "")
set(fast_float_FastFloat_fast_float_COMPILE_OPTIONS_CXX_RELEASE "")
set(fast_float_FastFloat_fast_float_LIBS_RELEASE )
set(fast_float_FastFloat_fast_float_SYSTEM_LIBS_RELEASE )
set(fast_float_FastFloat_fast_float_FRAMEWORK_DIRS_RELEASE )
set(fast_float_FastFloat_fast_float_FRAMEWORKS_RELEASE )
set(fast_float_FastFloat_fast_float_DEPENDENCIES_RELEASE )
set(fast_float_FastFloat_fast_float_SHARED_LINK_FLAGS_RELEASE )
set(fast_float_FastFloat_fast_float_EXE_LINK_FLAGS_RELEASE )
set(fast_float_FastFloat_fast_float_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(fast_float_FastFloat_fast_float_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${fast_float_FastFloat_fast_float_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${fast_float_FastFloat_fast_float_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${fast_float_FastFloat_fast_float_EXE_LINK_FLAGS_RELEASE}>
)
set(fast_float_FastFloat_fast_float_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${fast_float_FastFloat_fast_float_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${fast_float_FastFloat_fast_float_COMPILE_OPTIONS_C_RELEASE}>")