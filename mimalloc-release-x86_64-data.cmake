########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(mimalloc_COMPONENT_NAMES "")
if(DEFINED mimalloc_FIND_DEPENDENCY_NAMES)
  list(APPEND mimalloc_FIND_DEPENDENCY_NAMES )
  list(REMOVE_DUPLICATES mimalloc_FIND_DEPENDENCY_NAMES)
else()
  set(mimalloc_FIND_DEPENDENCY_NAMES )
endif()

########### VARIABLES #######################################################################
#############################################################################################
set(mimalloc_PACKAGE_FOLDER_RELEASE "/root/.conan2/p/b/mimald125d43a8ab32/p")
set(mimalloc_BUILD_MODULES_PATHS_RELEASE )


set(mimalloc_INCLUDE_DIRS_RELEASE "${mimalloc_PACKAGE_FOLDER_RELEASE}/include")
set(mimalloc_RES_DIRS_RELEASE )
set(mimalloc_DEFINITIONS_RELEASE )
set(mimalloc_SHARED_LINK_FLAGS_RELEASE )
set(mimalloc_EXE_LINK_FLAGS_RELEASE )
set(mimalloc_OBJECTS_RELEASE )
set(mimalloc_COMPILE_DEFINITIONS_RELEASE )
set(mimalloc_COMPILE_OPTIONS_C_RELEASE )
set(mimalloc_COMPILE_OPTIONS_CXX_RELEASE )
set(mimalloc_LIB_DIRS_RELEASE "${mimalloc_PACKAGE_FOLDER_RELEASE}/lib")
set(mimalloc_BIN_DIRS_RELEASE )
set(mimalloc_LIBRARY_TYPE_RELEASE STATIC)
set(mimalloc_IS_HOST_WINDOWS_RELEASE 0)
set(mimalloc_LIBS_RELEASE mimalloc)
set(mimalloc_SYSTEM_LIBS_RELEASE pthread rt)
set(mimalloc_FRAMEWORK_DIRS_RELEASE )
set(mimalloc_FRAMEWORKS_RELEASE )
set(mimalloc_BUILD_DIRS_RELEASE )
set(mimalloc_NO_SONAME_MODE_RELEASE FALSE)


# COMPOUND VARIABLES
set(mimalloc_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${mimalloc_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${mimalloc_COMPILE_OPTIONS_C_RELEASE}>")
set(mimalloc_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${mimalloc_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${mimalloc_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${mimalloc_EXE_LINK_FLAGS_RELEASE}>")


set(mimalloc_COMPONENTS_RELEASE )