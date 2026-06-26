########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(simdjson_COMPONENT_NAMES "")
if(DEFINED simdjson_FIND_DEPENDENCY_NAMES)
  list(APPEND simdjson_FIND_DEPENDENCY_NAMES )
  list(REMOVE_DUPLICATES simdjson_FIND_DEPENDENCY_NAMES)
else()
  set(simdjson_FIND_DEPENDENCY_NAMES )
endif()

########### VARIABLES #######################################################################
#############################################################################################
set(simdjson_PACKAGE_FOLDER_RELEASE "/root/.conan2/p/b/simdj0eff15109bf02/p")
set(simdjson_BUILD_MODULES_PATHS_RELEASE )


set(simdjson_INCLUDE_DIRS_RELEASE "${simdjson_PACKAGE_FOLDER_RELEASE}/include")
set(simdjson_RES_DIRS_RELEASE )
set(simdjson_DEFINITIONS_RELEASE "-DSIMDJSON_THREADS_ENABLED=1")
set(simdjson_SHARED_LINK_FLAGS_RELEASE )
set(simdjson_EXE_LINK_FLAGS_RELEASE )
set(simdjson_OBJECTS_RELEASE )
set(simdjson_COMPILE_DEFINITIONS_RELEASE "SIMDJSON_THREADS_ENABLED=1")
set(simdjson_COMPILE_OPTIONS_C_RELEASE )
set(simdjson_COMPILE_OPTIONS_CXX_RELEASE )
set(simdjson_LIB_DIRS_RELEASE "${simdjson_PACKAGE_FOLDER_RELEASE}/lib")
set(simdjson_BIN_DIRS_RELEASE )
set(simdjson_LIBRARY_TYPE_RELEASE STATIC)
set(simdjson_IS_HOST_WINDOWS_RELEASE 0)
set(simdjson_LIBS_RELEASE simdjson)
set(simdjson_SYSTEM_LIBS_RELEASE m pthread)
set(simdjson_FRAMEWORK_DIRS_RELEASE )
set(simdjson_FRAMEWORKS_RELEASE )
set(simdjson_BUILD_DIRS_RELEASE )
set(simdjson_NO_SONAME_MODE_RELEASE FALSE)


# COMPOUND VARIABLES
set(simdjson_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${simdjson_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${simdjson_COMPILE_OPTIONS_C_RELEASE}>")
set(simdjson_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${simdjson_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${simdjson_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${simdjson_EXE_LINK_FLAGS_RELEASE}>")


set(simdjson_COMPONENTS_RELEASE )