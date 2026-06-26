# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(arrow_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(arrow_FRAMEWORKS_FOUND_RELEASE "${arrow_FRAMEWORKS_RELEASE}" "${arrow_FRAMEWORK_DIRS_RELEASE}")

set(arrow_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET arrow_DEPS_TARGET)
    add_library(arrow_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET arrow_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${arrow_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${arrow_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:boost::boost;thrift::thrift-conan-do-not-use;ZLIB::ZLIB;Arrow::arrow_static;ArrowCompute::arrow_compute_static>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### arrow_DEPS_TARGET to all of them
conan_package_library_targets("${arrow_LIBS_RELEASE}"    # libraries
                              "${arrow_LIB_DIRS_RELEASE}" # package_libdir
                              "${arrow_BIN_DIRS_RELEASE}" # package_bindir
                              "${arrow_LIBRARY_TYPE_RELEASE}"
                              "${arrow_IS_HOST_WINDOWS_RELEASE}"
                              arrow_DEPS_TARGET
                              arrow_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "arrow"    # package_name
                              "${arrow_NO_SONAME_MODE_RELEASE}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${arrow_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## COMPONENTS TARGET PROPERTIES Release ########################################

    ########## COMPONENT ArrowAcero::arrow_acero_static #############

        set(arrow_ArrowAcero_arrow_acero_static_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(arrow_ArrowAcero_arrow_acero_static_FRAMEWORKS_FOUND_RELEASE "${arrow_ArrowAcero_arrow_acero_static_FRAMEWORKS_RELEASE}" "${arrow_ArrowAcero_arrow_acero_static_FRAMEWORK_DIRS_RELEASE}")

        set(arrow_ArrowAcero_arrow_acero_static_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET arrow_ArrowAcero_arrow_acero_static_DEPS_TARGET)
            add_library(arrow_ArrowAcero_arrow_acero_static_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET arrow_ArrowAcero_arrow_acero_static_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${arrow_ArrowAcero_arrow_acero_static_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${arrow_ArrowAcero_arrow_acero_static_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${arrow_ArrowAcero_arrow_acero_static_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'arrow_ArrowAcero_arrow_acero_static_DEPS_TARGET' to all of them
        conan_package_library_targets("${arrow_ArrowAcero_arrow_acero_static_LIBS_RELEASE}"
                              "${arrow_ArrowAcero_arrow_acero_static_LIB_DIRS_RELEASE}"
                              "${arrow_ArrowAcero_arrow_acero_static_BIN_DIRS_RELEASE}" # package_bindir
                              "${arrow_ArrowAcero_arrow_acero_static_LIBRARY_TYPE_RELEASE}"
                              "${arrow_ArrowAcero_arrow_acero_static_IS_HOST_WINDOWS_RELEASE}"
                              arrow_ArrowAcero_arrow_acero_static_DEPS_TARGET
                              arrow_ArrowAcero_arrow_acero_static_LIBRARIES_TARGETS
                              "_RELEASE"
                              "arrow_ArrowAcero_arrow_acero_static"
                              "${arrow_ArrowAcero_arrow_acero_static_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET ArrowAcero::arrow_acero_static
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${arrow_ArrowAcero_arrow_acero_static_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${arrow_ArrowAcero_arrow_acero_static_LIBRARIES_TARGETS}>
                     )

        if("${arrow_ArrowAcero_arrow_acero_static_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET ArrowAcero::arrow_acero_static
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         arrow_ArrowAcero_arrow_acero_static_DEPS_TARGET)
        endif()

        set_property(TARGET ArrowAcero::arrow_acero_static APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${arrow_ArrowAcero_arrow_acero_static_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET ArrowAcero::arrow_acero_static APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${arrow_ArrowAcero_arrow_acero_static_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET ArrowAcero::arrow_acero_static APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${arrow_ArrowAcero_arrow_acero_static_LIB_DIRS_RELEASE}>)
        set_property(TARGET ArrowAcero::arrow_acero_static APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${arrow_ArrowAcero_arrow_acero_static_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET ArrowAcero::arrow_acero_static APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${arrow_ArrowAcero_arrow_acero_static_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT ArrowCompute::arrow_compute_static #############

        set(arrow_ArrowCompute_arrow_compute_static_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(arrow_ArrowCompute_arrow_compute_static_FRAMEWORKS_FOUND_RELEASE "${arrow_ArrowCompute_arrow_compute_static_FRAMEWORKS_RELEASE}" "${arrow_ArrowCompute_arrow_compute_static_FRAMEWORK_DIRS_RELEASE}")

        set(arrow_ArrowCompute_arrow_compute_static_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET arrow_ArrowCompute_arrow_compute_static_DEPS_TARGET)
            add_library(arrow_ArrowCompute_arrow_compute_static_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET arrow_ArrowCompute_arrow_compute_static_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${arrow_ArrowCompute_arrow_compute_static_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${arrow_ArrowCompute_arrow_compute_static_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${arrow_ArrowCompute_arrow_compute_static_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'arrow_ArrowCompute_arrow_compute_static_DEPS_TARGET' to all of them
        conan_package_library_targets("${arrow_ArrowCompute_arrow_compute_static_LIBS_RELEASE}"
                              "${arrow_ArrowCompute_arrow_compute_static_LIB_DIRS_RELEASE}"
                              "${arrow_ArrowCompute_arrow_compute_static_BIN_DIRS_RELEASE}" # package_bindir
                              "${arrow_ArrowCompute_arrow_compute_static_LIBRARY_TYPE_RELEASE}"
                              "${arrow_ArrowCompute_arrow_compute_static_IS_HOST_WINDOWS_RELEASE}"
                              arrow_ArrowCompute_arrow_compute_static_DEPS_TARGET
                              arrow_ArrowCompute_arrow_compute_static_LIBRARIES_TARGETS
                              "_RELEASE"
                              "arrow_ArrowCompute_arrow_compute_static"
                              "${arrow_ArrowCompute_arrow_compute_static_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET ArrowCompute::arrow_compute_static
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${arrow_ArrowCompute_arrow_compute_static_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${arrow_ArrowCompute_arrow_compute_static_LIBRARIES_TARGETS}>
                     )

        if("${arrow_ArrowCompute_arrow_compute_static_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET ArrowCompute::arrow_compute_static
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         arrow_ArrowCompute_arrow_compute_static_DEPS_TARGET)
        endif()

        set_property(TARGET ArrowCompute::arrow_compute_static APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${arrow_ArrowCompute_arrow_compute_static_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET ArrowCompute::arrow_compute_static APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${arrow_ArrowCompute_arrow_compute_static_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET ArrowCompute::arrow_compute_static APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${arrow_ArrowCompute_arrow_compute_static_LIB_DIRS_RELEASE}>)
        set_property(TARGET ArrowCompute::arrow_compute_static APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${arrow_ArrowCompute_arrow_compute_static_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET ArrowCompute::arrow_compute_static APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${arrow_ArrowCompute_arrow_compute_static_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT Parquet::parquet_static #############

        set(arrow_Parquet_parquet_static_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(arrow_Parquet_parquet_static_FRAMEWORKS_FOUND_RELEASE "${arrow_Parquet_parquet_static_FRAMEWORKS_RELEASE}" "${arrow_Parquet_parquet_static_FRAMEWORK_DIRS_RELEASE}")

        set(arrow_Parquet_parquet_static_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET arrow_Parquet_parquet_static_DEPS_TARGET)
            add_library(arrow_Parquet_parquet_static_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET arrow_Parquet_parquet_static_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${arrow_Parquet_parquet_static_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${arrow_Parquet_parquet_static_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${arrow_Parquet_parquet_static_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'arrow_Parquet_parquet_static_DEPS_TARGET' to all of them
        conan_package_library_targets("${arrow_Parquet_parquet_static_LIBS_RELEASE}"
                              "${arrow_Parquet_parquet_static_LIB_DIRS_RELEASE}"
                              "${arrow_Parquet_parquet_static_BIN_DIRS_RELEASE}" # package_bindir
                              "${arrow_Parquet_parquet_static_LIBRARY_TYPE_RELEASE}"
                              "${arrow_Parquet_parquet_static_IS_HOST_WINDOWS_RELEASE}"
                              arrow_Parquet_parquet_static_DEPS_TARGET
                              arrow_Parquet_parquet_static_LIBRARIES_TARGETS
                              "_RELEASE"
                              "arrow_Parquet_parquet_static"
                              "${arrow_Parquet_parquet_static_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Parquet::parquet_static
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${arrow_Parquet_parquet_static_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${arrow_Parquet_parquet_static_LIBRARIES_TARGETS}>
                     )

        if("${arrow_Parquet_parquet_static_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Parquet::parquet_static
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         arrow_Parquet_parquet_static_DEPS_TARGET)
        endif()

        set_property(TARGET Parquet::parquet_static APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${arrow_Parquet_parquet_static_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Parquet::parquet_static APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${arrow_Parquet_parquet_static_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Parquet::parquet_static APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${arrow_Parquet_parquet_static_LIB_DIRS_RELEASE}>)
        set_property(TARGET Parquet::parquet_static APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${arrow_Parquet_parquet_static_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Parquet::parquet_static APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${arrow_Parquet_parquet_static_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT Arrow::arrow_static #############

        set(arrow_Arrow_arrow_static_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(arrow_Arrow_arrow_static_FRAMEWORKS_FOUND_RELEASE "${arrow_Arrow_arrow_static_FRAMEWORKS_RELEASE}" "${arrow_Arrow_arrow_static_FRAMEWORK_DIRS_RELEASE}")

        set(arrow_Arrow_arrow_static_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET arrow_Arrow_arrow_static_DEPS_TARGET)
            add_library(arrow_Arrow_arrow_static_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET arrow_Arrow_arrow_static_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${arrow_Arrow_arrow_static_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${arrow_Arrow_arrow_static_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${arrow_Arrow_arrow_static_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'arrow_Arrow_arrow_static_DEPS_TARGET' to all of them
        conan_package_library_targets("${arrow_Arrow_arrow_static_LIBS_RELEASE}"
                              "${arrow_Arrow_arrow_static_LIB_DIRS_RELEASE}"
                              "${arrow_Arrow_arrow_static_BIN_DIRS_RELEASE}" # package_bindir
                              "${arrow_Arrow_arrow_static_LIBRARY_TYPE_RELEASE}"
                              "${arrow_Arrow_arrow_static_IS_HOST_WINDOWS_RELEASE}"
                              arrow_Arrow_arrow_static_DEPS_TARGET
                              arrow_Arrow_arrow_static_LIBRARIES_TARGETS
                              "_RELEASE"
                              "arrow_Arrow_arrow_static"
                              "${arrow_Arrow_arrow_static_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Arrow::arrow_static
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${arrow_Arrow_arrow_static_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${arrow_Arrow_arrow_static_LIBRARIES_TARGETS}>
                     )

        if("${arrow_Arrow_arrow_static_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Arrow::arrow_static
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         arrow_Arrow_arrow_static_DEPS_TARGET)
        endif()

        set_property(TARGET Arrow::arrow_static APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${arrow_Arrow_arrow_static_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Arrow::arrow_static APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${arrow_Arrow_arrow_static_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Arrow::arrow_static APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${arrow_Arrow_arrow_static_LIB_DIRS_RELEASE}>)
        set_property(TARGET Arrow::arrow_static APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${arrow_Arrow_arrow_static_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Arrow::arrow_static APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${arrow_Arrow_arrow_static_COMPILE_OPTIONS_RELEASE}>)


    ########## AGGREGATED GLOBAL TARGET WITH THE COMPONENTS #####################
    set_property(TARGET arrow::arrow APPEND PROPERTY INTERFACE_LINK_LIBRARIES ArrowAcero::arrow_acero_static)
    set_property(TARGET arrow::arrow APPEND PROPERTY INTERFACE_LINK_LIBRARIES ArrowCompute::arrow_compute_static)
    set_property(TARGET arrow::arrow APPEND PROPERTY INTERFACE_LINK_LIBRARIES Parquet::parquet_static)
    set_property(TARGET arrow::arrow APPEND PROPERTY INTERFACE_LINK_LIBRARIES Arrow::arrow_static)

########## For the modules (FindXXX)
set(arrow_LIBRARIES_RELEASE arrow::arrow)
