# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(poco_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(poco_FRAMEWORKS_FOUND_RELEASE "${poco_FRAMEWORKS_RELEASE}" "${poco_FRAMEWORK_DIRS_RELEASE}")

set(poco_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET poco_DEPS_TARGET)
    add_library(poco_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET poco_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${poco_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${poco_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:pcre2::pcre2;ZLIB::ZLIB;utf8proc::utf8proc;Poco::Foundation;Poco::JSON>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### poco_DEPS_TARGET to all of them
conan_package_library_targets("${poco_LIBS_RELEASE}"    # libraries
                              "${poco_LIB_DIRS_RELEASE}" # package_libdir
                              "${poco_BIN_DIRS_RELEASE}" # package_bindir
                              "${poco_LIBRARY_TYPE_RELEASE}"
                              "${poco_IS_HOST_WINDOWS_RELEASE}"
                              poco_DEPS_TARGET
                              poco_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "poco"    # package_name
                              "${poco_NO_SONAME_MODE_RELEASE}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${poco_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## COMPONENTS TARGET PROPERTIES Release ########################################

    ########## COMPONENT Poco::Util #############

        set(poco_Poco_Util_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(poco_Poco_Util_FRAMEWORKS_FOUND_RELEASE "${poco_Poco_Util_FRAMEWORKS_RELEASE}" "${poco_Poco_Util_FRAMEWORK_DIRS_RELEASE}")

        set(poco_Poco_Util_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET poco_Poco_Util_DEPS_TARGET)
            add_library(poco_Poco_Util_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET poco_Poco_Util_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${poco_Poco_Util_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${poco_Poco_Util_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${poco_Poco_Util_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'poco_Poco_Util_DEPS_TARGET' to all of them
        conan_package_library_targets("${poco_Poco_Util_LIBS_RELEASE}"
                              "${poco_Poco_Util_LIB_DIRS_RELEASE}"
                              "${poco_Poco_Util_BIN_DIRS_RELEASE}" # package_bindir
                              "${poco_Poco_Util_LIBRARY_TYPE_RELEASE}"
                              "${poco_Poco_Util_IS_HOST_WINDOWS_RELEASE}"
                              poco_Poco_Util_DEPS_TARGET
                              poco_Poco_Util_LIBRARIES_TARGETS
                              "_RELEASE"
                              "poco_Poco_Util"
                              "${poco_Poco_Util_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Poco::Util
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${poco_Poco_Util_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${poco_Poco_Util_LIBRARIES_TARGETS}>
                     )

        if("${poco_Poco_Util_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Poco::Util
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         poco_Poco_Util_DEPS_TARGET)
        endif()

        set_property(TARGET Poco::Util APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${poco_Poco_Util_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Poco::Util APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${poco_Poco_Util_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Poco::Util APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${poco_Poco_Util_LIB_DIRS_RELEASE}>)
        set_property(TARGET Poco::Util APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${poco_Poco_Util_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Poco::Util APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${poco_Poco_Util_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT Poco::Net #############

        set(poco_Poco_Net_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(poco_Poco_Net_FRAMEWORKS_FOUND_RELEASE "${poco_Poco_Net_FRAMEWORKS_RELEASE}" "${poco_Poco_Net_FRAMEWORK_DIRS_RELEASE}")

        set(poco_Poco_Net_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET poco_Poco_Net_DEPS_TARGET)
            add_library(poco_Poco_Net_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET poco_Poco_Net_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${poco_Poco_Net_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${poco_Poco_Net_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${poco_Poco_Net_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'poco_Poco_Net_DEPS_TARGET' to all of them
        conan_package_library_targets("${poco_Poco_Net_LIBS_RELEASE}"
                              "${poco_Poco_Net_LIB_DIRS_RELEASE}"
                              "${poco_Poco_Net_BIN_DIRS_RELEASE}" # package_bindir
                              "${poco_Poco_Net_LIBRARY_TYPE_RELEASE}"
                              "${poco_Poco_Net_IS_HOST_WINDOWS_RELEASE}"
                              poco_Poco_Net_DEPS_TARGET
                              poco_Poco_Net_LIBRARIES_TARGETS
                              "_RELEASE"
                              "poco_Poco_Net"
                              "${poco_Poco_Net_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Poco::Net
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${poco_Poco_Net_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${poco_Poco_Net_LIBRARIES_TARGETS}>
                     )

        if("${poco_Poco_Net_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Poco::Net
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         poco_Poco_Net_DEPS_TARGET)
        endif()

        set_property(TARGET Poco::Net APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${poco_Poco_Net_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Poco::Net APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${poco_Poco_Net_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Poco::Net APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${poco_Poco_Net_LIB_DIRS_RELEASE}>)
        set_property(TARGET Poco::Net APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${poco_Poco_Net_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Poco::Net APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${poco_Poco_Net_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT Poco::JSON #############

        set(poco_Poco_JSON_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(poco_Poco_JSON_FRAMEWORKS_FOUND_RELEASE "${poco_Poco_JSON_FRAMEWORKS_RELEASE}" "${poco_Poco_JSON_FRAMEWORK_DIRS_RELEASE}")

        set(poco_Poco_JSON_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET poco_Poco_JSON_DEPS_TARGET)
            add_library(poco_Poco_JSON_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET poco_Poco_JSON_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${poco_Poco_JSON_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${poco_Poco_JSON_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${poco_Poco_JSON_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'poco_Poco_JSON_DEPS_TARGET' to all of them
        conan_package_library_targets("${poco_Poco_JSON_LIBS_RELEASE}"
                              "${poco_Poco_JSON_LIB_DIRS_RELEASE}"
                              "${poco_Poco_JSON_BIN_DIRS_RELEASE}" # package_bindir
                              "${poco_Poco_JSON_LIBRARY_TYPE_RELEASE}"
                              "${poco_Poco_JSON_IS_HOST_WINDOWS_RELEASE}"
                              poco_Poco_JSON_DEPS_TARGET
                              poco_Poco_JSON_LIBRARIES_TARGETS
                              "_RELEASE"
                              "poco_Poco_JSON"
                              "${poco_Poco_JSON_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Poco::JSON
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${poco_Poco_JSON_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${poco_Poco_JSON_LIBRARIES_TARGETS}>
                     )

        if("${poco_Poco_JSON_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Poco::JSON
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         poco_Poco_JSON_DEPS_TARGET)
        endif()

        set_property(TARGET Poco::JSON APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${poco_Poco_JSON_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Poco::JSON APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${poco_Poco_JSON_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Poco::JSON APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${poco_Poco_JSON_LIB_DIRS_RELEASE}>)
        set_property(TARGET Poco::JSON APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${poco_Poco_JSON_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Poco::JSON APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${poco_Poco_JSON_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT Poco::Foundation #############

        set(poco_Poco_Foundation_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(poco_Poco_Foundation_FRAMEWORKS_FOUND_RELEASE "${poco_Poco_Foundation_FRAMEWORKS_RELEASE}" "${poco_Poco_Foundation_FRAMEWORK_DIRS_RELEASE}")

        set(poco_Poco_Foundation_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET poco_Poco_Foundation_DEPS_TARGET)
            add_library(poco_Poco_Foundation_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET poco_Poco_Foundation_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${poco_Poco_Foundation_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${poco_Poco_Foundation_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${poco_Poco_Foundation_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'poco_Poco_Foundation_DEPS_TARGET' to all of them
        conan_package_library_targets("${poco_Poco_Foundation_LIBS_RELEASE}"
                              "${poco_Poco_Foundation_LIB_DIRS_RELEASE}"
                              "${poco_Poco_Foundation_BIN_DIRS_RELEASE}" # package_bindir
                              "${poco_Poco_Foundation_LIBRARY_TYPE_RELEASE}"
                              "${poco_Poco_Foundation_IS_HOST_WINDOWS_RELEASE}"
                              poco_Poco_Foundation_DEPS_TARGET
                              poco_Poco_Foundation_LIBRARIES_TARGETS
                              "_RELEASE"
                              "poco_Poco_Foundation"
                              "${poco_Poco_Foundation_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Poco::Foundation
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${poco_Poco_Foundation_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${poco_Poco_Foundation_LIBRARIES_TARGETS}>
                     )

        if("${poco_Poco_Foundation_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Poco::Foundation
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         poco_Poco_Foundation_DEPS_TARGET)
        endif()

        set_property(TARGET Poco::Foundation APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${poco_Poco_Foundation_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Poco::Foundation APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${poco_Poco_Foundation_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Poco::Foundation APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${poco_Poco_Foundation_LIB_DIRS_RELEASE}>)
        set_property(TARGET Poco::Foundation APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${poco_Poco_Foundation_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Poco::Foundation APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${poco_Poco_Foundation_COMPILE_OPTIONS_RELEASE}>)


    ########## AGGREGATED GLOBAL TARGET WITH THE COMPONENTS #####################
    set_property(TARGET Poco::Poco APPEND PROPERTY INTERFACE_LINK_LIBRARIES Poco::Util)
    set_property(TARGET Poco::Poco APPEND PROPERTY INTERFACE_LINK_LIBRARIES Poco::Net)
    set_property(TARGET Poco::Poco APPEND PROPERTY INTERFACE_LINK_LIBRARIES Poco::JSON)
    set_property(TARGET Poco::Poco APPEND PROPERTY INTERFACE_LINK_LIBRARIES Poco::Foundation)

########## For the modules (FindXXX)
set(poco_LIBRARIES_RELEASE Poco::Poco)
