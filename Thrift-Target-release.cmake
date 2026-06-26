# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(thrift_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(thrift_FRAMEWORKS_FOUND_RELEASE "${thrift_FRAMEWORKS_RELEASE}" "${thrift_FRAMEWORK_DIRS_RELEASE}")

set(thrift_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET thrift_DEPS_TARGET)
    add_library(thrift_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET thrift_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${thrift_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${thrift_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:Boost::headers;openssl::openssl;thrift::thrift;ZLIB::ZLIB;libevent::libevent>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### thrift_DEPS_TARGET to all of them
conan_package_library_targets("${thrift_LIBS_RELEASE}"    # libraries
                              "${thrift_LIB_DIRS_RELEASE}" # package_libdir
                              "${thrift_BIN_DIRS_RELEASE}" # package_bindir
                              "${thrift_LIBRARY_TYPE_RELEASE}"
                              "${thrift_IS_HOST_WINDOWS_RELEASE}"
                              thrift_DEPS_TARGET
                              thrift_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "thrift"    # package_name
                              "${thrift_NO_SONAME_MODE_RELEASE}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${thrift_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## COMPONENTS TARGET PROPERTIES Release ########################################

    ########## COMPONENT thriftnb::thriftnb #############

        set(thrift_thriftnb_thriftnb_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(thrift_thriftnb_thriftnb_FRAMEWORKS_FOUND_RELEASE "${thrift_thriftnb_thriftnb_FRAMEWORKS_RELEASE}" "${thrift_thriftnb_thriftnb_FRAMEWORK_DIRS_RELEASE}")

        set(thrift_thriftnb_thriftnb_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET thrift_thriftnb_thriftnb_DEPS_TARGET)
            add_library(thrift_thriftnb_thriftnb_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET thrift_thriftnb_thriftnb_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${thrift_thriftnb_thriftnb_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${thrift_thriftnb_thriftnb_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${thrift_thriftnb_thriftnb_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'thrift_thriftnb_thriftnb_DEPS_TARGET' to all of them
        conan_package_library_targets("${thrift_thriftnb_thriftnb_LIBS_RELEASE}"
                              "${thrift_thriftnb_thriftnb_LIB_DIRS_RELEASE}"
                              "${thrift_thriftnb_thriftnb_BIN_DIRS_RELEASE}" # package_bindir
                              "${thrift_thriftnb_thriftnb_LIBRARY_TYPE_RELEASE}"
                              "${thrift_thriftnb_thriftnb_IS_HOST_WINDOWS_RELEASE}"
                              thrift_thriftnb_thriftnb_DEPS_TARGET
                              thrift_thriftnb_thriftnb_LIBRARIES_TARGETS
                              "_RELEASE"
                              "thrift_thriftnb_thriftnb"
                              "${thrift_thriftnb_thriftnb_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET thriftnb::thriftnb
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${thrift_thriftnb_thriftnb_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${thrift_thriftnb_thriftnb_LIBRARIES_TARGETS}>
                     )

        if("${thrift_thriftnb_thriftnb_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET thriftnb::thriftnb
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         thrift_thriftnb_thriftnb_DEPS_TARGET)
        endif()

        set_property(TARGET thriftnb::thriftnb APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${thrift_thriftnb_thriftnb_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET thriftnb::thriftnb APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${thrift_thriftnb_thriftnb_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET thriftnb::thriftnb APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${thrift_thriftnb_thriftnb_LIB_DIRS_RELEASE}>)
        set_property(TARGET thriftnb::thriftnb APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${thrift_thriftnb_thriftnb_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET thriftnb::thriftnb APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${thrift_thriftnb_thriftnb_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT thriftz::thriftz #############

        set(thrift_thriftz_thriftz_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(thrift_thriftz_thriftz_FRAMEWORKS_FOUND_RELEASE "${thrift_thriftz_thriftz_FRAMEWORKS_RELEASE}" "${thrift_thriftz_thriftz_FRAMEWORK_DIRS_RELEASE}")

        set(thrift_thriftz_thriftz_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET thrift_thriftz_thriftz_DEPS_TARGET)
            add_library(thrift_thriftz_thriftz_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET thrift_thriftz_thriftz_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${thrift_thriftz_thriftz_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${thrift_thriftz_thriftz_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${thrift_thriftz_thriftz_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'thrift_thriftz_thriftz_DEPS_TARGET' to all of them
        conan_package_library_targets("${thrift_thriftz_thriftz_LIBS_RELEASE}"
                              "${thrift_thriftz_thriftz_LIB_DIRS_RELEASE}"
                              "${thrift_thriftz_thriftz_BIN_DIRS_RELEASE}" # package_bindir
                              "${thrift_thriftz_thriftz_LIBRARY_TYPE_RELEASE}"
                              "${thrift_thriftz_thriftz_IS_HOST_WINDOWS_RELEASE}"
                              thrift_thriftz_thriftz_DEPS_TARGET
                              thrift_thriftz_thriftz_LIBRARIES_TARGETS
                              "_RELEASE"
                              "thrift_thriftz_thriftz"
                              "${thrift_thriftz_thriftz_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET thriftz::thriftz
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${thrift_thriftz_thriftz_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${thrift_thriftz_thriftz_LIBRARIES_TARGETS}>
                     )

        if("${thrift_thriftz_thriftz_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET thriftz::thriftz
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         thrift_thriftz_thriftz_DEPS_TARGET)
        endif()

        set_property(TARGET thriftz::thriftz APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${thrift_thriftz_thriftz_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET thriftz::thriftz APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${thrift_thriftz_thriftz_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET thriftz::thriftz APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${thrift_thriftz_thriftz_LIB_DIRS_RELEASE}>)
        set_property(TARGET thriftz::thriftz APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${thrift_thriftz_thriftz_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET thriftz::thriftz APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${thrift_thriftz_thriftz_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT thrift::thrift #############

        set(thrift_thrift_thrift_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(thrift_thrift_thrift_FRAMEWORKS_FOUND_RELEASE "${thrift_thrift_thrift_FRAMEWORKS_RELEASE}" "${thrift_thrift_thrift_FRAMEWORK_DIRS_RELEASE}")

        set(thrift_thrift_thrift_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET thrift_thrift_thrift_DEPS_TARGET)
            add_library(thrift_thrift_thrift_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET thrift_thrift_thrift_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${thrift_thrift_thrift_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${thrift_thrift_thrift_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${thrift_thrift_thrift_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'thrift_thrift_thrift_DEPS_TARGET' to all of them
        conan_package_library_targets("${thrift_thrift_thrift_LIBS_RELEASE}"
                              "${thrift_thrift_thrift_LIB_DIRS_RELEASE}"
                              "${thrift_thrift_thrift_BIN_DIRS_RELEASE}" # package_bindir
                              "${thrift_thrift_thrift_LIBRARY_TYPE_RELEASE}"
                              "${thrift_thrift_thrift_IS_HOST_WINDOWS_RELEASE}"
                              thrift_thrift_thrift_DEPS_TARGET
                              thrift_thrift_thrift_LIBRARIES_TARGETS
                              "_RELEASE"
                              "thrift_thrift_thrift"
                              "${thrift_thrift_thrift_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET thrift::thrift
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${thrift_thrift_thrift_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${thrift_thrift_thrift_LIBRARIES_TARGETS}>
                     )

        if("${thrift_thrift_thrift_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET thrift::thrift
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         thrift_thrift_thrift_DEPS_TARGET)
        endif()

        set_property(TARGET thrift::thrift APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${thrift_thrift_thrift_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET thrift::thrift APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${thrift_thrift_thrift_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET thrift::thrift APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${thrift_thrift_thrift_LIB_DIRS_RELEASE}>)
        set_property(TARGET thrift::thrift APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${thrift_thrift_thrift_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET thrift::thrift APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${thrift_thrift_thrift_COMPILE_OPTIONS_RELEASE}>)


    ########## AGGREGATED GLOBAL TARGET WITH THE COMPONENTS #####################
    set_property(TARGET thrift::thrift-conan-do-not-use APPEND PROPERTY INTERFACE_LINK_LIBRARIES thriftnb::thriftnb)
    set_property(TARGET thrift::thrift-conan-do-not-use APPEND PROPERTY INTERFACE_LINK_LIBRARIES thriftz::thriftz)
    set_property(TARGET thrift::thrift-conan-do-not-use APPEND PROPERTY INTERFACE_LINK_LIBRARIES thrift::thrift)

########## For the modules (FindXXX)
set(thrift_LIBRARIES_RELEASE thrift::thrift-conan-do-not-use)
