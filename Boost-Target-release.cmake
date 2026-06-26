# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(boost_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(boost_FRAMEWORKS_FOUND_RELEASE "${boost_FRAMEWORKS_RELEASE}" "${boost_FRAMEWORK_DIRS_RELEASE}")

set(boost_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET boost_DEPS_TARGET)
    add_library(boost_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET boost_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${boost_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${boost_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:Boost::diagnostic_definitions;Boost::disable_autolinking;Boost::dynamic_linking;Boost::headers;boost::_libboost;Boost::random;Boost::regex;BZip2::BZip2;LibLZMA::LibLZMA;ZLIB::ZLIB;zstd::libzstd_static;Boost::system;Boost::serialization>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### boost_DEPS_TARGET to all of them
conan_package_library_targets("${boost_LIBS_RELEASE}"    # libraries
                              "${boost_LIB_DIRS_RELEASE}" # package_libdir
                              "${boost_BIN_DIRS_RELEASE}" # package_bindir
                              "${boost_LIBRARY_TYPE_RELEASE}"
                              "${boost_IS_HOST_WINDOWS_RELEASE}"
                              boost_DEPS_TARGET
                              boost_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "boost"    # package_name
                              "${boost_NO_SONAME_MODE_RELEASE}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${boost_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## COMPONENTS TARGET PROPERTIES Release ########################################

    ########## COMPONENT Boost::iostreams #############

        set(boost_Boost_iostreams_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(boost_Boost_iostreams_FRAMEWORKS_FOUND_RELEASE "${boost_Boost_iostreams_FRAMEWORKS_RELEASE}" "${boost_Boost_iostreams_FRAMEWORK_DIRS_RELEASE}")

        set(boost_Boost_iostreams_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET boost_Boost_iostreams_DEPS_TARGET)
            add_library(boost_Boost_iostreams_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET boost_Boost_iostreams_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${boost_Boost_iostreams_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${boost_Boost_iostreams_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${boost_Boost_iostreams_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'boost_Boost_iostreams_DEPS_TARGET' to all of them
        conan_package_library_targets("${boost_Boost_iostreams_LIBS_RELEASE}"
                              "${boost_Boost_iostreams_LIB_DIRS_RELEASE}"
                              "${boost_Boost_iostreams_BIN_DIRS_RELEASE}" # package_bindir
                              "${boost_Boost_iostreams_LIBRARY_TYPE_RELEASE}"
                              "${boost_Boost_iostreams_IS_HOST_WINDOWS_RELEASE}"
                              boost_Boost_iostreams_DEPS_TARGET
                              boost_Boost_iostreams_LIBRARIES_TARGETS
                              "_RELEASE"
                              "boost_Boost_iostreams"
                              "${boost_Boost_iostreams_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Boost::iostreams
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${boost_Boost_iostreams_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${boost_Boost_iostreams_LIBRARIES_TARGETS}>
                     )

        if("${boost_Boost_iostreams_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Boost::iostreams
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         boost_Boost_iostreams_DEPS_TARGET)
        endif()

        set_property(TARGET Boost::iostreams APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${boost_Boost_iostreams_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Boost::iostreams APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${boost_Boost_iostreams_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Boost::iostreams APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${boost_Boost_iostreams_LIB_DIRS_RELEASE}>)
        set_property(TARGET Boost::iostreams APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${boost_Boost_iostreams_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Boost::iostreams APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${boost_Boost_iostreams_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT Boost::wserialization #############

        set(boost_Boost_wserialization_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(boost_Boost_wserialization_FRAMEWORKS_FOUND_RELEASE "${boost_Boost_wserialization_FRAMEWORKS_RELEASE}" "${boost_Boost_wserialization_FRAMEWORK_DIRS_RELEASE}")

        set(boost_Boost_wserialization_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET boost_Boost_wserialization_DEPS_TARGET)
            add_library(boost_Boost_wserialization_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET boost_Boost_wserialization_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${boost_Boost_wserialization_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${boost_Boost_wserialization_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${boost_Boost_wserialization_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'boost_Boost_wserialization_DEPS_TARGET' to all of them
        conan_package_library_targets("${boost_Boost_wserialization_LIBS_RELEASE}"
                              "${boost_Boost_wserialization_LIB_DIRS_RELEASE}"
                              "${boost_Boost_wserialization_BIN_DIRS_RELEASE}" # package_bindir
                              "${boost_Boost_wserialization_LIBRARY_TYPE_RELEASE}"
                              "${boost_Boost_wserialization_IS_HOST_WINDOWS_RELEASE}"
                              boost_Boost_wserialization_DEPS_TARGET
                              boost_Boost_wserialization_LIBRARIES_TARGETS
                              "_RELEASE"
                              "boost_Boost_wserialization"
                              "${boost_Boost_wserialization_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Boost::wserialization
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${boost_Boost_wserialization_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${boost_Boost_wserialization_LIBRARIES_TARGETS}>
                     )

        if("${boost_Boost_wserialization_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Boost::wserialization
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         boost_Boost_wserialization_DEPS_TARGET)
        endif()

        set_property(TARGET Boost::wserialization APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${boost_Boost_wserialization_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Boost::wserialization APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${boost_Boost_wserialization_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Boost::wserialization APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${boost_Boost_wserialization_LIB_DIRS_RELEASE}>)
        set_property(TARGET Boost::wserialization APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${boost_Boost_wserialization_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Boost::wserialization APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${boost_Boost_wserialization_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT Boost::url #############

        set(boost_Boost_url_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(boost_Boost_url_FRAMEWORKS_FOUND_RELEASE "${boost_Boost_url_FRAMEWORKS_RELEASE}" "${boost_Boost_url_FRAMEWORK_DIRS_RELEASE}")

        set(boost_Boost_url_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET boost_Boost_url_DEPS_TARGET)
            add_library(boost_Boost_url_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET boost_Boost_url_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${boost_Boost_url_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${boost_Boost_url_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${boost_Boost_url_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'boost_Boost_url_DEPS_TARGET' to all of them
        conan_package_library_targets("${boost_Boost_url_LIBS_RELEASE}"
                              "${boost_Boost_url_LIB_DIRS_RELEASE}"
                              "${boost_Boost_url_BIN_DIRS_RELEASE}" # package_bindir
                              "${boost_Boost_url_LIBRARY_TYPE_RELEASE}"
                              "${boost_Boost_url_IS_HOST_WINDOWS_RELEASE}"
                              boost_Boost_url_DEPS_TARGET
                              boost_Boost_url_LIBRARIES_TARGETS
                              "_RELEASE"
                              "boost_Boost_url"
                              "${boost_Boost_url_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Boost::url
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${boost_Boost_url_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${boost_Boost_url_LIBRARIES_TARGETS}>
                     )

        if("${boost_Boost_url_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Boost::url
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         boost_Boost_url_DEPS_TARGET)
        endif()

        set_property(TARGET Boost::url APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${boost_Boost_url_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Boost::url APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${boost_Boost_url_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Boost::url APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${boost_Boost_url_LIB_DIRS_RELEASE}>)
        set_property(TARGET Boost::url APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${boost_Boost_url_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Boost::url APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${boost_Boost_url_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT Boost::random #############

        set(boost_Boost_random_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(boost_Boost_random_FRAMEWORKS_FOUND_RELEASE "${boost_Boost_random_FRAMEWORKS_RELEASE}" "${boost_Boost_random_FRAMEWORK_DIRS_RELEASE}")

        set(boost_Boost_random_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET boost_Boost_random_DEPS_TARGET)
            add_library(boost_Boost_random_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET boost_Boost_random_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${boost_Boost_random_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${boost_Boost_random_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${boost_Boost_random_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'boost_Boost_random_DEPS_TARGET' to all of them
        conan_package_library_targets("${boost_Boost_random_LIBS_RELEASE}"
                              "${boost_Boost_random_LIB_DIRS_RELEASE}"
                              "${boost_Boost_random_BIN_DIRS_RELEASE}" # package_bindir
                              "${boost_Boost_random_LIBRARY_TYPE_RELEASE}"
                              "${boost_Boost_random_IS_HOST_WINDOWS_RELEASE}"
                              boost_Boost_random_DEPS_TARGET
                              boost_Boost_random_LIBRARIES_TARGETS
                              "_RELEASE"
                              "boost_Boost_random"
                              "${boost_Boost_random_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Boost::random
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${boost_Boost_random_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${boost_Boost_random_LIBRARIES_TARGETS}>
                     )

        if("${boost_Boost_random_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Boost::random
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         boost_Boost_random_DEPS_TARGET)
        endif()

        set_property(TARGET Boost::random APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${boost_Boost_random_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Boost::random APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${boost_Boost_random_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Boost::random APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${boost_Boost_random_LIB_DIRS_RELEASE}>)
        set_property(TARGET Boost::random APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${boost_Boost_random_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Boost::random APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${boost_Boost_random_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT Boost::system #############

        set(boost_Boost_system_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(boost_Boost_system_FRAMEWORKS_FOUND_RELEASE "${boost_Boost_system_FRAMEWORKS_RELEASE}" "${boost_Boost_system_FRAMEWORK_DIRS_RELEASE}")

        set(boost_Boost_system_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET boost_Boost_system_DEPS_TARGET)
            add_library(boost_Boost_system_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET boost_Boost_system_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${boost_Boost_system_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${boost_Boost_system_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${boost_Boost_system_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'boost_Boost_system_DEPS_TARGET' to all of them
        conan_package_library_targets("${boost_Boost_system_LIBS_RELEASE}"
                              "${boost_Boost_system_LIB_DIRS_RELEASE}"
                              "${boost_Boost_system_BIN_DIRS_RELEASE}" # package_bindir
                              "${boost_Boost_system_LIBRARY_TYPE_RELEASE}"
                              "${boost_Boost_system_IS_HOST_WINDOWS_RELEASE}"
                              boost_Boost_system_DEPS_TARGET
                              boost_Boost_system_LIBRARIES_TARGETS
                              "_RELEASE"
                              "boost_Boost_system"
                              "${boost_Boost_system_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Boost::system
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${boost_Boost_system_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${boost_Boost_system_LIBRARIES_TARGETS}>
                     )

        if("${boost_Boost_system_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Boost::system
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         boost_Boost_system_DEPS_TARGET)
        endif()

        set_property(TARGET Boost::system APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${boost_Boost_system_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Boost::system APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${boost_Boost_system_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Boost::system APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${boost_Boost_system_LIB_DIRS_RELEASE}>)
        set_property(TARGET Boost::system APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${boost_Boost_system_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Boost::system APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${boost_Boost_system_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT Boost::serialization #############

        set(boost_Boost_serialization_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(boost_Boost_serialization_FRAMEWORKS_FOUND_RELEASE "${boost_Boost_serialization_FRAMEWORKS_RELEASE}" "${boost_Boost_serialization_FRAMEWORK_DIRS_RELEASE}")

        set(boost_Boost_serialization_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET boost_Boost_serialization_DEPS_TARGET)
            add_library(boost_Boost_serialization_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET boost_Boost_serialization_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${boost_Boost_serialization_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${boost_Boost_serialization_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${boost_Boost_serialization_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'boost_Boost_serialization_DEPS_TARGET' to all of them
        conan_package_library_targets("${boost_Boost_serialization_LIBS_RELEASE}"
                              "${boost_Boost_serialization_LIB_DIRS_RELEASE}"
                              "${boost_Boost_serialization_BIN_DIRS_RELEASE}" # package_bindir
                              "${boost_Boost_serialization_LIBRARY_TYPE_RELEASE}"
                              "${boost_Boost_serialization_IS_HOST_WINDOWS_RELEASE}"
                              boost_Boost_serialization_DEPS_TARGET
                              boost_Boost_serialization_LIBRARIES_TARGETS
                              "_RELEASE"
                              "boost_Boost_serialization"
                              "${boost_Boost_serialization_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Boost::serialization
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${boost_Boost_serialization_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${boost_Boost_serialization_LIBRARIES_TARGETS}>
                     )

        if("${boost_Boost_serialization_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Boost::serialization
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         boost_Boost_serialization_DEPS_TARGET)
        endif()

        set_property(TARGET Boost::serialization APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${boost_Boost_serialization_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Boost::serialization APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${boost_Boost_serialization_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Boost::serialization APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${boost_Boost_serialization_LIB_DIRS_RELEASE}>)
        set_property(TARGET Boost::serialization APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${boost_Boost_serialization_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Boost::serialization APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${boost_Boost_serialization_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT Boost::regex #############

        set(boost_Boost_regex_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(boost_Boost_regex_FRAMEWORKS_FOUND_RELEASE "${boost_Boost_regex_FRAMEWORKS_RELEASE}" "${boost_Boost_regex_FRAMEWORK_DIRS_RELEASE}")

        set(boost_Boost_regex_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET boost_Boost_regex_DEPS_TARGET)
            add_library(boost_Boost_regex_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET boost_Boost_regex_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${boost_Boost_regex_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${boost_Boost_regex_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${boost_Boost_regex_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'boost_Boost_regex_DEPS_TARGET' to all of them
        conan_package_library_targets("${boost_Boost_regex_LIBS_RELEASE}"
                              "${boost_Boost_regex_LIB_DIRS_RELEASE}"
                              "${boost_Boost_regex_BIN_DIRS_RELEASE}" # package_bindir
                              "${boost_Boost_regex_LIBRARY_TYPE_RELEASE}"
                              "${boost_Boost_regex_IS_HOST_WINDOWS_RELEASE}"
                              boost_Boost_regex_DEPS_TARGET
                              boost_Boost_regex_LIBRARIES_TARGETS
                              "_RELEASE"
                              "boost_Boost_regex"
                              "${boost_Boost_regex_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Boost::regex
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${boost_Boost_regex_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${boost_Boost_regex_LIBRARIES_TARGETS}>
                     )

        if("${boost_Boost_regex_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Boost::regex
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         boost_Boost_regex_DEPS_TARGET)
        endif()

        set_property(TARGET Boost::regex APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${boost_Boost_regex_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Boost::regex APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${boost_Boost_regex_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Boost::regex APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${boost_Boost_regex_LIB_DIRS_RELEASE}>)
        set_property(TARGET Boost::regex APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${boost_Boost_regex_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Boost::regex APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${boost_Boost_regex_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT Boost::container #############

        set(boost_Boost_container_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(boost_Boost_container_FRAMEWORKS_FOUND_RELEASE "${boost_Boost_container_FRAMEWORKS_RELEASE}" "${boost_Boost_container_FRAMEWORK_DIRS_RELEASE}")

        set(boost_Boost_container_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET boost_Boost_container_DEPS_TARGET)
            add_library(boost_Boost_container_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET boost_Boost_container_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${boost_Boost_container_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${boost_Boost_container_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${boost_Boost_container_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'boost_Boost_container_DEPS_TARGET' to all of them
        conan_package_library_targets("${boost_Boost_container_LIBS_RELEASE}"
                              "${boost_Boost_container_LIB_DIRS_RELEASE}"
                              "${boost_Boost_container_BIN_DIRS_RELEASE}" # package_bindir
                              "${boost_Boost_container_LIBRARY_TYPE_RELEASE}"
                              "${boost_Boost_container_IS_HOST_WINDOWS_RELEASE}"
                              boost_Boost_container_DEPS_TARGET
                              boost_Boost_container_LIBRARIES_TARGETS
                              "_RELEASE"
                              "boost_Boost_container"
                              "${boost_Boost_container_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Boost::container
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${boost_Boost_container_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${boost_Boost_container_LIBRARIES_TARGETS}>
                     )

        if("${boost_Boost_container_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Boost::container
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         boost_Boost_container_DEPS_TARGET)
        endif()

        set_property(TARGET Boost::container APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${boost_Boost_container_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Boost::container APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${boost_Boost_container_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Boost::container APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${boost_Boost_container_LIB_DIRS_RELEASE}>)
        set_property(TARGET Boost::container APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${boost_Boost_container_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Boost::container APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${boost_Boost_container_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT Boost::charconv #############

        set(boost_Boost_charconv_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(boost_Boost_charconv_FRAMEWORKS_FOUND_RELEASE "${boost_Boost_charconv_FRAMEWORKS_RELEASE}" "${boost_Boost_charconv_FRAMEWORK_DIRS_RELEASE}")

        set(boost_Boost_charconv_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET boost_Boost_charconv_DEPS_TARGET)
            add_library(boost_Boost_charconv_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET boost_Boost_charconv_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${boost_Boost_charconv_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${boost_Boost_charconv_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${boost_Boost_charconv_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'boost_Boost_charconv_DEPS_TARGET' to all of them
        conan_package_library_targets("${boost_Boost_charconv_LIBS_RELEASE}"
                              "${boost_Boost_charconv_LIB_DIRS_RELEASE}"
                              "${boost_Boost_charconv_BIN_DIRS_RELEASE}" # package_bindir
                              "${boost_Boost_charconv_LIBRARY_TYPE_RELEASE}"
                              "${boost_Boost_charconv_IS_HOST_WINDOWS_RELEASE}"
                              boost_Boost_charconv_DEPS_TARGET
                              boost_Boost_charconv_LIBRARIES_TARGETS
                              "_RELEASE"
                              "boost_Boost_charconv"
                              "${boost_Boost_charconv_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Boost::charconv
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${boost_Boost_charconv_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${boost_Boost_charconv_LIBRARIES_TARGETS}>
                     )

        if("${boost_Boost_charconv_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Boost::charconv
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         boost_Boost_charconv_DEPS_TARGET)
        endif()

        set_property(TARGET Boost::charconv APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${boost_Boost_charconv_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Boost::charconv APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${boost_Boost_charconv_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Boost::charconv APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${boost_Boost_charconv_LIB_DIRS_RELEASE}>)
        set_property(TARGET Boost::charconv APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${boost_Boost_charconv_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Boost::charconv APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${boost_Boost_charconv_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT boost::_libboost #############

        set(boost_boost__libboost_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(boost_boost__libboost_FRAMEWORKS_FOUND_RELEASE "${boost_boost__libboost_FRAMEWORKS_RELEASE}" "${boost_boost__libboost_FRAMEWORK_DIRS_RELEASE}")

        set(boost_boost__libboost_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET boost_boost__libboost_DEPS_TARGET)
            add_library(boost_boost__libboost_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET boost_boost__libboost_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${boost_boost__libboost_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${boost_boost__libboost_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${boost_boost__libboost_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'boost_boost__libboost_DEPS_TARGET' to all of them
        conan_package_library_targets("${boost_boost__libboost_LIBS_RELEASE}"
                              "${boost_boost__libboost_LIB_DIRS_RELEASE}"
                              "${boost_boost__libboost_BIN_DIRS_RELEASE}" # package_bindir
                              "${boost_boost__libboost_LIBRARY_TYPE_RELEASE}"
                              "${boost_boost__libboost_IS_HOST_WINDOWS_RELEASE}"
                              boost_boost__libboost_DEPS_TARGET
                              boost_boost__libboost_LIBRARIES_TARGETS
                              "_RELEASE"
                              "boost_boost__libboost"
                              "${boost_boost__libboost_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET boost::_libboost
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${boost_boost__libboost_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${boost_boost__libboost_LIBRARIES_TARGETS}>
                     )

        if("${boost_boost__libboost_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET boost::_libboost
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         boost_boost__libboost_DEPS_TARGET)
        endif()

        set_property(TARGET boost::_libboost APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${boost_boost__libboost_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET boost::_libboost APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${boost_boost__libboost_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET boost::_libboost APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${boost_boost__libboost_LIB_DIRS_RELEASE}>)
        set_property(TARGET boost::_libboost APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${boost_boost__libboost_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET boost::_libboost APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${boost_boost__libboost_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT Boost::boost #############

        set(boost_Boost_boost_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(boost_Boost_boost_FRAMEWORKS_FOUND_RELEASE "${boost_Boost_boost_FRAMEWORKS_RELEASE}" "${boost_Boost_boost_FRAMEWORK_DIRS_RELEASE}")

        set(boost_Boost_boost_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET boost_Boost_boost_DEPS_TARGET)
            add_library(boost_Boost_boost_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET boost_Boost_boost_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${boost_Boost_boost_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${boost_Boost_boost_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${boost_Boost_boost_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'boost_Boost_boost_DEPS_TARGET' to all of them
        conan_package_library_targets("${boost_Boost_boost_LIBS_RELEASE}"
                              "${boost_Boost_boost_LIB_DIRS_RELEASE}"
                              "${boost_Boost_boost_BIN_DIRS_RELEASE}" # package_bindir
                              "${boost_Boost_boost_LIBRARY_TYPE_RELEASE}"
                              "${boost_Boost_boost_IS_HOST_WINDOWS_RELEASE}"
                              boost_Boost_boost_DEPS_TARGET
                              boost_Boost_boost_LIBRARIES_TARGETS
                              "_RELEASE"
                              "boost_Boost_boost"
                              "${boost_Boost_boost_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Boost::boost
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${boost_Boost_boost_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${boost_Boost_boost_LIBRARIES_TARGETS}>
                     )

        if("${boost_Boost_boost_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Boost::boost
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         boost_Boost_boost_DEPS_TARGET)
        endif()

        set_property(TARGET Boost::boost APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${boost_Boost_boost_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Boost::boost APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${boost_Boost_boost_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Boost::boost APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${boost_Boost_boost_LIB_DIRS_RELEASE}>)
        set_property(TARGET Boost::boost APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${boost_Boost_boost_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Boost::boost APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${boost_Boost_boost_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT Boost::headers #############

        set(boost_Boost_headers_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(boost_Boost_headers_FRAMEWORKS_FOUND_RELEASE "${boost_Boost_headers_FRAMEWORKS_RELEASE}" "${boost_Boost_headers_FRAMEWORK_DIRS_RELEASE}")

        set(boost_Boost_headers_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET boost_Boost_headers_DEPS_TARGET)
            add_library(boost_Boost_headers_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET boost_Boost_headers_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${boost_Boost_headers_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${boost_Boost_headers_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${boost_Boost_headers_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'boost_Boost_headers_DEPS_TARGET' to all of them
        conan_package_library_targets("${boost_Boost_headers_LIBS_RELEASE}"
                              "${boost_Boost_headers_LIB_DIRS_RELEASE}"
                              "${boost_Boost_headers_BIN_DIRS_RELEASE}" # package_bindir
                              "${boost_Boost_headers_LIBRARY_TYPE_RELEASE}"
                              "${boost_Boost_headers_IS_HOST_WINDOWS_RELEASE}"
                              boost_Boost_headers_DEPS_TARGET
                              boost_Boost_headers_LIBRARIES_TARGETS
                              "_RELEASE"
                              "boost_Boost_headers"
                              "${boost_Boost_headers_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Boost::headers
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${boost_Boost_headers_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${boost_Boost_headers_LIBRARIES_TARGETS}>
                     )

        if("${boost_Boost_headers_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Boost::headers
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         boost_Boost_headers_DEPS_TARGET)
        endif()

        set_property(TARGET Boost::headers APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${boost_Boost_headers_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Boost::headers APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${boost_Boost_headers_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Boost::headers APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${boost_Boost_headers_LIB_DIRS_RELEASE}>)
        set_property(TARGET Boost::headers APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${boost_Boost_headers_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Boost::headers APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${boost_Boost_headers_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT Boost::dynamic_linking #############

        set(boost_Boost_dynamic_linking_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(boost_Boost_dynamic_linking_FRAMEWORKS_FOUND_RELEASE "${boost_Boost_dynamic_linking_FRAMEWORKS_RELEASE}" "${boost_Boost_dynamic_linking_FRAMEWORK_DIRS_RELEASE}")

        set(boost_Boost_dynamic_linking_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET boost_Boost_dynamic_linking_DEPS_TARGET)
            add_library(boost_Boost_dynamic_linking_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET boost_Boost_dynamic_linking_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${boost_Boost_dynamic_linking_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${boost_Boost_dynamic_linking_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${boost_Boost_dynamic_linking_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'boost_Boost_dynamic_linking_DEPS_TARGET' to all of them
        conan_package_library_targets("${boost_Boost_dynamic_linking_LIBS_RELEASE}"
                              "${boost_Boost_dynamic_linking_LIB_DIRS_RELEASE}"
                              "${boost_Boost_dynamic_linking_BIN_DIRS_RELEASE}" # package_bindir
                              "${boost_Boost_dynamic_linking_LIBRARY_TYPE_RELEASE}"
                              "${boost_Boost_dynamic_linking_IS_HOST_WINDOWS_RELEASE}"
                              boost_Boost_dynamic_linking_DEPS_TARGET
                              boost_Boost_dynamic_linking_LIBRARIES_TARGETS
                              "_RELEASE"
                              "boost_Boost_dynamic_linking"
                              "${boost_Boost_dynamic_linking_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Boost::dynamic_linking
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${boost_Boost_dynamic_linking_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${boost_Boost_dynamic_linking_LIBRARIES_TARGETS}>
                     )

        if("${boost_Boost_dynamic_linking_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Boost::dynamic_linking
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         boost_Boost_dynamic_linking_DEPS_TARGET)
        endif()

        set_property(TARGET Boost::dynamic_linking APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${boost_Boost_dynamic_linking_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Boost::dynamic_linking APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${boost_Boost_dynamic_linking_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Boost::dynamic_linking APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${boost_Boost_dynamic_linking_LIB_DIRS_RELEASE}>)
        set_property(TARGET Boost::dynamic_linking APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${boost_Boost_dynamic_linking_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Boost::dynamic_linking APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${boost_Boost_dynamic_linking_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT Boost::disable_autolinking #############

        set(boost_Boost_disable_autolinking_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(boost_Boost_disable_autolinking_FRAMEWORKS_FOUND_RELEASE "${boost_Boost_disable_autolinking_FRAMEWORKS_RELEASE}" "${boost_Boost_disable_autolinking_FRAMEWORK_DIRS_RELEASE}")

        set(boost_Boost_disable_autolinking_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET boost_Boost_disable_autolinking_DEPS_TARGET)
            add_library(boost_Boost_disable_autolinking_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET boost_Boost_disable_autolinking_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${boost_Boost_disable_autolinking_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${boost_Boost_disable_autolinking_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${boost_Boost_disable_autolinking_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'boost_Boost_disable_autolinking_DEPS_TARGET' to all of them
        conan_package_library_targets("${boost_Boost_disable_autolinking_LIBS_RELEASE}"
                              "${boost_Boost_disable_autolinking_LIB_DIRS_RELEASE}"
                              "${boost_Boost_disable_autolinking_BIN_DIRS_RELEASE}" # package_bindir
                              "${boost_Boost_disable_autolinking_LIBRARY_TYPE_RELEASE}"
                              "${boost_Boost_disable_autolinking_IS_HOST_WINDOWS_RELEASE}"
                              boost_Boost_disable_autolinking_DEPS_TARGET
                              boost_Boost_disable_autolinking_LIBRARIES_TARGETS
                              "_RELEASE"
                              "boost_Boost_disable_autolinking"
                              "${boost_Boost_disable_autolinking_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Boost::disable_autolinking
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${boost_Boost_disable_autolinking_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${boost_Boost_disable_autolinking_LIBRARIES_TARGETS}>
                     )

        if("${boost_Boost_disable_autolinking_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Boost::disable_autolinking
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         boost_Boost_disable_autolinking_DEPS_TARGET)
        endif()

        set_property(TARGET Boost::disable_autolinking APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${boost_Boost_disable_autolinking_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Boost::disable_autolinking APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${boost_Boost_disable_autolinking_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Boost::disable_autolinking APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${boost_Boost_disable_autolinking_LIB_DIRS_RELEASE}>)
        set_property(TARGET Boost::disable_autolinking APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${boost_Boost_disable_autolinking_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Boost::disable_autolinking APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${boost_Boost_disable_autolinking_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT Boost::diagnostic_definitions #############

        set(boost_Boost_diagnostic_definitions_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(boost_Boost_diagnostic_definitions_FRAMEWORKS_FOUND_RELEASE "${boost_Boost_diagnostic_definitions_FRAMEWORKS_RELEASE}" "${boost_Boost_diagnostic_definitions_FRAMEWORK_DIRS_RELEASE}")

        set(boost_Boost_diagnostic_definitions_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET boost_Boost_diagnostic_definitions_DEPS_TARGET)
            add_library(boost_Boost_diagnostic_definitions_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET boost_Boost_diagnostic_definitions_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${boost_Boost_diagnostic_definitions_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${boost_Boost_diagnostic_definitions_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${boost_Boost_diagnostic_definitions_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'boost_Boost_diagnostic_definitions_DEPS_TARGET' to all of them
        conan_package_library_targets("${boost_Boost_diagnostic_definitions_LIBS_RELEASE}"
                              "${boost_Boost_diagnostic_definitions_LIB_DIRS_RELEASE}"
                              "${boost_Boost_diagnostic_definitions_BIN_DIRS_RELEASE}" # package_bindir
                              "${boost_Boost_diagnostic_definitions_LIBRARY_TYPE_RELEASE}"
                              "${boost_Boost_diagnostic_definitions_IS_HOST_WINDOWS_RELEASE}"
                              boost_Boost_diagnostic_definitions_DEPS_TARGET
                              boost_Boost_diagnostic_definitions_LIBRARIES_TARGETS
                              "_RELEASE"
                              "boost_Boost_diagnostic_definitions"
                              "${boost_Boost_diagnostic_definitions_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Boost::diagnostic_definitions
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${boost_Boost_diagnostic_definitions_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${boost_Boost_diagnostic_definitions_LIBRARIES_TARGETS}>
                     )

        if("${boost_Boost_diagnostic_definitions_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Boost::diagnostic_definitions
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         boost_Boost_diagnostic_definitions_DEPS_TARGET)
        endif()

        set_property(TARGET Boost::diagnostic_definitions APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${boost_Boost_diagnostic_definitions_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Boost::diagnostic_definitions APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${boost_Boost_diagnostic_definitions_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Boost::diagnostic_definitions APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${boost_Boost_diagnostic_definitions_LIB_DIRS_RELEASE}>)
        set_property(TARGET Boost::diagnostic_definitions APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${boost_Boost_diagnostic_definitions_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Boost::diagnostic_definitions APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${boost_Boost_diagnostic_definitions_COMPILE_OPTIONS_RELEASE}>)


    ########## AGGREGATED GLOBAL TARGET WITH THE COMPONENTS #####################
    set_property(TARGET boost::boost APPEND PROPERTY INTERFACE_LINK_LIBRARIES Boost::iostreams)
    set_property(TARGET boost::boost APPEND PROPERTY INTERFACE_LINK_LIBRARIES Boost::wserialization)
    set_property(TARGET boost::boost APPEND PROPERTY INTERFACE_LINK_LIBRARIES Boost::url)
    set_property(TARGET boost::boost APPEND PROPERTY INTERFACE_LINK_LIBRARIES Boost::random)
    set_property(TARGET boost::boost APPEND PROPERTY INTERFACE_LINK_LIBRARIES Boost::system)
    set_property(TARGET boost::boost APPEND PROPERTY INTERFACE_LINK_LIBRARIES Boost::serialization)
    set_property(TARGET boost::boost APPEND PROPERTY INTERFACE_LINK_LIBRARIES Boost::regex)
    set_property(TARGET boost::boost APPEND PROPERTY INTERFACE_LINK_LIBRARIES Boost::container)
    set_property(TARGET boost::boost APPEND PROPERTY INTERFACE_LINK_LIBRARIES Boost::charconv)
    set_property(TARGET boost::boost APPEND PROPERTY INTERFACE_LINK_LIBRARIES boost::_libboost)
    set_property(TARGET boost::boost APPEND PROPERTY INTERFACE_LINK_LIBRARIES Boost::boost)
    set_property(TARGET boost::boost APPEND PROPERTY INTERFACE_LINK_LIBRARIES Boost::headers)
    set_property(TARGET boost::boost APPEND PROPERTY INTERFACE_LINK_LIBRARIES Boost::dynamic_linking)
    set_property(TARGET boost::boost APPEND PROPERTY INTERFACE_LINK_LIBRARIES Boost::disable_autolinking)
    set_property(TARGET boost::boost APPEND PROPERTY INTERFACE_LINK_LIBRARIES Boost::diagnostic_definitions)

########## For the modules (FindXXX)
set(boost_LIBRARIES_RELEASE boost::boost)
