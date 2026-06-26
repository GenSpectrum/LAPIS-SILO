########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

list(APPEND arrow_COMPONENT_NAMES Arrow::arrow_static Parquet::parquet_static ArrowCompute::arrow_compute_static ArrowAcero::arrow_acero_static)
list(REMOVE_DUPLICATES arrow_COMPONENT_NAMES)
if(DEFINED arrow_FIND_DEPENDENCY_NAMES)
  list(APPEND arrow_FIND_DEPENDENCY_NAMES Thrift Boost ZLIB)
  list(REMOVE_DUPLICATES arrow_FIND_DEPENDENCY_NAMES)
else()
  set(arrow_FIND_DEPENDENCY_NAMES Thrift Boost ZLIB)
endif()
set(Thrift_FIND_MODE "NO_MODULE")
set(Boost_FIND_MODE "NO_MODULE")
set(ZLIB_FIND_MODE "NO_MODULE")

########### VARIABLES #######################################################################
#############################################################################################
set(arrow_PACKAGE_FOLDER_RELEASE "/root/.conan2/p/b/arrow86fe9bfbce6b0/p")
set(arrow_BUILD_MODULES_PATHS_RELEASE )


set(arrow_INCLUDE_DIRS_RELEASE "${arrow_PACKAGE_FOLDER_RELEASE}/include")
set(arrow_RES_DIRS_RELEASE )
set(arrow_DEFINITIONS_RELEASE "-DPARQUET_STATIC"
			"-DARROW_STATIC")
set(arrow_SHARED_LINK_FLAGS_RELEASE )
set(arrow_EXE_LINK_FLAGS_RELEASE )
set(arrow_OBJECTS_RELEASE )
set(arrow_COMPILE_DEFINITIONS_RELEASE "PARQUET_STATIC"
			"ARROW_STATIC")
set(arrow_COMPILE_OPTIONS_C_RELEASE )
set(arrow_COMPILE_OPTIONS_CXX_RELEASE )
set(arrow_LIB_DIRS_RELEASE "${arrow_PACKAGE_FOLDER_RELEASE}/lib")
set(arrow_BIN_DIRS_RELEASE )
set(arrow_LIBRARY_TYPE_RELEASE STATIC)
set(arrow_IS_HOST_WINDOWS_RELEASE 0)
set(arrow_LIBS_RELEASE arrow_acero arrow_compute parquet arrow)
set(arrow_SYSTEM_LIBS_RELEASE pthread m dl rt)
set(arrow_FRAMEWORK_DIRS_RELEASE )
set(arrow_FRAMEWORKS_RELEASE )
set(arrow_BUILD_DIRS_RELEASE )
set(arrow_NO_SONAME_MODE_RELEASE FALSE)


# COMPOUND VARIABLES
set(arrow_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${arrow_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${arrow_COMPILE_OPTIONS_C_RELEASE}>")
set(arrow_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${arrow_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${arrow_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${arrow_EXE_LINK_FLAGS_RELEASE}>")


set(arrow_COMPONENTS_RELEASE Arrow::arrow_static Parquet::parquet_static ArrowCompute::arrow_compute_static ArrowAcero::arrow_acero_static)
########### COMPONENT ArrowAcero::arrow_acero_static VARIABLES ############################################

set(arrow_ArrowAcero_arrow_acero_static_INCLUDE_DIRS_RELEASE "${arrow_PACKAGE_FOLDER_RELEASE}/include")
set(arrow_ArrowAcero_arrow_acero_static_LIB_DIRS_RELEASE "${arrow_PACKAGE_FOLDER_RELEASE}/lib")
set(arrow_ArrowAcero_arrow_acero_static_BIN_DIRS_RELEASE )
set(arrow_ArrowAcero_arrow_acero_static_LIBRARY_TYPE_RELEASE STATIC)
set(arrow_ArrowAcero_arrow_acero_static_IS_HOST_WINDOWS_RELEASE 0)
set(arrow_ArrowAcero_arrow_acero_static_RES_DIRS_RELEASE )
set(arrow_ArrowAcero_arrow_acero_static_DEFINITIONS_RELEASE )
set(arrow_ArrowAcero_arrow_acero_static_OBJECTS_RELEASE )
set(arrow_ArrowAcero_arrow_acero_static_COMPILE_DEFINITIONS_RELEASE )
set(arrow_ArrowAcero_arrow_acero_static_COMPILE_OPTIONS_C_RELEASE "")
set(arrow_ArrowAcero_arrow_acero_static_COMPILE_OPTIONS_CXX_RELEASE "")
set(arrow_ArrowAcero_arrow_acero_static_LIBS_RELEASE arrow_acero)
set(arrow_ArrowAcero_arrow_acero_static_SYSTEM_LIBS_RELEASE )
set(arrow_ArrowAcero_arrow_acero_static_FRAMEWORK_DIRS_RELEASE )
set(arrow_ArrowAcero_arrow_acero_static_FRAMEWORKS_RELEASE )
set(arrow_ArrowAcero_arrow_acero_static_DEPENDENCIES_RELEASE Arrow::arrow_static ArrowCompute::arrow_compute_static)
set(arrow_ArrowAcero_arrow_acero_static_SHARED_LINK_FLAGS_RELEASE )
set(arrow_ArrowAcero_arrow_acero_static_EXE_LINK_FLAGS_RELEASE )
set(arrow_ArrowAcero_arrow_acero_static_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(arrow_ArrowAcero_arrow_acero_static_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${arrow_ArrowAcero_arrow_acero_static_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${arrow_ArrowAcero_arrow_acero_static_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${arrow_ArrowAcero_arrow_acero_static_EXE_LINK_FLAGS_RELEASE}>
)
set(arrow_ArrowAcero_arrow_acero_static_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${arrow_ArrowAcero_arrow_acero_static_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${arrow_ArrowAcero_arrow_acero_static_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT ArrowCompute::arrow_compute_static VARIABLES ############################################

set(arrow_ArrowCompute_arrow_compute_static_INCLUDE_DIRS_RELEASE "${arrow_PACKAGE_FOLDER_RELEASE}/include")
set(arrow_ArrowCompute_arrow_compute_static_LIB_DIRS_RELEASE "${arrow_PACKAGE_FOLDER_RELEASE}/lib")
set(arrow_ArrowCompute_arrow_compute_static_BIN_DIRS_RELEASE )
set(arrow_ArrowCompute_arrow_compute_static_LIBRARY_TYPE_RELEASE STATIC)
set(arrow_ArrowCompute_arrow_compute_static_IS_HOST_WINDOWS_RELEASE 0)
set(arrow_ArrowCompute_arrow_compute_static_RES_DIRS_RELEASE )
set(arrow_ArrowCompute_arrow_compute_static_DEFINITIONS_RELEASE )
set(arrow_ArrowCompute_arrow_compute_static_OBJECTS_RELEASE )
set(arrow_ArrowCompute_arrow_compute_static_COMPILE_DEFINITIONS_RELEASE )
set(arrow_ArrowCompute_arrow_compute_static_COMPILE_OPTIONS_C_RELEASE "")
set(arrow_ArrowCompute_arrow_compute_static_COMPILE_OPTIONS_CXX_RELEASE "")
set(arrow_ArrowCompute_arrow_compute_static_LIBS_RELEASE arrow_compute)
set(arrow_ArrowCompute_arrow_compute_static_SYSTEM_LIBS_RELEASE )
set(arrow_ArrowCompute_arrow_compute_static_FRAMEWORK_DIRS_RELEASE )
set(arrow_ArrowCompute_arrow_compute_static_FRAMEWORKS_RELEASE )
set(arrow_ArrowCompute_arrow_compute_static_DEPENDENCIES_RELEASE Arrow::arrow_static)
set(arrow_ArrowCompute_arrow_compute_static_SHARED_LINK_FLAGS_RELEASE )
set(arrow_ArrowCompute_arrow_compute_static_EXE_LINK_FLAGS_RELEASE )
set(arrow_ArrowCompute_arrow_compute_static_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(arrow_ArrowCompute_arrow_compute_static_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${arrow_ArrowCompute_arrow_compute_static_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${arrow_ArrowCompute_arrow_compute_static_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${arrow_ArrowCompute_arrow_compute_static_EXE_LINK_FLAGS_RELEASE}>
)
set(arrow_ArrowCompute_arrow_compute_static_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${arrow_ArrowCompute_arrow_compute_static_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${arrow_ArrowCompute_arrow_compute_static_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Parquet::parquet_static VARIABLES ############################################

set(arrow_Parquet_parquet_static_INCLUDE_DIRS_RELEASE "${arrow_PACKAGE_FOLDER_RELEASE}/include")
set(arrow_Parquet_parquet_static_LIB_DIRS_RELEASE "${arrow_PACKAGE_FOLDER_RELEASE}/lib")
set(arrow_Parquet_parquet_static_BIN_DIRS_RELEASE )
set(arrow_Parquet_parquet_static_LIBRARY_TYPE_RELEASE STATIC)
set(arrow_Parquet_parquet_static_IS_HOST_WINDOWS_RELEASE 0)
set(arrow_Parquet_parquet_static_RES_DIRS_RELEASE )
set(arrow_Parquet_parquet_static_DEFINITIONS_RELEASE "-DPARQUET_STATIC")
set(arrow_Parquet_parquet_static_OBJECTS_RELEASE )
set(arrow_Parquet_parquet_static_COMPILE_DEFINITIONS_RELEASE "PARQUET_STATIC")
set(arrow_Parquet_parquet_static_COMPILE_OPTIONS_C_RELEASE "")
set(arrow_Parquet_parquet_static_COMPILE_OPTIONS_CXX_RELEASE "")
set(arrow_Parquet_parquet_static_LIBS_RELEASE parquet)
set(arrow_Parquet_parquet_static_SYSTEM_LIBS_RELEASE )
set(arrow_Parquet_parquet_static_FRAMEWORK_DIRS_RELEASE )
set(arrow_Parquet_parquet_static_FRAMEWORKS_RELEASE )
set(arrow_Parquet_parquet_static_DEPENDENCIES_RELEASE Arrow::arrow_static)
set(arrow_Parquet_parquet_static_SHARED_LINK_FLAGS_RELEASE )
set(arrow_Parquet_parquet_static_EXE_LINK_FLAGS_RELEASE )
set(arrow_Parquet_parquet_static_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(arrow_Parquet_parquet_static_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${arrow_Parquet_parquet_static_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${arrow_Parquet_parquet_static_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${arrow_Parquet_parquet_static_EXE_LINK_FLAGS_RELEASE}>
)
set(arrow_Parquet_parquet_static_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${arrow_Parquet_parquet_static_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${arrow_Parquet_parquet_static_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Arrow::arrow_static VARIABLES ############################################

set(arrow_Arrow_arrow_static_INCLUDE_DIRS_RELEASE "${arrow_PACKAGE_FOLDER_RELEASE}/include")
set(arrow_Arrow_arrow_static_LIB_DIRS_RELEASE "${arrow_PACKAGE_FOLDER_RELEASE}/lib")
set(arrow_Arrow_arrow_static_BIN_DIRS_RELEASE )
set(arrow_Arrow_arrow_static_LIBRARY_TYPE_RELEASE STATIC)
set(arrow_Arrow_arrow_static_IS_HOST_WINDOWS_RELEASE 0)
set(arrow_Arrow_arrow_static_RES_DIRS_RELEASE )
set(arrow_Arrow_arrow_static_DEFINITIONS_RELEASE "-DARROW_STATIC")
set(arrow_Arrow_arrow_static_OBJECTS_RELEASE )
set(arrow_Arrow_arrow_static_COMPILE_DEFINITIONS_RELEASE "ARROW_STATIC")
set(arrow_Arrow_arrow_static_COMPILE_OPTIONS_C_RELEASE "")
set(arrow_Arrow_arrow_static_COMPILE_OPTIONS_CXX_RELEASE "")
set(arrow_Arrow_arrow_static_LIBS_RELEASE arrow)
set(arrow_Arrow_arrow_static_SYSTEM_LIBS_RELEASE pthread m dl rt)
set(arrow_Arrow_arrow_static_FRAMEWORK_DIRS_RELEASE )
set(arrow_Arrow_arrow_static_FRAMEWORKS_RELEASE )
set(arrow_Arrow_arrow_static_DEPENDENCIES_RELEASE boost::boost thrift::thrift-conan-do-not-use ZLIB::ZLIB)
set(arrow_Arrow_arrow_static_SHARED_LINK_FLAGS_RELEASE )
set(arrow_Arrow_arrow_static_EXE_LINK_FLAGS_RELEASE )
set(arrow_Arrow_arrow_static_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(arrow_Arrow_arrow_static_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${arrow_Arrow_arrow_static_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${arrow_Arrow_arrow_static_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${arrow_Arrow_arrow_static_EXE_LINK_FLAGS_RELEASE}>
)
set(arrow_Arrow_arrow_static_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${arrow_Arrow_arrow_static_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${arrow_Arrow_arrow_static_COMPILE_OPTIONS_C_RELEASE}>")