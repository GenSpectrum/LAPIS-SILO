########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

list(APPEND zstd_COMPONENT_NAMES zstd::libzstd_static)
list(REMOVE_DUPLICATES zstd_COMPONENT_NAMES)
if(DEFINED zstd_FIND_DEPENDENCY_NAMES)
  list(APPEND zstd_FIND_DEPENDENCY_NAMES )
  list(REMOVE_DUPLICATES zstd_FIND_DEPENDENCY_NAMES)
else()
  set(zstd_FIND_DEPENDENCY_NAMES )
endif()

########### VARIABLES #######################################################################
#############################################################################################
set(zstd_PACKAGE_FOLDER_RELEASE "/root/.conan2/p/b/zstd17a71dd8bb012/p")
set(zstd_BUILD_MODULES_PATHS_RELEASE )


set(zstd_INCLUDE_DIRS_RELEASE "${zstd_PACKAGE_FOLDER_RELEASE}/include")
set(zstd_RES_DIRS_RELEASE )
set(zstd_DEFINITIONS_RELEASE )
set(zstd_SHARED_LINK_FLAGS_RELEASE )
set(zstd_EXE_LINK_FLAGS_RELEASE )
set(zstd_OBJECTS_RELEASE )
set(zstd_COMPILE_DEFINITIONS_RELEASE )
set(zstd_COMPILE_OPTIONS_C_RELEASE )
set(zstd_COMPILE_OPTIONS_CXX_RELEASE )
set(zstd_LIB_DIRS_RELEASE "${zstd_PACKAGE_FOLDER_RELEASE}/lib")
set(zstd_BIN_DIRS_RELEASE )
set(zstd_LIBRARY_TYPE_RELEASE STATIC)
set(zstd_IS_HOST_WINDOWS_RELEASE 0)
set(zstd_LIBS_RELEASE zstd)
set(zstd_SYSTEM_LIBS_RELEASE pthread)
set(zstd_FRAMEWORK_DIRS_RELEASE )
set(zstd_FRAMEWORKS_RELEASE )
set(zstd_BUILD_DIRS_RELEASE )
set(zstd_NO_SONAME_MODE_RELEASE FALSE)


# COMPOUND VARIABLES
set(zstd_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${zstd_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${zstd_COMPILE_OPTIONS_C_RELEASE}>")
set(zstd_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${zstd_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${zstd_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${zstd_EXE_LINK_FLAGS_RELEASE}>")


set(zstd_COMPONENTS_RELEASE zstd::libzstd_static)
########### COMPONENT zstd::libzstd_static VARIABLES ############################################

set(zstd_zstd_libzstd_static_INCLUDE_DIRS_RELEASE "${zstd_PACKAGE_FOLDER_RELEASE}/include")
set(zstd_zstd_libzstd_static_LIB_DIRS_RELEASE "${zstd_PACKAGE_FOLDER_RELEASE}/lib")
set(zstd_zstd_libzstd_static_BIN_DIRS_RELEASE )
set(zstd_zstd_libzstd_static_LIBRARY_TYPE_RELEASE STATIC)
set(zstd_zstd_libzstd_static_IS_HOST_WINDOWS_RELEASE 0)
set(zstd_zstd_libzstd_static_RES_DIRS_RELEASE )
set(zstd_zstd_libzstd_static_DEFINITIONS_RELEASE )
set(zstd_zstd_libzstd_static_OBJECTS_RELEASE )
set(zstd_zstd_libzstd_static_COMPILE_DEFINITIONS_RELEASE )
set(zstd_zstd_libzstd_static_COMPILE_OPTIONS_C_RELEASE "")
set(zstd_zstd_libzstd_static_COMPILE_OPTIONS_CXX_RELEASE "")
set(zstd_zstd_libzstd_static_LIBS_RELEASE zstd)
set(zstd_zstd_libzstd_static_SYSTEM_LIBS_RELEASE pthread)
set(zstd_zstd_libzstd_static_FRAMEWORK_DIRS_RELEASE )
set(zstd_zstd_libzstd_static_FRAMEWORKS_RELEASE )
set(zstd_zstd_libzstd_static_DEPENDENCIES_RELEASE )
set(zstd_zstd_libzstd_static_SHARED_LINK_FLAGS_RELEASE )
set(zstd_zstd_libzstd_static_EXE_LINK_FLAGS_RELEASE )
set(zstd_zstd_libzstd_static_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(zstd_zstd_libzstd_static_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${zstd_zstd_libzstd_static_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${zstd_zstd_libzstd_static_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${zstd_zstd_libzstd_static_EXE_LINK_FLAGS_RELEASE}>
)
set(zstd_zstd_libzstd_static_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${zstd_zstd_libzstd_static_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${zstd_zstd_libzstd_static_COMPILE_OPTIONS_C_RELEASE}>")