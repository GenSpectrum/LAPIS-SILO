########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(simdutf_COMPONENT_NAMES "")
if(DEFINED simdutf_FIND_DEPENDENCY_NAMES)
  list(APPEND simdutf_FIND_DEPENDENCY_NAMES )
  list(REMOVE_DUPLICATES simdutf_FIND_DEPENDENCY_NAMES)
else()
  set(simdutf_FIND_DEPENDENCY_NAMES )
endif()

########### VARIABLES #######################################################################
#############################################################################################
set(simdutf_PACKAGE_FOLDER_RELEASE "/root/.conan2/p/b/simdu33ab6822884d0/p")
set(simdutf_BUILD_MODULES_PATHS_RELEASE )


set(simdutf_INCLUDE_DIRS_RELEASE "${simdutf_PACKAGE_FOLDER_RELEASE}/include")
set(simdutf_RES_DIRS_RELEASE )
set(simdutf_DEFINITIONS_RELEASE )
set(simdutf_SHARED_LINK_FLAGS_RELEASE )
set(simdutf_EXE_LINK_FLAGS_RELEASE )
set(simdutf_OBJECTS_RELEASE )
set(simdutf_COMPILE_DEFINITIONS_RELEASE )
set(simdutf_COMPILE_OPTIONS_C_RELEASE )
set(simdutf_COMPILE_OPTIONS_CXX_RELEASE )
set(simdutf_LIB_DIRS_RELEASE "${simdutf_PACKAGE_FOLDER_RELEASE}/lib")
set(simdutf_BIN_DIRS_RELEASE )
set(simdutf_LIBRARY_TYPE_RELEASE STATIC)
set(simdutf_IS_HOST_WINDOWS_RELEASE 0)
set(simdutf_LIBS_RELEASE simdutf)
set(simdutf_SYSTEM_LIBS_RELEASE m)
set(simdutf_FRAMEWORK_DIRS_RELEASE )
set(simdutf_FRAMEWORKS_RELEASE )
set(simdutf_BUILD_DIRS_RELEASE )
set(simdutf_NO_SONAME_MODE_RELEASE FALSE)


# COMPOUND VARIABLES
set(simdutf_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${simdutf_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${simdutf_COMPILE_OPTIONS_C_RELEASE}>")
set(simdutf_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${simdutf_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${simdutf_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${simdutf_EXE_LINK_FLAGS_RELEASE}>")


set(simdutf_COMPONENTS_RELEASE )