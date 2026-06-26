########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

list(APPEND gtest_COMPONENT_NAMES GTest::gtest GTest::gmock)
list(REMOVE_DUPLICATES gtest_COMPONENT_NAMES)
if(DEFINED gtest_FIND_DEPENDENCY_NAMES)
  list(APPEND gtest_FIND_DEPENDENCY_NAMES )
  list(REMOVE_DUPLICATES gtest_FIND_DEPENDENCY_NAMES)
else()
  set(gtest_FIND_DEPENDENCY_NAMES )
endif()

########### VARIABLES #######################################################################
#############################################################################################
set(gtest_PACKAGE_FOLDER_RELEASE "/root/.conan2/p/b/gtestd489d2e965c26/p")
set(gtest_BUILD_MODULES_PATHS_RELEASE )


set(gtest_INCLUDE_DIRS_RELEASE "${gtest_PACKAGE_FOLDER_RELEASE}/include")
set(gtest_RES_DIRS_RELEASE )
set(gtest_DEFINITIONS_RELEASE )
set(gtest_SHARED_LINK_FLAGS_RELEASE )
set(gtest_EXE_LINK_FLAGS_RELEASE )
set(gtest_OBJECTS_RELEASE )
set(gtest_COMPILE_DEFINITIONS_RELEASE )
set(gtest_COMPILE_OPTIONS_C_RELEASE )
set(gtest_COMPILE_OPTIONS_CXX_RELEASE )
set(gtest_LIB_DIRS_RELEASE "${gtest_PACKAGE_FOLDER_RELEASE}/lib")
set(gtest_BIN_DIRS_RELEASE )
set(gtest_LIBRARY_TYPE_RELEASE STATIC)
set(gtest_IS_HOST_WINDOWS_RELEASE 0)
set(gtest_LIBS_RELEASE gmock gtest)
set(gtest_SYSTEM_LIBS_RELEASE m pthread)
set(gtest_FRAMEWORK_DIRS_RELEASE )
set(gtest_FRAMEWORKS_RELEASE )
set(gtest_BUILD_DIRS_RELEASE )
set(gtest_NO_SONAME_MODE_RELEASE FALSE)


# COMPOUND VARIABLES
set(gtest_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${gtest_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${gtest_COMPILE_OPTIONS_C_RELEASE}>")
set(gtest_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${gtest_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${gtest_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${gtest_EXE_LINK_FLAGS_RELEASE}>")


set(gtest_COMPONENTS_RELEASE GTest::gtest GTest::gmock)
########### COMPONENT GTest::gmock VARIABLES ############################################

set(gtest_GTest_gmock_INCLUDE_DIRS_RELEASE "${gtest_PACKAGE_FOLDER_RELEASE}/include")
set(gtest_GTest_gmock_LIB_DIRS_RELEASE "${gtest_PACKAGE_FOLDER_RELEASE}/lib")
set(gtest_GTest_gmock_BIN_DIRS_RELEASE )
set(gtest_GTest_gmock_LIBRARY_TYPE_RELEASE STATIC)
set(gtest_GTest_gmock_IS_HOST_WINDOWS_RELEASE 0)
set(gtest_GTest_gmock_RES_DIRS_RELEASE )
set(gtest_GTest_gmock_DEFINITIONS_RELEASE )
set(gtest_GTest_gmock_OBJECTS_RELEASE )
set(gtest_GTest_gmock_COMPILE_DEFINITIONS_RELEASE )
set(gtest_GTest_gmock_COMPILE_OPTIONS_C_RELEASE "")
set(gtest_GTest_gmock_COMPILE_OPTIONS_CXX_RELEASE "")
set(gtest_GTest_gmock_LIBS_RELEASE gmock)
set(gtest_GTest_gmock_SYSTEM_LIBS_RELEASE )
set(gtest_GTest_gmock_FRAMEWORK_DIRS_RELEASE )
set(gtest_GTest_gmock_FRAMEWORKS_RELEASE )
set(gtest_GTest_gmock_DEPENDENCIES_RELEASE GTest::gtest)
set(gtest_GTest_gmock_SHARED_LINK_FLAGS_RELEASE )
set(gtest_GTest_gmock_EXE_LINK_FLAGS_RELEASE )
set(gtest_GTest_gmock_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(gtest_GTest_gmock_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${gtest_GTest_gmock_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${gtest_GTest_gmock_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${gtest_GTest_gmock_EXE_LINK_FLAGS_RELEASE}>
)
set(gtest_GTest_gmock_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${gtest_GTest_gmock_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${gtest_GTest_gmock_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT GTest::gtest VARIABLES ############################################

set(gtest_GTest_gtest_INCLUDE_DIRS_RELEASE "${gtest_PACKAGE_FOLDER_RELEASE}/include")
set(gtest_GTest_gtest_LIB_DIRS_RELEASE "${gtest_PACKAGE_FOLDER_RELEASE}/lib")
set(gtest_GTest_gtest_BIN_DIRS_RELEASE )
set(gtest_GTest_gtest_LIBRARY_TYPE_RELEASE STATIC)
set(gtest_GTest_gtest_IS_HOST_WINDOWS_RELEASE 0)
set(gtest_GTest_gtest_RES_DIRS_RELEASE )
set(gtest_GTest_gtest_DEFINITIONS_RELEASE )
set(gtest_GTest_gtest_OBJECTS_RELEASE )
set(gtest_GTest_gtest_COMPILE_DEFINITIONS_RELEASE )
set(gtest_GTest_gtest_COMPILE_OPTIONS_C_RELEASE "")
set(gtest_GTest_gtest_COMPILE_OPTIONS_CXX_RELEASE "")
set(gtest_GTest_gtest_LIBS_RELEASE gtest)
set(gtest_GTest_gtest_SYSTEM_LIBS_RELEASE m pthread)
set(gtest_GTest_gtest_FRAMEWORK_DIRS_RELEASE )
set(gtest_GTest_gtest_FRAMEWORKS_RELEASE )
set(gtest_GTest_gtest_DEPENDENCIES_RELEASE )
set(gtest_GTest_gtest_SHARED_LINK_FLAGS_RELEASE )
set(gtest_GTest_gtest_EXE_LINK_FLAGS_RELEASE )
set(gtest_GTest_gtest_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(gtest_GTest_gtest_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${gtest_GTest_gtest_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${gtest_GTest_gtest_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${gtest_GTest_gtest_EXE_LINK_FLAGS_RELEASE}>
)
set(gtest_GTest_gtest_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${gtest_GTest_gtest_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${gtest_GTest_gtest_COMPILE_OPTIONS_C_RELEASE}>")