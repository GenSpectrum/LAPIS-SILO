# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(libevent_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(libevent_FRAMEWORKS_FOUND_RELEASE "${libevent_FRAMEWORKS_RELEASE}" "${libevent_FRAMEWORK_DIRS_RELEASE}")

set(libevent_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET libevent_DEPS_TARGET)
    add_library(libevent_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET libevent_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${libevent_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${libevent_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:libevent::core;openssl::openssl>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### libevent_DEPS_TARGET to all of them
conan_package_library_targets("${libevent_LIBS_RELEASE}"    # libraries
                              "${libevent_LIB_DIRS_RELEASE}" # package_libdir
                              "${libevent_BIN_DIRS_RELEASE}" # package_bindir
                              "${libevent_LIBRARY_TYPE_RELEASE}"
                              "${libevent_IS_HOST_WINDOWS_RELEASE}"
                              libevent_DEPS_TARGET
                              libevent_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "libevent"    # package_name
                              "${libevent_NO_SONAME_MODE_RELEASE}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${libevent_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## COMPONENTS TARGET PROPERTIES Release ########################################

    ########## COMPONENT libevent::pthreads #############

        set(libevent_libevent_pthreads_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(libevent_libevent_pthreads_FRAMEWORKS_FOUND_RELEASE "${libevent_libevent_pthreads_FRAMEWORKS_RELEASE}" "${libevent_libevent_pthreads_FRAMEWORK_DIRS_RELEASE}")

        set(libevent_libevent_pthreads_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET libevent_libevent_pthreads_DEPS_TARGET)
            add_library(libevent_libevent_pthreads_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET libevent_libevent_pthreads_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${libevent_libevent_pthreads_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${libevent_libevent_pthreads_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${libevent_libevent_pthreads_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'libevent_libevent_pthreads_DEPS_TARGET' to all of them
        conan_package_library_targets("${libevent_libevent_pthreads_LIBS_RELEASE}"
                              "${libevent_libevent_pthreads_LIB_DIRS_RELEASE}"
                              "${libevent_libevent_pthreads_BIN_DIRS_RELEASE}" # package_bindir
                              "${libevent_libevent_pthreads_LIBRARY_TYPE_RELEASE}"
                              "${libevent_libevent_pthreads_IS_HOST_WINDOWS_RELEASE}"
                              libevent_libevent_pthreads_DEPS_TARGET
                              libevent_libevent_pthreads_LIBRARIES_TARGETS
                              "_RELEASE"
                              "libevent_libevent_pthreads"
                              "${libevent_libevent_pthreads_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET libevent::pthreads
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${libevent_libevent_pthreads_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${libevent_libevent_pthreads_LIBRARIES_TARGETS}>
                     )

        if("${libevent_libevent_pthreads_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET libevent::pthreads
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         libevent_libevent_pthreads_DEPS_TARGET)
        endif()

        set_property(TARGET libevent::pthreads APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${libevent_libevent_pthreads_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET libevent::pthreads APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${libevent_libevent_pthreads_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET libevent::pthreads APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${libevent_libevent_pthreads_LIB_DIRS_RELEASE}>)
        set_property(TARGET libevent::pthreads APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${libevent_libevent_pthreads_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET libevent::pthreads APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${libevent_libevent_pthreads_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT libevent::openssl #############

        set(libevent_libevent_openssl_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(libevent_libevent_openssl_FRAMEWORKS_FOUND_RELEASE "${libevent_libevent_openssl_FRAMEWORKS_RELEASE}" "${libevent_libevent_openssl_FRAMEWORK_DIRS_RELEASE}")

        set(libevent_libevent_openssl_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET libevent_libevent_openssl_DEPS_TARGET)
            add_library(libevent_libevent_openssl_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET libevent_libevent_openssl_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${libevent_libevent_openssl_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${libevent_libevent_openssl_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${libevent_libevent_openssl_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'libevent_libevent_openssl_DEPS_TARGET' to all of them
        conan_package_library_targets("${libevent_libevent_openssl_LIBS_RELEASE}"
                              "${libevent_libevent_openssl_LIB_DIRS_RELEASE}"
                              "${libevent_libevent_openssl_BIN_DIRS_RELEASE}" # package_bindir
                              "${libevent_libevent_openssl_LIBRARY_TYPE_RELEASE}"
                              "${libevent_libevent_openssl_IS_HOST_WINDOWS_RELEASE}"
                              libevent_libevent_openssl_DEPS_TARGET
                              libevent_libevent_openssl_LIBRARIES_TARGETS
                              "_RELEASE"
                              "libevent_libevent_openssl"
                              "${libevent_libevent_openssl_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET libevent::openssl
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${libevent_libevent_openssl_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${libevent_libevent_openssl_LIBRARIES_TARGETS}>
                     )

        if("${libevent_libevent_openssl_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET libevent::openssl
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         libevent_libevent_openssl_DEPS_TARGET)
        endif()

        set_property(TARGET libevent::openssl APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${libevent_libevent_openssl_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET libevent::openssl APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${libevent_libevent_openssl_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET libevent::openssl APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${libevent_libevent_openssl_LIB_DIRS_RELEASE}>)
        set_property(TARGET libevent::openssl APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${libevent_libevent_openssl_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET libevent::openssl APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${libevent_libevent_openssl_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT libevent::extra #############

        set(libevent_libevent_extra_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(libevent_libevent_extra_FRAMEWORKS_FOUND_RELEASE "${libevent_libevent_extra_FRAMEWORKS_RELEASE}" "${libevent_libevent_extra_FRAMEWORK_DIRS_RELEASE}")

        set(libevent_libevent_extra_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET libevent_libevent_extra_DEPS_TARGET)
            add_library(libevent_libevent_extra_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET libevent_libevent_extra_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${libevent_libevent_extra_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${libevent_libevent_extra_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${libevent_libevent_extra_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'libevent_libevent_extra_DEPS_TARGET' to all of them
        conan_package_library_targets("${libevent_libevent_extra_LIBS_RELEASE}"
                              "${libevent_libevent_extra_LIB_DIRS_RELEASE}"
                              "${libevent_libevent_extra_BIN_DIRS_RELEASE}" # package_bindir
                              "${libevent_libevent_extra_LIBRARY_TYPE_RELEASE}"
                              "${libevent_libevent_extra_IS_HOST_WINDOWS_RELEASE}"
                              libevent_libevent_extra_DEPS_TARGET
                              libevent_libevent_extra_LIBRARIES_TARGETS
                              "_RELEASE"
                              "libevent_libevent_extra"
                              "${libevent_libevent_extra_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET libevent::extra
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${libevent_libevent_extra_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${libevent_libevent_extra_LIBRARIES_TARGETS}>
                     )

        if("${libevent_libevent_extra_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET libevent::extra
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         libevent_libevent_extra_DEPS_TARGET)
        endif()

        set_property(TARGET libevent::extra APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${libevent_libevent_extra_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET libevent::extra APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${libevent_libevent_extra_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET libevent::extra APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${libevent_libevent_extra_LIB_DIRS_RELEASE}>)
        set_property(TARGET libevent::extra APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${libevent_libevent_extra_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET libevent::extra APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${libevent_libevent_extra_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT libevent::core #############

        set(libevent_libevent_core_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(libevent_libevent_core_FRAMEWORKS_FOUND_RELEASE "${libevent_libevent_core_FRAMEWORKS_RELEASE}" "${libevent_libevent_core_FRAMEWORK_DIRS_RELEASE}")

        set(libevent_libevent_core_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET libevent_libevent_core_DEPS_TARGET)
            add_library(libevent_libevent_core_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET libevent_libevent_core_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${libevent_libevent_core_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${libevent_libevent_core_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${libevent_libevent_core_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'libevent_libevent_core_DEPS_TARGET' to all of them
        conan_package_library_targets("${libevent_libevent_core_LIBS_RELEASE}"
                              "${libevent_libevent_core_LIB_DIRS_RELEASE}"
                              "${libevent_libevent_core_BIN_DIRS_RELEASE}" # package_bindir
                              "${libevent_libevent_core_LIBRARY_TYPE_RELEASE}"
                              "${libevent_libevent_core_IS_HOST_WINDOWS_RELEASE}"
                              libevent_libevent_core_DEPS_TARGET
                              libevent_libevent_core_LIBRARIES_TARGETS
                              "_RELEASE"
                              "libevent_libevent_core"
                              "${libevent_libevent_core_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET libevent::core
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${libevent_libevent_core_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${libevent_libevent_core_LIBRARIES_TARGETS}>
                     )

        if("${libevent_libevent_core_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET libevent::core
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         libevent_libevent_core_DEPS_TARGET)
        endif()

        set_property(TARGET libevent::core APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${libevent_libevent_core_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET libevent::core APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${libevent_libevent_core_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET libevent::core APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${libevent_libevent_core_LIB_DIRS_RELEASE}>)
        set_property(TARGET libevent::core APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${libevent_libevent_core_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET libevent::core APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${libevent_libevent_core_COMPILE_OPTIONS_RELEASE}>)


    ########## AGGREGATED GLOBAL TARGET WITH THE COMPONENTS #####################
    set_property(TARGET libevent::libevent APPEND PROPERTY INTERFACE_LINK_LIBRARIES libevent::pthreads)
    set_property(TARGET libevent::libevent APPEND PROPERTY INTERFACE_LINK_LIBRARIES libevent::openssl)
    set_property(TARGET libevent::libevent APPEND PROPERTY INTERFACE_LINK_LIBRARIES libevent::extra)
    set_property(TARGET libevent::libevent APPEND PROPERTY INTERFACE_LINK_LIBRARIES libevent::core)

########## For the modules (FindXXX)
set(libevent_LIBRARIES_RELEASE libevent::libevent)
