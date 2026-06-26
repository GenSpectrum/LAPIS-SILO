########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(utf8proc_COMPONENT_NAMES "")
if(DEFINED utf8proc_FIND_DEPENDENCY_NAMES)
  list(APPEND utf8proc_FIND_DEPENDENCY_NAMES )
  list(REMOVE_DUPLICATES utf8proc_FIND_DEPENDENCY_NAMES)
else()
  set(utf8proc_FIND_DEPENDENCY_NAMES )
endif()

########### VARIABLES #######################################################################
#############################################################################################
set(utf8proc_PACKAGE_FOLDER_RELEASE "/root/.conan2/p/b/utf8p39a1a7dcb4be3/p")
set(utf8proc_BUILD_MODULES_PATHS_RELEASE )


set(utf8proc_INCLUDE_DIRS_RELEASE )
set(utf8proc_RES_DIRS_RELEASE )
set(utf8proc_DEFINITIONS_RELEASE "-DUTF8PROC_STATIC")
set(utf8proc_SHARED_LINK_FLAGS_RELEASE )
set(utf8proc_EXE_LINK_FLAGS_RELEASE )
set(utf8proc_OBJECTS_RELEASE )
set(utf8proc_COMPILE_DEFINITIONS_RELEASE "UTF8PROC_STATIC")
set(utf8proc_COMPILE_OPTIONS_C_RELEASE )
set(utf8proc_COMPILE_OPTIONS_CXX_RELEASE )
set(utf8proc_LIB_DIRS_RELEASE "${utf8proc_PACKAGE_FOLDER_RELEASE}/lib")
set(utf8proc_BIN_DIRS_RELEASE )
set(utf8proc_LIBRARY_TYPE_RELEASE STATIC)
set(utf8proc_IS_HOST_WINDOWS_RELEASE 0)
set(utf8proc_LIBS_RELEASE utf8proc)
set(utf8proc_SYSTEM_LIBS_RELEASE )
set(utf8proc_FRAMEWORK_DIRS_RELEASE )
set(utf8proc_FRAMEWORKS_RELEASE )
set(utf8proc_BUILD_DIRS_RELEASE )
set(utf8proc_NO_SONAME_MODE_RELEASE FALSE)


# COMPOUND VARIABLES
set(utf8proc_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${utf8proc_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${utf8proc_COMPILE_OPTIONS_C_RELEASE}>")
set(utf8proc_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${utf8proc_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${utf8proc_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${utf8proc_EXE_LINK_FLAGS_RELEASE}>")


set(utf8proc_COMPONENTS_RELEASE )