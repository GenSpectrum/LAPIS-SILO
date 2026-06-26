########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(yaml-cpp_COMPONENT_NAMES "")
if(DEFINED yaml-cpp_FIND_DEPENDENCY_NAMES)
  list(APPEND yaml-cpp_FIND_DEPENDENCY_NAMES )
  list(REMOVE_DUPLICATES yaml-cpp_FIND_DEPENDENCY_NAMES)
else()
  set(yaml-cpp_FIND_DEPENDENCY_NAMES )
endif()

########### VARIABLES #######################################################################
#############################################################################################
set(yaml-cpp_PACKAGE_FOLDER_RELEASE "/root/.conan2/p/b/yaml-0efe35193b001/p")
set(yaml-cpp_BUILD_MODULES_PATHS_RELEASE )


set(yaml-cpp_INCLUDE_DIRS_RELEASE "${yaml-cpp_PACKAGE_FOLDER_RELEASE}/include")
set(yaml-cpp_RES_DIRS_RELEASE )
set(yaml-cpp_DEFINITIONS_RELEASE "-DYAML_CPP_STATIC_DEFINE")
set(yaml-cpp_SHARED_LINK_FLAGS_RELEASE )
set(yaml-cpp_EXE_LINK_FLAGS_RELEASE )
set(yaml-cpp_OBJECTS_RELEASE )
set(yaml-cpp_COMPILE_DEFINITIONS_RELEASE "YAML_CPP_STATIC_DEFINE")
set(yaml-cpp_COMPILE_OPTIONS_C_RELEASE )
set(yaml-cpp_COMPILE_OPTIONS_CXX_RELEASE )
set(yaml-cpp_LIB_DIRS_RELEASE "${yaml-cpp_PACKAGE_FOLDER_RELEASE}/lib")
set(yaml-cpp_BIN_DIRS_RELEASE )
set(yaml-cpp_LIBRARY_TYPE_RELEASE STATIC)
set(yaml-cpp_IS_HOST_WINDOWS_RELEASE 0)
set(yaml-cpp_LIBS_RELEASE yaml-cpp)
set(yaml-cpp_SYSTEM_LIBS_RELEASE m)
set(yaml-cpp_FRAMEWORK_DIRS_RELEASE )
set(yaml-cpp_FRAMEWORKS_RELEASE )
set(yaml-cpp_BUILD_DIRS_RELEASE )
set(yaml-cpp_NO_SONAME_MODE_RELEASE FALSE)


# COMPOUND VARIABLES
set(yaml-cpp_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${yaml-cpp_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${yaml-cpp_COMPILE_OPTIONS_C_RELEASE}>")
set(yaml-cpp_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${yaml-cpp_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${yaml-cpp_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${yaml-cpp_EXE_LINK_FLAGS_RELEASE}>")


set(yaml-cpp_COMPONENTS_RELEASE )