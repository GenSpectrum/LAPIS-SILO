########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

list(APPEND boost_COMPONENT_NAMES Boost::diagnostic_definitions Boost::disable_autolinking Boost::dynamic_linking Boost::headers Boost::boost boost::_libboost Boost::charconv Boost::container Boost::regex Boost::serialization Boost::system Boost::random Boost::url Boost::wserialization Boost::iostreams)
list(REMOVE_DUPLICATES boost_COMPONENT_NAMES)
if(DEFINED boost_FIND_DEPENDENCY_NAMES)
  list(APPEND boost_FIND_DEPENDENCY_NAMES LibLZMA BZip2 ZLIB zstd)
  list(REMOVE_DUPLICATES boost_FIND_DEPENDENCY_NAMES)
else()
  set(boost_FIND_DEPENDENCY_NAMES LibLZMA BZip2 ZLIB zstd)
endif()
set(LibLZMA_FIND_MODE "MODULE")
set(BZip2_FIND_MODE "MODULE")
set(ZLIB_FIND_MODE "MODULE")
set(zstd_FIND_MODE "MODULE")

########### VARIABLES #######################################################################
#############################################################################################
set(boost_PACKAGE_FOLDER_RELEASE "/root/.conan2/p/b/boost6d8eedbf763b4/p")
set(boost_BUILD_MODULES_PATHS_RELEASE )


set(boost_INCLUDE_DIRS_RELEASE "${boost_PACKAGE_FOLDER_RELEASE}/include")
set(boost_RES_DIRS_RELEASE )
set(boost_DEFINITIONS_RELEASE )
set(boost_SHARED_LINK_FLAGS_RELEASE )
set(boost_EXE_LINK_FLAGS_RELEASE )
set(boost_OBJECTS_RELEASE )
set(boost_COMPILE_DEFINITIONS_RELEASE )
set(boost_COMPILE_OPTIONS_C_RELEASE )
set(boost_COMPILE_OPTIONS_CXX_RELEASE )
set(boost_LIB_DIRS_RELEASE "${boost_PACKAGE_FOLDER_RELEASE}/lib")
set(boost_BIN_DIRS_RELEASE )
set(boost_LIBRARY_TYPE_RELEASE STATIC)
set(boost_IS_HOST_WINDOWS_RELEASE 0)
set(boost_LIBS_RELEASE boost_iostreams boost_wserialization boost_url boost_random boost_serialization boost_regex boost_container boost_charconv)
set(boost_SYSTEM_LIBS_RELEASE rt pthread)
set(boost_FRAMEWORK_DIRS_RELEASE )
set(boost_FRAMEWORKS_RELEASE )
set(boost_BUILD_DIRS_RELEASE )
set(boost_NO_SONAME_MODE_RELEASE FALSE)


# COMPOUND VARIABLES
set(boost_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${boost_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${boost_COMPILE_OPTIONS_C_RELEASE}>")
set(boost_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${boost_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${boost_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${boost_EXE_LINK_FLAGS_RELEASE}>")


set(boost_COMPONENTS_RELEASE Boost::diagnostic_definitions Boost::disable_autolinking Boost::dynamic_linking Boost::headers Boost::boost boost::_libboost Boost::charconv Boost::container Boost::regex Boost::serialization Boost::system Boost::random Boost::url Boost::wserialization Boost::iostreams)
########### COMPONENT Boost::iostreams VARIABLES ############################################

set(boost_Boost_iostreams_INCLUDE_DIRS_RELEASE "${boost_PACKAGE_FOLDER_RELEASE}/include")
set(boost_Boost_iostreams_LIB_DIRS_RELEASE "${boost_PACKAGE_FOLDER_RELEASE}/lib")
set(boost_Boost_iostreams_BIN_DIRS_RELEASE )
set(boost_Boost_iostreams_LIBRARY_TYPE_RELEASE STATIC)
set(boost_Boost_iostreams_IS_HOST_WINDOWS_RELEASE 0)
set(boost_Boost_iostreams_RES_DIRS_RELEASE )
set(boost_Boost_iostreams_DEFINITIONS_RELEASE )
set(boost_Boost_iostreams_OBJECTS_RELEASE )
set(boost_Boost_iostreams_COMPILE_DEFINITIONS_RELEASE )
set(boost_Boost_iostreams_COMPILE_OPTIONS_C_RELEASE "")
set(boost_Boost_iostreams_COMPILE_OPTIONS_CXX_RELEASE "")
set(boost_Boost_iostreams_LIBS_RELEASE boost_iostreams)
set(boost_Boost_iostreams_SYSTEM_LIBS_RELEASE )
set(boost_Boost_iostreams_FRAMEWORK_DIRS_RELEASE )
set(boost_Boost_iostreams_FRAMEWORKS_RELEASE )
set(boost_Boost_iostreams_DEPENDENCIES_RELEASE Boost::random Boost::regex boost::_libboost BZip2::BZip2 LibLZMA::LibLZMA ZLIB::ZLIB zstd::libzstd_static)
set(boost_Boost_iostreams_SHARED_LINK_FLAGS_RELEASE )
set(boost_Boost_iostreams_EXE_LINK_FLAGS_RELEASE )
set(boost_Boost_iostreams_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(boost_Boost_iostreams_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${boost_Boost_iostreams_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${boost_Boost_iostreams_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${boost_Boost_iostreams_EXE_LINK_FLAGS_RELEASE}>
)
set(boost_Boost_iostreams_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${boost_Boost_iostreams_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${boost_Boost_iostreams_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Boost::wserialization VARIABLES ############################################

set(boost_Boost_wserialization_INCLUDE_DIRS_RELEASE "${boost_PACKAGE_FOLDER_RELEASE}/include")
set(boost_Boost_wserialization_LIB_DIRS_RELEASE "${boost_PACKAGE_FOLDER_RELEASE}/lib")
set(boost_Boost_wserialization_BIN_DIRS_RELEASE )
set(boost_Boost_wserialization_LIBRARY_TYPE_RELEASE STATIC)
set(boost_Boost_wserialization_IS_HOST_WINDOWS_RELEASE 0)
set(boost_Boost_wserialization_RES_DIRS_RELEASE )
set(boost_Boost_wserialization_DEFINITIONS_RELEASE )
set(boost_Boost_wserialization_OBJECTS_RELEASE )
set(boost_Boost_wserialization_COMPILE_DEFINITIONS_RELEASE )
set(boost_Boost_wserialization_COMPILE_OPTIONS_C_RELEASE "")
set(boost_Boost_wserialization_COMPILE_OPTIONS_CXX_RELEASE "")
set(boost_Boost_wserialization_LIBS_RELEASE boost_wserialization)
set(boost_Boost_wserialization_SYSTEM_LIBS_RELEASE )
set(boost_Boost_wserialization_FRAMEWORK_DIRS_RELEASE )
set(boost_Boost_wserialization_FRAMEWORKS_RELEASE )
set(boost_Boost_wserialization_DEPENDENCIES_RELEASE Boost::serialization boost::_libboost)
set(boost_Boost_wserialization_SHARED_LINK_FLAGS_RELEASE )
set(boost_Boost_wserialization_EXE_LINK_FLAGS_RELEASE )
set(boost_Boost_wserialization_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(boost_Boost_wserialization_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${boost_Boost_wserialization_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${boost_Boost_wserialization_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${boost_Boost_wserialization_EXE_LINK_FLAGS_RELEASE}>
)
set(boost_Boost_wserialization_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${boost_Boost_wserialization_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${boost_Boost_wserialization_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Boost::url VARIABLES ############################################

set(boost_Boost_url_INCLUDE_DIRS_RELEASE "${boost_PACKAGE_FOLDER_RELEASE}/include")
set(boost_Boost_url_LIB_DIRS_RELEASE "${boost_PACKAGE_FOLDER_RELEASE}/lib")
set(boost_Boost_url_BIN_DIRS_RELEASE )
set(boost_Boost_url_LIBRARY_TYPE_RELEASE STATIC)
set(boost_Boost_url_IS_HOST_WINDOWS_RELEASE 0)
set(boost_Boost_url_RES_DIRS_RELEASE )
set(boost_Boost_url_DEFINITIONS_RELEASE )
set(boost_Boost_url_OBJECTS_RELEASE )
set(boost_Boost_url_COMPILE_DEFINITIONS_RELEASE )
set(boost_Boost_url_COMPILE_OPTIONS_C_RELEASE "")
set(boost_Boost_url_COMPILE_OPTIONS_CXX_RELEASE "")
set(boost_Boost_url_LIBS_RELEASE boost_url)
set(boost_Boost_url_SYSTEM_LIBS_RELEASE )
set(boost_Boost_url_FRAMEWORK_DIRS_RELEASE )
set(boost_Boost_url_FRAMEWORKS_RELEASE )
set(boost_Boost_url_DEPENDENCIES_RELEASE Boost::system boost::_libboost)
set(boost_Boost_url_SHARED_LINK_FLAGS_RELEASE )
set(boost_Boost_url_EXE_LINK_FLAGS_RELEASE )
set(boost_Boost_url_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(boost_Boost_url_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${boost_Boost_url_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${boost_Boost_url_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${boost_Boost_url_EXE_LINK_FLAGS_RELEASE}>
)
set(boost_Boost_url_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${boost_Boost_url_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${boost_Boost_url_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Boost::random VARIABLES ############################################

set(boost_Boost_random_INCLUDE_DIRS_RELEASE "${boost_PACKAGE_FOLDER_RELEASE}/include")
set(boost_Boost_random_LIB_DIRS_RELEASE "${boost_PACKAGE_FOLDER_RELEASE}/lib")
set(boost_Boost_random_BIN_DIRS_RELEASE )
set(boost_Boost_random_LIBRARY_TYPE_RELEASE STATIC)
set(boost_Boost_random_IS_HOST_WINDOWS_RELEASE 0)
set(boost_Boost_random_RES_DIRS_RELEASE )
set(boost_Boost_random_DEFINITIONS_RELEASE )
set(boost_Boost_random_OBJECTS_RELEASE )
set(boost_Boost_random_COMPILE_DEFINITIONS_RELEASE )
set(boost_Boost_random_COMPILE_OPTIONS_C_RELEASE "")
set(boost_Boost_random_COMPILE_OPTIONS_CXX_RELEASE "")
set(boost_Boost_random_LIBS_RELEASE boost_random)
set(boost_Boost_random_SYSTEM_LIBS_RELEASE )
set(boost_Boost_random_FRAMEWORK_DIRS_RELEASE )
set(boost_Boost_random_FRAMEWORKS_RELEASE )
set(boost_Boost_random_DEPENDENCIES_RELEASE Boost::system boost::_libboost)
set(boost_Boost_random_SHARED_LINK_FLAGS_RELEASE )
set(boost_Boost_random_EXE_LINK_FLAGS_RELEASE )
set(boost_Boost_random_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(boost_Boost_random_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${boost_Boost_random_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${boost_Boost_random_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${boost_Boost_random_EXE_LINK_FLAGS_RELEASE}>
)
set(boost_Boost_random_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${boost_Boost_random_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${boost_Boost_random_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Boost::system VARIABLES ############################################

set(boost_Boost_system_INCLUDE_DIRS_RELEASE "${boost_PACKAGE_FOLDER_RELEASE}/include")
set(boost_Boost_system_LIB_DIRS_RELEASE "${boost_PACKAGE_FOLDER_RELEASE}/lib")
set(boost_Boost_system_BIN_DIRS_RELEASE )
set(boost_Boost_system_LIBRARY_TYPE_RELEASE STATIC)
set(boost_Boost_system_IS_HOST_WINDOWS_RELEASE 0)
set(boost_Boost_system_RES_DIRS_RELEASE )
set(boost_Boost_system_DEFINITIONS_RELEASE )
set(boost_Boost_system_OBJECTS_RELEASE )
set(boost_Boost_system_COMPILE_DEFINITIONS_RELEASE )
set(boost_Boost_system_COMPILE_OPTIONS_C_RELEASE "")
set(boost_Boost_system_COMPILE_OPTIONS_CXX_RELEASE "")
set(boost_Boost_system_LIBS_RELEASE )
set(boost_Boost_system_SYSTEM_LIBS_RELEASE )
set(boost_Boost_system_FRAMEWORK_DIRS_RELEASE )
set(boost_Boost_system_FRAMEWORKS_RELEASE )
set(boost_Boost_system_DEPENDENCIES_RELEASE boost::_libboost)
set(boost_Boost_system_SHARED_LINK_FLAGS_RELEASE )
set(boost_Boost_system_EXE_LINK_FLAGS_RELEASE )
set(boost_Boost_system_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(boost_Boost_system_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${boost_Boost_system_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${boost_Boost_system_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${boost_Boost_system_EXE_LINK_FLAGS_RELEASE}>
)
set(boost_Boost_system_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${boost_Boost_system_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${boost_Boost_system_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Boost::serialization VARIABLES ############################################

set(boost_Boost_serialization_INCLUDE_DIRS_RELEASE "${boost_PACKAGE_FOLDER_RELEASE}/include")
set(boost_Boost_serialization_LIB_DIRS_RELEASE "${boost_PACKAGE_FOLDER_RELEASE}/lib")
set(boost_Boost_serialization_BIN_DIRS_RELEASE )
set(boost_Boost_serialization_LIBRARY_TYPE_RELEASE STATIC)
set(boost_Boost_serialization_IS_HOST_WINDOWS_RELEASE 0)
set(boost_Boost_serialization_RES_DIRS_RELEASE )
set(boost_Boost_serialization_DEFINITIONS_RELEASE )
set(boost_Boost_serialization_OBJECTS_RELEASE )
set(boost_Boost_serialization_COMPILE_DEFINITIONS_RELEASE )
set(boost_Boost_serialization_COMPILE_OPTIONS_C_RELEASE "")
set(boost_Boost_serialization_COMPILE_OPTIONS_CXX_RELEASE "")
set(boost_Boost_serialization_LIBS_RELEASE boost_serialization)
set(boost_Boost_serialization_SYSTEM_LIBS_RELEASE )
set(boost_Boost_serialization_FRAMEWORK_DIRS_RELEASE )
set(boost_Boost_serialization_FRAMEWORKS_RELEASE )
set(boost_Boost_serialization_DEPENDENCIES_RELEASE boost::_libboost)
set(boost_Boost_serialization_SHARED_LINK_FLAGS_RELEASE )
set(boost_Boost_serialization_EXE_LINK_FLAGS_RELEASE )
set(boost_Boost_serialization_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(boost_Boost_serialization_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${boost_Boost_serialization_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${boost_Boost_serialization_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${boost_Boost_serialization_EXE_LINK_FLAGS_RELEASE}>
)
set(boost_Boost_serialization_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${boost_Boost_serialization_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${boost_Boost_serialization_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Boost::regex VARIABLES ############################################

set(boost_Boost_regex_INCLUDE_DIRS_RELEASE "${boost_PACKAGE_FOLDER_RELEASE}/include")
set(boost_Boost_regex_LIB_DIRS_RELEASE "${boost_PACKAGE_FOLDER_RELEASE}/lib")
set(boost_Boost_regex_BIN_DIRS_RELEASE )
set(boost_Boost_regex_LIBRARY_TYPE_RELEASE STATIC)
set(boost_Boost_regex_IS_HOST_WINDOWS_RELEASE 0)
set(boost_Boost_regex_RES_DIRS_RELEASE )
set(boost_Boost_regex_DEFINITIONS_RELEASE )
set(boost_Boost_regex_OBJECTS_RELEASE )
set(boost_Boost_regex_COMPILE_DEFINITIONS_RELEASE )
set(boost_Boost_regex_COMPILE_OPTIONS_C_RELEASE "")
set(boost_Boost_regex_COMPILE_OPTIONS_CXX_RELEASE "")
set(boost_Boost_regex_LIBS_RELEASE boost_regex)
set(boost_Boost_regex_SYSTEM_LIBS_RELEASE )
set(boost_Boost_regex_FRAMEWORK_DIRS_RELEASE )
set(boost_Boost_regex_FRAMEWORKS_RELEASE )
set(boost_Boost_regex_DEPENDENCIES_RELEASE boost::_libboost)
set(boost_Boost_regex_SHARED_LINK_FLAGS_RELEASE )
set(boost_Boost_regex_EXE_LINK_FLAGS_RELEASE )
set(boost_Boost_regex_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(boost_Boost_regex_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${boost_Boost_regex_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${boost_Boost_regex_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${boost_Boost_regex_EXE_LINK_FLAGS_RELEASE}>
)
set(boost_Boost_regex_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${boost_Boost_regex_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${boost_Boost_regex_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Boost::container VARIABLES ############################################

set(boost_Boost_container_INCLUDE_DIRS_RELEASE "${boost_PACKAGE_FOLDER_RELEASE}/include")
set(boost_Boost_container_LIB_DIRS_RELEASE "${boost_PACKAGE_FOLDER_RELEASE}/lib")
set(boost_Boost_container_BIN_DIRS_RELEASE )
set(boost_Boost_container_LIBRARY_TYPE_RELEASE STATIC)
set(boost_Boost_container_IS_HOST_WINDOWS_RELEASE 0)
set(boost_Boost_container_RES_DIRS_RELEASE )
set(boost_Boost_container_DEFINITIONS_RELEASE )
set(boost_Boost_container_OBJECTS_RELEASE )
set(boost_Boost_container_COMPILE_DEFINITIONS_RELEASE )
set(boost_Boost_container_COMPILE_OPTIONS_C_RELEASE "")
set(boost_Boost_container_COMPILE_OPTIONS_CXX_RELEASE "")
set(boost_Boost_container_LIBS_RELEASE boost_container)
set(boost_Boost_container_SYSTEM_LIBS_RELEASE )
set(boost_Boost_container_FRAMEWORK_DIRS_RELEASE )
set(boost_Boost_container_FRAMEWORKS_RELEASE )
set(boost_Boost_container_DEPENDENCIES_RELEASE boost::_libboost)
set(boost_Boost_container_SHARED_LINK_FLAGS_RELEASE )
set(boost_Boost_container_EXE_LINK_FLAGS_RELEASE )
set(boost_Boost_container_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(boost_Boost_container_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${boost_Boost_container_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${boost_Boost_container_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${boost_Boost_container_EXE_LINK_FLAGS_RELEASE}>
)
set(boost_Boost_container_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${boost_Boost_container_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${boost_Boost_container_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Boost::charconv VARIABLES ############################################

set(boost_Boost_charconv_INCLUDE_DIRS_RELEASE "${boost_PACKAGE_FOLDER_RELEASE}/include")
set(boost_Boost_charconv_LIB_DIRS_RELEASE "${boost_PACKAGE_FOLDER_RELEASE}/lib")
set(boost_Boost_charconv_BIN_DIRS_RELEASE )
set(boost_Boost_charconv_LIBRARY_TYPE_RELEASE STATIC)
set(boost_Boost_charconv_IS_HOST_WINDOWS_RELEASE 0)
set(boost_Boost_charconv_RES_DIRS_RELEASE )
set(boost_Boost_charconv_DEFINITIONS_RELEASE )
set(boost_Boost_charconv_OBJECTS_RELEASE )
set(boost_Boost_charconv_COMPILE_DEFINITIONS_RELEASE )
set(boost_Boost_charconv_COMPILE_OPTIONS_C_RELEASE "")
set(boost_Boost_charconv_COMPILE_OPTIONS_CXX_RELEASE "")
set(boost_Boost_charconv_LIBS_RELEASE boost_charconv)
set(boost_Boost_charconv_SYSTEM_LIBS_RELEASE )
set(boost_Boost_charconv_FRAMEWORK_DIRS_RELEASE )
set(boost_Boost_charconv_FRAMEWORKS_RELEASE )
set(boost_Boost_charconv_DEPENDENCIES_RELEASE boost::_libboost)
set(boost_Boost_charconv_SHARED_LINK_FLAGS_RELEASE )
set(boost_Boost_charconv_EXE_LINK_FLAGS_RELEASE )
set(boost_Boost_charconv_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(boost_Boost_charconv_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${boost_Boost_charconv_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${boost_Boost_charconv_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${boost_Boost_charconv_EXE_LINK_FLAGS_RELEASE}>
)
set(boost_Boost_charconv_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${boost_Boost_charconv_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${boost_Boost_charconv_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT boost::_libboost VARIABLES ############################################

set(boost_boost__libboost_INCLUDE_DIRS_RELEASE "${boost_PACKAGE_FOLDER_RELEASE}/include")
set(boost_boost__libboost_LIB_DIRS_RELEASE "${boost_PACKAGE_FOLDER_RELEASE}/lib")
set(boost_boost__libboost_BIN_DIRS_RELEASE )
set(boost_boost__libboost_LIBRARY_TYPE_RELEASE STATIC)
set(boost_boost__libboost_IS_HOST_WINDOWS_RELEASE 0)
set(boost_boost__libboost_RES_DIRS_RELEASE )
set(boost_boost__libboost_DEFINITIONS_RELEASE )
set(boost_boost__libboost_OBJECTS_RELEASE )
set(boost_boost__libboost_COMPILE_DEFINITIONS_RELEASE )
set(boost_boost__libboost_COMPILE_OPTIONS_C_RELEASE "")
set(boost_boost__libboost_COMPILE_OPTIONS_CXX_RELEASE "")
set(boost_boost__libboost_LIBS_RELEASE )
set(boost_boost__libboost_SYSTEM_LIBS_RELEASE rt pthread)
set(boost_boost__libboost_FRAMEWORK_DIRS_RELEASE )
set(boost_boost__libboost_FRAMEWORKS_RELEASE )
set(boost_boost__libboost_DEPENDENCIES_RELEASE Boost::headers)
set(boost_boost__libboost_SHARED_LINK_FLAGS_RELEASE )
set(boost_boost__libboost_EXE_LINK_FLAGS_RELEASE )
set(boost_boost__libboost_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(boost_boost__libboost_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${boost_boost__libboost_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${boost_boost__libboost_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${boost_boost__libboost_EXE_LINK_FLAGS_RELEASE}>
)
set(boost_boost__libboost_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${boost_boost__libboost_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${boost_boost__libboost_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Boost::boost VARIABLES ############################################

set(boost_Boost_boost_INCLUDE_DIRS_RELEASE "${boost_PACKAGE_FOLDER_RELEASE}/include")
set(boost_Boost_boost_LIB_DIRS_RELEASE "${boost_PACKAGE_FOLDER_RELEASE}/lib")
set(boost_Boost_boost_BIN_DIRS_RELEASE )
set(boost_Boost_boost_LIBRARY_TYPE_RELEASE STATIC)
set(boost_Boost_boost_IS_HOST_WINDOWS_RELEASE 0)
set(boost_Boost_boost_RES_DIRS_RELEASE )
set(boost_Boost_boost_DEFINITIONS_RELEASE )
set(boost_Boost_boost_OBJECTS_RELEASE )
set(boost_Boost_boost_COMPILE_DEFINITIONS_RELEASE )
set(boost_Boost_boost_COMPILE_OPTIONS_C_RELEASE "")
set(boost_Boost_boost_COMPILE_OPTIONS_CXX_RELEASE "")
set(boost_Boost_boost_LIBS_RELEASE )
set(boost_Boost_boost_SYSTEM_LIBS_RELEASE )
set(boost_Boost_boost_FRAMEWORK_DIRS_RELEASE )
set(boost_Boost_boost_FRAMEWORKS_RELEASE )
set(boost_Boost_boost_DEPENDENCIES_RELEASE Boost::headers)
set(boost_Boost_boost_SHARED_LINK_FLAGS_RELEASE )
set(boost_Boost_boost_EXE_LINK_FLAGS_RELEASE )
set(boost_Boost_boost_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(boost_Boost_boost_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${boost_Boost_boost_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${boost_Boost_boost_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${boost_Boost_boost_EXE_LINK_FLAGS_RELEASE}>
)
set(boost_Boost_boost_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${boost_Boost_boost_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${boost_Boost_boost_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Boost::headers VARIABLES ############################################

set(boost_Boost_headers_INCLUDE_DIRS_RELEASE "${boost_PACKAGE_FOLDER_RELEASE}/include")
set(boost_Boost_headers_LIB_DIRS_RELEASE )
set(boost_Boost_headers_BIN_DIRS_RELEASE )
set(boost_Boost_headers_LIBRARY_TYPE_RELEASE STATIC)
set(boost_Boost_headers_IS_HOST_WINDOWS_RELEASE 0)
set(boost_Boost_headers_RES_DIRS_RELEASE )
set(boost_Boost_headers_DEFINITIONS_RELEASE )
set(boost_Boost_headers_OBJECTS_RELEASE )
set(boost_Boost_headers_COMPILE_DEFINITIONS_RELEASE )
set(boost_Boost_headers_COMPILE_OPTIONS_C_RELEASE "")
set(boost_Boost_headers_COMPILE_OPTIONS_CXX_RELEASE "")
set(boost_Boost_headers_LIBS_RELEASE )
set(boost_Boost_headers_SYSTEM_LIBS_RELEASE )
set(boost_Boost_headers_FRAMEWORK_DIRS_RELEASE )
set(boost_Boost_headers_FRAMEWORKS_RELEASE )
set(boost_Boost_headers_DEPENDENCIES_RELEASE Boost::diagnostic_definitions Boost::disable_autolinking Boost::dynamic_linking)
set(boost_Boost_headers_SHARED_LINK_FLAGS_RELEASE )
set(boost_Boost_headers_EXE_LINK_FLAGS_RELEASE )
set(boost_Boost_headers_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(boost_Boost_headers_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${boost_Boost_headers_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${boost_Boost_headers_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${boost_Boost_headers_EXE_LINK_FLAGS_RELEASE}>
)
set(boost_Boost_headers_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${boost_Boost_headers_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${boost_Boost_headers_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Boost::dynamic_linking VARIABLES ############################################

set(boost_Boost_dynamic_linking_INCLUDE_DIRS_RELEASE "${boost_PACKAGE_FOLDER_RELEASE}/include")
set(boost_Boost_dynamic_linking_LIB_DIRS_RELEASE "${boost_PACKAGE_FOLDER_RELEASE}/lib")
set(boost_Boost_dynamic_linking_BIN_DIRS_RELEASE )
set(boost_Boost_dynamic_linking_LIBRARY_TYPE_RELEASE STATIC)
set(boost_Boost_dynamic_linking_IS_HOST_WINDOWS_RELEASE 0)
set(boost_Boost_dynamic_linking_RES_DIRS_RELEASE )
set(boost_Boost_dynamic_linking_DEFINITIONS_RELEASE )
set(boost_Boost_dynamic_linking_OBJECTS_RELEASE )
set(boost_Boost_dynamic_linking_COMPILE_DEFINITIONS_RELEASE )
set(boost_Boost_dynamic_linking_COMPILE_OPTIONS_C_RELEASE "")
set(boost_Boost_dynamic_linking_COMPILE_OPTIONS_CXX_RELEASE "")
set(boost_Boost_dynamic_linking_LIBS_RELEASE )
set(boost_Boost_dynamic_linking_SYSTEM_LIBS_RELEASE )
set(boost_Boost_dynamic_linking_FRAMEWORK_DIRS_RELEASE )
set(boost_Boost_dynamic_linking_FRAMEWORKS_RELEASE )
set(boost_Boost_dynamic_linking_DEPENDENCIES_RELEASE )
set(boost_Boost_dynamic_linking_SHARED_LINK_FLAGS_RELEASE )
set(boost_Boost_dynamic_linking_EXE_LINK_FLAGS_RELEASE )
set(boost_Boost_dynamic_linking_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(boost_Boost_dynamic_linking_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${boost_Boost_dynamic_linking_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${boost_Boost_dynamic_linking_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${boost_Boost_dynamic_linking_EXE_LINK_FLAGS_RELEASE}>
)
set(boost_Boost_dynamic_linking_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${boost_Boost_dynamic_linking_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${boost_Boost_dynamic_linking_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Boost::disable_autolinking VARIABLES ############################################

set(boost_Boost_disable_autolinking_INCLUDE_DIRS_RELEASE "${boost_PACKAGE_FOLDER_RELEASE}/include")
set(boost_Boost_disable_autolinking_LIB_DIRS_RELEASE "${boost_PACKAGE_FOLDER_RELEASE}/lib")
set(boost_Boost_disable_autolinking_BIN_DIRS_RELEASE )
set(boost_Boost_disable_autolinking_LIBRARY_TYPE_RELEASE STATIC)
set(boost_Boost_disable_autolinking_IS_HOST_WINDOWS_RELEASE 0)
set(boost_Boost_disable_autolinking_RES_DIRS_RELEASE )
set(boost_Boost_disable_autolinking_DEFINITIONS_RELEASE )
set(boost_Boost_disable_autolinking_OBJECTS_RELEASE )
set(boost_Boost_disable_autolinking_COMPILE_DEFINITIONS_RELEASE )
set(boost_Boost_disable_autolinking_COMPILE_OPTIONS_C_RELEASE "")
set(boost_Boost_disable_autolinking_COMPILE_OPTIONS_CXX_RELEASE "")
set(boost_Boost_disable_autolinking_LIBS_RELEASE )
set(boost_Boost_disable_autolinking_SYSTEM_LIBS_RELEASE )
set(boost_Boost_disable_autolinking_FRAMEWORK_DIRS_RELEASE )
set(boost_Boost_disable_autolinking_FRAMEWORKS_RELEASE )
set(boost_Boost_disable_autolinking_DEPENDENCIES_RELEASE )
set(boost_Boost_disable_autolinking_SHARED_LINK_FLAGS_RELEASE )
set(boost_Boost_disable_autolinking_EXE_LINK_FLAGS_RELEASE )
set(boost_Boost_disable_autolinking_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(boost_Boost_disable_autolinking_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${boost_Boost_disable_autolinking_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${boost_Boost_disable_autolinking_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${boost_Boost_disable_autolinking_EXE_LINK_FLAGS_RELEASE}>
)
set(boost_Boost_disable_autolinking_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${boost_Boost_disable_autolinking_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${boost_Boost_disable_autolinking_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Boost::diagnostic_definitions VARIABLES ############################################

set(boost_Boost_diagnostic_definitions_INCLUDE_DIRS_RELEASE "${boost_PACKAGE_FOLDER_RELEASE}/include")
set(boost_Boost_diagnostic_definitions_LIB_DIRS_RELEASE "${boost_PACKAGE_FOLDER_RELEASE}/lib")
set(boost_Boost_diagnostic_definitions_BIN_DIRS_RELEASE )
set(boost_Boost_diagnostic_definitions_LIBRARY_TYPE_RELEASE STATIC)
set(boost_Boost_diagnostic_definitions_IS_HOST_WINDOWS_RELEASE 0)
set(boost_Boost_diagnostic_definitions_RES_DIRS_RELEASE )
set(boost_Boost_diagnostic_definitions_DEFINITIONS_RELEASE )
set(boost_Boost_diagnostic_definitions_OBJECTS_RELEASE )
set(boost_Boost_diagnostic_definitions_COMPILE_DEFINITIONS_RELEASE )
set(boost_Boost_diagnostic_definitions_COMPILE_OPTIONS_C_RELEASE "")
set(boost_Boost_diagnostic_definitions_COMPILE_OPTIONS_CXX_RELEASE "")
set(boost_Boost_diagnostic_definitions_LIBS_RELEASE )
set(boost_Boost_diagnostic_definitions_SYSTEM_LIBS_RELEASE )
set(boost_Boost_diagnostic_definitions_FRAMEWORK_DIRS_RELEASE )
set(boost_Boost_diagnostic_definitions_FRAMEWORKS_RELEASE )
set(boost_Boost_diagnostic_definitions_DEPENDENCIES_RELEASE )
set(boost_Boost_diagnostic_definitions_SHARED_LINK_FLAGS_RELEASE )
set(boost_Boost_diagnostic_definitions_EXE_LINK_FLAGS_RELEASE )
set(boost_Boost_diagnostic_definitions_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(boost_Boost_diagnostic_definitions_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${boost_Boost_diagnostic_definitions_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${boost_Boost_diagnostic_definitions_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${boost_Boost_diagnostic_definitions_EXE_LINK_FLAGS_RELEASE}>
)
set(boost_Boost_diagnostic_definitions_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${boost_Boost_diagnostic_definitions_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${boost_Boost_diagnostic_definitions_COMPILE_OPTIONS_C_RELEASE}>")