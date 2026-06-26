# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(fast_float_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(fast_float_FRAMEWORKS_FOUND_RELEASE "${fast_float_FRAMEWORKS_RELEASE}" "${fast_float_FRAMEWORK_DIRS_RELEASE}")

set(fast_float_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET fast_float_DEPS_TARGET)
    add_library(fast_float_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET fast_float_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${fast_float_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${fast_float_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### fast_float_DEPS_TARGET to all of them
conan_package_library_targets("${fast_float_LIBS_RELEASE}"    # libraries
                              "${fast_float_LIB_DIRS_RELEASE}" # package_libdir
                              "${fast_float_BIN_DIRS_RELEASE}" # package_bindir
                              "${fast_float_LIBRARY_TYPE_RELEASE}"
                              "${fast_float_IS_HOST_WINDOWS_RELEASE}"
                              fast_float_DEPS_TARGET
                              fast_float_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "fast_float"    # package_name
                              "${fast_float_NO_SONAME_MODE_RELEASE}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${fast_float_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## COMPONENTS TARGET PROPERTIES Release ########################################

    ########## COMPONENT FastFloat::fast_float #############

        set(fast_float_FastFloat_fast_float_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(fast_float_FastFloat_fast_float_FRAMEWORKS_FOUND_RELEASE "${fast_float_FastFloat_fast_float_FRAMEWORKS_RELEASE}" "${fast_float_FastFloat_fast_float_FRAMEWORK_DIRS_RELEASE}")

        set(fast_float_FastFloat_fast_float_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET fast_float_FastFloat_fast_float_DEPS_TARGET)
            add_library(fast_float_FastFloat_fast_float_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET fast_float_FastFloat_fast_float_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${fast_float_FastFloat_fast_float_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${fast_float_FastFloat_fast_float_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${fast_float_FastFloat_fast_float_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'fast_float_FastFloat_fast_float_DEPS_TARGET' to all of them
        conan_package_library_targets("${fast_float_FastFloat_fast_float_LIBS_RELEASE}"
                              "${fast_float_FastFloat_fast_float_LIB_DIRS_RELEASE}"
                              "${fast_float_FastFloat_fast_float_BIN_DIRS_RELEASE}" # package_bindir
                              "${fast_float_FastFloat_fast_float_LIBRARY_TYPE_RELEASE}"
                              "${fast_float_FastFloat_fast_float_IS_HOST_WINDOWS_RELEASE}"
                              fast_float_FastFloat_fast_float_DEPS_TARGET
                              fast_float_FastFloat_fast_float_LIBRARIES_TARGETS
                              "_RELEASE"
                              "fast_float_FastFloat_fast_float"
                              "${fast_float_FastFloat_fast_float_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET FastFloat::fast_float
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${fast_float_FastFloat_fast_float_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${fast_float_FastFloat_fast_float_LIBRARIES_TARGETS}>
                     )

        if("${fast_float_FastFloat_fast_float_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET FastFloat::fast_float
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         fast_float_FastFloat_fast_float_DEPS_TARGET)
        endif()

        set_property(TARGET FastFloat::fast_float APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${fast_float_FastFloat_fast_float_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET FastFloat::fast_float APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${fast_float_FastFloat_fast_float_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET FastFloat::fast_float APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${fast_float_FastFloat_fast_float_LIB_DIRS_RELEASE}>)
        set_property(TARGET FastFloat::fast_float APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${fast_float_FastFloat_fast_float_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET FastFloat::fast_float APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${fast_float_FastFloat_fast_float_COMPILE_OPTIONS_RELEASE}>)


    ########## AGGREGATED GLOBAL TARGET WITH THE COMPONENTS #####################
    set_property(TARGET FastFloat::fast_float APPEND PROPERTY INTERFACE_LINK_LIBRARIES FastFloat::fast_float)

########## For the modules (FindXXX)
set(fast_float_LIBRARIES_RELEASE FastFloat::fast_float)
