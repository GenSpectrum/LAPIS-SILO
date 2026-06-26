########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(xz_utils_COMPONENT_NAMES "")
if(DEFINED xz_utils_FIND_DEPENDENCY_NAMES)
  list(APPEND xz_utils_FIND_DEPENDENCY_NAMES )
  list(REMOVE_DUPLICATES xz_utils_FIND_DEPENDENCY_NAMES)
else()
  set(xz_utils_FIND_DEPENDENCY_NAMES )
endif()

########### VARIABLES #######################################################################
#############################################################################################
set(xz_utils_PACKAGE_FOLDER_RELEASE "/root/.conan2/p/b/xz_ut0f2cde3b1ea01/p")
set(xz_utils_BUILD_MODULES_PATHS_RELEASE "${xz_utils_PACKAGE_FOLDER_RELEASE}/lib/cmake/conan-official-xz_utils-variables.cmake")


set(xz_utils_INCLUDE_DIRS_RELEASE )
set(xz_utils_RES_DIRS_RELEASE )
set(xz_utils_DEFINITIONS_RELEASE "-DLZMA_API_STATIC")
set(xz_utils_SHARED_LINK_FLAGS_RELEASE )
set(xz_utils_EXE_LINK_FLAGS_RELEASE )
set(xz_utils_OBJECTS_RELEASE )
set(xz_utils_COMPILE_DEFINITIONS_RELEASE "LZMA_API_STATIC")
set(xz_utils_COMPILE_OPTIONS_C_RELEASE )
set(xz_utils_COMPILE_OPTIONS_CXX_RELEASE )
set(xz_utils_LIB_DIRS_RELEASE "${xz_utils_PACKAGE_FOLDER_RELEASE}/lib")
set(xz_utils_BIN_DIRS_RELEASE )
set(xz_utils_LIBRARY_TYPE_RELEASE STATIC)
set(xz_utils_IS_HOST_WINDOWS_RELEASE 0)
set(xz_utils_LIBS_RELEASE lzma)
set(xz_utils_SYSTEM_LIBS_RELEASE pthread)
set(xz_utils_FRAMEWORK_DIRS_RELEASE )
set(xz_utils_FRAMEWORKS_RELEASE )
set(xz_utils_BUILD_DIRS_RELEASE )
set(xz_utils_NO_SONAME_MODE_RELEASE FALSE)


# COMPOUND VARIABLES
set(xz_utils_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xz_utils_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xz_utils_COMPILE_OPTIONS_C_RELEASE}>")
set(xz_utils_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xz_utils_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xz_utils_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xz_utils_EXE_LINK_FLAGS_RELEASE}>")


set(xz_utils_COMPONENTS_RELEASE )