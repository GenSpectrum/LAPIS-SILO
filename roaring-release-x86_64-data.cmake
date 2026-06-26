########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(roaring_COMPONENT_NAMES "")
if(DEFINED roaring_FIND_DEPENDENCY_NAMES)
  list(APPEND roaring_FIND_DEPENDENCY_NAMES )
  list(REMOVE_DUPLICATES roaring_FIND_DEPENDENCY_NAMES)
else()
  set(roaring_FIND_DEPENDENCY_NAMES )
endif()

########### VARIABLES #######################################################################
#############################################################################################
set(roaring_PACKAGE_FOLDER_RELEASE "/root/.conan2/p/b/roaria0d4334e77cfc/p")
set(roaring_BUILD_MODULES_PATHS_RELEASE )


set(roaring_INCLUDE_DIRS_RELEASE "${roaring_PACKAGE_FOLDER_RELEASE}/include")
set(roaring_RES_DIRS_RELEASE )
set(roaring_DEFINITIONS_RELEASE )
set(roaring_SHARED_LINK_FLAGS_RELEASE )
set(roaring_EXE_LINK_FLAGS_RELEASE )
set(roaring_OBJECTS_RELEASE )
set(roaring_COMPILE_DEFINITIONS_RELEASE )
set(roaring_COMPILE_OPTIONS_C_RELEASE )
set(roaring_COMPILE_OPTIONS_CXX_RELEASE )
set(roaring_LIB_DIRS_RELEASE "${roaring_PACKAGE_FOLDER_RELEASE}/lib")
set(roaring_BIN_DIRS_RELEASE )
set(roaring_LIBRARY_TYPE_RELEASE STATIC)
set(roaring_IS_HOST_WINDOWS_RELEASE 0)
set(roaring_LIBS_RELEASE roaring)
set(roaring_SYSTEM_LIBS_RELEASE )
set(roaring_FRAMEWORK_DIRS_RELEASE )
set(roaring_FRAMEWORKS_RELEASE )
set(roaring_BUILD_DIRS_RELEASE )
set(roaring_NO_SONAME_MODE_RELEASE FALSE)


# COMPOUND VARIABLES
set(roaring_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${roaring_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${roaring_COMPILE_OPTIONS_C_RELEASE}>")
set(roaring_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${roaring_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${roaring_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${roaring_EXE_LINK_FLAGS_RELEASE}>")


set(roaring_COMPONENTS_RELEASE )