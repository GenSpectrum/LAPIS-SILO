# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(abseil_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(abseil_FRAMEWORKS_FOUND_RELEASE "${abseil_FRAMEWORKS_RELEASE}" "${abseil_FRAMEWORK_DIRS_RELEASE}")

set(abseil_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET abseil_DEPS_TARGET)
    add_library(abseil_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET abseil_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${abseil_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${abseil_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:absl::config;absl::core_headers;absl::nullability;absl::atomic_hook;absl::errno_saver;absl::log_severity;absl::base_internal;absl::base;absl::dynamic_annotations;absl::raw_logging_internal;absl::type_traits;absl::spinlock_wait;absl::malloc_internal;absl::algorithm;absl::meta;absl::utility;absl::cleanup_internal;absl::common_policy_traits;absl::compare;absl::compressed_tuple;absl::container_common;absl::container_memory;absl::cord;absl::layout;absl::memory;absl::strings;absl::throw_delegate;absl::weakly_mixed_integer;absl::iterator_traits_internal;absl::span;absl::inlined_vector_internal;absl::hash_container_defaults;absl::raw_hash_map;absl::algorithm_container;absl::raw_hash_set;absl::node_slot_policy;absl::hash_function_defaults;absl::hash;absl::exponential_biased;absl::no_destructor;absl::sample_recorder;absl::synchronization;absl::time;absl::hashtable_debug_hooks;absl::bits;absl::endian;absl::function_ref;absl::hash_policy_traits;absl::hashtable_control_bytes;absl::hashtablez_sampler;absl::optional;absl::prefetch;absl::debugging_internal;absl::flat_hash_set;absl::crc_cpu_detect;absl::crc_internal;absl::non_temporal_memcpy;absl::str_format;absl::non_temporal_arm_intrinsics;absl::crc32c;absl::borrowed_fixup_buffer;absl::demangle_internal;absl::stacktrace;absl::symbolize;absl::examine_stack;absl::demangle_rust;absl::bounded_utf8_length_sequence;absl::utf8_for_code_point;absl::decode_rust_punycode;absl::leak_check;absl::flags_path_util;absl::flags_program_name;absl::int128;absl::fast_type_id;absl::flags_commandlineflag_internal;absl::flags_commandlineflag;absl::flags_private_handle_accessor;absl::flags_config;absl::flat_hash_map;absl::flags_marshalling;absl::flags_internal;absl::flags_reflection;absl::flags;absl::flags_usage_internal;absl::flags_usage;absl::any_invocable;absl::city;absl::fixed_array;absl::variant;absl::log_internal_check_op;absl::log_internal_conditions;absl::log_internal_strip;absl::has_ostream_operator;absl::log_internal_nullguard;absl::log_internal_nullstream;absl::log_internal_voidify;absl::log_internal_append_truncated;absl::log_internal_config;absl::log_internal_globals;absl::log_internal_message;absl::absl_vlog_is_on;absl::inlined_vector;absl::log_internal_format;absl::log_internal_proto;absl::log_internal_log_sink_set;absl::log_internal_structured_proto;absl::log_globals;absl::log_entry;absl::log_sink;absl::log_sink_registry;absl::strerror;absl::strings_internal;absl::cleanup;absl::log_internal_check_impl;absl::log_internal_log_impl;absl::log;absl::nullability_traits_internal;absl::log_internal_flags;absl::vlog_config_internal;absl::vlog_is_on;absl::absl_log;absl::log_internal_structured;absl::log_internal_fnmatch;absl::requires_internal;absl::btree;absl::profile_builder;absl::random_distributions;absl::random_internal_nonsecure_base;absl::random_internal_pcg_engine;absl::random_internal_randen_engine;absl::random_seed_sequences;absl::random_internal_distribution_caller;absl::random_internal_fast_uniform_bits;absl::random_internal_generate_real;absl::random_internal_fastmath;absl::random_internal_iostream_state_saver;absl::random_internal_traits;absl::random_internal_uniform_helper;absl::random_internal_wide_multiply;absl::random_internal_entropy_pool;absl::random_internal_salted_seed_seq;absl::random_internal_seed_material;absl::random_seed_gen_exception;absl::string_view;absl::random_internal_platform;absl::random_internal_randen;absl::random_internal_randen_hwaes;absl::random_internal_randen_slow;absl::random_internal_randen_hwaes_impl;absl::status;absl::strings_append_and_overwrite;absl::strings_resize_and_overwrite;absl::charset;absl::str_format_internal;absl::numeric_representation;absl::crc_cord_state;absl::cordz_update_tracker;absl::cord_internal;absl::cordz_functions;absl::cordz_handle;absl::cordz_statistics;absl::cordz_info;absl::cordz_update_scope;absl::log_internal_container;absl::graphcycles_internal;absl::kernel_timeout_internal;absl::tracing_internal;absl::civil_time;absl::time_zone>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### abseil_DEPS_TARGET to all of them
conan_package_library_targets("${abseil_LIBS_RELEASE}"    # libraries
                              "${abseil_LIB_DIRS_RELEASE}" # package_libdir
                              "${abseil_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_LIBRARY_TYPE_RELEASE}"
                              "${abseil_IS_HOST_WINDOWS_RELEASE}"
                              abseil_DEPS_TARGET
                              abseil_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "abseil"    # package_name
                              "${abseil_NO_SONAME_MODE_RELEASE}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${abseil_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## COMPONENTS TARGET PROPERTIES Release ########################################

    ########## COMPONENT absl::flags_parse #############

        set(abseil_absl_flags_parse_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_flags_parse_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_flags_parse_FRAMEWORKS_RELEASE}" "${abseil_absl_flags_parse_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_flags_parse_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_flags_parse_DEPS_TARGET)
            add_library(abseil_absl_flags_parse_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_flags_parse_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_parse_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flags_parse_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flags_parse_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_flags_parse_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_flags_parse_LIBS_RELEASE}"
                              "${abseil_absl_flags_parse_LIB_DIRS_RELEASE}"
                              "${abseil_absl_flags_parse_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_flags_parse_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_flags_parse_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_flags_parse_DEPS_TARGET
                              abseil_absl_flags_parse_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_flags_parse"
                              "${abseil_absl_flags_parse_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::flags_parse
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_parse_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flags_parse_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_flags_parse_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::flags_parse
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_flags_parse_DEPS_TARGET)
        endif()

        set_property(TARGET absl::flags_parse APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_flags_parse_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::flags_parse APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_parse_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::flags_parse APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_parse_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::flags_parse APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_flags_parse_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::flags_parse APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_flags_parse_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::log_flags #############

        set(abseil_absl_log_flags_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_log_flags_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_log_flags_FRAMEWORKS_RELEASE}" "${abseil_absl_log_flags_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_log_flags_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_log_flags_DEPS_TARGET)
            add_library(abseil_absl_log_flags_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_log_flags_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_flags_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_flags_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_flags_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_log_flags_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_log_flags_LIBS_RELEASE}"
                              "${abseil_absl_log_flags_LIB_DIRS_RELEASE}"
                              "${abseil_absl_log_flags_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_log_flags_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_log_flags_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_log_flags_DEPS_TARGET
                              abseil_absl_log_flags_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_log_flags"
                              "${abseil_absl_log_flags_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::log_flags
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_flags_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_flags_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_log_flags_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::log_flags
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_log_flags_DEPS_TARGET)
        endif()

        set_property(TARGET absl::log_flags APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_flags_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::log_flags APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_flags_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::log_flags APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_flags_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::log_flags APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_flags_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::log_flags APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_flags_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::flags_usage #############

        set(abseil_absl_flags_usage_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_flags_usage_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_flags_usage_FRAMEWORKS_RELEASE}" "${abseil_absl_flags_usage_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_flags_usage_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_flags_usage_DEPS_TARGET)
            add_library(abseil_absl_flags_usage_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_flags_usage_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_usage_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flags_usage_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flags_usage_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_flags_usage_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_flags_usage_LIBS_RELEASE}"
                              "${abseil_absl_flags_usage_LIB_DIRS_RELEASE}"
                              "${abseil_absl_flags_usage_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_flags_usage_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_flags_usage_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_flags_usage_DEPS_TARGET
                              abseil_absl_flags_usage_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_flags_usage"
                              "${abseil_absl_flags_usage_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::flags_usage
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_usage_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flags_usage_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_flags_usage_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::flags_usage
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_flags_usage_DEPS_TARGET)
        endif()

        set_property(TARGET absl::flags_usage APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_flags_usage_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::flags_usage APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_usage_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::flags_usage APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_usage_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::flags_usage APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_flags_usage_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::flags_usage APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_flags_usage_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::log_internal_flags #############

        set(abseil_absl_log_internal_flags_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_log_internal_flags_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_log_internal_flags_FRAMEWORKS_RELEASE}" "${abseil_absl_log_internal_flags_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_log_internal_flags_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_log_internal_flags_DEPS_TARGET)
            add_library(abseil_absl_log_internal_flags_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_log_internal_flags_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_flags_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_flags_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_flags_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_log_internal_flags_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_log_internal_flags_LIBS_RELEASE}"
                              "${abseil_absl_log_internal_flags_LIB_DIRS_RELEASE}"
                              "${abseil_absl_log_internal_flags_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_log_internal_flags_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_log_internal_flags_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_log_internal_flags_DEPS_TARGET
                              abseil_absl_log_internal_flags_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_log_internal_flags"
                              "${abseil_absl_log_internal_flags_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::log_internal_flags
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_flags_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_flags_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_log_internal_flags_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::log_internal_flags
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_log_internal_flags_DEPS_TARGET)
        endif()

        set_property(TARGET absl::log_internal_flags APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_flags_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::log_internal_flags APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_flags_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::log_internal_flags APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_flags_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::log_internal_flags APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_flags_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::log_internal_flags APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_flags_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::flags_usage_internal #############

        set(abseil_absl_flags_usage_internal_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_flags_usage_internal_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_flags_usage_internal_FRAMEWORKS_RELEASE}" "${abseil_absl_flags_usage_internal_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_flags_usage_internal_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_flags_usage_internal_DEPS_TARGET)
            add_library(abseil_absl_flags_usage_internal_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_flags_usage_internal_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_usage_internal_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flags_usage_internal_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flags_usage_internal_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_flags_usage_internal_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_flags_usage_internal_LIBS_RELEASE}"
                              "${abseil_absl_flags_usage_internal_LIB_DIRS_RELEASE}"
                              "${abseil_absl_flags_usage_internal_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_flags_usage_internal_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_flags_usage_internal_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_flags_usage_internal_DEPS_TARGET
                              abseil_absl_flags_usage_internal_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_flags_usage_internal"
                              "${abseil_absl_flags_usage_internal_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::flags_usage_internal
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_usage_internal_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flags_usage_internal_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_flags_usage_internal_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::flags_usage_internal
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_flags_usage_internal_DEPS_TARGET)
        endif()

        set_property(TARGET absl::flags_usage_internal APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_flags_usage_internal_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::flags_usage_internal APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_usage_internal_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::flags_usage_internal APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_usage_internal_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::flags_usage_internal APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_flags_usage_internal_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::flags_usage_internal APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_flags_usage_internal_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::hashtable_profiler #############

        set(abseil_absl_hashtable_profiler_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_hashtable_profiler_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_hashtable_profiler_FRAMEWORKS_RELEASE}" "${abseil_absl_hashtable_profiler_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_hashtable_profiler_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_hashtable_profiler_DEPS_TARGET)
            add_library(abseil_absl_hashtable_profiler_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_hashtable_profiler_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_hashtable_profiler_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_hashtable_profiler_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_hashtable_profiler_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_hashtable_profiler_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_hashtable_profiler_LIBS_RELEASE}"
                              "${abseil_absl_hashtable_profiler_LIB_DIRS_RELEASE}"
                              "${abseil_absl_hashtable_profiler_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_hashtable_profiler_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_hashtable_profiler_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_hashtable_profiler_DEPS_TARGET
                              abseil_absl_hashtable_profiler_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_hashtable_profiler"
                              "${abseil_absl_hashtable_profiler_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::hashtable_profiler
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_hashtable_profiler_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_hashtable_profiler_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_hashtable_profiler_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::hashtable_profiler
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_hashtable_profiler_DEPS_TARGET)
        endif()

        set_property(TARGET absl::hashtable_profiler APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_hashtable_profiler_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::hashtable_profiler APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_hashtable_profiler_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::hashtable_profiler APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_hashtable_profiler_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::hashtable_profiler APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_hashtable_profiler_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::hashtable_profiler APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_hashtable_profiler_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::flags #############

        set(abseil_absl_flags_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_flags_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_flags_FRAMEWORKS_RELEASE}" "${abseil_absl_flags_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_flags_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_flags_DEPS_TARGET)
            add_library(abseil_absl_flags_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_flags_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flags_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flags_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_flags_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_flags_LIBS_RELEASE}"
                              "${abseil_absl_flags_LIB_DIRS_RELEASE}"
                              "${abseil_absl_flags_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_flags_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_flags_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_flags_DEPS_TARGET
                              abseil_absl_flags_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_flags"
                              "${abseil_absl_flags_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::flags
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flags_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_flags_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::flags
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_flags_DEPS_TARGET)
        endif()

        set_property(TARGET absl::flags APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_flags_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::flags APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::flags APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::flags APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_flags_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::flags APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_flags_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::profile_builder #############

        set(abseil_absl_profile_builder_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_profile_builder_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_profile_builder_FRAMEWORKS_RELEASE}" "${abseil_absl_profile_builder_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_profile_builder_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_profile_builder_DEPS_TARGET)
            add_library(abseil_absl_profile_builder_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_profile_builder_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_profile_builder_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_profile_builder_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_profile_builder_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_profile_builder_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_profile_builder_LIBS_RELEASE}"
                              "${abseil_absl_profile_builder_LIB_DIRS_RELEASE}"
                              "${abseil_absl_profile_builder_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_profile_builder_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_profile_builder_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_profile_builder_DEPS_TARGET
                              abseil_absl_profile_builder_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_profile_builder"
                              "${abseil_absl_profile_builder_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::profile_builder
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_profile_builder_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_profile_builder_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_profile_builder_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::profile_builder
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_profile_builder_DEPS_TARGET)
        endif()

        set_property(TARGET absl::profile_builder APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_profile_builder_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::profile_builder APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_profile_builder_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::profile_builder APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_profile_builder_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::profile_builder APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_profile_builder_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::profile_builder APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_profile_builder_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::log_streamer #############

        set(abseil_absl_log_streamer_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_log_streamer_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_log_streamer_FRAMEWORKS_RELEASE}" "${abseil_absl_log_streamer_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_log_streamer_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_log_streamer_DEPS_TARGET)
            add_library(abseil_absl_log_streamer_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_log_streamer_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_streamer_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_streamer_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_streamer_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_log_streamer_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_log_streamer_LIBS_RELEASE}"
                              "${abseil_absl_log_streamer_LIB_DIRS_RELEASE}"
                              "${abseil_absl_log_streamer_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_log_streamer_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_log_streamer_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_log_streamer_DEPS_TARGET
                              abseil_absl_log_streamer_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_log_streamer"
                              "${abseil_absl_log_streamer_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::log_streamer
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_streamer_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_streamer_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_log_streamer_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::log_streamer
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_log_streamer_DEPS_TARGET)
        endif()

        set_property(TARGET absl::log_streamer APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_streamer_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::log_streamer APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_streamer_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::log_streamer APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_streamer_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::log_streamer APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_streamer_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::log_streamer APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_streamer_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::die_if_null #############

        set(abseil_absl_die_if_null_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_die_if_null_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_die_if_null_FRAMEWORKS_RELEASE}" "${abseil_absl_die_if_null_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_die_if_null_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_die_if_null_DEPS_TARGET)
            add_library(abseil_absl_die_if_null_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_die_if_null_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_die_if_null_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_die_if_null_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_die_if_null_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_die_if_null_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_die_if_null_LIBS_RELEASE}"
                              "${abseil_absl_die_if_null_LIB_DIRS_RELEASE}"
                              "${abseil_absl_die_if_null_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_die_if_null_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_die_if_null_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_die_if_null_DEPS_TARGET
                              abseil_absl_die_if_null_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_die_if_null"
                              "${abseil_absl_die_if_null_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::die_if_null
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_die_if_null_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_die_if_null_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_die_if_null_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::die_if_null
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_die_if_null_DEPS_TARGET)
        endif()

        set_property(TARGET absl::die_if_null APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_die_if_null_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::die_if_null APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_die_if_null_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::die_if_null APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_die_if_null_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::die_if_null APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_die_if_null_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::die_if_null APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_die_if_null_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::check #############

        set(abseil_absl_check_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_check_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_check_FRAMEWORKS_RELEASE}" "${abseil_absl_check_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_check_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_check_DEPS_TARGET)
            add_library(abseil_absl_check_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_check_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_check_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_check_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_check_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_check_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_check_LIBS_RELEASE}"
                              "${abseil_absl_check_LIB_DIRS_RELEASE}"
                              "${abseil_absl_check_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_check_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_check_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_check_DEPS_TARGET
                              abseil_absl_check_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_check"
                              "${abseil_absl_check_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::check
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_check_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_check_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_check_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::check
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_check_DEPS_TARGET)
        endif()

        set_property(TARGET absl::check APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_check_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::check APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_check_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::check APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_check_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::check APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_check_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::check APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_check_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::absl_check #############

        set(abseil_absl_absl_check_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_absl_check_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_absl_check_FRAMEWORKS_RELEASE}" "${abseil_absl_absl_check_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_absl_check_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_absl_check_DEPS_TARGET)
            add_library(abseil_absl_absl_check_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_absl_check_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_absl_check_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_absl_check_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_absl_check_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_absl_check_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_absl_check_LIBS_RELEASE}"
                              "${abseil_absl_absl_check_LIB_DIRS_RELEASE}"
                              "${abseil_absl_absl_check_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_absl_check_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_absl_check_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_absl_check_DEPS_TARGET
                              abseil_absl_absl_check_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_absl_check"
                              "${abseil_absl_absl_check_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::absl_check
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_absl_check_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_absl_check_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_absl_check_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::absl_check
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_absl_check_DEPS_TARGET)
        endif()

        set_property(TARGET absl::absl_check APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_absl_check_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::absl_check APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_absl_check_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::absl_check APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_absl_check_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::absl_check APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_absl_check_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::absl_check APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_absl_check_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::flags_reflection #############

        set(abseil_absl_flags_reflection_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_flags_reflection_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_flags_reflection_FRAMEWORKS_RELEASE}" "${abseil_absl_flags_reflection_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_flags_reflection_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_flags_reflection_DEPS_TARGET)
            add_library(abseil_absl_flags_reflection_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_flags_reflection_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_reflection_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flags_reflection_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flags_reflection_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_flags_reflection_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_flags_reflection_LIBS_RELEASE}"
                              "${abseil_absl_flags_reflection_LIB_DIRS_RELEASE}"
                              "${abseil_absl_flags_reflection_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_flags_reflection_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_flags_reflection_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_flags_reflection_DEPS_TARGET
                              abseil_absl_flags_reflection_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_flags_reflection"
                              "${abseil_absl_flags_reflection_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::flags_reflection
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_reflection_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flags_reflection_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_flags_reflection_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::flags_reflection
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_flags_reflection_DEPS_TARGET)
        endif()

        set_property(TARGET absl::flags_reflection APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_flags_reflection_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::flags_reflection APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_reflection_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::flags_reflection APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_reflection_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::flags_reflection APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_flags_reflection_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::flags_reflection APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_flags_reflection_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::log #############

        set(abseil_absl_log_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_log_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_log_FRAMEWORKS_RELEASE}" "${abseil_absl_log_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_log_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_log_DEPS_TARGET)
            add_library(abseil_absl_log_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_log_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_log_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_log_LIBS_RELEASE}"
                              "${abseil_absl_log_LIB_DIRS_RELEASE}"
                              "${abseil_absl_log_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_log_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_log_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_log_DEPS_TARGET
                              abseil_absl_log_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_log"
                              "${abseil_absl_log_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::log
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_log_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::log
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_log_DEPS_TARGET)
        endif()

        set_property(TARGET absl::log APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::log APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::log APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::log APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::log APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::absl_log #############

        set(abseil_absl_absl_log_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_absl_log_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_absl_log_FRAMEWORKS_RELEASE}" "${abseil_absl_absl_log_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_absl_log_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_absl_log_DEPS_TARGET)
            add_library(abseil_absl_absl_log_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_absl_log_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_absl_log_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_absl_log_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_absl_log_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_absl_log_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_absl_log_LIBS_RELEASE}"
                              "${abseil_absl_absl_log_LIB_DIRS_RELEASE}"
                              "${abseil_absl_absl_log_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_absl_log_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_absl_log_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_absl_log_DEPS_TARGET
                              abseil_absl_absl_log_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_absl_log"
                              "${abseil_absl_absl_log_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::absl_log
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_absl_log_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_absl_log_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_absl_log_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::absl_log
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_absl_log_DEPS_TARGET)
        endif()

        set_property(TARGET absl::absl_log APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_absl_log_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::absl_log APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_absl_log_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::absl_log APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_absl_log_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::absl_log APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_absl_log_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::absl_log APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_absl_log_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::log_internal_check_impl #############

        set(abseil_absl_log_internal_check_impl_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_log_internal_check_impl_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_log_internal_check_impl_FRAMEWORKS_RELEASE}" "${abseil_absl_log_internal_check_impl_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_log_internal_check_impl_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_log_internal_check_impl_DEPS_TARGET)
            add_library(abseil_absl_log_internal_check_impl_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_log_internal_check_impl_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_check_impl_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_check_impl_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_check_impl_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_log_internal_check_impl_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_log_internal_check_impl_LIBS_RELEASE}"
                              "${abseil_absl_log_internal_check_impl_LIB_DIRS_RELEASE}"
                              "${abseil_absl_log_internal_check_impl_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_log_internal_check_impl_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_log_internal_check_impl_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_log_internal_check_impl_DEPS_TARGET
                              abseil_absl_log_internal_check_impl_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_log_internal_check_impl"
                              "${abseil_absl_log_internal_check_impl_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::log_internal_check_impl
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_check_impl_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_check_impl_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_log_internal_check_impl_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::log_internal_check_impl
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_log_internal_check_impl_DEPS_TARGET)
        endif()

        set_property(TARGET absl::log_internal_check_impl APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_check_impl_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::log_internal_check_impl APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_check_impl_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::log_internal_check_impl APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_check_impl_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::log_internal_check_impl APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_check_impl_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::log_internal_check_impl APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_check_impl_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::linked_hash_map #############

        set(abseil_absl_linked_hash_map_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_linked_hash_map_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_linked_hash_map_FRAMEWORKS_RELEASE}" "${abseil_absl_linked_hash_map_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_linked_hash_map_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_linked_hash_map_DEPS_TARGET)
            add_library(abseil_absl_linked_hash_map_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_linked_hash_map_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_linked_hash_map_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_linked_hash_map_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_linked_hash_map_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_linked_hash_map_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_linked_hash_map_LIBS_RELEASE}"
                              "${abseil_absl_linked_hash_map_LIB_DIRS_RELEASE}"
                              "${abseil_absl_linked_hash_map_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_linked_hash_map_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_linked_hash_map_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_linked_hash_map_DEPS_TARGET
                              abseil_absl_linked_hash_map_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_linked_hash_map"
                              "${abseil_absl_linked_hash_map_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::linked_hash_map
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_linked_hash_map_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_linked_hash_map_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_linked_hash_map_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::linked_hash_map
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_linked_hash_map_DEPS_TARGET)
        endif()

        set_property(TARGET absl::linked_hash_map APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_linked_hash_map_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::linked_hash_map APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_linked_hash_map_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::linked_hash_map APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_linked_hash_map_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::linked_hash_map APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_linked_hash_map_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::linked_hash_map APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_linked_hash_map_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::linked_hash_set #############

        set(abseil_absl_linked_hash_set_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_linked_hash_set_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_linked_hash_set_FRAMEWORKS_RELEASE}" "${abseil_absl_linked_hash_set_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_linked_hash_set_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_linked_hash_set_DEPS_TARGET)
            add_library(abseil_absl_linked_hash_set_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_linked_hash_set_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_linked_hash_set_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_linked_hash_set_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_linked_hash_set_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_linked_hash_set_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_linked_hash_set_LIBS_RELEASE}"
                              "${abseil_absl_linked_hash_set_LIB_DIRS_RELEASE}"
                              "${abseil_absl_linked_hash_set_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_linked_hash_set_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_linked_hash_set_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_linked_hash_set_DEPS_TARGET
                              abseil_absl_linked_hash_set_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_linked_hash_set"
                              "${abseil_absl_linked_hash_set_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::linked_hash_set
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_linked_hash_set_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_linked_hash_set_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_linked_hash_set_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::linked_hash_set
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_linked_hash_set_DEPS_TARGET)
        endif()

        set_property(TARGET absl::linked_hash_set APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_linked_hash_set_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::linked_hash_set APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_linked_hash_set_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::linked_hash_set APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_linked_hash_set_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::linked_hash_set APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_linked_hash_set_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::linked_hash_set APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_linked_hash_set_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::node_hash_map #############

        set(abseil_absl_node_hash_map_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_node_hash_map_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_node_hash_map_FRAMEWORKS_RELEASE}" "${abseil_absl_node_hash_map_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_node_hash_map_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_node_hash_map_DEPS_TARGET)
            add_library(abseil_absl_node_hash_map_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_node_hash_map_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_node_hash_map_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_node_hash_map_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_node_hash_map_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_node_hash_map_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_node_hash_map_LIBS_RELEASE}"
                              "${abseil_absl_node_hash_map_LIB_DIRS_RELEASE}"
                              "${abseil_absl_node_hash_map_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_node_hash_map_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_node_hash_map_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_node_hash_map_DEPS_TARGET
                              abseil_absl_node_hash_map_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_node_hash_map"
                              "${abseil_absl_node_hash_map_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::node_hash_map
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_node_hash_map_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_node_hash_map_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_node_hash_map_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::node_hash_map
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_node_hash_map_DEPS_TARGET)
        endif()

        set_property(TARGET absl::node_hash_map APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_node_hash_map_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::node_hash_map APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_node_hash_map_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::node_hash_map APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_node_hash_map_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::node_hash_map APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_node_hash_map_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::node_hash_map APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_node_hash_map_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::flat_hash_map #############

        set(abseil_absl_flat_hash_map_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_flat_hash_map_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_flat_hash_map_FRAMEWORKS_RELEASE}" "${abseil_absl_flat_hash_map_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_flat_hash_map_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_flat_hash_map_DEPS_TARGET)
            add_library(abseil_absl_flat_hash_map_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_flat_hash_map_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_flat_hash_map_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flat_hash_map_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flat_hash_map_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_flat_hash_map_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_flat_hash_map_LIBS_RELEASE}"
                              "${abseil_absl_flat_hash_map_LIB_DIRS_RELEASE}"
                              "${abseil_absl_flat_hash_map_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_flat_hash_map_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_flat_hash_map_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_flat_hash_map_DEPS_TARGET
                              abseil_absl_flat_hash_map_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_flat_hash_map"
                              "${abseil_absl_flat_hash_map_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::flat_hash_map
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_flat_hash_map_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flat_hash_map_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_flat_hash_map_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::flat_hash_map
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_flat_hash_map_DEPS_TARGET)
        endif()

        set_property(TARGET absl::flat_hash_map APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_flat_hash_map_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::flat_hash_map APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_flat_hash_map_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::flat_hash_map APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_flat_hash_map_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::flat_hash_map APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_flat_hash_map_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::flat_hash_map APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_flat_hash_map_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::log_structured #############

        set(abseil_absl_log_structured_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_log_structured_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_log_structured_FRAMEWORKS_RELEASE}" "${abseil_absl_log_structured_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_log_structured_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_log_structured_DEPS_TARGET)
            add_library(abseil_absl_log_structured_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_log_structured_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_structured_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_structured_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_structured_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_log_structured_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_log_structured_LIBS_RELEASE}"
                              "${abseil_absl_log_structured_LIB_DIRS_RELEASE}"
                              "${abseil_absl_log_structured_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_log_structured_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_log_structured_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_log_structured_DEPS_TARGET
                              abseil_absl_log_structured_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_log_structured"
                              "${abseil_absl_log_structured_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::log_structured
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_structured_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_structured_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_log_structured_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::log_structured
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_log_structured_DEPS_TARGET)
        endif()

        set_property(TARGET absl::log_structured APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_structured_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::log_structured APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_structured_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::log_structured APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_structured_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::log_structured APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_structured_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::log_structured APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_structured_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::log_internal_log_impl #############

        set(abseil_absl_log_internal_log_impl_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_log_internal_log_impl_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_log_internal_log_impl_FRAMEWORKS_RELEASE}" "${abseil_absl_log_internal_log_impl_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_log_internal_log_impl_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_log_internal_log_impl_DEPS_TARGET)
            add_library(abseil_absl_log_internal_log_impl_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_log_internal_log_impl_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_log_impl_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_log_impl_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_log_impl_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_log_internal_log_impl_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_log_internal_log_impl_LIBS_RELEASE}"
                              "${abseil_absl_log_internal_log_impl_LIB_DIRS_RELEASE}"
                              "${abseil_absl_log_internal_log_impl_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_log_internal_log_impl_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_log_internal_log_impl_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_log_internal_log_impl_DEPS_TARGET
                              abseil_absl_log_internal_log_impl_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_log_internal_log_impl"
                              "${abseil_absl_log_internal_log_impl_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::log_internal_log_impl
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_log_impl_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_log_impl_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_log_internal_log_impl_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::log_internal_log_impl
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_log_internal_log_impl_DEPS_TARGET)
        endif()

        set_property(TARGET absl::log_internal_log_impl APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_log_impl_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::log_internal_log_impl APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_log_impl_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::log_internal_log_impl APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_log_impl_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::log_internal_log_impl APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_log_impl_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::log_internal_log_impl APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_log_impl_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::log_internal_check_op #############

        set(abseil_absl_log_internal_check_op_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_log_internal_check_op_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_log_internal_check_op_FRAMEWORKS_RELEASE}" "${abseil_absl_log_internal_check_op_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_log_internal_check_op_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_log_internal_check_op_DEPS_TARGET)
            add_library(abseil_absl_log_internal_check_op_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_log_internal_check_op_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_check_op_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_check_op_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_check_op_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_log_internal_check_op_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_log_internal_check_op_LIBS_RELEASE}"
                              "${abseil_absl_log_internal_check_op_LIB_DIRS_RELEASE}"
                              "${abseil_absl_log_internal_check_op_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_log_internal_check_op_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_log_internal_check_op_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_log_internal_check_op_DEPS_TARGET
                              abseil_absl_log_internal_check_op_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_log_internal_check_op"
                              "${abseil_absl_log_internal_check_op_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::log_internal_check_op
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_check_op_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_check_op_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_log_internal_check_op_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::log_internal_check_op
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_log_internal_check_op_DEPS_TARGET)
        endif()

        set_property(TARGET absl::log_internal_check_op APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_check_op_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::log_internal_check_op APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_check_op_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::log_internal_check_op APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_check_op_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::log_internal_check_op APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_check_op_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::log_internal_check_op APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_check_op_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::raw_hash_map #############

        set(abseil_absl_raw_hash_map_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_raw_hash_map_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_raw_hash_map_FRAMEWORKS_RELEASE}" "${abseil_absl_raw_hash_map_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_raw_hash_map_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_raw_hash_map_DEPS_TARGET)
            add_library(abseil_absl_raw_hash_map_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_raw_hash_map_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_raw_hash_map_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_raw_hash_map_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_raw_hash_map_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_raw_hash_map_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_raw_hash_map_LIBS_RELEASE}"
                              "${abseil_absl_raw_hash_map_LIB_DIRS_RELEASE}"
                              "${abseil_absl_raw_hash_map_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_raw_hash_map_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_raw_hash_map_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_raw_hash_map_DEPS_TARGET
                              abseil_absl_raw_hash_map_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_raw_hash_map"
                              "${abseil_absl_raw_hash_map_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::raw_hash_map
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_raw_hash_map_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_raw_hash_map_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_raw_hash_map_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::raw_hash_map
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_raw_hash_map_DEPS_TARGET)
        endif()

        set_property(TARGET absl::raw_hash_map APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_raw_hash_map_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::raw_hash_map APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_raw_hash_map_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::raw_hash_map APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_raw_hash_map_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::raw_hash_map APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_raw_hash_map_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::raw_hash_map APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_raw_hash_map_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::node_hash_set #############

        set(abseil_absl_node_hash_set_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_node_hash_set_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_node_hash_set_FRAMEWORKS_RELEASE}" "${abseil_absl_node_hash_set_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_node_hash_set_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_node_hash_set_DEPS_TARGET)
            add_library(abseil_absl_node_hash_set_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_node_hash_set_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_node_hash_set_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_node_hash_set_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_node_hash_set_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_node_hash_set_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_node_hash_set_LIBS_RELEASE}"
                              "${abseil_absl_node_hash_set_LIB_DIRS_RELEASE}"
                              "${abseil_absl_node_hash_set_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_node_hash_set_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_node_hash_set_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_node_hash_set_DEPS_TARGET
                              abseil_absl_node_hash_set_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_node_hash_set"
                              "${abseil_absl_node_hash_set_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::node_hash_set
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_node_hash_set_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_node_hash_set_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_node_hash_set_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::node_hash_set
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_node_hash_set_DEPS_TARGET)
        endif()

        set_property(TARGET absl::node_hash_set APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_node_hash_set_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::node_hash_set APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_node_hash_set_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::node_hash_set APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_node_hash_set_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::node_hash_set APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_node_hash_set_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::node_hash_set APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_node_hash_set_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::flat_hash_set #############

        set(abseil_absl_flat_hash_set_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_flat_hash_set_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_flat_hash_set_FRAMEWORKS_RELEASE}" "${abseil_absl_flat_hash_set_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_flat_hash_set_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_flat_hash_set_DEPS_TARGET)
            add_library(abseil_absl_flat_hash_set_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_flat_hash_set_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_flat_hash_set_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flat_hash_set_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flat_hash_set_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_flat_hash_set_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_flat_hash_set_LIBS_RELEASE}"
                              "${abseil_absl_flat_hash_set_LIB_DIRS_RELEASE}"
                              "${abseil_absl_flat_hash_set_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_flat_hash_set_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_flat_hash_set_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_flat_hash_set_DEPS_TARGET
                              abseil_absl_flat_hash_set_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_flat_hash_set"
                              "${abseil_absl_flat_hash_set_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::flat_hash_set
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_flat_hash_set_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flat_hash_set_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_flat_hash_set_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::flat_hash_set
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_flat_hash_set_DEPS_TARGET)
        endif()

        set_property(TARGET absl::flat_hash_set APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_flat_hash_set_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::flat_hash_set APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_flat_hash_set_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::flat_hash_set APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_flat_hash_set_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::flat_hash_set APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_flat_hash_set_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::flat_hash_set APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_flat_hash_set_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::statusor #############

        set(abseil_absl_statusor_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_statusor_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_statusor_FRAMEWORKS_RELEASE}" "${abseil_absl_statusor_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_statusor_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_statusor_DEPS_TARGET)
            add_library(abseil_absl_statusor_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_statusor_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_statusor_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_statusor_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_statusor_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_statusor_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_statusor_LIBS_RELEASE}"
                              "${abseil_absl_statusor_LIB_DIRS_RELEASE}"
                              "${abseil_absl_statusor_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_statusor_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_statusor_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_statusor_DEPS_TARGET
                              abseil_absl_statusor_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_statusor"
                              "${abseil_absl_statusor_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::statusor
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_statusor_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_statusor_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_statusor_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::statusor
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_statusor_DEPS_TARGET)
        endif()

        set_property(TARGET absl::statusor APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_statusor_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::statusor APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_statusor_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::statusor APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_statusor_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::statusor APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_statusor_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::statusor APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_statusor_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::log_internal_structured #############

        set(abseil_absl_log_internal_structured_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_log_internal_structured_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_log_internal_structured_FRAMEWORKS_RELEASE}" "${abseil_absl_log_internal_structured_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_log_internal_structured_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_log_internal_structured_DEPS_TARGET)
            add_library(abseil_absl_log_internal_structured_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_log_internal_structured_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_structured_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_structured_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_structured_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_log_internal_structured_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_log_internal_structured_LIBS_RELEASE}"
                              "${abseil_absl_log_internal_structured_LIB_DIRS_RELEASE}"
                              "${abseil_absl_log_internal_structured_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_log_internal_structured_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_log_internal_structured_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_log_internal_structured_DEPS_TARGET
                              abseil_absl_log_internal_structured_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_log_internal_structured"
                              "${abseil_absl_log_internal_structured_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::log_internal_structured
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_structured_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_structured_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_log_internal_structured_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::log_internal_structured
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_log_internal_structured_DEPS_TARGET)
        endif()

        set_property(TARGET absl::log_internal_structured APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_structured_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::log_internal_structured APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_structured_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::log_internal_structured APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_structured_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::log_internal_structured APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_structured_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::log_internal_structured APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_structured_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::log_internal_strip #############

        set(abseil_absl_log_internal_strip_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_log_internal_strip_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_log_internal_strip_FRAMEWORKS_RELEASE}" "${abseil_absl_log_internal_strip_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_log_internal_strip_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_log_internal_strip_DEPS_TARGET)
            add_library(abseil_absl_log_internal_strip_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_log_internal_strip_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_strip_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_strip_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_strip_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_log_internal_strip_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_log_internal_strip_LIBS_RELEASE}"
                              "${abseil_absl_log_internal_strip_LIB_DIRS_RELEASE}"
                              "${abseil_absl_log_internal_strip_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_log_internal_strip_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_log_internal_strip_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_log_internal_strip_DEPS_TARGET
                              abseil_absl_log_internal_strip_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_log_internal_strip"
                              "${abseil_absl_log_internal_strip_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::log_internal_strip
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_strip_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_strip_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_log_internal_strip_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::log_internal_strip
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_log_internal_strip_DEPS_TARGET)
        endif()

        set_property(TARGET absl::log_internal_strip APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_strip_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::log_internal_strip APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_strip_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::log_internal_strip APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_strip_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::log_internal_strip APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_strip_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::log_internal_strip APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_strip_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::raw_hash_set #############

        set(abseil_absl_raw_hash_set_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_raw_hash_set_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_raw_hash_set_FRAMEWORKS_RELEASE}" "${abseil_absl_raw_hash_set_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_raw_hash_set_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_raw_hash_set_DEPS_TARGET)
            add_library(abseil_absl_raw_hash_set_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_raw_hash_set_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_raw_hash_set_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_raw_hash_set_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_raw_hash_set_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_raw_hash_set_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_raw_hash_set_LIBS_RELEASE}"
                              "${abseil_absl_raw_hash_set_LIB_DIRS_RELEASE}"
                              "${abseil_absl_raw_hash_set_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_raw_hash_set_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_raw_hash_set_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_raw_hash_set_DEPS_TARGET
                              abseil_absl_raw_hash_set_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_raw_hash_set"
                              "${abseil_absl_raw_hash_set_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::raw_hash_set
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_raw_hash_set_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_raw_hash_set_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_raw_hash_set_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::raw_hash_set
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_raw_hash_set_DEPS_TARGET)
        endif()

        set_property(TARGET absl::raw_hash_set APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_raw_hash_set_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::raw_hash_set APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_raw_hash_set_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::raw_hash_set APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_raw_hash_set_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::raw_hash_set APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_raw_hash_set_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::raw_hash_set APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_raw_hash_set_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::hash_container_defaults #############

        set(abseil_absl_hash_container_defaults_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_hash_container_defaults_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_hash_container_defaults_FRAMEWORKS_RELEASE}" "${abseil_absl_hash_container_defaults_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_hash_container_defaults_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_hash_container_defaults_DEPS_TARGET)
            add_library(abseil_absl_hash_container_defaults_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_hash_container_defaults_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_hash_container_defaults_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_hash_container_defaults_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_hash_container_defaults_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_hash_container_defaults_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_hash_container_defaults_LIBS_RELEASE}"
                              "${abseil_absl_hash_container_defaults_LIB_DIRS_RELEASE}"
                              "${abseil_absl_hash_container_defaults_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_hash_container_defaults_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_hash_container_defaults_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_hash_container_defaults_DEPS_TARGET
                              abseil_absl_hash_container_defaults_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_hash_container_defaults"
                              "${abseil_absl_hash_container_defaults_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::hash_container_defaults
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_hash_container_defaults_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_hash_container_defaults_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_hash_container_defaults_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::hash_container_defaults
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_hash_container_defaults_DEPS_TARGET)
        endif()

        set_property(TARGET absl::hash_container_defaults APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_hash_container_defaults_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::hash_container_defaults APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_hash_container_defaults_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::hash_container_defaults APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_hash_container_defaults_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::hash_container_defaults APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_hash_container_defaults_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::hash_container_defaults APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_hash_container_defaults_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::status #############

        set(abseil_absl_status_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_status_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_status_FRAMEWORKS_RELEASE}" "${abseil_absl_status_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_status_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_status_DEPS_TARGET)
            add_library(abseil_absl_status_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_status_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_status_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_status_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_status_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_status_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_status_LIBS_RELEASE}"
                              "${abseil_absl_status_LIB_DIRS_RELEASE}"
                              "${abseil_absl_status_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_status_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_status_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_status_DEPS_TARGET
                              abseil_absl_status_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_status"
                              "${abseil_absl_status_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::status
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_status_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_status_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_status_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::status
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_status_DEPS_TARGET)
        endif()

        set_property(TARGET absl::status APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_status_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::status APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_status_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::status APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_status_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::status APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_status_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::status APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_status_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::log_internal_message #############

        set(abseil_absl_log_internal_message_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_log_internal_message_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_log_internal_message_FRAMEWORKS_RELEASE}" "${abseil_absl_log_internal_message_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_log_internal_message_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_log_internal_message_DEPS_TARGET)
            add_library(abseil_absl_log_internal_message_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_log_internal_message_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_message_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_message_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_message_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_log_internal_message_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_log_internal_message_LIBS_RELEASE}"
                              "${abseil_absl_log_internal_message_LIB_DIRS_RELEASE}"
                              "${abseil_absl_log_internal_message_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_log_internal_message_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_log_internal_message_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_log_internal_message_DEPS_TARGET
                              abseil_absl_log_internal_message_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_log_internal_message"
                              "${abseil_absl_log_internal_message_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::log_internal_message
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_message_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_message_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_log_internal_message_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::log_internal_message
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_log_internal_message_DEPS_TARGET)
        endif()

        set_property(TARGET absl::log_internal_message APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_message_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::log_internal_message APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_message_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::log_internal_message APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_message_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::log_internal_message APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_message_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::log_internal_message APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_message_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::hash_function_defaults #############

        set(abseil_absl_hash_function_defaults_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_hash_function_defaults_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_hash_function_defaults_FRAMEWORKS_RELEASE}" "${abseil_absl_hash_function_defaults_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_hash_function_defaults_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_hash_function_defaults_DEPS_TARGET)
            add_library(abseil_absl_hash_function_defaults_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_hash_function_defaults_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_hash_function_defaults_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_hash_function_defaults_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_hash_function_defaults_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_hash_function_defaults_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_hash_function_defaults_LIBS_RELEASE}"
                              "${abseil_absl_hash_function_defaults_LIB_DIRS_RELEASE}"
                              "${abseil_absl_hash_function_defaults_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_hash_function_defaults_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_hash_function_defaults_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_hash_function_defaults_DEPS_TARGET
                              abseil_absl_hash_function_defaults_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_hash_function_defaults"
                              "${abseil_absl_hash_function_defaults_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::hash_function_defaults
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_hash_function_defaults_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_hash_function_defaults_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_hash_function_defaults_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::hash_function_defaults
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_hash_function_defaults_DEPS_TARGET)
        endif()

        set_property(TARGET absl::hash_function_defaults APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_hash_function_defaults_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::hash_function_defaults APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_hash_function_defaults_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::hash_function_defaults APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_hash_function_defaults_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::hash_function_defaults APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_hash_function_defaults_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::hash_function_defaults APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_hash_function_defaults_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::btree #############

        set(abseil_absl_btree_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_btree_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_btree_FRAMEWORKS_RELEASE}" "${abseil_absl_btree_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_btree_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_btree_DEPS_TARGET)
            add_library(abseil_absl_btree_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_btree_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_btree_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_btree_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_btree_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_btree_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_btree_LIBS_RELEASE}"
                              "${abseil_absl_btree_LIB_DIRS_RELEASE}"
                              "${abseil_absl_btree_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_btree_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_btree_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_btree_DEPS_TARGET
                              abseil_absl_btree_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_btree"
                              "${abseil_absl_btree_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::btree
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_btree_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_btree_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_btree_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::btree
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_btree_DEPS_TARGET)
        endif()

        set_property(TARGET absl::btree APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_btree_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::btree APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_btree_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::btree APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_btree_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::btree APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_btree_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::btree APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_btree_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::cord #############

        set(abseil_absl_cord_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_cord_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_cord_FRAMEWORKS_RELEASE}" "${abseil_absl_cord_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_cord_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_cord_DEPS_TARGET)
            add_library(abseil_absl_cord_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_cord_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_cord_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_cord_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_cord_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_cord_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_cord_LIBS_RELEASE}"
                              "${abseil_absl_cord_LIB_DIRS_RELEASE}"
                              "${abseil_absl_cord_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_cord_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_cord_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_cord_DEPS_TARGET
                              abseil_absl_cord_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_cord"
                              "${abseil_absl_cord_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::cord
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_cord_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_cord_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_cord_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::cord
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_cord_DEPS_TARGET)
        endif()

        set_property(TARGET absl::cord APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_cord_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::cord APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_cord_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::cord APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_cord_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::cord APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_cord_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::cord APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_cord_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::log_sink_registry #############

        set(abseil_absl_log_sink_registry_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_log_sink_registry_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_log_sink_registry_FRAMEWORKS_RELEASE}" "${abseil_absl_log_sink_registry_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_log_sink_registry_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_log_sink_registry_DEPS_TARGET)
            add_library(abseil_absl_log_sink_registry_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_log_sink_registry_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_sink_registry_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_sink_registry_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_sink_registry_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_log_sink_registry_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_log_sink_registry_LIBS_RELEASE}"
                              "${abseil_absl_log_sink_registry_LIB_DIRS_RELEASE}"
                              "${abseil_absl_log_sink_registry_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_log_sink_registry_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_log_sink_registry_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_log_sink_registry_DEPS_TARGET
                              abseil_absl_log_sink_registry_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_log_sink_registry"
                              "${abseil_absl_log_sink_registry_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::log_sink_registry
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_sink_registry_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_sink_registry_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_log_sink_registry_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::log_sink_registry
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_log_sink_registry_DEPS_TARGET)
        endif()

        set_property(TARGET absl::log_sink_registry APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_sink_registry_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::log_sink_registry APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_sink_registry_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::log_sink_registry APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_sink_registry_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::log_sink_registry APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_sink_registry_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::log_sink_registry APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_sink_registry_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::cordz_update_scope #############

        set(abseil_absl_cordz_update_scope_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_cordz_update_scope_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_cordz_update_scope_FRAMEWORKS_RELEASE}" "${abseil_absl_cordz_update_scope_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_cordz_update_scope_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_cordz_update_scope_DEPS_TARGET)
            add_library(abseil_absl_cordz_update_scope_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_cordz_update_scope_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_cordz_update_scope_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_cordz_update_scope_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_cordz_update_scope_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_cordz_update_scope_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_cordz_update_scope_LIBS_RELEASE}"
                              "${abseil_absl_cordz_update_scope_LIB_DIRS_RELEASE}"
                              "${abseil_absl_cordz_update_scope_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_cordz_update_scope_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_cordz_update_scope_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_cordz_update_scope_DEPS_TARGET
                              abseil_absl_cordz_update_scope_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_cordz_update_scope"
                              "${abseil_absl_cordz_update_scope_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::cordz_update_scope
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_cordz_update_scope_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_cordz_update_scope_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_cordz_update_scope_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::cordz_update_scope
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_cordz_update_scope_DEPS_TARGET)
        endif()

        set_property(TARGET absl::cordz_update_scope APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_cordz_update_scope_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::cordz_update_scope APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_cordz_update_scope_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::cordz_update_scope APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_cordz_update_scope_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::cordz_update_scope APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_cordz_update_scope_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::cordz_update_scope APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_cordz_update_scope_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::cordz_sample_token #############

        set(abseil_absl_cordz_sample_token_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_cordz_sample_token_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_cordz_sample_token_FRAMEWORKS_RELEASE}" "${abseil_absl_cordz_sample_token_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_cordz_sample_token_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_cordz_sample_token_DEPS_TARGET)
            add_library(abseil_absl_cordz_sample_token_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_cordz_sample_token_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_cordz_sample_token_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_cordz_sample_token_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_cordz_sample_token_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_cordz_sample_token_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_cordz_sample_token_LIBS_RELEASE}"
                              "${abseil_absl_cordz_sample_token_LIB_DIRS_RELEASE}"
                              "${abseil_absl_cordz_sample_token_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_cordz_sample_token_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_cordz_sample_token_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_cordz_sample_token_DEPS_TARGET
                              abseil_absl_cordz_sample_token_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_cordz_sample_token"
                              "${abseil_absl_cordz_sample_token_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::cordz_sample_token
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_cordz_sample_token_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_cordz_sample_token_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_cordz_sample_token_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::cordz_sample_token
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_cordz_sample_token_DEPS_TARGET)
        endif()

        set_property(TARGET absl::cordz_sample_token APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_cordz_sample_token_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::cordz_sample_token APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_cordz_sample_token_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::cordz_sample_token APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_cordz_sample_token_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::cordz_sample_token APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_cordz_sample_token_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::cordz_sample_token APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_cordz_sample_token_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::random_random #############

        set(abseil_absl_random_random_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_random_random_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_random_random_FRAMEWORKS_RELEASE}" "${abseil_absl_random_random_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_random_random_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_random_random_DEPS_TARGET)
            add_library(abseil_absl_random_random_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_random_random_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_random_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_random_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_random_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_random_random_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_random_random_LIBS_RELEASE}"
                              "${abseil_absl_random_random_LIB_DIRS_RELEASE}"
                              "${abseil_absl_random_random_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_random_random_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_random_random_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_random_random_DEPS_TARGET
                              abseil_absl_random_random_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_random_random"
                              "${abseil_absl_random_random_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::random_random
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_random_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_random_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_random_random_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::random_random
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_random_random_DEPS_TARGET)
        endif()

        set_property(TARGET absl::random_random APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_random_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::random_random APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_random_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::random_random APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_random_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::random_random APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_random_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::random_random APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_random_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::vlog_is_on #############

        set(abseil_absl_vlog_is_on_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_vlog_is_on_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_vlog_is_on_FRAMEWORKS_RELEASE}" "${abseil_absl_vlog_is_on_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_vlog_is_on_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_vlog_is_on_DEPS_TARGET)
            add_library(abseil_absl_vlog_is_on_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_vlog_is_on_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_vlog_is_on_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_vlog_is_on_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_vlog_is_on_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_vlog_is_on_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_vlog_is_on_LIBS_RELEASE}"
                              "${abseil_absl_vlog_is_on_LIB_DIRS_RELEASE}"
                              "${abseil_absl_vlog_is_on_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_vlog_is_on_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_vlog_is_on_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_vlog_is_on_DEPS_TARGET
                              abseil_absl_vlog_is_on_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_vlog_is_on"
                              "${abseil_absl_vlog_is_on_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::vlog_is_on
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_vlog_is_on_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_vlog_is_on_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_vlog_is_on_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::vlog_is_on
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_vlog_is_on_DEPS_TARGET)
        endif()

        set_property(TARGET absl::vlog_is_on APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_vlog_is_on_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::vlog_is_on APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_vlog_is_on_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::vlog_is_on APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_vlog_is_on_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::vlog_is_on APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_vlog_is_on_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::vlog_is_on APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_vlog_is_on_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::log_initialize #############

        set(abseil_absl_log_initialize_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_log_initialize_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_log_initialize_FRAMEWORKS_RELEASE}" "${abseil_absl_log_initialize_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_log_initialize_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_log_initialize_DEPS_TARGET)
            add_library(abseil_absl_log_initialize_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_log_initialize_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_initialize_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_initialize_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_initialize_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_log_initialize_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_log_initialize_LIBS_RELEASE}"
                              "${abseil_absl_log_initialize_LIB_DIRS_RELEASE}"
                              "${abseil_absl_log_initialize_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_log_initialize_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_log_initialize_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_log_initialize_DEPS_TARGET
                              abseil_absl_log_initialize_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_log_initialize"
                              "${abseil_absl_log_initialize_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::log_initialize
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_initialize_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_initialize_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_log_initialize_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::log_initialize
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_log_initialize_DEPS_TARGET)
        endif()

        set_property(TARGET absl::log_initialize APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_initialize_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::log_initialize APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_initialize_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::log_initialize APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_initialize_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::log_initialize APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_initialize_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::log_initialize APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_initialize_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::log_internal_log_sink_set #############

        set(abseil_absl_log_internal_log_sink_set_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_log_internal_log_sink_set_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_log_internal_log_sink_set_FRAMEWORKS_RELEASE}" "${abseil_absl_log_internal_log_sink_set_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_log_internal_log_sink_set_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_log_internal_log_sink_set_DEPS_TARGET)
            add_library(abseil_absl_log_internal_log_sink_set_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_log_internal_log_sink_set_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_log_sink_set_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_log_sink_set_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_log_sink_set_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_log_internal_log_sink_set_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_log_internal_log_sink_set_LIBS_RELEASE}"
                              "${abseil_absl_log_internal_log_sink_set_LIB_DIRS_RELEASE}"
                              "${abseil_absl_log_internal_log_sink_set_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_log_internal_log_sink_set_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_log_internal_log_sink_set_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_log_internal_log_sink_set_DEPS_TARGET
                              abseil_absl_log_internal_log_sink_set_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_log_internal_log_sink_set"
                              "${abseil_absl_log_internal_log_sink_set_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::log_internal_log_sink_set
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_log_sink_set_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_log_sink_set_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_log_internal_log_sink_set_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::log_internal_log_sink_set
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_log_internal_log_sink_set_DEPS_TARGET)
        endif()

        set_property(TARGET absl::log_internal_log_sink_set APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_log_sink_set_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::log_internal_log_sink_set APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_log_sink_set_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::log_internal_log_sink_set APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_log_sink_set_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::log_internal_log_sink_set APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_log_sink_set_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::log_internal_log_sink_set APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_log_sink_set_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::flags_internal #############

        set(abseil_absl_flags_internal_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_flags_internal_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_flags_internal_FRAMEWORKS_RELEASE}" "${abseil_absl_flags_internal_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_flags_internal_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_flags_internal_DEPS_TARGET)
            add_library(abseil_absl_flags_internal_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_flags_internal_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_internal_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flags_internal_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flags_internal_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_flags_internal_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_flags_internal_LIBS_RELEASE}"
                              "${abseil_absl_flags_internal_LIB_DIRS_RELEASE}"
                              "${abseil_absl_flags_internal_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_flags_internal_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_flags_internal_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_flags_internal_DEPS_TARGET
                              abseil_absl_flags_internal_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_flags_internal"
                              "${abseil_absl_flags_internal_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::flags_internal
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_internal_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flags_internal_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_flags_internal_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::flags_internal
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_flags_internal_DEPS_TARGET)
        endif()

        set_property(TARGET absl::flags_internal APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_flags_internal_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::flags_internal APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_internal_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::flags_internal APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_internal_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::flags_internal APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_flags_internal_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::flags_internal APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_flags_internal_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::cordz_info #############

        set(abseil_absl_cordz_info_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_cordz_info_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_cordz_info_FRAMEWORKS_RELEASE}" "${abseil_absl_cordz_info_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_cordz_info_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_cordz_info_DEPS_TARGET)
            add_library(abseil_absl_cordz_info_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_cordz_info_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_cordz_info_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_cordz_info_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_cordz_info_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_cordz_info_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_cordz_info_LIBS_RELEASE}"
                              "${abseil_absl_cordz_info_LIB_DIRS_RELEASE}"
                              "${abseil_absl_cordz_info_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_cordz_info_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_cordz_info_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_cordz_info_DEPS_TARGET
                              abseil_absl_cordz_info_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_cordz_info"
                              "${abseil_absl_cordz_info_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::cordz_info
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_cordz_info_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_cordz_info_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_cordz_info_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::cordz_info
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_cordz_info_DEPS_TARGET)
        endif()

        set_property(TARGET absl::cordz_info APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_cordz_info_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::cordz_info APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_cordz_info_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::cordz_info APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_cordz_info_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::cordz_info APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_cordz_info_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::cordz_info APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_cordz_info_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::random_internal_nonsecure_base #############

        set(abseil_absl_random_internal_nonsecure_base_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_random_internal_nonsecure_base_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_random_internal_nonsecure_base_FRAMEWORKS_RELEASE}" "${abseil_absl_random_internal_nonsecure_base_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_random_internal_nonsecure_base_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_random_internal_nonsecure_base_DEPS_TARGET)
            add_library(abseil_absl_random_internal_nonsecure_base_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_random_internal_nonsecure_base_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_nonsecure_base_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_nonsecure_base_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_nonsecure_base_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_random_internal_nonsecure_base_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_random_internal_nonsecure_base_LIBS_RELEASE}"
                              "${abseil_absl_random_internal_nonsecure_base_LIB_DIRS_RELEASE}"
                              "${abseil_absl_random_internal_nonsecure_base_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_random_internal_nonsecure_base_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_random_internal_nonsecure_base_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_random_internal_nonsecure_base_DEPS_TARGET
                              abseil_absl_random_internal_nonsecure_base_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_random_internal_nonsecure_base"
                              "${abseil_absl_random_internal_nonsecure_base_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::random_internal_nonsecure_base
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_nonsecure_base_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_nonsecure_base_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_random_internal_nonsecure_base_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::random_internal_nonsecure_base
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_random_internal_nonsecure_base_DEPS_TARGET)
        endif()

        set_property(TARGET absl::random_internal_nonsecure_base APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_nonsecure_base_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::random_internal_nonsecure_base APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_nonsecure_base_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::random_internal_nonsecure_base APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_nonsecure_base_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::random_internal_nonsecure_base APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_nonsecure_base_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::random_internal_nonsecure_base APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_nonsecure_base_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::random_seed_sequences #############

        set(abseil_absl_random_seed_sequences_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_random_seed_sequences_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_random_seed_sequences_FRAMEWORKS_RELEASE}" "${abseil_absl_random_seed_sequences_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_random_seed_sequences_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_random_seed_sequences_DEPS_TARGET)
            add_library(abseil_absl_random_seed_sequences_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_random_seed_sequences_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_seed_sequences_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_seed_sequences_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_seed_sequences_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_random_seed_sequences_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_random_seed_sequences_LIBS_RELEASE}"
                              "${abseil_absl_random_seed_sequences_LIB_DIRS_RELEASE}"
                              "${abseil_absl_random_seed_sequences_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_random_seed_sequences_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_random_seed_sequences_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_random_seed_sequences_DEPS_TARGET
                              abseil_absl_random_seed_sequences_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_random_seed_sequences"
                              "${abseil_absl_random_seed_sequences_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::random_seed_sequences
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_seed_sequences_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_seed_sequences_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_random_seed_sequences_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::random_seed_sequences
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_random_seed_sequences_DEPS_TARGET)
        endif()

        set_property(TARGET absl::random_seed_sequences APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_seed_sequences_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::random_seed_sequences APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_seed_sequences_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::random_seed_sequences APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_seed_sequences_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::random_seed_sequences APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_seed_sequences_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::random_seed_sequences APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_seed_sequences_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::absl_vlog_is_on #############

        set(abseil_absl_absl_vlog_is_on_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_absl_vlog_is_on_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_absl_vlog_is_on_FRAMEWORKS_RELEASE}" "${abseil_absl_absl_vlog_is_on_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_absl_vlog_is_on_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_absl_vlog_is_on_DEPS_TARGET)
            add_library(abseil_absl_absl_vlog_is_on_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_absl_vlog_is_on_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_absl_vlog_is_on_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_absl_vlog_is_on_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_absl_vlog_is_on_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_absl_vlog_is_on_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_absl_vlog_is_on_LIBS_RELEASE}"
                              "${abseil_absl_absl_vlog_is_on_LIB_DIRS_RELEASE}"
                              "${abseil_absl_absl_vlog_is_on_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_absl_vlog_is_on_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_absl_vlog_is_on_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_absl_vlog_is_on_DEPS_TARGET
                              abseil_absl_absl_vlog_is_on_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_absl_vlog_is_on"
                              "${abseil_absl_absl_vlog_is_on_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::absl_vlog_is_on
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_absl_vlog_is_on_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_absl_vlog_is_on_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_absl_vlog_is_on_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::absl_vlog_is_on
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_absl_vlog_is_on_DEPS_TARGET)
        endif()

        set_property(TARGET absl::absl_vlog_is_on APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_absl_vlog_is_on_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::absl_vlog_is_on APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_absl_vlog_is_on_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::absl_vlog_is_on APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_absl_vlog_is_on_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::absl_vlog_is_on APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_absl_vlog_is_on_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::absl_vlog_is_on APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_absl_vlog_is_on_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::log_globals #############

        set(abseil_absl_log_globals_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_log_globals_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_log_globals_FRAMEWORKS_RELEASE}" "${abseil_absl_log_globals_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_log_globals_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_log_globals_DEPS_TARGET)
            add_library(abseil_absl_log_globals_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_log_globals_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_globals_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_globals_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_globals_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_log_globals_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_log_globals_LIBS_RELEASE}"
                              "${abseil_absl_log_globals_LIB_DIRS_RELEASE}"
                              "${abseil_absl_log_globals_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_log_globals_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_log_globals_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_log_globals_DEPS_TARGET
                              abseil_absl_log_globals_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_log_globals"
                              "${abseil_absl_log_globals_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::log_globals
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_globals_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_globals_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_log_globals_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::log_globals
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_log_globals_DEPS_TARGET)
        endif()

        set_property(TARGET absl::log_globals APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_globals_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::log_globals APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_globals_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::log_globals APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_globals_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::log_globals APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_globals_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::log_globals APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_globals_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::flags_config #############

        set(abseil_absl_flags_config_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_flags_config_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_flags_config_FRAMEWORKS_RELEASE}" "${abseil_absl_flags_config_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_flags_config_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_flags_config_DEPS_TARGET)
            add_library(abseil_absl_flags_config_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_flags_config_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_config_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flags_config_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flags_config_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_flags_config_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_flags_config_LIBS_RELEASE}"
                              "${abseil_absl_flags_config_LIB_DIRS_RELEASE}"
                              "${abseil_absl_flags_config_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_flags_config_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_flags_config_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_flags_config_DEPS_TARGET
                              abseil_absl_flags_config_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_flags_config"
                              "${abseil_absl_flags_config_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::flags_config
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_config_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flags_config_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_flags_config_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::flags_config
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_flags_config_DEPS_TARGET)
        endif()

        set_property(TARGET absl::flags_config APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_flags_config_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::flags_config APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_config_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::flags_config APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_config_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::flags_config APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_flags_config_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::flags_config APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_flags_config_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::hashtablez_sampler #############

        set(abseil_absl_hashtablez_sampler_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_hashtablez_sampler_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_hashtablez_sampler_FRAMEWORKS_RELEASE}" "${abseil_absl_hashtablez_sampler_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_hashtablez_sampler_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_hashtablez_sampler_DEPS_TARGET)
            add_library(abseil_absl_hashtablez_sampler_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_hashtablez_sampler_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_hashtablez_sampler_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_hashtablez_sampler_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_hashtablez_sampler_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_hashtablez_sampler_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_hashtablez_sampler_LIBS_RELEASE}"
                              "${abseil_absl_hashtablez_sampler_LIB_DIRS_RELEASE}"
                              "${abseil_absl_hashtablez_sampler_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_hashtablez_sampler_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_hashtablez_sampler_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_hashtablez_sampler_DEPS_TARGET
                              abseil_absl_hashtablez_sampler_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_hashtablez_sampler"
                              "${abseil_absl_hashtablez_sampler_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::hashtablez_sampler
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_hashtablez_sampler_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_hashtablez_sampler_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_hashtablez_sampler_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::hashtablez_sampler
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_hashtablez_sampler_DEPS_TARGET)
        endif()

        set_property(TARGET absl::hashtablez_sampler APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_hashtablez_sampler_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::hashtablez_sampler APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_hashtablez_sampler_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::hashtablez_sampler APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_hashtablez_sampler_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::hashtablez_sampler APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_hashtablez_sampler_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::hashtablez_sampler APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_hashtablez_sampler_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::cordz_handle #############

        set(abseil_absl_cordz_handle_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_cordz_handle_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_cordz_handle_FRAMEWORKS_RELEASE}" "${abseil_absl_cordz_handle_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_cordz_handle_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_cordz_handle_DEPS_TARGET)
            add_library(abseil_absl_cordz_handle_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_cordz_handle_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_cordz_handle_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_cordz_handle_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_cordz_handle_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_cordz_handle_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_cordz_handle_LIBS_RELEASE}"
                              "${abseil_absl_cordz_handle_LIB_DIRS_RELEASE}"
                              "${abseil_absl_cordz_handle_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_cordz_handle_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_cordz_handle_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_cordz_handle_DEPS_TARGET
                              abseil_absl_cordz_handle_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_cordz_handle"
                              "${abseil_absl_cordz_handle_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::cordz_handle
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_cordz_handle_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_cordz_handle_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_cordz_handle_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::cordz_handle
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_cordz_handle_DEPS_TARGET)
        endif()

        set_property(TARGET absl::cordz_handle APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_cordz_handle_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::cordz_handle APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_cordz_handle_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::cordz_handle APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_cordz_handle_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::cordz_handle APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_cordz_handle_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::cordz_handle APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_cordz_handle_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::cordz_statistics #############

        set(abseil_absl_cordz_statistics_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_cordz_statistics_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_cordz_statistics_FRAMEWORKS_RELEASE}" "${abseil_absl_cordz_statistics_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_cordz_statistics_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_cordz_statistics_DEPS_TARGET)
            add_library(abseil_absl_cordz_statistics_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_cordz_statistics_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_cordz_statistics_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_cordz_statistics_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_cordz_statistics_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_cordz_statistics_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_cordz_statistics_LIBS_RELEASE}"
                              "${abseil_absl_cordz_statistics_LIB_DIRS_RELEASE}"
                              "${abseil_absl_cordz_statistics_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_cordz_statistics_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_cordz_statistics_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_cordz_statistics_DEPS_TARGET
                              abseil_absl_cordz_statistics_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_cordz_statistics"
                              "${abseil_absl_cordz_statistics_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::cordz_statistics
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_cordz_statistics_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_cordz_statistics_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_cordz_statistics_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::cordz_statistics
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_cordz_statistics_DEPS_TARGET)
        endif()

        set_property(TARGET absl::cordz_statistics APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_cordz_statistics_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::cordz_statistics APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_cordz_statistics_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::cordz_statistics APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_cordz_statistics_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::cordz_statistics APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_cordz_statistics_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::cordz_statistics APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_cordz_statistics_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::cord_internal #############

        set(abseil_absl_cord_internal_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_cord_internal_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_cord_internal_FRAMEWORKS_RELEASE}" "${abseil_absl_cord_internal_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_cord_internal_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_cord_internal_DEPS_TARGET)
            add_library(abseil_absl_cord_internal_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_cord_internal_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_cord_internal_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_cord_internal_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_cord_internal_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_cord_internal_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_cord_internal_LIBS_RELEASE}"
                              "${abseil_absl_cord_internal_LIB_DIRS_RELEASE}"
                              "${abseil_absl_cord_internal_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_cord_internal_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_cord_internal_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_cord_internal_DEPS_TARGET
                              abseil_absl_cord_internal_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_cord_internal"
                              "${abseil_absl_cord_internal_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::cord_internal
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_cord_internal_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_cord_internal_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_cord_internal_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::cord_internal
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_cord_internal_DEPS_TARGET)
        endif()

        set_property(TARGET absl::cord_internal APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_cord_internal_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::cord_internal APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_cord_internal_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::cord_internal APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_cord_internal_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::cord_internal APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_cord_internal_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::cord_internal APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_cord_internal_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::random_internal_entropy_pool #############

        set(abseil_absl_random_internal_entropy_pool_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_random_internal_entropy_pool_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_random_internal_entropy_pool_FRAMEWORKS_RELEASE}" "${abseil_absl_random_internal_entropy_pool_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_random_internal_entropy_pool_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_random_internal_entropy_pool_DEPS_TARGET)
            add_library(abseil_absl_random_internal_entropy_pool_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_random_internal_entropy_pool_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_entropy_pool_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_entropy_pool_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_entropy_pool_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_random_internal_entropy_pool_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_random_internal_entropy_pool_LIBS_RELEASE}"
                              "${abseil_absl_random_internal_entropy_pool_LIB_DIRS_RELEASE}"
                              "${abseil_absl_random_internal_entropy_pool_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_random_internal_entropy_pool_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_random_internal_entropy_pool_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_random_internal_entropy_pool_DEPS_TARGET
                              abseil_absl_random_internal_entropy_pool_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_random_internal_entropy_pool"
                              "${abseil_absl_random_internal_entropy_pool_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::random_internal_entropy_pool
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_entropy_pool_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_entropy_pool_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_random_internal_entropy_pool_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::random_internal_entropy_pool
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_random_internal_entropy_pool_DEPS_TARGET)
        endif()

        set_property(TARGET absl::random_internal_entropy_pool APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_entropy_pool_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::random_internal_entropy_pool APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_entropy_pool_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::random_internal_entropy_pool APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_entropy_pool_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::random_internal_entropy_pool APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_entropy_pool_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::random_internal_entropy_pool APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_entropy_pool_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::sample_recorder #############

        set(abseil_absl_sample_recorder_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_sample_recorder_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_sample_recorder_FRAMEWORKS_RELEASE}" "${abseil_absl_sample_recorder_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_sample_recorder_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_sample_recorder_DEPS_TARGET)
            add_library(abseil_absl_sample_recorder_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_sample_recorder_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_sample_recorder_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_sample_recorder_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_sample_recorder_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_sample_recorder_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_sample_recorder_LIBS_RELEASE}"
                              "${abseil_absl_sample_recorder_LIB_DIRS_RELEASE}"
                              "${abseil_absl_sample_recorder_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_sample_recorder_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_sample_recorder_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_sample_recorder_DEPS_TARGET
                              abseil_absl_sample_recorder_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_sample_recorder"
                              "${abseil_absl_sample_recorder_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::sample_recorder
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_sample_recorder_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_sample_recorder_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_sample_recorder_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::sample_recorder
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_sample_recorder_DEPS_TARGET)
        endif()

        set_property(TARGET absl::sample_recorder APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_sample_recorder_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::sample_recorder APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_sample_recorder_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::sample_recorder APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_sample_recorder_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::sample_recorder APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_sample_recorder_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::sample_recorder APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_sample_recorder_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::vlog_config_internal #############

        set(abseil_absl_vlog_config_internal_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_vlog_config_internal_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_vlog_config_internal_FRAMEWORKS_RELEASE}" "${abseil_absl_vlog_config_internal_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_vlog_config_internal_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_vlog_config_internal_DEPS_TARGET)
            add_library(abseil_absl_vlog_config_internal_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_vlog_config_internal_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_vlog_config_internal_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_vlog_config_internal_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_vlog_config_internal_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_vlog_config_internal_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_vlog_config_internal_LIBS_RELEASE}"
                              "${abseil_absl_vlog_config_internal_LIB_DIRS_RELEASE}"
                              "${abseil_absl_vlog_config_internal_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_vlog_config_internal_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_vlog_config_internal_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_vlog_config_internal_DEPS_TARGET
                              abseil_absl_vlog_config_internal_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_vlog_config_internal"
                              "${abseil_absl_vlog_config_internal_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::vlog_config_internal
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_vlog_config_internal_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_vlog_config_internal_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_vlog_config_internal_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::vlog_config_internal
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_vlog_config_internal_DEPS_TARGET)
        endif()

        set_property(TARGET absl::vlog_config_internal APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_vlog_config_internal_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::vlog_config_internal APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_vlog_config_internal_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::vlog_config_internal APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_vlog_config_internal_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::vlog_config_internal APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_vlog_config_internal_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::vlog_config_internal APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_vlog_config_internal_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::flags_program_name #############

        set(abseil_absl_flags_program_name_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_flags_program_name_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_flags_program_name_FRAMEWORKS_RELEASE}" "${abseil_absl_flags_program_name_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_flags_program_name_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_flags_program_name_DEPS_TARGET)
            add_library(abseil_absl_flags_program_name_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_flags_program_name_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_program_name_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flags_program_name_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flags_program_name_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_flags_program_name_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_flags_program_name_LIBS_RELEASE}"
                              "${abseil_absl_flags_program_name_LIB_DIRS_RELEASE}"
                              "${abseil_absl_flags_program_name_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_flags_program_name_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_flags_program_name_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_flags_program_name_DEPS_TARGET
                              abseil_absl_flags_program_name_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_flags_program_name"
                              "${abseil_absl_flags_program_name_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::flags_program_name
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_program_name_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flags_program_name_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_flags_program_name_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::flags_program_name
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_flags_program_name_DEPS_TARGET)
        endif()

        set_property(TARGET absl::flags_program_name APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_flags_program_name_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::flags_program_name APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_program_name_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::flags_program_name APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_program_name_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::flags_program_name APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_flags_program_name_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::flags_program_name APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_flags_program_name_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::failure_signal_handler #############

        set(abseil_absl_failure_signal_handler_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_failure_signal_handler_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_failure_signal_handler_FRAMEWORKS_RELEASE}" "${abseil_absl_failure_signal_handler_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_failure_signal_handler_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_failure_signal_handler_DEPS_TARGET)
            add_library(abseil_absl_failure_signal_handler_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_failure_signal_handler_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_failure_signal_handler_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_failure_signal_handler_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_failure_signal_handler_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_failure_signal_handler_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_failure_signal_handler_LIBS_RELEASE}"
                              "${abseil_absl_failure_signal_handler_LIB_DIRS_RELEASE}"
                              "${abseil_absl_failure_signal_handler_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_failure_signal_handler_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_failure_signal_handler_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_failure_signal_handler_DEPS_TARGET
                              abseil_absl_failure_signal_handler_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_failure_signal_handler"
                              "${abseil_absl_failure_signal_handler_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::failure_signal_handler
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_failure_signal_handler_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_failure_signal_handler_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_failure_signal_handler_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::failure_signal_handler
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_failure_signal_handler_DEPS_TARGET)
        endif()

        set_property(TARGET absl::failure_signal_handler APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_failure_signal_handler_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::failure_signal_handler APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_failure_signal_handler_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::failure_signal_handler APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_failure_signal_handler_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::failure_signal_handler APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_failure_signal_handler_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::failure_signal_handler APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_failure_signal_handler_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::synchronization #############

        set(abseil_absl_synchronization_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_synchronization_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_synchronization_FRAMEWORKS_RELEASE}" "${abseil_absl_synchronization_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_synchronization_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_synchronization_DEPS_TARGET)
            add_library(abseil_absl_synchronization_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_synchronization_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_synchronization_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_synchronization_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_synchronization_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_synchronization_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_synchronization_LIBS_RELEASE}"
                              "${abseil_absl_synchronization_LIB_DIRS_RELEASE}"
                              "${abseil_absl_synchronization_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_synchronization_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_synchronization_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_synchronization_DEPS_TARGET
                              abseil_absl_synchronization_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_synchronization"
                              "${abseil_absl_synchronization_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::synchronization
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_synchronization_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_synchronization_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_synchronization_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::synchronization
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_synchronization_DEPS_TARGET)
        endif()

        set_property(TARGET absl::synchronization APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_synchronization_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::synchronization APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_synchronization_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::synchronization APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_synchronization_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::synchronization APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_synchronization_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::synchronization APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_synchronization_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::debugging #############

        set(abseil_absl_debugging_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_debugging_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_debugging_FRAMEWORKS_RELEASE}" "${abseil_absl_debugging_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_debugging_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_debugging_DEPS_TARGET)
            add_library(abseil_absl_debugging_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_debugging_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_debugging_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_debugging_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_debugging_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_debugging_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_debugging_LIBS_RELEASE}"
                              "${abseil_absl_debugging_LIB_DIRS_RELEASE}"
                              "${abseil_absl_debugging_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_debugging_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_debugging_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_debugging_DEPS_TARGET
                              abseil_absl_debugging_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_debugging"
                              "${abseil_absl_debugging_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::debugging
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_debugging_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_debugging_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_debugging_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::debugging
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_debugging_DEPS_TARGET)
        endif()

        set_property(TARGET absl::debugging APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_debugging_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::debugging APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_debugging_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::debugging APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_debugging_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::debugging APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_debugging_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::debugging APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_debugging_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::examine_stack #############

        set(abseil_absl_examine_stack_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_examine_stack_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_examine_stack_FRAMEWORKS_RELEASE}" "${abseil_absl_examine_stack_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_examine_stack_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_examine_stack_DEPS_TARGET)
            add_library(abseil_absl_examine_stack_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_examine_stack_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_examine_stack_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_examine_stack_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_examine_stack_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_examine_stack_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_examine_stack_LIBS_RELEASE}"
                              "${abseil_absl_examine_stack_LIB_DIRS_RELEASE}"
                              "${abseil_absl_examine_stack_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_examine_stack_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_examine_stack_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_examine_stack_DEPS_TARGET
                              abseil_absl_examine_stack_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_examine_stack"
                              "${abseil_absl_examine_stack_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::examine_stack
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_examine_stack_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_examine_stack_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_examine_stack_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::examine_stack
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_examine_stack_DEPS_TARGET)
        endif()

        set_property(TARGET absl::examine_stack APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_examine_stack_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::examine_stack APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_examine_stack_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::examine_stack APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_examine_stack_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::examine_stack APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_examine_stack_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::examine_stack APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_examine_stack_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::crc_cord_state #############

        set(abseil_absl_crc_cord_state_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_crc_cord_state_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_crc_cord_state_FRAMEWORKS_RELEASE}" "${abseil_absl_crc_cord_state_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_crc_cord_state_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_crc_cord_state_DEPS_TARGET)
            add_library(abseil_absl_crc_cord_state_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_crc_cord_state_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_crc_cord_state_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_crc_cord_state_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_crc_cord_state_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_crc_cord_state_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_crc_cord_state_LIBS_RELEASE}"
                              "${abseil_absl_crc_cord_state_LIB_DIRS_RELEASE}"
                              "${abseil_absl_crc_cord_state_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_crc_cord_state_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_crc_cord_state_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_crc_cord_state_DEPS_TARGET
                              abseil_absl_crc_cord_state_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_crc_cord_state"
                              "${abseil_absl_crc_cord_state_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::crc_cord_state
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_crc_cord_state_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_crc_cord_state_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_crc_cord_state_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::crc_cord_state
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_crc_cord_state_DEPS_TARGET)
        endif()

        set_property(TARGET absl::crc_cord_state APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_crc_cord_state_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::crc_cord_state APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_crc_cord_state_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::crc_cord_state APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_crc_cord_state_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::crc_cord_state APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_crc_cord_state_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::crc_cord_state APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_crc_cord_state_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::generic_printer_internal #############

        set(abseil_absl_generic_printer_internal_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_generic_printer_internal_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_generic_printer_internal_FRAMEWORKS_RELEASE}" "${abseil_absl_generic_printer_internal_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_generic_printer_internal_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_generic_printer_internal_DEPS_TARGET)
            add_library(abseil_absl_generic_printer_internal_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_generic_printer_internal_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_generic_printer_internal_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_generic_printer_internal_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_generic_printer_internal_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_generic_printer_internal_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_generic_printer_internal_LIBS_RELEASE}"
                              "${abseil_absl_generic_printer_internal_LIB_DIRS_RELEASE}"
                              "${abseil_absl_generic_printer_internal_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_generic_printer_internal_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_generic_printer_internal_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_generic_printer_internal_DEPS_TARGET
                              abseil_absl_generic_printer_internal_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_generic_printer_internal"
                              "${abseil_absl_generic_printer_internal_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::generic_printer_internal
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_generic_printer_internal_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_generic_printer_internal_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_generic_printer_internal_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::generic_printer_internal
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_generic_printer_internal_DEPS_TARGET)
        endif()

        set_property(TARGET absl::generic_printer_internal APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_generic_printer_internal_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::generic_printer_internal APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_generic_printer_internal_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::generic_printer_internal APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_generic_printer_internal_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::generic_printer_internal APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_generic_printer_internal_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::generic_printer_internal APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_generic_printer_internal_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::random_internal_distribution_test_util #############

        set(abseil_absl_random_internal_distribution_test_util_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_random_internal_distribution_test_util_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_random_internal_distribution_test_util_FRAMEWORKS_RELEASE}" "${abseil_absl_random_internal_distribution_test_util_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_random_internal_distribution_test_util_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_random_internal_distribution_test_util_DEPS_TARGET)
            add_library(abseil_absl_random_internal_distribution_test_util_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_random_internal_distribution_test_util_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_distribution_test_util_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_distribution_test_util_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_distribution_test_util_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_random_internal_distribution_test_util_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_random_internal_distribution_test_util_LIBS_RELEASE}"
                              "${abseil_absl_random_internal_distribution_test_util_LIB_DIRS_RELEASE}"
                              "${abseil_absl_random_internal_distribution_test_util_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_random_internal_distribution_test_util_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_random_internal_distribution_test_util_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_random_internal_distribution_test_util_DEPS_TARGET
                              abseil_absl_random_internal_distribution_test_util_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_random_internal_distribution_test_util"
                              "${abseil_absl_random_internal_distribution_test_util_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::random_internal_distribution_test_util
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_distribution_test_util_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_distribution_test_util_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_random_internal_distribution_test_util_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::random_internal_distribution_test_util
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_random_internal_distribution_test_util_DEPS_TARGET)
        endif()

        set_property(TARGET absl::random_internal_distribution_test_util APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_distribution_test_util_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::random_internal_distribution_test_util APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_distribution_test_util_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::random_internal_distribution_test_util APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_distribution_test_util_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::random_internal_distribution_test_util APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_distribution_test_util_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::random_internal_distribution_test_util APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_distribution_test_util_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::log_sink #############

        set(abseil_absl_log_sink_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_log_sink_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_log_sink_FRAMEWORKS_RELEASE}" "${abseil_absl_log_sink_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_log_sink_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_log_sink_DEPS_TARGET)
            add_library(abseil_absl_log_sink_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_log_sink_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_sink_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_sink_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_sink_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_log_sink_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_log_sink_LIBS_RELEASE}"
                              "${abseil_absl_log_sink_LIB_DIRS_RELEASE}"
                              "${abseil_absl_log_sink_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_log_sink_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_log_sink_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_log_sink_DEPS_TARGET
                              abseil_absl_log_sink_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_log_sink"
                              "${abseil_absl_log_sink_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::log_sink
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_sink_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_sink_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_log_sink_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::log_sink
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_log_sink_DEPS_TARGET)
        endif()

        set_property(TARGET absl::log_sink APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_sink_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::log_sink APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_sink_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::log_sink APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_sink_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::log_sink APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_sink_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::log_sink APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_sink_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::log_internal_format #############

        set(abseil_absl_log_internal_format_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_log_internal_format_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_log_internal_format_FRAMEWORKS_RELEASE}" "${abseil_absl_log_internal_format_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_log_internal_format_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_log_internal_format_DEPS_TARGET)
            add_library(abseil_absl_log_internal_format_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_log_internal_format_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_format_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_format_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_format_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_log_internal_format_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_log_internal_format_LIBS_RELEASE}"
                              "${abseil_absl_log_internal_format_LIB_DIRS_RELEASE}"
                              "${abseil_absl_log_internal_format_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_log_internal_format_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_log_internal_format_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_log_internal_format_DEPS_TARGET
                              abseil_absl_log_internal_format_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_log_internal_format"
                              "${abseil_absl_log_internal_format_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::log_internal_format
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_format_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_format_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_log_internal_format_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::log_internal_format
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_log_internal_format_DEPS_TARGET)
        endif()

        set_property(TARGET absl::log_internal_format APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_format_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::log_internal_format APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_format_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::log_internal_format APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_format_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::log_internal_format APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_format_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::log_internal_format APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_format_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::flags_marshalling #############

        set(abseil_absl_flags_marshalling_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_flags_marshalling_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_flags_marshalling_FRAMEWORKS_RELEASE}" "${abseil_absl_flags_marshalling_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_flags_marshalling_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_flags_marshalling_DEPS_TARGET)
            add_library(abseil_absl_flags_marshalling_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_flags_marshalling_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_marshalling_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flags_marshalling_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flags_marshalling_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_flags_marshalling_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_flags_marshalling_LIBS_RELEASE}"
                              "${abseil_absl_flags_marshalling_LIB_DIRS_RELEASE}"
                              "${abseil_absl_flags_marshalling_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_flags_marshalling_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_flags_marshalling_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_flags_marshalling_DEPS_TARGET
                              abseil_absl_flags_marshalling_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_flags_marshalling"
                              "${abseil_absl_flags_marshalling_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::flags_marshalling
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_marshalling_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flags_marshalling_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_flags_marshalling_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::flags_marshalling
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_flags_marshalling_DEPS_TARGET)
        endif()

        set_property(TARGET absl::flags_marshalling APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_flags_marshalling_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::flags_marshalling APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_marshalling_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::flags_marshalling APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_marshalling_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::flags_marshalling APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_flags_marshalling_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::flags_marshalling APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_flags_marshalling_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::symbolize #############

        set(abseil_absl_symbolize_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_symbolize_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_symbolize_FRAMEWORKS_RELEASE}" "${abseil_absl_symbolize_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_symbolize_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_symbolize_DEPS_TARGET)
            add_library(abseil_absl_symbolize_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_symbolize_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_symbolize_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_symbolize_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_symbolize_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_symbolize_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_symbolize_LIBS_RELEASE}"
                              "${abseil_absl_symbolize_LIB_DIRS_RELEASE}"
                              "${abseil_absl_symbolize_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_symbolize_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_symbolize_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_symbolize_DEPS_TARGET
                              abseil_absl_symbolize_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_symbolize"
                              "${abseil_absl_symbolize_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::symbolize
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_symbolize_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_symbolize_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_symbolize_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::symbolize
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_symbolize_DEPS_TARGET)
        endif()

        set_property(TARGET absl::symbolize APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_symbolize_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::symbolize APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_symbolize_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::symbolize APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_symbolize_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::symbolize APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_symbolize_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::symbolize APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_symbolize_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::stacktrace #############

        set(abseil_absl_stacktrace_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_stacktrace_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_stacktrace_FRAMEWORKS_RELEASE}" "${abseil_absl_stacktrace_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_stacktrace_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_stacktrace_DEPS_TARGET)
            add_library(abseil_absl_stacktrace_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_stacktrace_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_stacktrace_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_stacktrace_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_stacktrace_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_stacktrace_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_stacktrace_LIBS_RELEASE}"
                              "${abseil_absl_stacktrace_LIB_DIRS_RELEASE}"
                              "${abseil_absl_stacktrace_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_stacktrace_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_stacktrace_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_stacktrace_DEPS_TARGET
                              abseil_absl_stacktrace_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_stacktrace"
                              "${abseil_absl_stacktrace_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::stacktrace
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_stacktrace_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_stacktrace_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_stacktrace_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::stacktrace
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_stacktrace_DEPS_TARGET)
        endif()

        set_property(TARGET absl::stacktrace APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_stacktrace_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::stacktrace APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_stacktrace_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::stacktrace APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_stacktrace_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::stacktrace APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_stacktrace_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::stacktrace APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_stacktrace_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::crc32c #############

        set(abseil_absl_crc32c_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_crc32c_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_crc32c_FRAMEWORKS_RELEASE}" "${abseil_absl_crc32c_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_crc32c_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_crc32c_DEPS_TARGET)
            add_library(abseil_absl_crc32c_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_crc32c_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_crc32c_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_crc32c_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_crc32c_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_crc32c_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_crc32c_LIBS_RELEASE}"
                              "${abseil_absl_crc32c_LIB_DIRS_RELEASE}"
                              "${abseil_absl_crc32c_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_crc32c_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_crc32c_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_crc32c_DEPS_TARGET
                              abseil_absl_crc32c_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_crc32c"
                              "${abseil_absl_crc32c_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::crc32c
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_crc32c_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_crc32c_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_crc32c_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::crc32c
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_crc32c_DEPS_TARGET)
        endif()

        set_property(TARGET absl::crc32c APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_crc32c_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::crc32c APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_crc32c_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::crc32c APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_crc32c_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::crc32c APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_crc32c_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::crc32c APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_crc32c_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::hash_policy_traits #############

        set(abseil_absl_hash_policy_traits_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_hash_policy_traits_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_hash_policy_traits_FRAMEWORKS_RELEASE}" "${abseil_absl_hash_policy_traits_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_hash_policy_traits_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_hash_policy_traits_DEPS_TARGET)
            add_library(abseil_absl_hash_policy_traits_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_hash_policy_traits_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_hash_policy_traits_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_hash_policy_traits_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_hash_policy_traits_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_hash_policy_traits_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_hash_policy_traits_LIBS_RELEASE}"
                              "${abseil_absl_hash_policy_traits_LIB_DIRS_RELEASE}"
                              "${abseil_absl_hash_policy_traits_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_hash_policy_traits_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_hash_policy_traits_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_hash_policy_traits_DEPS_TARGET
                              abseil_absl_hash_policy_traits_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_hash_policy_traits"
                              "${abseil_absl_hash_policy_traits_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::hash_policy_traits
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_hash_policy_traits_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_hash_policy_traits_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_hash_policy_traits_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::hash_policy_traits
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_hash_policy_traits_DEPS_TARGET)
        endif()

        set_property(TARGET absl::hash_policy_traits APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_hash_policy_traits_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::hash_policy_traits APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_hash_policy_traits_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::hash_policy_traits APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_hash_policy_traits_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::hash_policy_traits APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_hash_policy_traits_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::hash_policy_traits APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_hash_policy_traits_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::kernel_timeout_internal #############

        set(abseil_absl_kernel_timeout_internal_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_kernel_timeout_internal_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_kernel_timeout_internal_FRAMEWORKS_RELEASE}" "${abseil_absl_kernel_timeout_internal_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_kernel_timeout_internal_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_kernel_timeout_internal_DEPS_TARGET)
            add_library(abseil_absl_kernel_timeout_internal_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_kernel_timeout_internal_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_kernel_timeout_internal_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_kernel_timeout_internal_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_kernel_timeout_internal_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_kernel_timeout_internal_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_kernel_timeout_internal_LIBS_RELEASE}"
                              "${abseil_absl_kernel_timeout_internal_LIB_DIRS_RELEASE}"
                              "${abseil_absl_kernel_timeout_internal_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_kernel_timeout_internal_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_kernel_timeout_internal_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_kernel_timeout_internal_DEPS_TARGET
                              abseil_absl_kernel_timeout_internal_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_kernel_timeout_internal"
                              "${abseil_absl_kernel_timeout_internal_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::kernel_timeout_internal
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_kernel_timeout_internal_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_kernel_timeout_internal_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_kernel_timeout_internal_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::kernel_timeout_internal
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_kernel_timeout_internal_DEPS_TARGET)
        endif()

        set_property(TARGET absl::kernel_timeout_internal APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_kernel_timeout_internal_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::kernel_timeout_internal APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_kernel_timeout_internal_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::kernel_timeout_internal APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_kernel_timeout_internal_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::kernel_timeout_internal APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_kernel_timeout_internal_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::kernel_timeout_internal APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_kernel_timeout_internal_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::str_format #############

        set(abseil_absl_str_format_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_str_format_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_str_format_FRAMEWORKS_RELEASE}" "${abseil_absl_str_format_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_str_format_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_str_format_DEPS_TARGET)
            add_library(abseil_absl_str_format_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_str_format_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_str_format_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_str_format_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_str_format_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_str_format_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_str_format_LIBS_RELEASE}"
                              "${abseil_absl_str_format_LIB_DIRS_RELEASE}"
                              "${abseil_absl_str_format_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_str_format_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_str_format_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_str_format_DEPS_TARGET
                              abseil_absl_str_format_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_str_format"
                              "${abseil_absl_str_format_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::str_format
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_str_format_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_str_format_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_str_format_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::str_format
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_str_format_DEPS_TARGET)
        endif()

        set_property(TARGET absl::str_format APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_str_format_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::str_format APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_str_format_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::str_format APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_str_format_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::str_format APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_str_format_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::str_format APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_str_format_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::random_internal_salted_seed_seq #############

        set(abseil_absl_random_internal_salted_seed_seq_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_random_internal_salted_seed_seq_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_random_internal_salted_seed_seq_FRAMEWORKS_RELEASE}" "${abseil_absl_random_internal_salted_seed_seq_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_random_internal_salted_seed_seq_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_random_internal_salted_seed_seq_DEPS_TARGET)
            add_library(abseil_absl_random_internal_salted_seed_seq_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_random_internal_salted_seed_seq_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_salted_seed_seq_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_salted_seed_seq_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_salted_seed_seq_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_random_internal_salted_seed_seq_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_random_internal_salted_seed_seq_LIBS_RELEASE}"
                              "${abseil_absl_random_internal_salted_seed_seq_LIB_DIRS_RELEASE}"
                              "${abseil_absl_random_internal_salted_seed_seq_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_random_internal_salted_seed_seq_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_random_internal_salted_seed_seq_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_random_internal_salted_seed_seq_DEPS_TARGET
                              abseil_absl_random_internal_salted_seed_seq_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_random_internal_salted_seed_seq"
                              "${abseil_absl_random_internal_salted_seed_seq_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::random_internal_salted_seed_seq
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_salted_seed_seq_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_salted_seed_seq_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_random_internal_salted_seed_seq_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::random_internal_salted_seed_seq
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_random_internal_salted_seed_seq_DEPS_TARGET)
        endif()

        set_property(TARGET absl::random_internal_salted_seed_seq APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_salted_seed_seq_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::random_internal_salted_seed_seq APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_salted_seed_seq_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::random_internal_salted_seed_seq APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_salted_seed_seq_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::random_internal_salted_seed_seq APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_salted_seed_seq_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::random_internal_salted_seed_seq APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_salted_seed_seq_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::log_internal_structured_proto #############

        set(abseil_absl_log_internal_structured_proto_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_log_internal_structured_proto_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_log_internal_structured_proto_FRAMEWORKS_RELEASE}" "${abseil_absl_log_internal_structured_proto_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_log_internal_structured_proto_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_log_internal_structured_proto_DEPS_TARGET)
            add_library(abseil_absl_log_internal_structured_proto_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_log_internal_structured_proto_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_structured_proto_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_structured_proto_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_structured_proto_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_log_internal_structured_proto_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_log_internal_structured_proto_LIBS_RELEASE}"
                              "${abseil_absl_log_internal_structured_proto_LIB_DIRS_RELEASE}"
                              "${abseil_absl_log_internal_structured_proto_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_log_internal_structured_proto_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_log_internal_structured_proto_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_log_internal_structured_proto_DEPS_TARGET
                              abseil_absl_log_internal_structured_proto_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_log_internal_structured_proto"
                              "${abseil_absl_log_internal_structured_proto_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::log_internal_structured_proto
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_structured_proto_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_structured_proto_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_log_internal_structured_proto_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::log_internal_structured_proto
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_log_internal_structured_proto_DEPS_TARGET)
        endif()

        set_property(TARGET absl::log_internal_structured_proto APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_structured_proto_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::log_internal_structured_proto APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_structured_proto_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::log_internal_structured_proto APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_structured_proto_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::log_internal_structured_proto APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_structured_proto_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::log_internal_structured_proto APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_structured_proto_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::log_entry #############

        set(abseil_absl_log_entry_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_log_entry_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_log_entry_FRAMEWORKS_RELEASE}" "${abseil_absl_log_entry_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_log_entry_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_log_entry_DEPS_TARGET)
            add_library(abseil_absl_log_entry_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_log_entry_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_entry_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_entry_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_entry_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_log_entry_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_log_entry_LIBS_RELEASE}"
                              "${abseil_absl_log_entry_LIB_DIRS_RELEASE}"
                              "${abseil_absl_log_entry_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_log_entry_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_log_entry_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_log_entry_DEPS_TARGET
                              abseil_absl_log_entry_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_log_entry"
                              "${abseil_absl_log_entry_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::log_entry
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_entry_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_entry_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_log_entry_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::log_entry
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_log_entry_DEPS_TARGET)
        endif()

        set_property(TARGET absl::log_entry APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_entry_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::log_entry APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_entry_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::log_entry APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_entry_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::log_entry APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_entry_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::log_entry APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_entry_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::log_internal_globals #############

        set(abseil_absl_log_internal_globals_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_log_internal_globals_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_log_internal_globals_FRAMEWORKS_RELEASE}" "${abseil_absl_log_internal_globals_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_log_internal_globals_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_log_internal_globals_DEPS_TARGET)
            add_library(abseil_absl_log_internal_globals_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_log_internal_globals_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_globals_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_globals_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_globals_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_log_internal_globals_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_log_internal_globals_LIBS_RELEASE}"
                              "${abseil_absl_log_internal_globals_LIB_DIRS_RELEASE}"
                              "${abseil_absl_log_internal_globals_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_log_internal_globals_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_log_internal_globals_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_log_internal_globals_DEPS_TARGET
                              abseil_absl_log_internal_globals_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_log_internal_globals"
                              "${abseil_absl_log_internal_globals_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::log_internal_globals
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_globals_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_globals_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_log_internal_globals_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::log_internal_globals
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_log_internal_globals_DEPS_TARGET)
        endif()

        set_property(TARGET absl::log_internal_globals APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_globals_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::log_internal_globals APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_globals_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::log_internal_globals APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_globals_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::log_internal_globals APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_globals_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::log_internal_globals APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_globals_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::flags_private_handle_accessor #############

        set(abseil_absl_flags_private_handle_accessor_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_flags_private_handle_accessor_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_flags_private_handle_accessor_FRAMEWORKS_RELEASE}" "${abseil_absl_flags_private_handle_accessor_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_flags_private_handle_accessor_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_flags_private_handle_accessor_DEPS_TARGET)
            add_library(abseil_absl_flags_private_handle_accessor_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_flags_private_handle_accessor_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_private_handle_accessor_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flags_private_handle_accessor_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flags_private_handle_accessor_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_flags_private_handle_accessor_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_flags_private_handle_accessor_LIBS_RELEASE}"
                              "${abseil_absl_flags_private_handle_accessor_LIB_DIRS_RELEASE}"
                              "${abseil_absl_flags_private_handle_accessor_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_flags_private_handle_accessor_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_flags_private_handle_accessor_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_flags_private_handle_accessor_DEPS_TARGET
                              abseil_absl_flags_private_handle_accessor_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_flags_private_handle_accessor"
                              "${abseil_absl_flags_private_handle_accessor_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::flags_private_handle_accessor
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_private_handle_accessor_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flags_private_handle_accessor_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_flags_private_handle_accessor_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::flags_private_handle_accessor
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_flags_private_handle_accessor_DEPS_TARGET)
        endif()

        set_property(TARGET absl::flags_private_handle_accessor APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_flags_private_handle_accessor_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::flags_private_handle_accessor APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_private_handle_accessor_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::flags_private_handle_accessor APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_private_handle_accessor_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::flags_private_handle_accessor APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_flags_private_handle_accessor_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::flags_private_handle_accessor APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_flags_private_handle_accessor_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::demangle_internal #############

        set(abseil_absl_demangle_internal_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_demangle_internal_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_demangle_internal_FRAMEWORKS_RELEASE}" "${abseil_absl_demangle_internal_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_demangle_internal_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_demangle_internal_DEPS_TARGET)
            add_library(abseil_absl_demangle_internal_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_demangle_internal_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_demangle_internal_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_demangle_internal_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_demangle_internal_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_demangle_internal_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_demangle_internal_LIBS_RELEASE}"
                              "${abseil_absl_demangle_internal_LIB_DIRS_RELEASE}"
                              "${abseil_absl_demangle_internal_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_demangle_internal_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_demangle_internal_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_demangle_internal_DEPS_TARGET
                              abseil_absl_demangle_internal_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_demangle_internal"
                              "${abseil_absl_demangle_internal_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::demangle_internal
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_demangle_internal_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_demangle_internal_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_demangle_internal_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::demangle_internal
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_demangle_internal_DEPS_TARGET)
        endif()

        set_property(TARGET absl::demangle_internal APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_demangle_internal_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::demangle_internal APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_demangle_internal_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::demangle_internal APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_demangle_internal_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::demangle_internal APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_demangle_internal_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::demangle_internal APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_demangle_internal_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::borrowed_fixup_buffer #############

        set(abseil_absl_borrowed_fixup_buffer_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_borrowed_fixup_buffer_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_borrowed_fixup_buffer_FRAMEWORKS_RELEASE}" "${abseil_absl_borrowed_fixup_buffer_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_borrowed_fixup_buffer_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_borrowed_fixup_buffer_DEPS_TARGET)
            add_library(abseil_absl_borrowed_fixup_buffer_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_borrowed_fixup_buffer_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_borrowed_fixup_buffer_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_borrowed_fixup_buffer_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_borrowed_fixup_buffer_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_borrowed_fixup_buffer_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_borrowed_fixup_buffer_LIBS_RELEASE}"
                              "${abseil_absl_borrowed_fixup_buffer_LIB_DIRS_RELEASE}"
                              "${abseil_absl_borrowed_fixup_buffer_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_borrowed_fixup_buffer_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_borrowed_fixup_buffer_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_borrowed_fixup_buffer_DEPS_TARGET
                              abseil_absl_borrowed_fixup_buffer_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_borrowed_fixup_buffer"
                              "${abseil_absl_borrowed_fixup_buffer_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::borrowed_fixup_buffer
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_borrowed_fixup_buffer_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_borrowed_fixup_buffer_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_borrowed_fixup_buffer_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::borrowed_fixup_buffer
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_borrowed_fixup_buffer_DEPS_TARGET)
        endif()

        set_property(TARGET absl::borrowed_fixup_buffer APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_borrowed_fixup_buffer_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::borrowed_fixup_buffer APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_borrowed_fixup_buffer_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::borrowed_fixup_buffer APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_borrowed_fixup_buffer_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::borrowed_fixup_buffer APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_borrowed_fixup_buffer_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::borrowed_fixup_buffer APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_borrowed_fixup_buffer_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::chunked_queue #############

        set(abseil_absl_chunked_queue_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_chunked_queue_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_chunked_queue_FRAMEWORKS_RELEASE}" "${abseil_absl_chunked_queue_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_chunked_queue_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_chunked_queue_DEPS_TARGET)
            add_library(abseil_absl_chunked_queue_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_chunked_queue_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_chunked_queue_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_chunked_queue_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_chunked_queue_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_chunked_queue_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_chunked_queue_LIBS_RELEASE}"
                              "${abseil_absl_chunked_queue_LIB_DIRS_RELEASE}"
                              "${abseil_absl_chunked_queue_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_chunked_queue_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_chunked_queue_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_chunked_queue_DEPS_TARGET
                              abseil_absl_chunked_queue_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_chunked_queue"
                              "${abseil_absl_chunked_queue_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::chunked_queue
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_chunked_queue_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_chunked_queue_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_chunked_queue_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::chunked_queue
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_chunked_queue_DEPS_TARGET)
        endif()

        set_property(TARGET absl::chunked_queue APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_chunked_queue_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::chunked_queue APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_chunked_queue_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::chunked_queue APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_chunked_queue_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::chunked_queue APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_chunked_queue_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::chunked_queue APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_chunked_queue_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::container_memory #############

        set(abseil_absl_container_memory_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_container_memory_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_container_memory_FRAMEWORKS_RELEASE}" "${abseil_absl_container_memory_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_container_memory_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_container_memory_DEPS_TARGET)
            add_library(abseil_absl_container_memory_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_container_memory_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_container_memory_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_container_memory_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_container_memory_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_container_memory_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_container_memory_LIBS_RELEASE}"
                              "${abseil_absl_container_memory_LIB_DIRS_RELEASE}"
                              "${abseil_absl_container_memory_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_container_memory_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_container_memory_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_container_memory_DEPS_TARGET
                              abseil_absl_container_memory_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_container_memory"
                              "${abseil_absl_container_memory_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::container_memory
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_container_memory_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_container_memory_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_container_memory_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::container_memory
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_container_memory_DEPS_TARGET)
        endif()

        set_property(TARGET absl::container_memory APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_container_memory_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::container_memory APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_container_memory_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::container_memory APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_container_memory_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::container_memory APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_container_memory_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::container_memory APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_container_memory_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::time #############

        set(abseil_absl_time_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_time_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_time_FRAMEWORKS_RELEASE}" "${abseil_absl_time_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_time_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_time_DEPS_TARGET)
            add_library(abseil_absl_time_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_time_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_time_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_time_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_time_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_time_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_time_LIBS_RELEASE}"
                              "${abseil_absl_time_LIB_DIRS_RELEASE}"
                              "${abseil_absl_time_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_time_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_time_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_time_DEPS_TARGET
                              abseil_absl_time_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_time"
                              "${abseil_absl_time_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::time
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_time_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_time_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_time_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::time
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_time_DEPS_TARGET)
        endif()

        set_property(TARGET absl::time APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_time_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::time APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_time_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::time APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_time_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::time APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_time_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::time APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_time_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::str_format_internal #############

        set(abseil_absl_str_format_internal_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_str_format_internal_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_str_format_internal_FRAMEWORKS_RELEASE}" "${abseil_absl_str_format_internal_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_str_format_internal_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_str_format_internal_DEPS_TARGET)
            add_library(abseil_absl_str_format_internal_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_str_format_internal_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_str_format_internal_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_str_format_internal_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_str_format_internal_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_str_format_internal_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_str_format_internal_LIBS_RELEASE}"
                              "${abseil_absl_str_format_internal_LIB_DIRS_RELEASE}"
                              "${abseil_absl_str_format_internal_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_str_format_internal_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_str_format_internal_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_str_format_internal_DEPS_TARGET
                              abseil_absl_str_format_internal_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_str_format_internal"
                              "${abseil_absl_str_format_internal_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::str_format_internal
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_str_format_internal_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_str_format_internal_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_str_format_internal_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::str_format_internal
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_str_format_internal_DEPS_TARGET)
        endif()

        set_property(TARGET absl::str_format_internal APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_str_format_internal_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::str_format_internal APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_str_format_internal_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::str_format_internal APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_str_format_internal_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::str_format_internal APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_str_format_internal_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::str_format_internal APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_str_format_internal_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::random_internal_randen_engine #############

        set(abseil_absl_random_internal_randen_engine_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_random_internal_randen_engine_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_random_internal_randen_engine_FRAMEWORKS_RELEASE}" "${abseil_absl_random_internal_randen_engine_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_random_internal_randen_engine_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_random_internal_randen_engine_DEPS_TARGET)
            add_library(abseil_absl_random_internal_randen_engine_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_random_internal_randen_engine_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_engine_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_engine_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_engine_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_random_internal_randen_engine_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_random_internal_randen_engine_LIBS_RELEASE}"
                              "${abseil_absl_random_internal_randen_engine_LIB_DIRS_RELEASE}"
                              "${abseil_absl_random_internal_randen_engine_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_random_internal_randen_engine_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_random_internal_randen_engine_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_random_internal_randen_engine_DEPS_TARGET
                              abseil_absl_random_internal_randen_engine_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_random_internal_randen_engine"
                              "${abseil_absl_random_internal_randen_engine_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::random_internal_randen_engine
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_engine_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_engine_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_random_internal_randen_engine_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::random_internal_randen_engine
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_random_internal_randen_engine_DEPS_TARGET)
        endif()

        set_property(TARGET absl::random_internal_randen_engine APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_engine_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::random_internal_randen_engine APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_engine_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::random_internal_randen_engine APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_engine_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::random_internal_randen_engine APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_engine_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::random_internal_randen_engine APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_engine_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::random_internal_pcg_engine #############

        set(abseil_absl_random_internal_pcg_engine_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_random_internal_pcg_engine_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_random_internal_pcg_engine_FRAMEWORKS_RELEASE}" "${abseil_absl_random_internal_pcg_engine_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_random_internal_pcg_engine_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_random_internal_pcg_engine_DEPS_TARGET)
            add_library(abseil_absl_random_internal_pcg_engine_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_random_internal_pcg_engine_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_pcg_engine_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_pcg_engine_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_pcg_engine_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_random_internal_pcg_engine_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_random_internal_pcg_engine_LIBS_RELEASE}"
                              "${abseil_absl_random_internal_pcg_engine_LIB_DIRS_RELEASE}"
                              "${abseil_absl_random_internal_pcg_engine_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_random_internal_pcg_engine_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_random_internal_pcg_engine_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_random_internal_pcg_engine_DEPS_TARGET
                              abseil_absl_random_internal_pcg_engine_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_random_internal_pcg_engine"
                              "${abseil_absl_random_internal_pcg_engine_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::random_internal_pcg_engine
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_pcg_engine_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_pcg_engine_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_random_internal_pcg_engine_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::random_internal_pcg_engine
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_random_internal_pcg_engine_DEPS_TARGET)
        endif()

        set_property(TARGET absl::random_internal_pcg_engine APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_pcg_engine_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::random_internal_pcg_engine APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_pcg_engine_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::random_internal_pcg_engine APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_pcg_engine_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::random_internal_pcg_engine APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_pcg_engine_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::random_internal_pcg_engine APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_pcg_engine_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::random_internal_seed_material #############

        set(abseil_absl_random_internal_seed_material_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_random_internal_seed_material_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_random_internal_seed_material_FRAMEWORKS_RELEASE}" "${abseil_absl_random_internal_seed_material_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_random_internal_seed_material_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_random_internal_seed_material_DEPS_TARGET)
            add_library(abseil_absl_random_internal_seed_material_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_random_internal_seed_material_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_seed_material_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_seed_material_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_seed_material_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_random_internal_seed_material_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_random_internal_seed_material_LIBS_RELEASE}"
                              "${abseil_absl_random_internal_seed_material_LIB_DIRS_RELEASE}"
                              "${abseil_absl_random_internal_seed_material_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_random_internal_seed_material_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_random_internal_seed_material_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_random_internal_seed_material_DEPS_TARGET
                              abseil_absl_random_internal_seed_material_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_random_internal_seed_material"
                              "${abseil_absl_random_internal_seed_material_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::random_internal_seed_material
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_seed_material_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_seed_material_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_random_internal_seed_material_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::random_internal_seed_material
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_random_internal_seed_material_DEPS_TARGET)
        endif()

        set_property(TARGET absl::random_internal_seed_material APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_seed_material_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::random_internal_seed_material APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_seed_material_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::random_internal_seed_material APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_seed_material_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::random_internal_seed_material APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_seed_material_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::random_internal_seed_material APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_seed_material_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::random_distributions #############

        set(abseil_absl_random_distributions_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_random_distributions_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_random_distributions_FRAMEWORKS_RELEASE}" "${abseil_absl_random_distributions_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_random_distributions_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_random_distributions_DEPS_TARGET)
            add_library(abseil_absl_random_distributions_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_random_distributions_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_distributions_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_distributions_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_distributions_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_random_distributions_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_random_distributions_LIBS_RELEASE}"
                              "${abseil_absl_random_distributions_LIB_DIRS_RELEASE}"
                              "${abseil_absl_random_distributions_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_random_distributions_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_random_distributions_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_random_distributions_DEPS_TARGET
                              abseil_absl_random_distributions_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_random_distributions"
                              "${abseil_absl_random_distributions_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::random_distributions
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_distributions_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_distributions_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_random_distributions_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::random_distributions
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_random_distributions_DEPS_TARGET)
        endif()

        set_property(TARGET absl::random_distributions APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_distributions_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::random_distributions APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_distributions_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::random_distributions APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_distributions_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::random_distributions APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_distributions_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::random_distributions APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_distributions_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::log_internal_container #############

        set(abseil_absl_log_internal_container_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_log_internal_container_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_log_internal_container_FRAMEWORKS_RELEASE}" "${abseil_absl_log_internal_container_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_log_internal_container_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_log_internal_container_DEPS_TARGET)
            add_library(abseil_absl_log_internal_container_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_log_internal_container_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_container_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_container_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_container_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_log_internal_container_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_log_internal_container_LIBS_RELEASE}"
                              "${abseil_absl_log_internal_container_LIB_DIRS_RELEASE}"
                              "${abseil_absl_log_internal_container_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_log_internal_container_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_log_internal_container_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_log_internal_container_DEPS_TARGET
                              abseil_absl_log_internal_container_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_log_internal_container"
                              "${abseil_absl_log_internal_container_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::log_internal_container
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_container_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_container_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_log_internal_container_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::log_internal_container
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_log_internal_container_DEPS_TARGET)
        endif()

        set_property(TARGET absl::log_internal_container APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_container_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::log_internal_container APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_container_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::log_internal_container APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_container_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::log_internal_container APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_container_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::log_internal_container APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_container_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::log_internal_fnmatch #############

        set(abseil_absl_log_internal_fnmatch_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_log_internal_fnmatch_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_log_internal_fnmatch_FRAMEWORKS_RELEASE}" "${abseil_absl_log_internal_fnmatch_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_log_internal_fnmatch_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_log_internal_fnmatch_DEPS_TARGET)
            add_library(abseil_absl_log_internal_fnmatch_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_log_internal_fnmatch_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_fnmatch_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_fnmatch_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_fnmatch_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_log_internal_fnmatch_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_log_internal_fnmatch_LIBS_RELEASE}"
                              "${abseil_absl_log_internal_fnmatch_LIB_DIRS_RELEASE}"
                              "${abseil_absl_log_internal_fnmatch_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_log_internal_fnmatch_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_log_internal_fnmatch_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_log_internal_fnmatch_DEPS_TARGET
                              abseil_absl_log_internal_fnmatch_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_log_internal_fnmatch"
                              "${abseil_absl_log_internal_fnmatch_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::log_internal_fnmatch
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_fnmatch_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_fnmatch_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_log_internal_fnmatch_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::log_internal_fnmatch
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_log_internal_fnmatch_DEPS_TARGET)
        endif()

        set_property(TARGET absl::log_internal_fnmatch APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_fnmatch_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::log_internal_fnmatch APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_fnmatch_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::log_internal_fnmatch APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_fnmatch_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::log_internal_fnmatch APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_fnmatch_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::log_internal_fnmatch APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_fnmatch_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::log_internal_append_truncated #############

        set(abseil_absl_log_internal_append_truncated_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_log_internal_append_truncated_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_log_internal_append_truncated_FRAMEWORKS_RELEASE}" "${abseil_absl_log_internal_append_truncated_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_log_internal_append_truncated_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_log_internal_append_truncated_DEPS_TARGET)
            add_library(abseil_absl_log_internal_append_truncated_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_log_internal_append_truncated_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_append_truncated_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_append_truncated_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_append_truncated_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_log_internal_append_truncated_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_log_internal_append_truncated_LIBS_RELEASE}"
                              "${abseil_absl_log_internal_append_truncated_LIB_DIRS_RELEASE}"
                              "${abseil_absl_log_internal_append_truncated_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_log_internal_append_truncated_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_log_internal_append_truncated_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_log_internal_append_truncated_DEPS_TARGET
                              abseil_absl_log_internal_append_truncated_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_log_internal_append_truncated"
                              "${abseil_absl_log_internal_append_truncated_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::log_internal_append_truncated
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_append_truncated_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_append_truncated_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_log_internal_append_truncated_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::log_internal_append_truncated
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_log_internal_append_truncated_DEPS_TARGET)
        endif()

        set_property(TARGET absl::log_internal_append_truncated APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_append_truncated_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::log_internal_append_truncated APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_append_truncated_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::log_internal_append_truncated APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_append_truncated_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::log_internal_append_truncated APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_append_truncated_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::log_internal_append_truncated APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_append_truncated_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::log_internal_nullstream #############

        set(abseil_absl_log_internal_nullstream_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_log_internal_nullstream_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_log_internal_nullstream_FRAMEWORKS_RELEASE}" "${abseil_absl_log_internal_nullstream_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_log_internal_nullstream_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_log_internal_nullstream_DEPS_TARGET)
            add_library(abseil_absl_log_internal_nullstream_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_log_internal_nullstream_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_nullstream_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_nullstream_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_nullstream_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_log_internal_nullstream_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_log_internal_nullstream_LIBS_RELEASE}"
                              "${abseil_absl_log_internal_nullstream_LIB_DIRS_RELEASE}"
                              "${abseil_absl_log_internal_nullstream_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_log_internal_nullstream_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_log_internal_nullstream_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_log_internal_nullstream_DEPS_TARGET
                              abseil_absl_log_internal_nullstream_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_log_internal_nullstream"
                              "${abseil_absl_log_internal_nullstream_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::log_internal_nullstream
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_nullstream_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_nullstream_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_log_internal_nullstream_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::log_internal_nullstream
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_log_internal_nullstream_DEPS_TARGET)
        endif()

        set_property(TARGET absl::log_internal_nullstream APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_nullstream_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::log_internal_nullstream APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_nullstream_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::log_internal_nullstream APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_nullstream_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::log_internal_nullstream APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_nullstream_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::log_internal_nullstream APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_nullstream_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::log_internal_proto #############

        set(abseil_absl_log_internal_proto_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_log_internal_proto_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_log_internal_proto_FRAMEWORKS_RELEASE}" "${abseil_absl_log_internal_proto_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_log_internal_proto_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_log_internal_proto_DEPS_TARGET)
            add_library(abseil_absl_log_internal_proto_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_log_internal_proto_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_proto_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_proto_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_proto_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_log_internal_proto_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_log_internal_proto_LIBS_RELEASE}"
                              "${abseil_absl_log_internal_proto_LIB_DIRS_RELEASE}"
                              "${abseil_absl_log_internal_proto_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_log_internal_proto_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_log_internal_proto_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_log_internal_proto_DEPS_TARGET
                              abseil_absl_log_internal_proto_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_log_internal_proto"
                              "${abseil_absl_log_internal_proto_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::log_internal_proto
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_proto_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_proto_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_log_internal_proto_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::log_internal_proto
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_log_internal_proto_DEPS_TARGET)
        endif()

        set_property(TARGET absl::log_internal_proto APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_proto_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::log_internal_proto APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_proto_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::log_internal_proto APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_proto_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::log_internal_proto APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_proto_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::log_internal_proto APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_proto_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::hash #############

        set(abseil_absl_hash_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_hash_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_hash_FRAMEWORKS_RELEASE}" "${abseil_absl_hash_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_hash_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_hash_DEPS_TARGET)
            add_library(abseil_absl_hash_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_hash_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_hash_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_hash_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_hash_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_hash_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_hash_LIBS_RELEASE}"
                              "${abseil_absl_hash_LIB_DIRS_RELEASE}"
                              "${abseil_absl_hash_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_hash_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_hash_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_hash_DEPS_TARGET
                              abseil_absl_hash_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_hash"
                              "${abseil_absl_hash_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::hash
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_hash_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_hash_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_hash_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::hash
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_hash_DEPS_TARGET)
        endif()

        set_property(TARGET absl::hash APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_hash_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::hash APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_hash_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::hash APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_hash_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::hash APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_hash_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::hash APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_hash_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::flags_commandlineflag #############

        set(abseil_absl_flags_commandlineflag_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_flags_commandlineflag_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_flags_commandlineflag_FRAMEWORKS_RELEASE}" "${abseil_absl_flags_commandlineflag_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_flags_commandlineflag_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_flags_commandlineflag_DEPS_TARGET)
            add_library(abseil_absl_flags_commandlineflag_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_flags_commandlineflag_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_commandlineflag_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flags_commandlineflag_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flags_commandlineflag_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_flags_commandlineflag_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_flags_commandlineflag_LIBS_RELEASE}"
                              "${abseil_absl_flags_commandlineflag_LIB_DIRS_RELEASE}"
                              "${abseil_absl_flags_commandlineflag_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_flags_commandlineflag_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_flags_commandlineflag_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_flags_commandlineflag_DEPS_TARGET
                              abseil_absl_flags_commandlineflag_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_flags_commandlineflag"
                              "${abseil_absl_flags_commandlineflag_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::flags_commandlineflag
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_commandlineflag_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flags_commandlineflag_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_flags_commandlineflag_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::flags_commandlineflag
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_flags_commandlineflag_DEPS_TARGET)
        endif()

        set_property(TARGET absl::flags_commandlineflag APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_flags_commandlineflag_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::flags_commandlineflag APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_commandlineflag_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::flags_commandlineflag APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_commandlineflag_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::flags_commandlineflag APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_flags_commandlineflag_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::flags_commandlineflag APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_flags_commandlineflag_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::flags_path_util #############

        set(abseil_absl_flags_path_util_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_flags_path_util_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_flags_path_util_FRAMEWORKS_RELEASE}" "${abseil_absl_flags_path_util_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_flags_path_util_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_flags_path_util_DEPS_TARGET)
            add_library(abseil_absl_flags_path_util_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_flags_path_util_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_path_util_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flags_path_util_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flags_path_util_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_flags_path_util_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_flags_path_util_LIBS_RELEASE}"
                              "${abseil_absl_flags_path_util_LIB_DIRS_RELEASE}"
                              "${abseil_absl_flags_path_util_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_flags_path_util_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_flags_path_util_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_flags_path_util_DEPS_TARGET
                              abseil_absl_flags_path_util_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_flags_path_util"
                              "${abseil_absl_flags_path_util_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::flags_path_util
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_path_util_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flags_path_util_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_flags_path_util_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::flags_path_util
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_flags_path_util_DEPS_TARGET)
        endif()

        set_property(TARGET absl::flags_path_util APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_flags_path_util_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::flags_path_util APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_path_util_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::flags_path_util APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_path_util_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::flags_path_util APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_flags_path_util_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::flags_path_util APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_flags_path_util_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::demangle_rust #############

        set(abseil_absl_demangle_rust_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_demangle_rust_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_demangle_rust_FRAMEWORKS_RELEASE}" "${abseil_absl_demangle_rust_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_demangle_rust_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_demangle_rust_DEPS_TARGET)
            add_library(abseil_absl_demangle_rust_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_demangle_rust_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_demangle_rust_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_demangle_rust_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_demangle_rust_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_demangle_rust_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_demangle_rust_LIBS_RELEASE}"
                              "${abseil_absl_demangle_rust_LIB_DIRS_RELEASE}"
                              "${abseil_absl_demangle_rust_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_demangle_rust_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_demangle_rust_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_demangle_rust_DEPS_TARGET
                              abseil_absl_demangle_rust_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_demangle_rust"
                              "${abseil_absl_demangle_rust_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::demangle_rust
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_demangle_rust_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_demangle_rust_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_demangle_rust_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::demangle_rust
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_demangle_rust_DEPS_TARGET)
        endif()

        set_property(TARGET absl::demangle_rust APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_demangle_rust_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::demangle_rust APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_demangle_rust_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::demangle_rust APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_demangle_rust_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::demangle_rust APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_demangle_rust_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::demangle_rust APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_demangle_rust_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::layout #############

        set(abseil_absl_layout_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_layout_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_layout_FRAMEWORKS_RELEASE}" "${abseil_absl_layout_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_layout_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_layout_DEPS_TARGET)
            add_library(abseil_absl_layout_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_layout_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_layout_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_layout_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_layout_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_layout_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_layout_LIBS_RELEASE}"
                              "${abseil_absl_layout_LIB_DIRS_RELEASE}"
                              "${abseil_absl_layout_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_layout_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_layout_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_layout_DEPS_TARGET
                              abseil_absl_layout_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_layout"
                              "${abseil_absl_layout_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::layout
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_layout_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_layout_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_layout_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::layout
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_layout_DEPS_TARGET)
        endif()

        set_property(TARGET absl::layout APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_layout_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::layout APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_layout_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::layout APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_layout_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::layout APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_layout_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::layout APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_layout_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::strings #############

        set(abseil_absl_strings_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_strings_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_strings_FRAMEWORKS_RELEASE}" "${abseil_absl_strings_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_strings_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_strings_DEPS_TARGET)
            add_library(abseil_absl_strings_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_strings_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_strings_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_strings_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_strings_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_strings_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_strings_LIBS_RELEASE}"
                              "${abseil_absl_strings_LIB_DIRS_RELEASE}"
                              "${abseil_absl_strings_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_strings_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_strings_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_strings_DEPS_TARGET
                              abseil_absl_strings_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_strings"
                              "${abseil_absl_strings_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::strings
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_strings_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_strings_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_strings_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::strings
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_strings_DEPS_TARGET)
        endif()

        set_property(TARGET absl::strings APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_strings_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::strings APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_strings_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::strings APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_strings_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::strings APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_strings_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::strings APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_strings_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::random_internal_wide_multiply #############

        set(abseil_absl_random_internal_wide_multiply_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_random_internal_wide_multiply_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_random_internal_wide_multiply_FRAMEWORKS_RELEASE}" "${abseil_absl_random_internal_wide_multiply_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_random_internal_wide_multiply_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_random_internal_wide_multiply_DEPS_TARGET)
            add_library(abseil_absl_random_internal_wide_multiply_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_random_internal_wide_multiply_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_wide_multiply_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_wide_multiply_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_wide_multiply_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_random_internal_wide_multiply_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_random_internal_wide_multiply_LIBS_RELEASE}"
                              "${abseil_absl_random_internal_wide_multiply_LIB_DIRS_RELEASE}"
                              "${abseil_absl_random_internal_wide_multiply_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_random_internal_wide_multiply_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_random_internal_wide_multiply_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_random_internal_wide_multiply_DEPS_TARGET
                              abseil_absl_random_internal_wide_multiply_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_random_internal_wide_multiply"
                              "${abseil_absl_random_internal_wide_multiply_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::random_internal_wide_multiply
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_wide_multiply_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_wide_multiply_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_random_internal_wide_multiply_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::random_internal_wide_multiply
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_random_internal_wide_multiply_DEPS_TARGET)
        endif()

        set_property(TARGET absl::random_internal_wide_multiply APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_wide_multiply_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::random_internal_wide_multiply APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_wide_multiply_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::random_internal_wide_multiply APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_wide_multiply_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::random_internal_wide_multiply APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_wide_multiply_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::random_internal_wide_multiply APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_wide_multiply_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::random_internal_generate_real #############

        set(abseil_absl_random_internal_generate_real_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_random_internal_generate_real_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_random_internal_generate_real_FRAMEWORKS_RELEASE}" "${abseil_absl_random_internal_generate_real_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_random_internal_generate_real_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_random_internal_generate_real_DEPS_TARGET)
            add_library(abseil_absl_random_internal_generate_real_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_random_internal_generate_real_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_generate_real_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_generate_real_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_generate_real_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_random_internal_generate_real_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_random_internal_generate_real_LIBS_RELEASE}"
                              "${abseil_absl_random_internal_generate_real_LIB_DIRS_RELEASE}"
                              "${abseil_absl_random_internal_generate_real_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_random_internal_generate_real_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_random_internal_generate_real_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_random_internal_generate_real_DEPS_TARGET
                              abseil_absl_random_internal_generate_real_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_random_internal_generate_real"
                              "${abseil_absl_random_internal_generate_real_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::random_internal_generate_real
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_generate_real_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_generate_real_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_random_internal_generate_real_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::random_internal_generate_real
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_random_internal_generate_real_DEPS_TARGET)
        endif()

        set_property(TARGET absl::random_internal_generate_real APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_generate_real_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::random_internal_generate_real APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_generate_real_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::random_internal_generate_real APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_generate_real_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::random_internal_generate_real APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_generate_real_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::random_internal_generate_real APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_generate_real_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::random_internal_iostream_state_saver #############

        set(abseil_absl_random_internal_iostream_state_saver_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_random_internal_iostream_state_saver_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_random_internal_iostream_state_saver_FRAMEWORKS_RELEASE}" "${abseil_absl_random_internal_iostream_state_saver_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_random_internal_iostream_state_saver_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_random_internal_iostream_state_saver_DEPS_TARGET)
            add_library(abseil_absl_random_internal_iostream_state_saver_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_random_internal_iostream_state_saver_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_iostream_state_saver_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_iostream_state_saver_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_iostream_state_saver_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_random_internal_iostream_state_saver_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_random_internal_iostream_state_saver_LIBS_RELEASE}"
                              "${abseil_absl_random_internal_iostream_state_saver_LIB_DIRS_RELEASE}"
                              "${abseil_absl_random_internal_iostream_state_saver_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_random_internal_iostream_state_saver_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_random_internal_iostream_state_saver_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_random_internal_iostream_state_saver_DEPS_TARGET
                              abseil_absl_random_internal_iostream_state_saver_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_random_internal_iostream_state_saver"
                              "${abseil_absl_random_internal_iostream_state_saver_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::random_internal_iostream_state_saver
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_iostream_state_saver_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_iostream_state_saver_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_random_internal_iostream_state_saver_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::random_internal_iostream_state_saver
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_random_internal_iostream_state_saver_DEPS_TARGET)
        endif()

        set_property(TARGET absl::random_internal_iostream_state_saver APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_iostream_state_saver_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::random_internal_iostream_state_saver APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_iostream_state_saver_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::random_internal_iostream_state_saver APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_iostream_state_saver_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::random_internal_iostream_state_saver APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_iostream_state_saver_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::random_internal_iostream_state_saver APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_iostream_state_saver_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::numeric #############

        set(abseil_absl_numeric_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_numeric_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_numeric_FRAMEWORKS_RELEASE}" "${abseil_absl_numeric_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_numeric_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_numeric_DEPS_TARGET)
            add_library(abseil_absl_numeric_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_numeric_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_numeric_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_numeric_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_numeric_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_numeric_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_numeric_LIBS_RELEASE}"
                              "${abseil_absl_numeric_LIB_DIRS_RELEASE}"
                              "${abseil_absl_numeric_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_numeric_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_numeric_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_numeric_DEPS_TARGET
                              abseil_absl_numeric_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_numeric"
                              "${abseil_absl_numeric_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::numeric
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_numeric_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_numeric_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_numeric_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::numeric
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_numeric_DEPS_TARGET)
        endif()

        set_property(TARGET absl::numeric APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_numeric_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::numeric APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_numeric_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::numeric APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_numeric_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::numeric APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_numeric_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::numeric APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_numeric_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::decode_rust_punycode #############

        set(abseil_absl_decode_rust_punycode_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_decode_rust_punycode_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_decode_rust_punycode_FRAMEWORKS_RELEASE}" "${abseil_absl_decode_rust_punycode_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_decode_rust_punycode_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_decode_rust_punycode_DEPS_TARGET)
            add_library(abseil_absl_decode_rust_punycode_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_decode_rust_punycode_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_decode_rust_punycode_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_decode_rust_punycode_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_decode_rust_punycode_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_decode_rust_punycode_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_decode_rust_punycode_LIBS_RELEASE}"
                              "${abseil_absl_decode_rust_punycode_LIB_DIRS_RELEASE}"
                              "${abseil_absl_decode_rust_punycode_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_decode_rust_punycode_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_decode_rust_punycode_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_decode_rust_punycode_DEPS_TARGET
                              abseil_absl_decode_rust_punycode_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_decode_rust_punycode"
                              "${abseil_absl_decode_rust_punycode_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::decode_rust_punycode
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_decode_rust_punycode_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_decode_rust_punycode_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_decode_rust_punycode_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::decode_rust_punycode
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_decode_rust_punycode_DEPS_TARGET)
        endif()

        set_property(TARGET absl::decode_rust_punycode APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_decode_rust_punycode_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::decode_rust_punycode APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_decode_rust_punycode_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::decode_rust_punycode APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_decode_rust_punycode_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::decode_rust_punycode APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_decode_rust_punycode_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::decode_rust_punycode APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_decode_rust_punycode_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::random_internal_fastmath #############

        set(abseil_absl_random_internal_fastmath_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_random_internal_fastmath_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_random_internal_fastmath_FRAMEWORKS_RELEASE}" "${abseil_absl_random_internal_fastmath_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_random_internal_fastmath_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_random_internal_fastmath_DEPS_TARGET)
            add_library(abseil_absl_random_internal_fastmath_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_random_internal_fastmath_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_fastmath_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_fastmath_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_fastmath_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_random_internal_fastmath_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_random_internal_fastmath_LIBS_RELEASE}"
                              "${abseil_absl_random_internal_fastmath_LIB_DIRS_RELEASE}"
                              "${abseil_absl_random_internal_fastmath_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_random_internal_fastmath_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_random_internal_fastmath_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_random_internal_fastmath_DEPS_TARGET
                              abseil_absl_random_internal_fastmath_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_random_internal_fastmath"
                              "${abseil_absl_random_internal_fastmath_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::random_internal_fastmath
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_fastmath_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_fastmath_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_random_internal_fastmath_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::random_internal_fastmath
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_random_internal_fastmath_DEPS_TARGET)
        endif()

        set_property(TARGET absl::random_internal_fastmath APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_fastmath_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::random_internal_fastmath APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_fastmath_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::random_internal_fastmath APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_fastmath_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::random_internal_fastmath APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_fastmath_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::random_internal_fastmath APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_fastmath_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::int128 #############

        set(abseil_absl_int128_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_int128_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_int128_FRAMEWORKS_RELEASE}" "${abseil_absl_int128_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_int128_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_int128_DEPS_TARGET)
            add_library(abseil_absl_int128_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_int128_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_int128_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_int128_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_int128_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_int128_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_int128_LIBS_RELEASE}"
                              "${abseil_absl_int128_LIB_DIRS_RELEASE}"
                              "${abseil_absl_int128_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_int128_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_int128_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_int128_DEPS_TARGET
                              abseil_absl_int128_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_int128"
                              "${abseil_absl_int128_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::int128
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_int128_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_int128_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_int128_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::int128
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_int128_DEPS_TARGET)
        endif()

        set_property(TARGET absl::int128 APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_int128_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::int128 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_int128_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::int128 APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_int128_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::int128 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_int128_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::int128 APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_int128_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::bounded_utf8_length_sequence #############

        set(abseil_absl_bounded_utf8_length_sequence_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_bounded_utf8_length_sequence_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_bounded_utf8_length_sequence_FRAMEWORKS_RELEASE}" "${abseil_absl_bounded_utf8_length_sequence_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_bounded_utf8_length_sequence_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_bounded_utf8_length_sequence_DEPS_TARGET)
            add_library(abseil_absl_bounded_utf8_length_sequence_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_bounded_utf8_length_sequence_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_bounded_utf8_length_sequence_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_bounded_utf8_length_sequence_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_bounded_utf8_length_sequence_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_bounded_utf8_length_sequence_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_bounded_utf8_length_sequence_LIBS_RELEASE}"
                              "${abseil_absl_bounded_utf8_length_sequence_LIB_DIRS_RELEASE}"
                              "${abseil_absl_bounded_utf8_length_sequence_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_bounded_utf8_length_sequence_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_bounded_utf8_length_sequence_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_bounded_utf8_length_sequence_DEPS_TARGET
                              abseil_absl_bounded_utf8_length_sequence_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_bounded_utf8_length_sequence"
                              "${abseil_absl_bounded_utf8_length_sequence_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::bounded_utf8_length_sequence
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_bounded_utf8_length_sequence_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_bounded_utf8_length_sequence_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_bounded_utf8_length_sequence_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::bounded_utf8_length_sequence
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_bounded_utf8_length_sequence_DEPS_TARGET)
        endif()

        set_property(TARGET absl::bounded_utf8_length_sequence APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_bounded_utf8_length_sequence_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::bounded_utf8_length_sequence APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_bounded_utf8_length_sequence_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::bounded_utf8_length_sequence APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_bounded_utf8_length_sequence_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::bounded_utf8_length_sequence APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_bounded_utf8_length_sequence_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::bounded_utf8_length_sequence APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_bounded_utf8_length_sequence_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::crc_internal #############

        set(abseil_absl_crc_internal_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_crc_internal_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_crc_internal_FRAMEWORKS_RELEASE}" "${abseil_absl_crc_internal_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_crc_internal_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_crc_internal_DEPS_TARGET)
            add_library(abseil_absl_crc_internal_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_crc_internal_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_crc_internal_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_crc_internal_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_crc_internal_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_crc_internal_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_crc_internal_LIBS_RELEASE}"
                              "${abseil_absl_crc_internal_LIB_DIRS_RELEASE}"
                              "${abseil_absl_crc_internal_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_crc_internal_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_crc_internal_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_crc_internal_DEPS_TARGET
                              abseil_absl_crc_internal_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_crc_internal"
                              "${abseil_absl_crc_internal_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::crc_internal
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_crc_internal_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_crc_internal_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_crc_internal_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::crc_internal
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_crc_internal_DEPS_TARGET)
        endif()

        set_property(TARGET absl::crc_internal APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_crc_internal_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::crc_internal APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_crc_internal_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::crc_internal APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_crc_internal_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::crc_internal APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_crc_internal_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::crc_internal APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_crc_internal_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::hashtable_control_bytes #############

        set(abseil_absl_hashtable_control_bytes_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_hashtable_control_bytes_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_hashtable_control_bytes_FRAMEWORKS_RELEASE}" "${abseil_absl_hashtable_control_bytes_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_hashtable_control_bytes_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_hashtable_control_bytes_DEPS_TARGET)
            add_library(abseil_absl_hashtable_control_bytes_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_hashtable_control_bytes_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_hashtable_control_bytes_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_hashtable_control_bytes_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_hashtable_control_bytes_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_hashtable_control_bytes_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_hashtable_control_bytes_LIBS_RELEASE}"
                              "${abseil_absl_hashtable_control_bytes_LIB_DIRS_RELEASE}"
                              "${abseil_absl_hashtable_control_bytes_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_hashtable_control_bytes_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_hashtable_control_bytes_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_hashtable_control_bytes_DEPS_TARGET
                              abseil_absl_hashtable_control_bytes_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_hashtable_control_bytes"
                              "${abseil_absl_hashtable_control_bytes_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::hashtable_control_bytes
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_hashtable_control_bytes_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_hashtable_control_bytes_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_hashtable_control_bytes_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::hashtable_control_bytes
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_hashtable_control_bytes_DEPS_TARGET)
        endif()

        set_property(TARGET absl::hashtable_control_bytes APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_hashtable_control_bytes_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::hashtable_control_bytes APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_hashtable_control_bytes_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::hashtable_control_bytes APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_hashtable_control_bytes_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::hashtable_control_bytes APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_hashtable_control_bytes_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::hashtable_control_bytes APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_hashtable_control_bytes_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::graphcycles_internal #############

        set(abseil_absl_graphcycles_internal_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_graphcycles_internal_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_graphcycles_internal_FRAMEWORKS_RELEASE}" "${abseil_absl_graphcycles_internal_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_graphcycles_internal_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_graphcycles_internal_DEPS_TARGET)
            add_library(abseil_absl_graphcycles_internal_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_graphcycles_internal_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_graphcycles_internal_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_graphcycles_internal_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_graphcycles_internal_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_graphcycles_internal_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_graphcycles_internal_LIBS_RELEASE}"
                              "${abseil_absl_graphcycles_internal_LIB_DIRS_RELEASE}"
                              "${abseil_absl_graphcycles_internal_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_graphcycles_internal_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_graphcycles_internal_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_graphcycles_internal_DEPS_TARGET
                              abseil_absl_graphcycles_internal_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_graphcycles_internal"
                              "${abseil_absl_graphcycles_internal_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::graphcycles_internal
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_graphcycles_internal_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_graphcycles_internal_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_graphcycles_internal_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::graphcycles_internal
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_graphcycles_internal_DEPS_TARGET)
        endif()

        set_property(TARGET absl::graphcycles_internal APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_graphcycles_internal_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::graphcycles_internal APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_graphcycles_internal_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::graphcycles_internal APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_graphcycles_internal_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::graphcycles_internal APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_graphcycles_internal_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::graphcycles_internal APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_graphcycles_internal_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::strings_internal #############

        set(abseil_absl_strings_internal_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_strings_internal_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_strings_internal_FRAMEWORKS_RELEASE}" "${abseil_absl_strings_internal_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_strings_internal_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_strings_internal_DEPS_TARGET)
            add_library(abseil_absl_strings_internal_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_strings_internal_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_strings_internal_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_strings_internal_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_strings_internal_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_strings_internal_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_strings_internal_LIBS_RELEASE}"
                              "${abseil_absl_strings_internal_LIB_DIRS_RELEASE}"
                              "${abseil_absl_strings_internal_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_strings_internal_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_strings_internal_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_strings_internal_DEPS_TARGET
                              abseil_absl_strings_internal_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_strings_internal"
                              "${abseil_absl_strings_internal_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::strings_internal
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_strings_internal_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_strings_internal_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_strings_internal_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::strings_internal
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_strings_internal_DEPS_TARGET)
        endif()

        set_property(TARGET absl::strings_internal APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_strings_internal_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::strings_internal APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_strings_internal_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::strings_internal APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_strings_internal_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::strings_internal APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_strings_internal_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::strings_internal APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_strings_internal_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::bits #############

        set(abseil_absl_bits_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_bits_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_bits_FRAMEWORKS_RELEASE}" "${abseil_absl_bits_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_bits_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_bits_DEPS_TARGET)
            add_library(abseil_absl_bits_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_bits_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_bits_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_bits_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_bits_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_bits_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_bits_LIBS_RELEASE}"
                              "${abseil_absl_bits_LIB_DIRS_RELEASE}"
                              "${abseil_absl_bits_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_bits_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_bits_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_bits_DEPS_TARGET
                              abseil_absl_bits_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_bits"
                              "${abseil_absl_bits_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::bits
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_bits_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_bits_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_bits_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::bits
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_bits_DEPS_TARGET)
        endif()

        set_property(TARGET absl::bits APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_bits_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::bits APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_bits_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::bits APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_bits_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::bits APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_bits_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::bits APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_bits_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::city #############

        set(abseil_absl_city_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_city_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_city_FRAMEWORKS_RELEASE}" "${abseil_absl_city_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_city_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_city_DEPS_TARGET)
            add_library(abseil_absl_city_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_city_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_city_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_city_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_city_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_city_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_city_LIBS_RELEASE}"
                              "${abseil_absl_city_LIB_DIRS_RELEASE}"
                              "${abseil_absl_city_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_city_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_city_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_city_DEPS_TARGET
                              abseil_absl_city_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_city"
                              "${abseil_absl_city_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::city
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_city_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_city_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_city_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::city
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_city_DEPS_TARGET)
        endif()

        set_property(TARGET absl::city APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_city_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::city APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_city_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::city APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_city_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::city APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_city_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::city APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_city_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::function_ref #############

        set(abseil_absl_function_ref_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_function_ref_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_function_ref_FRAMEWORKS_RELEASE}" "${abseil_absl_function_ref_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_function_ref_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_function_ref_DEPS_TARGET)
            add_library(abseil_absl_function_ref_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_function_ref_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_function_ref_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_function_ref_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_function_ref_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_function_ref_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_function_ref_LIBS_RELEASE}"
                              "${abseil_absl_function_ref_LIB_DIRS_RELEASE}"
                              "${abseil_absl_function_ref_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_function_ref_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_function_ref_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_function_ref_DEPS_TARGET
                              abseil_absl_function_ref_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_function_ref"
                              "${abseil_absl_function_ref_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::function_ref
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_function_ref_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_function_ref_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_function_ref_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::function_ref
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_function_ref_DEPS_TARGET)
        endif()

        set_property(TARGET absl::function_ref APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_function_ref_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::function_ref APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_function_ref_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::function_ref APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_function_ref_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::function_ref APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_function_ref_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::function_ref APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_function_ref_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::inlined_vector #############

        set(abseil_absl_inlined_vector_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_inlined_vector_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_inlined_vector_FRAMEWORKS_RELEASE}" "${abseil_absl_inlined_vector_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_inlined_vector_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_inlined_vector_DEPS_TARGET)
            add_library(abseil_absl_inlined_vector_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_inlined_vector_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_inlined_vector_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_inlined_vector_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_inlined_vector_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_inlined_vector_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_inlined_vector_LIBS_RELEASE}"
                              "${abseil_absl_inlined_vector_LIB_DIRS_RELEASE}"
                              "${abseil_absl_inlined_vector_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_inlined_vector_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_inlined_vector_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_inlined_vector_DEPS_TARGET
                              abseil_absl_inlined_vector_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_inlined_vector"
                              "${abseil_absl_inlined_vector_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::inlined_vector
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_inlined_vector_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_inlined_vector_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_inlined_vector_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::inlined_vector
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_inlined_vector_DEPS_TARGET)
        endif()

        set_property(TARGET absl::inlined_vector APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_inlined_vector_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::inlined_vector APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_inlined_vector_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::inlined_vector APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_inlined_vector_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::inlined_vector APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_inlined_vector_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::inlined_vector APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_inlined_vector_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::poison #############

        set(abseil_absl_poison_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_poison_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_poison_FRAMEWORKS_RELEASE}" "${abseil_absl_poison_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_poison_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_poison_DEPS_TARGET)
            add_library(abseil_absl_poison_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_poison_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_poison_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_poison_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_poison_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_poison_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_poison_LIBS_RELEASE}"
                              "${abseil_absl_poison_LIB_DIRS_RELEASE}"
                              "${abseil_absl_poison_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_poison_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_poison_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_poison_DEPS_TARGET
                              abseil_absl_poison_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_poison"
                              "${abseil_absl_poison_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::poison
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_poison_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_poison_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_poison_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::poison
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_poison_DEPS_TARGET)
        endif()

        set_property(TARGET absl::poison APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_poison_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::poison APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_poison_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::poison APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_poison_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::poison APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_poison_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::poison APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_poison_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::strings_append_and_overwrite #############

        set(abseil_absl_strings_append_and_overwrite_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_strings_append_and_overwrite_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_strings_append_and_overwrite_FRAMEWORKS_RELEASE}" "${abseil_absl_strings_append_and_overwrite_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_strings_append_and_overwrite_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_strings_append_and_overwrite_DEPS_TARGET)
            add_library(abseil_absl_strings_append_and_overwrite_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_strings_append_and_overwrite_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_strings_append_and_overwrite_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_strings_append_and_overwrite_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_strings_append_and_overwrite_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_strings_append_and_overwrite_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_strings_append_and_overwrite_LIBS_RELEASE}"
                              "${abseil_absl_strings_append_and_overwrite_LIB_DIRS_RELEASE}"
                              "${abseil_absl_strings_append_and_overwrite_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_strings_append_and_overwrite_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_strings_append_and_overwrite_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_strings_append_and_overwrite_DEPS_TARGET
                              abseil_absl_strings_append_and_overwrite_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_strings_append_and_overwrite"
                              "${abseil_absl_strings_append_and_overwrite_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::strings_append_and_overwrite
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_strings_append_and_overwrite_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_strings_append_and_overwrite_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_strings_append_and_overwrite_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::strings_append_and_overwrite
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_strings_append_and_overwrite_DEPS_TARGET)
        endif()

        set_property(TARGET absl::strings_append_and_overwrite APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_strings_append_and_overwrite_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::strings_append_and_overwrite APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_strings_append_and_overwrite_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::strings_append_and_overwrite APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_strings_append_and_overwrite_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::strings_append_and_overwrite APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_strings_append_and_overwrite_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::strings_append_and_overwrite APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_strings_append_and_overwrite_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::random_internal_randen #############

        set(abseil_absl_random_internal_randen_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_random_internal_randen_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_random_internal_randen_FRAMEWORKS_RELEASE}" "${abseil_absl_random_internal_randen_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_random_internal_randen_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_random_internal_randen_DEPS_TARGET)
            add_library(abseil_absl_random_internal_randen_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_random_internal_randen_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_random_internal_randen_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_random_internal_randen_LIBS_RELEASE}"
                              "${abseil_absl_random_internal_randen_LIB_DIRS_RELEASE}"
                              "${abseil_absl_random_internal_randen_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_random_internal_randen_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_random_internal_randen_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_random_internal_randen_DEPS_TARGET
                              abseil_absl_random_internal_randen_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_random_internal_randen"
                              "${abseil_absl_random_internal_randen_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::random_internal_randen
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_random_internal_randen_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::random_internal_randen
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_random_internal_randen_DEPS_TARGET)
        endif()

        set_property(TARGET absl::random_internal_randen APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::random_internal_randen APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::random_internal_randen APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::random_internal_randen APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::random_internal_randen APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::log_internal_conditions #############

        set(abseil_absl_log_internal_conditions_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_log_internal_conditions_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_log_internal_conditions_FRAMEWORKS_RELEASE}" "${abseil_absl_log_internal_conditions_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_log_internal_conditions_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_log_internal_conditions_DEPS_TARGET)
            add_library(abseil_absl_log_internal_conditions_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_log_internal_conditions_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_conditions_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_conditions_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_conditions_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_log_internal_conditions_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_log_internal_conditions_LIBS_RELEASE}"
                              "${abseil_absl_log_internal_conditions_LIB_DIRS_RELEASE}"
                              "${abseil_absl_log_internal_conditions_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_log_internal_conditions_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_log_internal_conditions_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_log_internal_conditions_DEPS_TARGET
                              abseil_absl_log_internal_conditions_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_log_internal_conditions"
                              "${abseil_absl_log_internal_conditions_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::log_internal_conditions
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_conditions_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_conditions_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_log_internal_conditions_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::log_internal_conditions
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_log_internal_conditions_DEPS_TARGET)
        endif()

        set_property(TARGET absl::log_internal_conditions APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_conditions_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::log_internal_conditions APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_conditions_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::log_internal_conditions APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_conditions_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::log_internal_conditions APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_conditions_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::log_internal_conditions APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_conditions_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::any_invocable #############

        set(abseil_absl_any_invocable_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_any_invocable_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_any_invocable_FRAMEWORKS_RELEASE}" "${abseil_absl_any_invocable_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_any_invocable_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_any_invocable_DEPS_TARGET)
            add_library(abseil_absl_any_invocable_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_any_invocable_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_any_invocable_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_any_invocable_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_any_invocable_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_any_invocable_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_any_invocable_LIBS_RELEASE}"
                              "${abseil_absl_any_invocable_LIB_DIRS_RELEASE}"
                              "${abseil_absl_any_invocable_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_any_invocable_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_any_invocable_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_any_invocable_DEPS_TARGET
                              abseil_absl_any_invocable_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_any_invocable"
                              "${abseil_absl_any_invocable_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::any_invocable
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_any_invocable_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_any_invocable_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_any_invocable_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::any_invocable
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_any_invocable_DEPS_TARGET)
        endif()

        set_property(TARGET absl::any_invocable APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_any_invocable_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::any_invocable APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_any_invocable_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::any_invocable APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_any_invocable_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::any_invocable APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_any_invocable_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::any_invocable APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_any_invocable_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::crc_cpu_detect #############

        set(abseil_absl_crc_cpu_detect_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_crc_cpu_detect_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_crc_cpu_detect_FRAMEWORKS_RELEASE}" "${abseil_absl_crc_cpu_detect_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_crc_cpu_detect_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_crc_cpu_detect_DEPS_TARGET)
            add_library(abseil_absl_crc_cpu_detect_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_crc_cpu_detect_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_crc_cpu_detect_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_crc_cpu_detect_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_crc_cpu_detect_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_crc_cpu_detect_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_crc_cpu_detect_LIBS_RELEASE}"
                              "${abseil_absl_crc_cpu_detect_LIB_DIRS_RELEASE}"
                              "${abseil_absl_crc_cpu_detect_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_crc_cpu_detect_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_crc_cpu_detect_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_crc_cpu_detect_DEPS_TARGET
                              abseil_absl_crc_cpu_detect_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_crc_cpu_detect"
                              "${abseil_absl_crc_cpu_detect_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::crc_cpu_detect
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_crc_cpu_detect_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_crc_cpu_detect_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_crc_cpu_detect_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::crc_cpu_detect
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_crc_cpu_detect_DEPS_TARGET)
        endif()

        set_property(TARGET absl::crc_cpu_detect APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_crc_cpu_detect_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::crc_cpu_detect APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_crc_cpu_detect_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::crc_cpu_detect APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_crc_cpu_detect_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::crc_cpu_detect APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_crc_cpu_detect_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::crc_cpu_detect APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_crc_cpu_detect_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::inlined_vector_internal #############

        set(abseil_absl_inlined_vector_internal_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_inlined_vector_internal_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_inlined_vector_internal_FRAMEWORKS_RELEASE}" "${abseil_absl_inlined_vector_internal_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_inlined_vector_internal_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_inlined_vector_internal_DEPS_TARGET)
            add_library(abseil_absl_inlined_vector_internal_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_inlined_vector_internal_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_inlined_vector_internal_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_inlined_vector_internal_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_inlined_vector_internal_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_inlined_vector_internal_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_inlined_vector_internal_LIBS_RELEASE}"
                              "${abseil_absl_inlined_vector_internal_LIB_DIRS_RELEASE}"
                              "${abseil_absl_inlined_vector_internal_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_inlined_vector_internal_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_inlined_vector_internal_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_inlined_vector_internal_DEPS_TARGET
                              abseil_absl_inlined_vector_internal_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_inlined_vector_internal"
                              "${abseil_absl_inlined_vector_internal_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::inlined_vector_internal
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_inlined_vector_internal_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_inlined_vector_internal_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_inlined_vector_internal_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::inlined_vector_internal
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_inlined_vector_internal_DEPS_TARGET)
        endif()

        set_property(TARGET absl::inlined_vector_internal APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_inlined_vector_internal_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::inlined_vector_internal APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_inlined_vector_internal_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::inlined_vector_internal APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_inlined_vector_internal_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::inlined_vector_internal APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_inlined_vector_internal_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::inlined_vector_internal APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_inlined_vector_internal_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::tracing_internal #############

        set(abseil_absl_tracing_internal_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_tracing_internal_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_tracing_internal_FRAMEWORKS_RELEASE}" "${abseil_absl_tracing_internal_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_tracing_internal_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_tracing_internal_DEPS_TARGET)
            add_library(abseil_absl_tracing_internal_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_tracing_internal_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_tracing_internal_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_tracing_internal_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_tracing_internal_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_tracing_internal_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_tracing_internal_LIBS_RELEASE}"
                              "${abseil_absl_tracing_internal_LIB_DIRS_RELEASE}"
                              "${abseil_absl_tracing_internal_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_tracing_internal_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_tracing_internal_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_tracing_internal_DEPS_TARGET
                              abseil_absl_tracing_internal_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_tracing_internal"
                              "${abseil_absl_tracing_internal_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::tracing_internal
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_tracing_internal_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_tracing_internal_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_tracing_internal_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::tracing_internal
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_tracing_internal_DEPS_TARGET)
        endif()

        set_property(TARGET absl::tracing_internal APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_tracing_internal_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::tracing_internal APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_tracing_internal_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::tracing_internal APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_tracing_internal_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::tracing_internal APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_tracing_internal_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::tracing_internal APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_tracing_internal_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::endian #############

        set(abseil_absl_endian_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_endian_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_endian_FRAMEWORKS_RELEASE}" "${abseil_absl_endian_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_endian_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_endian_DEPS_TARGET)
            add_library(abseil_absl_endian_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_endian_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_endian_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_endian_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_endian_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_endian_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_endian_LIBS_RELEASE}"
                              "${abseil_absl_endian_LIB_DIRS_RELEASE}"
                              "${abseil_absl_endian_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_endian_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_endian_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_endian_DEPS_TARGET
                              abseil_absl_endian_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_endian"
                              "${abseil_absl_endian_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::endian
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_endian_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_endian_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_endian_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::endian
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_endian_DEPS_TARGET)
        endif()

        set_property(TARGET absl::endian APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_endian_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::endian APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_endian_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::endian APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_endian_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::endian APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_endian_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::endian APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_endian_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::malloc_internal #############

        set(abseil_absl_malloc_internal_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_malloc_internal_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_malloc_internal_FRAMEWORKS_RELEASE}" "${abseil_absl_malloc_internal_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_malloc_internal_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_malloc_internal_DEPS_TARGET)
            add_library(abseil_absl_malloc_internal_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_malloc_internal_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_malloc_internal_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_malloc_internal_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_malloc_internal_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_malloc_internal_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_malloc_internal_LIBS_RELEASE}"
                              "${abseil_absl_malloc_internal_LIB_DIRS_RELEASE}"
                              "${abseil_absl_malloc_internal_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_malloc_internal_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_malloc_internal_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_malloc_internal_DEPS_TARGET
                              abseil_absl_malloc_internal_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_malloc_internal"
                              "${abseil_absl_malloc_internal_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::malloc_internal
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_malloc_internal_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_malloc_internal_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_malloc_internal_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::malloc_internal
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_malloc_internal_DEPS_TARGET)
        endif()

        set_property(TARGET absl::malloc_internal APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_malloc_internal_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::malloc_internal APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_malloc_internal_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::malloc_internal APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_malloc_internal_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::malloc_internal APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_malloc_internal_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::malloc_internal APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_malloc_internal_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::span #############

        set(abseil_absl_span_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_span_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_span_FRAMEWORKS_RELEASE}" "${abseil_absl_span_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_span_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_span_DEPS_TARGET)
            add_library(abseil_absl_span_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_span_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_span_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_span_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_span_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_span_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_span_LIBS_RELEASE}"
                              "${abseil_absl_span_LIB_DIRS_RELEASE}"
                              "${abseil_absl_span_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_span_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_span_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_span_DEPS_TARGET
                              abseil_absl_span_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_span"
                              "${abseil_absl_span_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::span
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_span_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_span_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_span_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::span
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_span_DEPS_TARGET)
        endif()

        set_property(TARGET absl::span APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_span_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::span APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_span_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::span APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_span_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::span APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_span_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::span APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_span_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::strings_resize_and_overwrite #############

        set(abseil_absl_strings_resize_and_overwrite_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_strings_resize_and_overwrite_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_strings_resize_and_overwrite_FRAMEWORKS_RELEASE}" "${abseil_absl_strings_resize_and_overwrite_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_strings_resize_and_overwrite_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_strings_resize_and_overwrite_DEPS_TARGET)
            add_library(abseil_absl_strings_resize_and_overwrite_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_strings_resize_and_overwrite_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_strings_resize_and_overwrite_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_strings_resize_and_overwrite_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_strings_resize_and_overwrite_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_strings_resize_and_overwrite_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_strings_resize_and_overwrite_LIBS_RELEASE}"
                              "${abseil_absl_strings_resize_and_overwrite_LIB_DIRS_RELEASE}"
                              "${abseil_absl_strings_resize_and_overwrite_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_strings_resize_and_overwrite_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_strings_resize_and_overwrite_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_strings_resize_and_overwrite_DEPS_TARGET
                              abseil_absl_strings_resize_and_overwrite_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_strings_resize_and_overwrite"
                              "${abseil_absl_strings_resize_and_overwrite_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::strings_resize_and_overwrite
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_strings_resize_and_overwrite_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_strings_resize_and_overwrite_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_strings_resize_and_overwrite_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::strings_resize_and_overwrite
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_strings_resize_and_overwrite_DEPS_TARGET)
        endif()

        set_property(TARGET absl::strings_resize_and_overwrite APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_strings_resize_and_overwrite_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::strings_resize_and_overwrite APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_strings_resize_and_overwrite_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::strings_resize_and_overwrite APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_strings_resize_and_overwrite_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::strings_resize_and_overwrite APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_strings_resize_and_overwrite_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::strings_resize_and_overwrite APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_strings_resize_and_overwrite_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::random_internal_randen_hwaes #############

        set(abseil_absl_random_internal_randen_hwaes_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_random_internal_randen_hwaes_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_random_internal_randen_hwaes_FRAMEWORKS_RELEASE}" "${abseil_absl_random_internal_randen_hwaes_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_random_internal_randen_hwaes_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_random_internal_randen_hwaes_DEPS_TARGET)
            add_library(abseil_absl_random_internal_randen_hwaes_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_random_internal_randen_hwaes_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_hwaes_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_hwaes_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_hwaes_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_random_internal_randen_hwaes_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_random_internal_randen_hwaes_LIBS_RELEASE}"
                              "${abseil_absl_random_internal_randen_hwaes_LIB_DIRS_RELEASE}"
                              "${abseil_absl_random_internal_randen_hwaes_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_random_internal_randen_hwaes_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_random_internal_randen_hwaes_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_random_internal_randen_hwaes_DEPS_TARGET
                              abseil_absl_random_internal_randen_hwaes_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_random_internal_randen_hwaes"
                              "${abseil_absl_random_internal_randen_hwaes_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::random_internal_randen_hwaes
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_hwaes_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_hwaes_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_random_internal_randen_hwaes_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::random_internal_randen_hwaes
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_random_internal_randen_hwaes_DEPS_TARGET)
        endif()

        set_property(TARGET absl::random_internal_randen_hwaes APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_hwaes_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::random_internal_randen_hwaes APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_hwaes_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::random_internal_randen_hwaes APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_hwaes_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::random_internal_randen_hwaes APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_hwaes_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::random_internal_randen_hwaes APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_hwaes_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::random_internal_mock_helpers #############

        set(abseil_absl_random_internal_mock_helpers_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_random_internal_mock_helpers_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_random_internal_mock_helpers_FRAMEWORKS_RELEASE}" "${abseil_absl_random_internal_mock_helpers_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_random_internal_mock_helpers_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_random_internal_mock_helpers_DEPS_TARGET)
            add_library(abseil_absl_random_internal_mock_helpers_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_random_internal_mock_helpers_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_mock_helpers_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_mock_helpers_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_mock_helpers_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_random_internal_mock_helpers_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_random_internal_mock_helpers_LIBS_RELEASE}"
                              "${abseil_absl_random_internal_mock_helpers_LIB_DIRS_RELEASE}"
                              "${abseil_absl_random_internal_mock_helpers_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_random_internal_mock_helpers_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_random_internal_mock_helpers_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_random_internal_mock_helpers_DEPS_TARGET
                              abseil_absl_random_internal_mock_helpers_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_random_internal_mock_helpers"
                              "${abseil_absl_random_internal_mock_helpers_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::random_internal_mock_helpers
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_mock_helpers_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_mock_helpers_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_random_internal_mock_helpers_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::random_internal_mock_helpers
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_random_internal_mock_helpers_DEPS_TARGET)
        endif()

        set_property(TARGET absl::random_internal_mock_helpers APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_mock_helpers_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::random_internal_mock_helpers APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_mock_helpers_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::random_internal_mock_helpers APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_mock_helpers_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::random_internal_mock_helpers APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_mock_helpers_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::random_internal_mock_helpers APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_mock_helpers_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::random_bit_gen_ref #############

        set(abseil_absl_random_bit_gen_ref_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_random_bit_gen_ref_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_random_bit_gen_ref_FRAMEWORKS_RELEASE}" "${abseil_absl_random_bit_gen_ref_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_random_bit_gen_ref_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_random_bit_gen_ref_DEPS_TARGET)
            add_library(abseil_absl_random_bit_gen_ref_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_random_bit_gen_ref_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_bit_gen_ref_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_bit_gen_ref_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_bit_gen_ref_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_random_bit_gen_ref_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_random_bit_gen_ref_LIBS_RELEASE}"
                              "${abseil_absl_random_bit_gen_ref_LIB_DIRS_RELEASE}"
                              "${abseil_absl_random_bit_gen_ref_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_random_bit_gen_ref_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_random_bit_gen_ref_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_random_bit_gen_ref_DEPS_TARGET
                              abseil_absl_random_bit_gen_ref_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_random_bit_gen_ref"
                              "${abseil_absl_random_bit_gen_ref_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::random_bit_gen_ref
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_bit_gen_ref_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_bit_gen_ref_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_random_bit_gen_ref_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::random_bit_gen_ref
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_random_bit_gen_ref_DEPS_TARGET)
        endif()

        set_property(TARGET absl::random_bit_gen_ref APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_bit_gen_ref_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::random_bit_gen_ref APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_bit_gen_ref_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::random_bit_gen_ref APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_bit_gen_ref_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::random_bit_gen_ref APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_bit_gen_ref_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::random_bit_gen_ref APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_bit_gen_ref_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::bind_front #############

        set(abseil_absl_bind_front_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_bind_front_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_bind_front_FRAMEWORKS_RELEASE}" "${abseil_absl_bind_front_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_bind_front_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_bind_front_DEPS_TARGET)
            add_library(abseil_absl_bind_front_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_bind_front_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_bind_front_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_bind_front_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_bind_front_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_bind_front_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_bind_front_LIBS_RELEASE}"
                              "${abseil_absl_bind_front_LIB_DIRS_RELEASE}"
                              "${abseil_absl_bind_front_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_bind_front_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_bind_front_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_bind_front_DEPS_TARGET
                              abseil_absl_bind_front_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_bind_front"
                              "${abseil_absl_bind_front_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::bind_front
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_bind_front_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_bind_front_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_bind_front_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::bind_front
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_bind_front_DEPS_TARGET)
        endif()

        set_property(TARGET absl::bind_front APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_bind_front_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::bind_front APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_bind_front_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::bind_front APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_bind_front_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::bind_front APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_bind_front_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::bind_front APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_bind_front_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::fixed_array #############

        set(abseil_absl_fixed_array_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_fixed_array_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_fixed_array_FRAMEWORKS_RELEASE}" "${abseil_absl_fixed_array_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_fixed_array_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_fixed_array_DEPS_TARGET)
            add_library(abseil_absl_fixed_array_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_fixed_array_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_fixed_array_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_fixed_array_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_fixed_array_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_fixed_array_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_fixed_array_LIBS_RELEASE}"
                              "${abseil_absl_fixed_array_LIB_DIRS_RELEASE}"
                              "${abseil_absl_fixed_array_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_fixed_array_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_fixed_array_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_fixed_array_DEPS_TARGET
                              abseil_absl_fixed_array_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_fixed_array"
                              "${abseil_absl_fixed_array_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::fixed_array
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_fixed_array_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_fixed_array_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_fixed_array_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::fixed_array
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_fixed_array_DEPS_TARGET)
        endif()

        set_property(TARGET absl::fixed_array APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_fixed_array_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::fixed_array APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_fixed_array_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::fixed_array APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_fixed_array_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::fixed_array APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_fixed_array_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::fixed_array APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_fixed_array_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::cleanup #############

        set(abseil_absl_cleanup_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_cleanup_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_cleanup_FRAMEWORKS_RELEASE}" "${abseil_absl_cleanup_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_cleanup_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_cleanup_DEPS_TARGET)
            add_library(abseil_absl_cleanup_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_cleanup_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_cleanup_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_cleanup_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_cleanup_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_cleanup_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_cleanup_LIBS_RELEASE}"
                              "${abseil_absl_cleanup_LIB_DIRS_RELEASE}"
                              "${abseil_absl_cleanup_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_cleanup_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_cleanup_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_cleanup_DEPS_TARGET
                              abseil_absl_cleanup_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_cleanup"
                              "${abseil_absl_cleanup_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::cleanup
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_cleanup_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_cleanup_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_cleanup_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::cleanup
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_cleanup_DEPS_TARGET)
        endif()

        set_property(TARGET absl::cleanup APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_cleanup_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::cleanup APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_cleanup_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::cleanup APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_cleanup_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::cleanup APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_cleanup_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::cleanup APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_cleanup_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::base #############

        set(abseil_absl_base_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_base_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_base_FRAMEWORKS_RELEASE}" "${abseil_absl_base_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_base_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_base_DEPS_TARGET)
            add_library(abseil_absl_base_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_base_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_base_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_base_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_base_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_base_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_base_LIBS_RELEASE}"
                              "${abseil_absl_base_LIB_DIRS_RELEASE}"
                              "${abseil_absl_base_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_base_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_base_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_base_DEPS_TARGET
                              abseil_absl_base_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_base"
                              "${abseil_absl_base_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::base
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_base_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_base_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_base_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::base
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_base_DEPS_TARGET)
        endif()

        set_property(TARGET absl::base APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_base_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::base APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_base_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::base APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_base_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::base APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_base_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::base APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_base_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::variant #############

        set(abseil_absl_variant_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_variant_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_variant_FRAMEWORKS_RELEASE}" "${abseil_absl_variant_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_variant_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_variant_DEPS_TARGET)
            add_library(abseil_absl_variant_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_variant_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_variant_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_variant_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_variant_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_variant_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_variant_LIBS_RELEASE}"
                              "${abseil_absl_variant_LIB_DIRS_RELEASE}"
                              "${abseil_absl_variant_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_variant_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_variant_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_variant_DEPS_TARGET
                              abseil_absl_variant_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_variant"
                              "${abseil_absl_variant_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::variant
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_variant_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_variant_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_variant_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::variant
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_variant_DEPS_TARGET)
        endif()

        set_property(TARGET absl::variant APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_variant_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::variant APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_variant_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::variant APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_variant_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::variant APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_variant_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::variant APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_variant_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::optional #############

        set(abseil_absl_optional_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_optional_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_optional_FRAMEWORKS_RELEASE}" "${abseil_absl_optional_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_optional_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_optional_DEPS_TARGET)
            add_library(abseil_absl_optional_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_optional_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_optional_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_optional_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_optional_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_optional_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_optional_LIBS_RELEASE}"
                              "${abseil_absl_optional_LIB_DIRS_RELEASE}"
                              "${abseil_absl_optional_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_optional_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_optional_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_optional_DEPS_TARGET
                              abseil_absl_optional_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_optional"
                              "${abseil_absl_optional_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::optional
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_optional_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_optional_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_optional_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::optional
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_optional_DEPS_TARGET)
        endif()

        set_property(TARGET absl::optional APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_optional_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::optional APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_optional_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::optional APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_optional_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::optional APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_optional_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::optional APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_optional_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::any #############

        set(abseil_absl_any_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_any_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_any_FRAMEWORKS_RELEASE}" "${abseil_absl_any_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_any_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_any_DEPS_TARGET)
            add_library(abseil_absl_any_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_any_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_any_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_any_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_any_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_any_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_any_LIBS_RELEASE}"
                              "${abseil_absl_any_LIB_DIRS_RELEASE}"
                              "${abseil_absl_any_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_any_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_any_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_any_DEPS_TARGET
                              abseil_absl_any_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_any"
                              "${abseil_absl_any_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::any
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_any_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_any_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_any_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::any
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_any_DEPS_TARGET)
        endif()

        set_property(TARGET absl::any APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_any_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::any APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_any_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::any APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_any_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::any APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_any_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::any APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_any_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::cordz_functions #############

        set(abseil_absl_cordz_functions_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_cordz_functions_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_cordz_functions_FRAMEWORKS_RELEASE}" "${abseil_absl_cordz_functions_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_cordz_functions_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_cordz_functions_DEPS_TARGET)
            add_library(abseil_absl_cordz_functions_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_cordz_functions_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_cordz_functions_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_cordz_functions_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_cordz_functions_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_cordz_functions_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_cordz_functions_LIBS_RELEASE}"
                              "${abseil_absl_cordz_functions_LIB_DIRS_RELEASE}"
                              "${abseil_absl_cordz_functions_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_cordz_functions_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_cordz_functions_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_cordz_functions_DEPS_TARGET
                              abseil_absl_cordz_functions_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_cordz_functions"
                              "${abseil_absl_cordz_functions_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::cordz_functions
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_cordz_functions_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_cordz_functions_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_cordz_functions_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::cordz_functions
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_cordz_functions_DEPS_TARGET)
        endif()

        set_property(TARGET absl::cordz_functions APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_cordz_functions_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::cordz_functions APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_cordz_functions_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::cordz_functions APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_cordz_functions_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::cordz_functions APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_cordz_functions_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::cordz_functions APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_cordz_functions_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::random_internal_distribution_caller #############

        set(abseil_absl_random_internal_distribution_caller_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_random_internal_distribution_caller_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_random_internal_distribution_caller_FRAMEWORKS_RELEASE}" "${abseil_absl_random_internal_distribution_caller_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_random_internal_distribution_caller_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_random_internal_distribution_caller_DEPS_TARGET)
            add_library(abseil_absl_random_internal_distribution_caller_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_random_internal_distribution_caller_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_distribution_caller_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_distribution_caller_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_distribution_caller_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_random_internal_distribution_caller_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_random_internal_distribution_caller_LIBS_RELEASE}"
                              "${abseil_absl_random_internal_distribution_caller_LIB_DIRS_RELEASE}"
                              "${abseil_absl_random_internal_distribution_caller_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_random_internal_distribution_caller_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_random_internal_distribution_caller_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_random_internal_distribution_caller_DEPS_TARGET
                              abseil_absl_random_internal_distribution_caller_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_random_internal_distribution_caller"
                              "${abseil_absl_random_internal_distribution_caller_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::random_internal_distribution_caller
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_distribution_caller_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_distribution_caller_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_random_internal_distribution_caller_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::random_internal_distribution_caller
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_random_internal_distribution_caller_DEPS_TARGET)
        endif()

        set_property(TARGET absl::random_internal_distribution_caller APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_distribution_caller_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::random_internal_distribution_caller APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_distribution_caller_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::random_internal_distribution_caller APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_distribution_caller_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::random_internal_distribution_caller APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_distribution_caller_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::random_internal_distribution_caller APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_distribution_caller_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::random_seed_gen_exception #############

        set(abseil_absl_random_seed_gen_exception_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_random_seed_gen_exception_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_random_seed_gen_exception_FRAMEWORKS_RELEASE}" "${abseil_absl_random_seed_gen_exception_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_random_seed_gen_exception_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_random_seed_gen_exception_DEPS_TARGET)
            add_library(abseil_absl_random_seed_gen_exception_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_random_seed_gen_exception_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_seed_gen_exception_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_seed_gen_exception_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_seed_gen_exception_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_random_seed_gen_exception_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_random_seed_gen_exception_LIBS_RELEASE}"
                              "${abseil_absl_random_seed_gen_exception_LIB_DIRS_RELEASE}"
                              "${abseil_absl_random_seed_gen_exception_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_random_seed_gen_exception_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_random_seed_gen_exception_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_random_seed_gen_exception_DEPS_TARGET
                              abseil_absl_random_seed_gen_exception_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_random_seed_gen_exception"
                              "${abseil_absl_random_seed_gen_exception_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::random_seed_gen_exception
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_seed_gen_exception_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_seed_gen_exception_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_random_seed_gen_exception_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::random_seed_gen_exception
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_random_seed_gen_exception_DEPS_TARGET)
        endif()

        set_property(TARGET absl::random_seed_gen_exception APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_seed_gen_exception_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::random_seed_gen_exception APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_seed_gen_exception_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::random_seed_gen_exception APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_seed_gen_exception_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::random_seed_gen_exception APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_seed_gen_exception_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::random_seed_gen_exception APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_seed_gen_exception_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::memory #############

        set(abseil_absl_memory_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_memory_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_memory_FRAMEWORKS_RELEASE}" "${abseil_absl_memory_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_memory_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_memory_DEPS_TARGET)
            add_library(abseil_absl_memory_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_memory_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_memory_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_memory_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_memory_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_memory_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_memory_LIBS_RELEASE}"
                              "${abseil_absl_memory_LIB_DIRS_RELEASE}"
                              "${abseil_absl_memory_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_memory_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_memory_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_memory_DEPS_TARGET
                              abseil_absl_memory_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_memory"
                              "${abseil_absl_memory_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::memory
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_memory_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_memory_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_memory_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::memory
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_memory_DEPS_TARGET)
        endif()

        set_property(TARGET absl::memory APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_memory_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::memory APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_memory_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::memory APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_memory_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::memory APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_memory_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::memory APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_memory_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::overload #############

        set(abseil_absl_overload_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_overload_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_overload_FRAMEWORKS_RELEASE}" "${abseil_absl_overload_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_overload_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_overload_DEPS_TARGET)
            add_library(abseil_absl_overload_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_overload_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_overload_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_overload_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_overload_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_overload_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_overload_LIBS_RELEASE}"
                              "${abseil_absl_overload_LIB_DIRS_RELEASE}"
                              "${abseil_absl_overload_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_overload_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_overload_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_overload_DEPS_TARGET
                              abseil_absl_overload_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_overload"
                              "${abseil_absl_overload_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::overload
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_overload_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_overload_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_overload_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::overload
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_overload_DEPS_TARGET)
        endif()

        set_property(TARGET absl::overload APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_overload_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::overload APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_overload_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::overload APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_overload_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::overload APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_overload_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::overload APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_overload_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::debugging_internal #############

        set(abseil_absl_debugging_internal_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_debugging_internal_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_debugging_internal_FRAMEWORKS_RELEASE}" "${abseil_absl_debugging_internal_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_debugging_internal_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_debugging_internal_DEPS_TARGET)
            add_library(abseil_absl_debugging_internal_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_debugging_internal_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_debugging_internal_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_debugging_internal_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_debugging_internal_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_debugging_internal_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_debugging_internal_LIBS_RELEASE}"
                              "${abseil_absl_debugging_internal_LIB_DIRS_RELEASE}"
                              "${abseil_absl_debugging_internal_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_debugging_internal_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_debugging_internal_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_debugging_internal_DEPS_TARGET
                              abseil_absl_debugging_internal_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_debugging_internal"
                              "${abseil_absl_debugging_internal_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::debugging_internal
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_debugging_internal_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_debugging_internal_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_debugging_internal_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::debugging_internal
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_debugging_internal_DEPS_TARGET)
        endif()

        set_property(TARGET absl::debugging_internal APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_debugging_internal_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::debugging_internal APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_debugging_internal_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::debugging_internal APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_debugging_internal_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::debugging_internal APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_debugging_internal_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::debugging_internal APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_debugging_internal_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::common_policy_traits #############

        set(abseil_absl_common_policy_traits_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_common_policy_traits_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_common_policy_traits_FRAMEWORKS_RELEASE}" "${abseil_absl_common_policy_traits_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_common_policy_traits_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_common_policy_traits_DEPS_TARGET)
            add_library(abseil_absl_common_policy_traits_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_common_policy_traits_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_common_policy_traits_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_common_policy_traits_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_common_policy_traits_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_common_policy_traits_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_common_policy_traits_LIBS_RELEASE}"
                              "${abseil_absl_common_policy_traits_LIB_DIRS_RELEASE}"
                              "${abseil_absl_common_policy_traits_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_common_policy_traits_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_common_policy_traits_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_common_policy_traits_DEPS_TARGET
                              abseil_absl_common_policy_traits_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_common_policy_traits"
                              "${abseil_absl_common_policy_traits_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::common_policy_traits
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_common_policy_traits_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_common_policy_traits_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_common_policy_traits_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::common_policy_traits
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_common_policy_traits_DEPS_TARGET)
        endif()

        set_property(TARGET absl::common_policy_traits APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_common_policy_traits_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::common_policy_traits APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_common_policy_traits_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::common_policy_traits APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_common_policy_traits_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::common_policy_traits APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_common_policy_traits_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::common_policy_traits APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_common_policy_traits_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::compressed_tuple #############

        set(abseil_absl_compressed_tuple_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_compressed_tuple_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_compressed_tuple_FRAMEWORKS_RELEASE}" "${abseil_absl_compressed_tuple_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_compressed_tuple_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_compressed_tuple_DEPS_TARGET)
            add_library(abseil_absl_compressed_tuple_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_compressed_tuple_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_compressed_tuple_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_compressed_tuple_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_compressed_tuple_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_compressed_tuple_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_compressed_tuple_LIBS_RELEASE}"
                              "${abseil_absl_compressed_tuple_LIB_DIRS_RELEASE}"
                              "${abseil_absl_compressed_tuple_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_compressed_tuple_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_compressed_tuple_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_compressed_tuple_DEPS_TARGET
                              abseil_absl_compressed_tuple_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_compressed_tuple"
                              "${abseil_absl_compressed_tuple_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::compressed_tuple
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_compressed_tuple_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_compressed_tuple_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_compressed_tuple_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::compressed_tuple
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_compressed_tuple_DEPS_TARGET)
        endif()

        set_property(TARGET absl::compressed_tuple APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_compressed_tuple_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::compressed_tuple APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_compressed_tuple_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::compressed_tuple APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_compressed_tuple_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::compressed_tuple APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_compressed_tuple_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::compressed_tuple APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_compressed_tuple_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::cleanup_internal #############

        set(abseil_absl_cleanup_internal_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_cleanup_internal_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_cleanup_internal_FRAMEWORKS_RELEASE}" "${abseil_absl_cleanup_internal_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_cleanup_internal_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_cleanup_internal_DEPS_TARGET)
            add_library(abseil_absl_cleanup_internal_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_cleanup_internal_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_cleanup_internal_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_cleanup_internal_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_cleanup_internal_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_cleanup_internal_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_cleanup_internal_LIBS_RELEASE}"
                              "${abseil_absl_cleanup_internal_LIB_DIRS_RELEASE}"
                              "${abseil_absl_cleanup_internal_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_cleanup_internal_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_cleanup_internal_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_cleanup_internal_DEPS_TARGET
                              abseil_absl_cleanup_internal_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_cleanup_internal"
                              "${abseil_absl_cleanup_internal_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::cleanup_internal
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_cleanup_internal_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_cleanup_internal_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_cleanup_internal_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::cleanup_internal
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_cleanup_internal_DEPS_TARGET)
        endif()

        set_property(TARGET absl::cleanup_internal APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_cleanup_internal_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::cleanup_internal APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_cleanup_internal_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::cleanup_internal APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_cleanup_internal_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::cleanup_internal APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_cleanup_internal_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::cleanup_internal APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_cleanup_internal_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::algorithm_container #############

        set(abseil_absl_algorithm_container_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_algorithm_container_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_algorithm_container_FRAMEWORKS_RELEASE}" "${abseil_absl_algorithm_container_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_algorithm_container_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_algorithm_container_DEPS_TARGET)
            add_library(abseil_absl_algorithm_container_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_algorithm_container_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_algorithm_container_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_algorithm_container_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_algorithm_container_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_algorithm_container_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_algorithm_container_LIBS_RELEASE}"
                              "${abseil_absl_algorithm_container_LIB_DIRS_RELEASE}"
                              "${abseil_absl_algorithm_container_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_algorithm_container_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_algorithm_container_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_algorithm_container_DEPS_TARGET
                              abseil_absl_algorithm_container_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_algorithm_container"
                              "${abseil_absl_algorithm_container_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::algorithm_container
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_algorithm_container_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_algorithm_container_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_algorithm_container_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::algorithm_container
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_algorithm_container_DEPS_TARGET)
        endif()

        set_property(TARGET absl::algorithm_container APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_algorithm_container_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::algorithm_container APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_algorithm_container_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::algorithm_container APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_algorithm_container_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::algorithm_container APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_algorithm_container_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::algorithm_container APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_algorithm_container_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::scoped_set_env #############

        set(abseil_absl_scoped_set_env_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_scoped_set_env_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_scoped_set_env_FRAMEWORKS_RELEASE}" "${abseil_absl_scoped_set_env_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_scoped_set_env_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_scoped_set_env_DEPS_TARGET)
            add_library(abseil_absl_scoped_set_env_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_scoped_set_env_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_scoped_set_env_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_scoped_set_env_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_scoped_set_env_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_scoped_set_env_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_scoped_set_env_LIBS_RELEASE}"
                              "${abseil_absl_scoped_set_env_LIB_DIRS_RELEASE}"
                              "${abseil_absl_scoped_set_env_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_scoped_set_env_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_scoped_set_env_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_scoped_set_env_DEPS_TARGET
                              abseil_absl_scoped_set_env_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_scoped_set_env"
                              "${abseil_absl_scoped_set_env_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::scoped_set_env
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_scoped_set_env_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_scoped_set_env_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_scoped_set_env_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::scoped_set_env
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_scoped_set_env_DEPS_TARGET)
        endif()

        set_property(TARGET absl::scoped_set_env APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_scoped_set_env_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::scoped_set_env APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_scoped_set_env_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::scoped_set_env APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_scoped_set_env_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::scoped_set_env APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_scoped_set_env_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::scoped_set_env APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_scoped_set_env_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::throw_delegate #############

        set(abseil_absl_throw_delegate_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_throw_delegate_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_throw_delegate_FRAMEWORKS_RELEASE}" "${abseil_absl_throw_delegate_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_throw_delegate_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_throw_delegate_DEPS_TARGET)
            add_library(abseil_absl_throw_delegate_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_throw_delegate_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_throw_delegate_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_throw_delegate_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_throw_delegate_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_throw_delegate_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_throw_delegate_LIBS_RELEASE}"
                              "${abseil_absl_throw_delegate_LIB_DIRS_RELEASE}"
                              "${abseil_absl_throw_delegate_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_throw_delegate_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_throw_delegate_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_throw_delegate_DEPS_TARGET
                              abseil_absl_throw_delegate_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_throw_delegate"
                              "${abseil_absl_throw_delegate_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::throw_delegate
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_throw_delegate_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_throw_delegate_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_throw_delegate_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::throw_delegate
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_throw_delegate_DEPS_TARGET)
        endif()

        set_property(TARGET absl::throw_delegate APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_throw_delegate_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::throw_delegate APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_throw_delegate_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::throw_delegate APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_throw_delegate_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::throw_delegate APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_throw_delegate_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::throw_delegate APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_throw_delegate_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::spinlock_wait #############

        set(abseil_absl_spinlock_wait_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_spinlock_wait_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_spinlock_wait_FRAMEWORKS_RELEASE}" "${abseil_absl_spinlock_wait_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_spinlock_wait_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_spinlock_wait_DEPS_TARGET)
            add_library(abseil_absl_spinlock_wait_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_spinlock_wait_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_spinlock_wait_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_spinlock_wait_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_spinlock_wait_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_spinlock_wait_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_spinlock_wait_LIBS_RELEASE}"
                              "${abseil_absl_spinlock_wait_LIB_DIRS_RELEASE}"
                              "${abseil_absl_spinlock_wait_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_spinlock_wait_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_spinlock_wait_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_spinlock_wait_DEPS_TARGET
                              abseil_absl_spinlock_wait_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_spinlock_wait"
                              "${abseil_absl_spinlock_wait_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::spinlock_wait
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_spinlock_wait_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_spinlock_wait_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_spinlock_wait_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::spinlock_wait
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_spinlock_wait_DEPS_TARGET)
        endif()

        set_property(TARGET absl::spinlock_wait APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_spinlock_wait_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::spinlock_wait APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_spinlock_wait_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::spinlock_wait APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_spinlock_wait_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::spinlock_wait APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_spinlock_wait_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::spinlock_wait APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_spinlock_wait_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::utility #############

        set(abseil_absl_utility_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_utility_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_utility_FRAMEWORKS_RELEASE}" "${abseil_absl_utility_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_utility_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_utility_DEPS_TARGET)
            add_library(abseil_absl_utility_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_utility_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_utility_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_utility_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_utility_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_utility_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_utility_LIBS_RELEASE}"
                              "${abseil_absl_utility_LIB_DIRS_RELEASE}"
                              "${abseil_absl_utility_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_utility_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_utility_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_utility_DEPS_TARGET
                              abseil_absl_utility_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_utility"
                              "${abseil_absl_utility_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::utility
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_utility_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_utility_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_utility_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::utility
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_utility_DEPS_TARGET)
        endif()

        set_property(TARGET absl::utility APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_utility_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::utility APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_utility_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::utility APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_utility_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::utility APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_utility_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::utility APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_utility_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::compare #############

        set(abseil_absl_compare_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_compare_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_compare_FRAMEWORKS_RELEASE}" "${abseil_absl_compare_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_compare_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_compare_DEPS_TARGET)
            add_library(abseil_absl_compare_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_compare_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_compare_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_compare_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_compare_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_compare_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_compare_LIBS_RELEASE}"
                              "${abseil_absl_compare_LIB_DIRS_RELEASE}"
                              "${abseil_absl_compare_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_compare_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_compare_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_compare_DEPS_TARGET
                              abseil_absl_compare_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_compare"
                              "${abseil_absl_compare_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::compare
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_compare_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_compare_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_compare_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::compare
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_compare_DEPS_TARGET)
        endif()

        set_property(TARGET absl::compare APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_compare_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::compare APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_compare_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::compare APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_compare_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::compare APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_compare_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::compare APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_compare_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::charset #############

        set(abseil_absl_charset_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_charset_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_charset_FRAMEWORKS_RELEASE}" "${abseil_absl_charset_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_charset_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_charset_DEPS_TARGET)
            add_library(abseil_absl_charset_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_charset_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_charset_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_charset_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_charset_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_charset_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_charset_LIBS_RELEASE}"
                              "${abseil_absl_charset_LIB_DIRS_RELEASE}"
                              "${abseil_absl_charset_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_charset_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_charset_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_charset_DEPS_TARGET
                              abseil_absl_charset_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_charset"
                              "${abseil_absl_charset_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::charset
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_charset_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_charset_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_charset_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::charset
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_charset_DEPS_TARGET)
        endif()

        set_property(TARGET absl::charset APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_charset_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::charset APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_charset_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::charset APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_charset_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::charset APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_charset_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::charset APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_charset_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::random_internal_uniform_helper #############

        set(abseil_absl_random_internal_uniform_helper_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_random_internal_uniform_helper_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_random_internal_uniform_helper_FRAMEWORKS_RELEASE}" "${abseil_absl_random_internal_uniform_helper_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_random_internal_uniform_helper_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_random_internal_uniform_helper_DEPS_TARGET)
            add_library(abseil_absl_random_internal_uniform_helper_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_random_internal_uniform_helper_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_uniform_helper_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_uniform_helper_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_uniform_helper_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_random_internal_uniform_helper_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_random_internal_uniform_helper_LIBS_RELEASE}"
                              "${abseil_absl_random_internal_uniform_helper_LIB_DIRS_RELEASE}"
                              "${abseil_absl_random_internal_uniform_helper_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_random_internal_uniform_helper_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_random_internal_uniform_helper_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_random_internal_uniform_helper_DEPS_TARGET
                              abseil_absl_random_internal_uniform_helper_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_random_internal_uniform_helper"
                              "${abseil_absl_random_internal_uniform_helper_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::random_internal_uniform_helper
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_uniform_helper_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_uniform_helper_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_random_internal_uniform_helper_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::random_internal_uniform_helper
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_random_internal_uniform_helper_DEPS_TARGET)
        endif()

        set_property(TARGET absl::random_internal_uniform_helper APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_uniform_helper_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::random_internal_uniform_helper APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_uniform_helper_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::random_internal_uniform_helper APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_uniform_helper_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::random_internal_uniform_helper APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_uniform_helper_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::random_internal_uniform_helper APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_uniform_helper_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::periodic_sampler #############

        set(abseil_absl_periodic_sampler_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_periodic_sampler_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_periodic_sampler_FRAMEWORKS_RELEASE}" "${abseil_absl_periodic_sampler_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_periodic_sampler_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_periodic_sampler_DEPS_TARGET)
            add_library(abseil_absl_periodic_sampler_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_periodic_sampler_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_periodic_sampler_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_periodic_sampler_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_periodic_sampler_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_periodic_sampler_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_periodic_sampler_LIBS_RELEASE}"
                              "${abseil_absl_periodic_sampler_LIB_DIRS_RELEASE}"
                              "${abseil_absl_periodic_sampler_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_periodic_sampler_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_periodic_sampler_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_periodic_sampler_DEPS_TARGET
                              abseil_absl_periodic_sampler_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_periodic_sampler"
                              "${abseil_absl_periodic_sampler_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::periodic_sampler
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_periodic_sampler_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_periodic_sampler_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_periodic_sampler_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::periodic_sampler
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_periodic_sampler_DEPS_TARGET)
        endif()

        set_property(TARGET absl::periodic_sampler APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_periodic_sampler_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::periodic_sampler APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_periodic_sampler_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::periodic_sampler APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_periodic_sampler_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::periodic_sampler APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_periodic_sampler_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::periodic_sampler APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_periodic_sampler_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::meta #############

        set(abseil_absl_meta_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_meta_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_meta_FRAMEWORKS_RELEASE}" "${abseil_absl_meta_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_meta_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_meta_DEPS_TARGET)
            add_library(abseil_absl_meta_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_meta_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_meta_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_meta_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_meta_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_meta_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_meta_LIBS_RELEASE}"
                              "${abseil_absl_meta_LIB_DIRS_RELEASE}"
                              "${abseil_absl_meta_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_meta_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_meta_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_meta_DEPS_TARGET
                              abseil_absl_meta_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_meta"
                              "${abseil_absl_meta_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::meta
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_meta_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_meta_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_meta_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::meta
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_meta_DEPS_TARGET)
        endif()

        set_property(TARGET absl::meta APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_meta_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::meta APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_meta_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::meta APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_meta_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::meta APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_meta_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::meta APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_meta_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::container_common #############

        set(abseil_absl_container_common_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_container_common_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_container_common_FRAMEWORKS_RELEASE}" "${abseil_absl_container_common_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_container_common_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_container_common_DEPS_TARGET)
            add_library(abseil_absl_container_common_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_container_common_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_container_common_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_container_common_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_container_common_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_container_common_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_container_common_LIBS_RELEASE}"
                              "${abseil_absl_container_common_LIB_DIRS_RELEASE}"
                              "${abseil_absl_container_common_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_container_common_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_container_common_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_container_common_DEPS_TARGET
                              abseil_absl_container_common_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_container_common"
                              "${abseil_absl_container_common_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::container_common
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_container_common_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_container_common_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_container_common_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::container_common
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_container_common_DEPS_TARGET)
        endif()

        set_property(TARGET absl::container_common APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_container_common_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::container_common APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_container_common_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::container_common APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_container_common_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::container_common APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_container_common_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::container_common APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_container_common_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::iterator_traits_internal #############

        set(abseil_absl_iterator_traits_internal_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_iterator_traits_internal_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_iterator_traits_internal_FRAMEWORKS_RELEASE}" "${abseil_absl_iterator_traits_internal_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_iterator_traits_internal_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_iterator_traits_internal_DEPS_TARGET)
            add_library(abseil_absl_iterator_traits_internal_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_iterator_traits_internal_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_iterator_traits_internal_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_iterator_traits_internal_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_iterator_traits_internal_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_iterator_traits_internal_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_iterator_traits_internal_LIBS_RELEASE}"
                              "${abseil_absl_iterator_traits_internal_LIB_DIRS_RELEASE}"
                              "${abseil_absl_iterator_traits_internal_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_iterator_traits_internal_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_iterator_traits_internal_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_iterator_traits_internal_DEPS_TARGET
                              abseil_absl_iterator_traits_internal_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_iterator_traits_internal"
                              "${abseil_absl_iterator_traits_internal_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::iterator_traits_internal
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_iterator_traits_internal_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_iterator_traits_internal_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_iterator_traits_internal_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::iterator_traits_internal
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_iterator_traits_internal_DEPS_TARGET)
        endif()

        set_property(TARGET absl::iterator_traits_internal APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_iterator_traits_internal_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::iterator_traits_internal APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_iterator_traits_internal_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::iterator_traits_internal APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_iterator_traits_internal_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::iterator_traits_internal APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_iterator_traits_internal_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::iterator_traits_internal APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_iterator_traits_internal_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::base_internal #############

        set(abseil_absl_base_internal_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_base_internal_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_base_internal_FRAMEWORKS_RELEASE}" "${abseil_absl_base_internal_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_base_internal_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_base_internal_DEPS_TARGET)
            add_library(abseil_absl_base_internal_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_base_internal_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_base_internal_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_base_internal_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_base_internal_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_base_internal_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_base_internal_LIBS_RELEASE}"
                              "${abseil_absl_base_internal_LIB_DIRS_RELEASE}"
                              "${abseil_absl_base_internal_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_base_internal_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_base_internal_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_base_internal_DEPS_TARGET
                              abseil_absl_base_internal_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_base_internal"
                              "${abseil_absl_base_internal_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::base_internal
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_base_internal_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_base_internal_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_base_internal_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::base_internal
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_base_internal_DEPS_TARGET)
        endif()

        set_property(TARGET absl::base_internal APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_base_internal_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::base_internal APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_base_internal_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::base_internal APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_base_internal_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::base_internal APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_base_internal_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::base_internal APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_base_internal_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::raw_logging_internal #############

        set(abseil_absl_raw_logging_internal_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_raw_logging_internal_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_raw_logging_internal_FRAMEWORKS_RELEASE}" "${abseil_absl_raw_logging_internal_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_raw_logging_internal_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_raw_logging_internal_DEPS_TARGET)
            add_library(abseil_absl_raw_logging_internal_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_raw_logging_internal_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_raw_logging_internal_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_raw_logging_internal_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_raw_logging_internal_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_raw_logging_internal_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_raw_logging_internal_LIBS_RELEASE}"
                              "${abseil_absl_raw_logging_internal_LIB_DIRS_RELEASE}"
                              "${abseil_absl_raw_logging_internal_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_raw_logging_internal_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_raw_logging_internal_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_raw_logging_internal_DEPS_TARGET
                              abseil_absl_raw_logging_internal_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_raw_logging_internal"
                              "${abseil_absl_raw_logging_internal_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::raw_logging_internal
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_raw_logging_internal_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_raw_logging_internal_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_raw_logging_internal_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::raw_logging_internal
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_raw_logging_internal_DEPS_TARGET)
        endif()

        set_property(TARGET absl::raw_logging_internal APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_raw_logging_internal_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::raw_logging_internal APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_raw_logging_internal_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::raw_logging_internal APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_raw_logging_internal_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::raw_logging_internal APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_raw_logging_internal_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::raw_logging_internal APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_raw_logging_internal_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::string_view #############

        set(abseil_absl_string_view_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_string_view_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_string_view_FRAMEWORKS_RELEASE}" "${abseil_absl_string_view_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_string_view_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_string_view_DEPS_TARGET)
            add_library(abseil_absl_string_view_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_string_view_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_string_view_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_string_view_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_string_view_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_string_view_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_string_view_LIBS_RELEASE}"
                              "${abseil_absl_string_view_LIB_DIRS_RELEASE}"
                              "${abseil_absl_string_view_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_string_view_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_string_view_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_string_view_DEPS_TARGET
                              abseil_absl_string_view_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_string_view"
                              "${abseil_absl_string_view_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::string_view
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_string_view_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_string_view_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_string_view_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::string_view
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_string_view_DEPS_TARGET)
        endif()

        set_property(TARGET absl::string_view APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_string_view_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::string_view APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_string_view_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::string_view APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_string_view_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::string_view APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_string_view_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::string_view APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_string_view_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::random_internal_randen_hwaes_impl #############

        set(abseil_absl_random_internal_randen_hwaes_impl_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_random_internal_randen_hwaes_impl_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_random_internal_randen_hwaes_impl_FRAMEWORKS_RELEASE}" "${abseil_absl_random_internal_randen_hwaes_impl_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_random_internal_randen_hwaes_impl_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_random_internal_randen_hwaes_impl_DEPS_TARGET)
            add_library(abseil_absl_random_internal_randen_hwaes_impl_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_random_internal_randen_hwaes_impl_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_hwaes_impl_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_hwaes_impl_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_hwaes_impl_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_random_internal_randen_hwaes_impl_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_random_internal_randen_hwaes_impl_LIBS_RELEASE}"
                              "${abseil_absl_random_internal_randen_hwaes_impl_LIB_DIRS_RELEASE}"
                              "${abseil_absl_random_internal_randen_hwaes_impl_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_random_internal_randen_hwaes_impl_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_random_internal_randen_hwaes_impl_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_random_internal_randen_hwaes_impl_DEPS_TARGET
                              abseil_absl_random_internal_randen_hwaes_impl_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_random_internal_randen_hwaes_impl"
                              "${abseil_absl_random_internal_randen_hwaes_impl_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::random_internal_randen_hwaes_impl
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_hwaes_impl_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_hwaes_impl_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_random_internal_randen_hwaes_impl_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::random_internal_randen_hwaes_impl
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_random_internal_randen_hwaes_impl_DEPS_TARGET)
        endif()

        set_property(TARGET absl::random_internal_randen_hwaes_impl APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_hwaes_impl_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::random_internal_randen_hwaes_impl APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_hwaes_impl_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::random_internal_randen_hwaes_impl APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_hwaes_impl_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::random_internal_randen_hwaes_impl APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_hwaes_impl_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::random_internal_randen_hwaes_impl APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_hwaes_impl_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::random_internal_randen_slow #############

        set(abseil_absl_random_internal_randen_slow_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_random_internal_randen_slow_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_random_internal_randen_slow_FRAMEWORKS_RELEASE}" "${abseil_absl_random_internal_randen_slow_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_random_internal_randen_slow_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_random_internal_randen_slow_DEPS_TARGET)
            add_library(abseil_absl_random_internal_randen_slow_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_random_internal_randen_slow_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_slow_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_slow_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_slow_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_random_internal_randen_slow_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_random_internal_randen_slow_LIBS_RELEASE}"
                              "${abseil_absl_random_internal_randen_slow_LIB_DIRS_RELEASE}"
                              "${abseil_absl_random_internal_randen_slow_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_random_internal_randen_slow_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_random_internal_randen_slow_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_random_internal_randen_slow_DEPS_TARGET
                              abseil_absl_random_internal_randen_slow_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_random_internal_randen_slow"
                              "${abseil_absl_random_internal_randen_slow_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::random_internal_randen_slow
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_slow_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_slow_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_random_internal_randen_slow_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::random_internal_randen_slow
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_random_internal_randen_slow_DEPS_TARGET)
        endif()

        set_property(TARGET absl::random_internal_randen_slow APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_slow_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::random_internal_randen_slow APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_slow_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::random_internal_randen_slow APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_slow_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::random_internal_randen_slow APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_slow_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::random_internal_randen_slow APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_randen_slow_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::exponential_biased #############

        set(abseil_absl_exponential_biased_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_exponential_biased_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_exponential_biased_FRAMEWORKS_RELEASE}" "${abseil_absl_exponential_biased_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_exponential_biased_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_exponential_biased_DEPS_TARGET)
            add_library(abseil_absl_exponential_biased_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_exponential_biased_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_exponential_biased_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_exponential_biased_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_exponential_biased_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_exponential_biased_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_exponential_biased_LIBS_RELEASE}"
                              "${abseil_absl_exponential_biased_LIB_DIRS_RELEASE}"
                              "${abseil_absl_exponential_biased_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_exponential_biased_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_exponential_biased_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_exponential_biased_DEPS_TARGET
                              abseil_absl_exponential_biased_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_exponential_biased"
                              "${abseil_absl_exponential_biased_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::exponential_biased
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_exponential_biased_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_exponential_biased_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_exponential_biased_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::exponential_biased
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_exponential_biased_DEPS_TARGET)
        endif()

        set_property(TARGET absl::exponential_biased APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_exponential_biased_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::exponential_biased APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_exponential_biased_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::exponential_biased APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_exponential_biased_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::exponential_biased APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_exponential_biased_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::exponential_biased APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_exponential_biased_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::type_traits #############

        set(abseil_absl_type_traits_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_type_traits_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_type_traits_FRAMEWORKS_RELEASE}" "${abseil_absl_type_traits_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_type_traits_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_type_traits_DEPS_TARGET)
            add_library(abseil_absl_type_traits_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_type_traits_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_type_traits_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_type_traits_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_type_traits_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_type_traits_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_type_traits_LIBS_RELEASE}"
                              "${abseil_absl_type_traits_LIB_DIRS_RELEASE}"
                              "${abseil_absl_type_traits_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_type_traits_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_type_traits_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_type_traits_DEPS_TARGET
                              abseil_absl_type_traits_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_type_traits"
                              "${abseil_absl_type_traits_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::type_traits
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_type_traits_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_type_traits_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_type_traits_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::type_traits
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_type_traits_DEPS_TARGET)
        endif()

        set_property(TARGET absl::type_traits APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_type_traits_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::type_traits APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_type_traits_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::type_traits APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_type_traits_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::type_traits APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_type_traits_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::type_traits APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_type_traits_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::log_internal_voidify #############

        set(abseil_absl_log_internal_voidify_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_log_internal_voidify_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_log_internal_voidify_FRAMEWORKS_RELEASE}" "${abseil_absl_log_internal_voidify_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_log_internal_voidify_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_log_internal_voidify_DEPS_TARGET)
            add_library(abseil_absl_log_internal_voidify_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_log_internal_voidify_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_voidify_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_voidify_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_voidify_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_log_internal_voidify_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_log_internal_voidify_LIBS_RELEASE}"
                              "${abseil_absl_log_internal_voidify_LIB_DIRS_RELEASE}"
                              "${abseil_absl_log_internal_voidify_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_log_internal_voidify_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_log_internal_voidify_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_log_internal_voidify_DEPS_TARGET
                              abseil_absl_log_internal_voidify_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_log_internal_voidify"
                              "${abseil_absl_log_internal_voidify_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::log_internal_voidify
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_voidify_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_voidify_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_log_internal_voidify_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::log_internal_voidify
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_log_internal_voidify_DEPS_TARGET)
        endif()

        set_property(TARGET absl::log_internal_voidify APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_voidify_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::log_internal_voidify APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_voidify_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::log_internal_voidify APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_voidify_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::log_internal_voidify APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_voidify_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::log_internal_voidify APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_voidify_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::log_internal_nullguard #############

        set(abseil_absl_log_internal_nullguard_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_log_internal_nullguard_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_log_internal_nullguard_FRAMEWORKS_RELEASE}" "${abseil_absl_log_internal_nullguard_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_log_internal_nullguard_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_log_internal_nullguard_DEPS_TARGET)
            add_library(abseil_absl_log_internal_nullguard_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_log_internal_nullguard_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_nullguard_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_nullguard_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_nullguard_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_log_internal_nullguard_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_log_internal_nullguard_LIBS_RELEASE}"
                              "${abseil_absl_log_internal_nullguard_LIB_DIRS_RELEASE}"
                              "${abseil_absl_log_internal_nullguard_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_log_internal_nullguard_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_log_internal_nullguard_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_log_internal_nullguard_DEPS_TARGET
                              abseil_absl_log_internal_nullguard_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_log_internal_nullguard"
                              "${abseil_absl_log_internal_nullguard_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::log_internal_nullguard
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_nullguard_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_nullguard_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_log_internal_nullguard_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::log_internal_nullguard
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_log_internal_nullguard_DEPS_TARGET)
        endif()

        set_property(TARGET absl::log_internal_nullguard APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_nullguard_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::log_internal_nullguard APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_nullguard_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::log_internal_nullguard APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_nullguard_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::log_internal_nullguard APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_nullguard_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::log_internal_nullguard APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_nullguard_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::log_internal_config #############

        set(abseil_absl_log_internal_config_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_log_internal_config_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_log_internal_config_FRAMEWORKS_RELEASE}" "${abseil_absl_log_internal_config_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_log_internal_config_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_log_internal_config_DEPS_TARGET)
            add_library(abseil_absl_log_internal_config_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_log_internal_config_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_config_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_config_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_config_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_log_internal_config_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_log_internal_config_LIBS_RELEASE}"
                              "${abseil_absl_log_internal_config_LIB_DIRS_RELEASE}"
                              "${abseil_absl_log_internal_config_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_log_internal_config_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_log_internal_config_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_log_internal_config_DEPS_TARGET
                              abseil_absl_log_internal_config_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_log_internal_config"
                              "${abseil_absl_log_internal_config_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::log_internal_config
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_config_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_config_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_log_internal_config_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::log_internal_config
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_log_internal_config_DEPS_TARGET)
        endif()

        set_property(TARGET absl::log_internal_config APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_config_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::log_internal_config APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_config_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::log_internal_config APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_config_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::log_internal_config APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_config_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::log_internal_config APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_internal_config_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::flags_commandlineflag_internal #############

        set(abseil_absl_flags_commandlineflag_internal_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_flags_commandlineflag_internal_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_flags_commandlineflag_internal_FRAMEWORKS_RELEASE}" "${abseil_absl_flags_commandlineflag_internal_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_flags_commandlineflag_internal_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_flags_commandlineflag_internal_DEPS_TARGET)
            add_library(abseil_absl_flags_commandlineflag_internal_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_flags_commandlineflag_internal_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_commandlineflag_internal_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flags_commandlineflag_internal_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flags_commandlineflag_internal_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_flags_commandlineflag_internal_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_flags_commandlineflag_internal_LIBS_RELEASE}"
                              "${abseil_absl_flags_commandlineflag_internal_LIB_DIRS_RELEASE}"
                              "${abseil_absl_flags_commandlineflag_internal_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_flags_commandlineflag_internal_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_flags_commandlineflag_internal_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_flags_commandlineflag_internal_DEPS_TARGET
                              abseil_absl_flags_commandlineflag_internal_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_flags_commandlineflag_internal"
                              "${abseil_absl_flags_commandlineflag_internal_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::flags_commandlineflag_internal
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_commandlineflag_internal_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_flags_commandlineflag_internal_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_flags_commandlineflag_internal_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::flags_commandlineflag_internal
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_flags_commandlineflag_internal_DEPS_TARGET)
        endif()

        set_property(TARGET absl::flags_commandlineflag_internal APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_flags_commandlineflag_internal_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::flags_commandlineflag_internal APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_commandlineflag_internal_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::flags_commandlineflag_internal APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_flags_commandlineflag_internal_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::flags_commandlineflag_internal APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_flags_commandlineflag_internal_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::flags_commandlineflag_internal APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_flags_commandlineflag_internal_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::leak_check #############

        set(abseil_absl_leak_check_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_leak_check_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_leak_check_FRAMEWORKS_RELEASE}" "${abseil_absl_leak_check_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_leak_check_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_leak_check_DEPS_TARGET)
            add_library(abseil_absl_leak_check_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_leak_check_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_leak_check_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_leak_check_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_leak_check_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_leak_check_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_leak_check_LIBS_RELEASE}"
                              "${abseil_absl_leak_check_LIB_DIRS_RELEASE}"
                              "${abseil_absl_leak_check_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_leak_check_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_leak_check_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_leak_check_DEPS_TARGET
                              abseil_absl_leak_check_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_leak_check"
                              "${abseil_absl_leak_check_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::leak_check
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_leak_check_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_leak_check_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_leak_check_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::leak_check
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_leak_check_DEPS_TARGET)
        endif()

        set_property(TARGET absl::leak_check APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_leak_check_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::leak_check APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_leak_check_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::leak_check APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_leak_check_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::leak_check APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_leak_check_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::leak_check APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_leak_check_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::non_temporal_memcpy #############

        set(abseil_absl_non_temporal_memcpy_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_non_temporal_memcpy_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_non_temporal_memcpy_FRAMEWORKS_RELEASE}" "${abseil_absl_non_temporal_memcpy_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_non_temporal_memcpy_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_non_temporal_memcpy_DEPS_TARGET)
            add_library(abseil_absl_non_temporal_memcpy_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_non_temporal_memcpy_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_non_temporal_memcpy_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_non_temporal_memcpy_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_non_temporal_memcpy_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_non_temporal_memcpy_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_non_temporal_memcpy_LIBS_RELEASE}"
                              "${abseil_absl_non_temporal_memcpy_LIB_DIRS_RELEASE}"
                              "${abseil_absl_non_temporal_memcpy_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_non_temporal_memcpy_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_non_temporal_memcpy_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_non_temporal_memcpy_DEPS_TARGET
                              abseil_absl_non_temporal_memcpy_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_non_temporal_memcpy"
                              "${abseil_absl_non_temporal_memcpy_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::non_temporal_memcpy
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_non_temporal_memcpy_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_non_temporal_memcpy_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_non_temporal_memcpy_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::non_temporal_memcpy
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_non_temporal_memcpy_DEPS_TARGET)
        endif()

        set_property(TARGET absl::non_temporal_memcpy APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_non_temporal_memcpy_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::non_temporal_memcpy APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_non_temporal_memcpy_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::non_temporal_memcpy APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_non_temporal_memcpy_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::non_temporal_memcpy APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_non_temporal_memcpy_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::non_temporal_memcpy APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_non_temporal_memcpy_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::hashtable_debug #############

        set(abseil_absl_hashtable_debug_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_hashtable_debug_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_hashtable_debug_FRAMEWORKS_RELEASE}" "${abseil_absl_hashtable_debug_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_hashtable_debug_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_hashtable_debug_DEPS_TARGET)
            add_library(abseil_absl_hashtable_debug_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_hashtable_debug_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_hashtable_debug_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_hashtable_debug_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_hashtable_debug_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_hashtable_debug_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_hashtable_debug_LIBS_RELEASE}"
                              "${abseil_absl_hashtable_debug_LIB_DIRS_RELEASE}"
                              "${abseil_absl_hashtable_debug_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_hashtable_debug_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_hashtable_debug_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_hashtable_debug_DEPS_TARGET
                              abseil_absl_hashtable_debug_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_hashtable_debug"
                              "${abseil_absl_hashtable_debug_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::hashtable_debug
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_hashtable_debug_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_hashtable_debug_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_hashtable_debug_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::hashtable_debug
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_hashtable_debug_DEPS_TARGET)
        endif()

        set_property(TARGET absl::hashtable_debug APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_hashtable_debug_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::hashtable_debug APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_hashtable_debug_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::hashtable_debug APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_hashtable_debug_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::hashtable_debug APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_hashtable_debug_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::hashtable_debug APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_hashtable_debug_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::prefetch #############

        set(abseil_absl_prefetch_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_prefetch_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_prefetch_FRAMEWORKS_RELEASE}" "${abseil_absl_prefetch_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_prefetch_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_prefetch_DEPS_TARGET)
            add_library(abseil_absl_prefetch_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_prefetch_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_prefetch_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_prefetch_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_prefetch_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_prefetch_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_prefetch_LIBS_RELEASE}"
                              "${abseil_absl_prefetch_LIB_DIRS_RELEASE}"
                              "${abseil_absl_prefetch_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_prefetch_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_prefetch_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_prefetch_DEPS_TARGET
                              abseil_absl_prefetch_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_prefetch"
                              "${abseil_absl_prefetch_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::prefetch
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_prefetch_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_prefetch_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_prefetch_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::prefetch
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_prefetch_DEPS_TARGET)
        endif()

        set_property(TARGET absl::prefetch APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_prefetch_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::prefetch APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_prefetch_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::prefetch APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_prefetch_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::prefetch APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_prefetch_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::prefetch APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_prefetch_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::strerror #############

        set(abseil_absl_strerror_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_strerror_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_strerror_FRAMEWORKS_RELEASE}" "${abseil_absl_strerror_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_strerror_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_strerror_DEPS_TARGET)
            add_library(abseil_absl_strerror_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_strerror_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_strerror_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_strerror_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_strerror_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_strerror_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_strerror_LIBS_RELEASE}"
                              "${abseil_absl_strerror_LIB_DIRS_RELEASE}"
                              "${abseil_absl_strerror_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_strerror_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_strerror_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_strerror_DEPS_TARGET
                              abseil_absl_strerror_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_strerror"
                              "${abseil_absl_strerror_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::strerror
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_strerror_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_strerror_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_strerror_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::strerror
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_strerror_DEPS_TARGET)
        endif()

        set_property(TARGET absl::strerror APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_strerror_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::strerror APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_strerror_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::strerror APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_strerror_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::strerror APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_strerror_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::strerror APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_strerror_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::nullability_traits_internal #############

        set(abseil_absl_nullability_traits_internal_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_nullability_traits_internal_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_nullability_traits_internal_FRAMEWORKS_RELEASE}" "${abseil_absl_nullability_traits_internal_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_nullability_traits_internal_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_nullability_traits_internal_DEPS_TARGET)
            add_library(abseil_absl_nullability_traits_internal_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_nullability_traits_internal_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_nullability_traits_internal_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_nullability_traits_internal_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_nullability_traits_internal_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_nullability_traits_internal_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_nullability_traits_internal_LIBS_RELEASE}"
                              "${abseil_absl_nullability_traits_internal_LIB_DIRS_RELEASE}"
                              "${abseil_absl_nullability_traits_internal_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_nullability_traits_internal_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_nullability_traits_internal_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_nullability_traits_internal_DEPS_TARGET
                              abseil_absl_nullability_traits_internal_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_nullability_traits_internal"
                              "${abseil_absl_nullability_traits_internal_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::nullability_traits_internal
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_nullability_traits_internal_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_nullability_traits_internal_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_nullability_traits_internal_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::nullability_traits_internal
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_nullability_traits_internal_DEPS_TARGET)
        endif()

        set_property(TARGET absl::nullability_traits_internal APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_nullability_traits_internal_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::nullability_traits_internal APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_nullability_traits_internal_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::nullability_traits_internal APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_nullability_traits_internal_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::nullability_traits_internal APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_nullability_traits_internal_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::nullability_traits_internal APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_nullability_traits_internal_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::no_destructor #############

        set(abseil_absl_no_destructor_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_no_destructor_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_no_destructor_FRAMEWORKS_RELEASE}" "${abseil_absl_no_destructor_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_no_destructor_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_no_destructor_DEPS_TARGET)
            add_library(abseil_absl_no_destructor_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_no_destructor_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_no_destructor_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_no_destructor_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_no_destructor_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_no_destructor_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_no_destructor_LIBS_RELEASE}"
                              "${abseil_absl_no_destructor_LIB_DIRS_RELEASE}"
                              "${abseil_absl_no_destructor_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_no_destructor_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_no_destructor_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_no_destructor_DEPS_TARGET
                              abseil_absl_no_destructor_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_no_destructor"
                              "${abseil_absl_no_destructor_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::no_destructor
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_no_destructor_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_no_destructor_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_no_destructor_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::no_destructor
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_no_destructor_DEPS_TARGET)
        endif()

        set_property(TARGET absl::no_destructor APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_no_destructor_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::no_destructor APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_no_destructor_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::no_destructor APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_no_destructor_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::no_destructor APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_no_destructor_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::no_destructor APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_no_destructor_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::log_severity #############

        set(abseil_absl_log_severity_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_log_severity_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_log_severity_FRAMEWORKS_RELEASE}" "${abseil_absl_log_severity_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_log_severity_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_log_severity_DEPS_TARGET)
            add_library(abseil_absl_log_severity_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_log_severity_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_severity_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_severity_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_severity_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_log_severity_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_log_severity_LIBS_RELEASE}"
                              "${abseil_absl_log_severity_LIB_DIRS_RELEASE}"
                              "${abseil_absl_log_severity_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_log_severity_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_log_severity_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_log_severity_DEPS_TARGET
                              abseil_absl_log_severity_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_log_severity"
                              "${abseil_absl_log_severity_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::log_severity
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_log_severity_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_log_severity_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_log_severity_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::log_severity
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_log_severity_DEPS_TARGET)
        endif()

        set_property(TARGET absl::log_severity APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_severity_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::log_severity APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_severity_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::log_severity APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_log_severity_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::log_severity APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_severity_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::log_severity APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_log_severity_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::atomic_hook #############

        set(abseil_absl_atomic_hook_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_atomic_hook_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_atomic_hook_FRAMEWORKS_RELEASE}" "${abseil_absl_atomic_hook_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_atomic_hook_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_atomic_hook_DEPS_TARGET)
            add_library(abseil_absl_atomic_hook_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_atomic_hook_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_atomic_hook_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_atomic_hook_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_atomic_hook_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_atomic_hook_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_atomic_hook_LIBS_RELEASE}"
                              "${abseil_absl_atomic_hook_LIB_DIRS_RELEASE}"
                              "${abseil_absl_atomic_hook_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_atomic_hook_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_atomic_hook_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_atomic_hook_DEPS_TARGET
                              abseil_absl_atomic_hook_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_atomic_hook"
                              "${abseil_absl_atomic_hook_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::atomic_hook
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_atomic_hook_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_atomic_hook_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_atomic_hook_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::atomic_hook
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_atomic_hook_DEPS_TARGET)
        endif()

        set_property(TARGET absl::atomic_hook APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_atomic_hook_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::atomic_hook APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_atomic_hook_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::atomic_hook APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_atomic_hook_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::atomic_hook APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_atomic_hook_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::atomic_hook APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_atomic_hook_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::cordz_update_tracker #############

        set(abseil_absl_cordz_update_tracker_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_cordz_update_tracker_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_cordz_update_tracker_FRAMEWORKS_RELEASE}" "${abseil_absl_cordz_update_tracker_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_cordz_update_tracker_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_cordz_update_tracker_DEPS_TARGET)
            add_library(abseil_absl_cordz_update_tracker_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_cordz_update_tracker_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_cordz_update_tracker_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_cordz_update_tracker_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_cordz_update_tracker_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_cordz_update_tracker_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_cordz_update_tracker_LIBS_RELEASE}"
                              "${abseil_absl_cordz_update_tracker_LIB_DIRS_RELEASE}"
                              "${abseil_absl_cordz_update_tracker_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_cordz_update_tracker_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_cordz_update_tracker_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_cordz_update_tracker_DEPS_TARGET
                              abseil_absl_cordz_update_tracker_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_cordz_update_tracker"
                              "${abseil_absl_cordz_update_tracker_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::cordz_update_tracker
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_cordz_update_tracker_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_cordz_update_tracker_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_cordz_update_tracker_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::cordz_update_tracker
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_cordz_update_tracker_DEPS_TARGET)
        endif()

        set_property(TARGET absl::cordz_update_tracker APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_cordz_update_tracker_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::cordz_update_tracker APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_cordz_update_tracker_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::cordz_update_tracker APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_cordz_update_tracker_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::cordz_update_tracker APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_cordz_update_tracker_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::cordz_update_tracker APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_cordz_update_tracker_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::has_ostream_operator #############

        set(abseil_absl_has_ostream_operator_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_has_ostream_operator_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_has_ostream_operator_FRAMEWORKS_RELEASE}" "${abseil_absl_has_ostream_operator_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_has_ostream_operator_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_has_ostream_operator_DEPS_TARGET)
            add_library(abseil_absl_has_ostream_operator_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_has_ostream_operator_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_has_ostream_operator_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_has_ostream_operator_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_has_ostream_operator_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_has_ostream_operator_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_has_ostream_operator_LIBS_RELEASE}"
                              "${abseil_absl_has_ostream_operator_LIB_DIRS_RELEASE}"
                              "${abseil_absl_has_ostream_operator_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_has_ostream_operator_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_has_ostream_operator_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_has_ostream_operator_DEPS_TARGET
                              abseil_absl_has_ostream_operator_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_has_ostream_operator"
                              "${abseil_absl_has_ostream_operator_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::has_ostream_operator
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_has_ostream_operator_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_has_ostream_operator_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_has_ostream_operator_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::has_ostream_operator
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_has_ostream_operator_DEPS_TARGET)
        endif()

        set_property(TARGET absl::has_ostream_operator APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_has_ostream_operator_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::has_ostream_operator APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_has_ostream_operator_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::has_ostream_operator APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_has_ostream_operator_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::has_ostream_operator APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_has_ostream_operator_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::has_ostream_operator APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_has_ostream_operator_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::random_internal_platform #############

        set(abseil_absl_random_internal_platform_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_random_internal_platform_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_random_internal_platform_FRAMEWORKS_RELEASE}" "${abseil_absl_random_internal_platform_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_random_internal_platform_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_random_internal_platform_DEPS_TARGET)
            add_library(abseil_absl_random_internal_platform_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_random_internal_platform_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_platform_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_platform_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_platform_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_random_internal_platform_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_random_internal_platform_LIBS_RELEASE}"
                              "${abseil_absl_random_internal_platform_LIB_DIRS_RELEASE}"
                              "${abseil_absl_random_internal_platform_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_random_internal_platform_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_random_internal_platform_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_random_internal_platform_DEPS_TARGET
                              abseil_absl_random_internal_platform_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_random_internal_platform"
                              "${abseil_absl_random_internal_platform_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::random_internal_platform
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_platform_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_platform_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_random_internal_platform_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::random_internal_platform
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_random_internal_platform_DEPS_TARGET)
        endif()

        set_property(TARGET absl::random_internal_platform APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_platform_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::random_internal_platform APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_platform_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::random_internal_platform APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_platform_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::random_internal_platform APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_platform_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::random_internal_platform APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_platform_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::random_internal_fast_uniform_bits #############

        set(abseil_absl_random_internal_fast_uniform_bits_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_random_internal_fast_uniform_bits_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_random_internal_fast_uniform_bits_FRAMEWORKS_RELEASE}" "${abseil_absl_random_internal_fast_uniform_bits_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_random_internal_fast_uniform_bits_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_random_internal_fast_uniform_bits_DEPS_TARGET)
            add_library(abseil_absl_random_internal_fast_uniform_bits_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_random_internal_fast_uniform_bits_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_fast_uniform_bits_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_fast_uniform_bits_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_fast_uniform_bits_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_random_internal_fast_uniform_bits_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_random_internal_fast_uniform_bits_LIBS_RELEASE}"
                              "${abseil_absl_random_internal_fast_uniform_bits_LIB_DIRS_RELEASE}"
                              "${abseil_absl_random_internal_fast_uniform_bits_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_random_internal_fast_uniform_bits_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_random_internal_fast_uniform_bits_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_random_internal_fast_uniform_bits_DEPS_TARGET
                              abseil_absl_random_internal_fast_uniform_bits_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_random_internal_fast_uniform_bits"
                              "${abseil_absl_random_internal_fast_uniform_bits_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::random_internal_fast_uniform_bits
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_fast_uniform_bits_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_fast_uniform_bits_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_random_internal_fast_uniform_bits_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::random_internal_fast_uniform_bits
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_random_internal_fast_uniform_bits_DEPS_TARGET)
        endif()

        set_property(TARGET absl::random_internal_fast_uniform_bits APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_fast_uniform_bits_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::random_internal_fast_uniform_bits APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_fast_uniform_bits_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::random_internal_fast_uniform_bits APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_fast_uniform_bits_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::random_internal_fast_uniform_bits APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_fast_uniform_bits_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::random_internal_fast_uniform_bits APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_fast_uniform_bits_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::random_internal_traits #############

        set(abseil_absl_random_internal_traits_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_random_internal_traits_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_random_internal_traits_FRAMEWORKS_RELEASE}" "${abseil_absl_random_internal_traits_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_random_internal_traits_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_random_internal_traits_DEPS_TARGET)
            add_library(abseil_absl_random_internal_traits_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_random_internal_traits_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_traits_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_traits_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_traits_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_random_internal_traits_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_random_internal_traits_LIBS_RELEASE}"
                              "${abseil_absl_random_internal_traits_LIB_DIRS_RELEASE}"
                              "${abseil_absl_random_internal_traits_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_random_internal_traits_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_random_internal_traits_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_random_internal_traits_DEPS_TARGET
                              abseil_absl_random_internal_traits_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_random_internal_traits"
                              "${abseil_absl_random_internal_traits_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::random_internal_traits
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_traits_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_traits_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_random_internal_traits_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::random_internal_traits
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_random_internal_traits_DEPS_TARGET)
        endif()

        set_property(TARGET absl::random_internal_traits APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_traits_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::random_internal_traits APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_traits_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::random_internal_traits APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_traits_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::random_internal_traits APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_traits_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::random_internal_traits APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_random_internal_traits_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::numeric_representation #############

        set(abseil_absl_numeric_representation_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_numeric_representation_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_numeric_representation_FRAMEWORKS_RELEASE}" "${abseil_absl_numeric_representation_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_numeric_representation_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_numeric_representation_DEPS_TARGET)
            add_library(abseil_absl_numeric_representation_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_numeric_representation_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_numeric_representation_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_numeric_representation_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_numeric_representation_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_numeric_representation_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_numeric_representation_LIBS_RELEASE}"
                              "${abseil_absl_numeric_representation_LIB_DIRS_RELEASE}"
                              "${abseil_absl_numeric_representation_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_numeric_representation_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_numeric_representation_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_numeric_representation_DEPS_TARGET
                              abseil_absl_numeric_representation_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_numeric_representation"
                              "${abseil_absl_numeric_representation_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::numeric_representation
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_numeric_representation_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_numeric_representation_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_numeric_representation_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::numeric_representation
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_numeric_representation_DEPS_TARGET)
        endif()

        set_property(TARGET absl::numeric_representation APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_numeric_representation_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::numeric_representation APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_numeric_representation_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::numeric_representation APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_numeric_representation_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::numeric_representation APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_numeric_representation_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::numeric_representation APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_numeric_representation_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::requires_internal #############

        set(abseil_absl_requires_internal_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_requires_internal_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_requires_internal_FRAMEWORKS_RELEASE}" "${abseil_absl_requires_internal_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_requires_internal_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_requires_internal_DEPS_TARGET)
            add_library(abseil_absl_requires_internal_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_requires_internal_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_requires_internal_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_requires_internal_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_requires_internal_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_requires_internal_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_requires_internal_LIBS_RELEASE}"
                              "${abseil_absl_requires_internal_LIB_DIRS_RELEASE}"
                              "${abseil_absl_requires_internal_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_requires_internal_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_requires_internal_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_requires_internal_DEPS_TARGET
                              abseil_absl_requires_internal_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_requires_internal"
                              "${abseil_absl_requires_internal_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::requires_internal
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_requires_internal_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_requires_internal_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_requires_internal_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::requires_internal
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_requires_internal_DEPS_TARGET)
        endif()

        set_property(TARGET absl::requires_internal APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_requires_internal_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::requires_internal APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_requires_internal_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::requires_internal APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_requires_internal_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::requires_internal APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_requires_internal_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::requires_internal APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_requires_internal_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::constexpr_testing_internal #############

        set(abseil_absl_constexpr_testing_internal_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_constexpr_testing_internal_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_constexpr_testing_internal_FRAMEWORKS_RELEASE}" "${abseil_absl_constexpr_testing_internal_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_constexpr_testing_internal_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_constexpr_testing_internal_DEPS_TARGET)
            add_library(abseil_absl_constexpr_testing_internal_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_constexpr_testing_internal_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_constexpr_testing_internal_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_constexpr_testing_internal_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_constexpr_testing_internal_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_constexpr_testing_internal_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_constexpr_testing_internal_LIBS_RELEASE}"
                              "${abseil_absl_constexpr_testing_internal_LIB_DIRS_RELEASE}"
                              "${abseil_absl_constexpr_testing_internal_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_constexpr_testing_internal_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_constexpr_testing_internal_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_constexpr_testing_internal_DEPS_TARGET
                              abseil_absl_constexpr_testing_internal_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_constexpr_testing_internal"
                              "${abseil_absl_constexpr_testing_internal_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::constexpr_testing_internal
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_constexpr_testing_internal_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_constexpr_testing_internal_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_constexpr_testing_internal_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::constexpr_testing_internal
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_constexpr_testing_internal_DEPS_TARGET)
        endif()

        set_property(TARGET absl::constexpr_testing_internal APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_constexpr_testing_internal_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::constexpr_testing_internal APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_constexpr_testing_internal_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::constexpr_testing_internal APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_constexpr_testing_internal_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::constexpr_testing_internal APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_constexpr_testing_internal_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::constexpr_testing_internal APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_constexpr_testing_internal_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::weakly_mixed_integer #############

        set(abseil_absl_weakly_mixed_integer_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_weakly_mixed_integer_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_weakly_mixed_integer_FRAMEWORKS_RELEASE}" "${abseil_absl_weakly_mixed_integer_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_weakly_mixed_integer_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_weakly_mixed_integer_DEPS_TARGET)
            add_library(abseil_absl_weakly_mixed_integer_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_weakly_mixed_integer_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_weakly_mixed_integer_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_weakly_mixed_integer_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_weakly_mixed_integer_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_weakly_mixed_integer_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_weakly_mixed_integer_LIBS_RELEASE}"
                              "${abseil_absl_weakly_mixed_integer_LIB_DIRS_RELEASE}"
                              "${abseil_absl_weakly_mixed_integer_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_weakly_mixed_integer_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_weakly_mixed_integer_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_weakly_mixed_integer_DEPS_TARGET
                              abseil_absl_weakly_mixed_integer_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_weakly_mixed_integer"
                              "${abseil_absl_weakly_mixed_integer_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::weakly_mixed_integer
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_weakly_mixed_integer_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_weakly_mixed_integer_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_weakly_mixed_integer_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::weakly_mixed_integer
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_weakly_mixed_integer_DEPS_TARGET)
        endif()

        set_property(TARGET absl::weakly_mixed_integer APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_weakly_mixed_integer_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::weakly_mixed_integer APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_weakly_mixed_integer_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::weakly_mixed_integer APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_weakly_mixed_integer_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::weakly_mixed_integer APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_weakly_mixed_integer_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::weakly_mixed_integer APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_weakly_mixed_integer_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::utf8_for_code_point #############

        set(abseil_absl_utf8_for_code_point_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_utf8_for_code_point_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_utf8_for_code_point_FRAMEWORKS_RELEASE}" "${abseil_absl_utf8_for_code_point_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_utf8_for_code_point_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_utf8_for_code_point_DEPS_TARGET)
            add_library(abseil_absl_utf8_for_code_point_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_utf8_for_code_point_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_utf8_for_code_point_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_utf8_for_code_point_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_utf8_for_code_point_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_utf8_for_code_point_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_utf8_for_code_point_LIBS_RELEASE}"
                              "${abseil_absl_utf8_for_code_point_LIB_DIRS_RELEASE}"
                              "${abseil_absl_utf8_for_code_point_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_utf8_for_code_point_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_utf8_for_code_point_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_utf8_for_code_point_DEPS_TARGET
                              abseil_absl_utf8_for_code_point_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_utf8_for_code_point"
                              "${abseil_absl_utf8_for_code_point_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::utf8_for_code_point
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_utf8_for_code_point_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_utf8_for_code_point_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_utf8_for_code_point_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::utf8_for_code_point
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_utf8_for_code_point_DEPS_TARGET)
        endif()

        set_property(TARGET absl::utf8_for_code_point APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_utf8_for_code_point_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::utf8_for_code_point APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_utf8_for_code_point_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::utf8_for_code_point APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_utf8_for_code_point_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::utf8_for_code_point APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_utf8_for_code_point_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::utf8_for_code_point APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_utf8_for_code_point_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::non_temporal_arm_intrinsics #############

        set(abseil_absl_non_temporal_arm_intrinsics_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_non_temporal_arm_intrinsics_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_non_temporal_arm_intrinsics_FRAMEWORKS_RELEASE}" "${abseil_absl_non_temporal_arm_intrinsics_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_non_temporal_arm_intrinsics_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_non_temporal_arm_intrinsics_DEPS_TARGET)
            add_library(abseil_absl_non_temporal_arm_intrinsics_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_non_temporal_arm_intrinsics_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_non_temporal_arm_intrinsics_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_non_temporal_arm_intrinsics_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_non_temporal_arm_intrinsics_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_non_temporal_arm_intrinsics_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_non_temporal_arm_intrinsics_LIBS_RELEASE}"
                              "${abseil_absl_non_temporal_arm_intrinsics_LIB_DIRS_RELEASE}"
                              "${abseil_absl_non_temporal_arm_intrinsics_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_non_temporal_arm_intrinsics_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_non_temporal_arm_intrinsics_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_non_temporal_arm_intrinsics_DEPS_TARGET
                              abseil_absl_non_temporal_arm_intrinsics_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_non_temporal_arm_intrinsics"
                              "${abseil_absl_non_temporal_arm_intrinsics_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::non_temporal_arm_intrinsics
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_non_temporal_arm_intrinsics_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_non_temporal_arm_intrinsics_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_non_temporal_arm_intrinsics_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::non_temporal_arm_intrinsics
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_non_temporal_arm_intrinsics_DEPS_TARGET)
        endif()

        set_property(TARGET absl::non_temporal_arm_intrinsics APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_non_temporal_arm_intrinsics_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::non_temporal_arm_intrinsics APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_non_temporal_arm_intrinsics_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::non_temporal_arm_intrinsics APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_non_temporal_arm_intrinsics_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::non_temporal_arm_intrinsics APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_non_temporal_arm_intrinsics_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::non_temporal_arm_intrinsics APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_non_temporal_arm_intrinsics_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::raw_hash_set_resize_impl #############

        set(abseil_absl_raw_hash_set_resize_impl_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_raw_hash_set_resize_impl_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_raw_hash_set_resize_impl_FRAMEWORKS_RELEASE}" "${abseil_absl_raw_hash_set_resize_impl_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_raw_hash_set_resize_impl_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_raw_hash_set_resize_impl_DEPS_TARGET)
            add_library(abseil_absl_raw_hash_set_resize_impl_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_raw_hash_set_resize_impl_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_raw_hash_set_resize_impl_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_raw_hash_set_resize_impl_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_raw_hash_set_resize_impl_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_raw_hash_set_resize_impl_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_raw_hash_set_resize_impl_LIBS_RELEASE}"
                              "${abseil_absl_raw_hash_set_resize_impl_LIB_DIRS_RELEASE}"
                              "${abseil_absl_raw_hash_set_resize_impl_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_raw_hash_set_resize_impl_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_raw_hash_set_resize_impl_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_raw_hash_set_resize_impl_DEPS_TARGET
                              abseil_absl_raw_hash_set_resize_impl_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_raw_hash_set_resize_impl"
                              "${abseil_absl_raw_hash_set_resize_impl_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::raw_hash_set_resize_impl
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_raw_hash_set_resize_impl_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_raw_hash_set_resize_impl_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_raw_hash_set_resize_impl_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::raw_hash_set_resize_impl
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_raw_hash_set_resize_impl_DEPS_TARGET)
        endif()

        set_property(TARGET absl::raw_hash_set_resize_impl APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_raw_hash_set_resize_impl_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::raw_hash_set_resize_impl APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_raw_hash_set_resize_impl_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::raw_hash_set_resize_impl APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_raw_hash_set_resize_impl_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::raw_hash_set_resize_impl APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_raw_hash_set_resize_impl_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::raw_hash_set_resize_impl APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_raw_hash_set_resize_impl_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::node_slot_policy #############

        set(abseil_absl_node_slot_policy_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_node_slot_policy_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_node_slot_policy_FRAMEWORKS_RELEASE}" "${abseil_absl_node_slot_policy_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_node_slot_policy_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_node_slot_policy_DEPS_TARGET)
            add_library(abseil_absl_node_slot_policy_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_node_slot_policy_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_node_slot_policy_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_node_slot_policy_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_node_slot_policy_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_node_slot_policy_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_node_slot_policy_LIBS_RELEASE}"
                              "${abseil_absl_node_slot_policy_LIB_DIRS_RELEASE}"
                              "${abseil_absl_node_slot_policy_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_node_slot_policy_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_node_slot_policy_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_node_slot_policy_DEPS_TARGET
                              abseil_absl_node_slot_policy_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_node_slot_policy"
                              "${abseil_absl_node_slot_policy_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::node_slot_policy
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_node_slot_policy_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_node_slot_policy_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_node_slot_policy_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::node_slot_policy
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_node_slot_policy_DEPS_TARGET)
        endif()

        set_property(TARGET absl::node_slot_policy APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_node_slot_policy_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::node_slot_policy APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_node_slot_policy_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::node_slot_policy APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_node_slot_policy_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::node_slot_policy APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_node_slot_policy_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::node_slot_policy APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_node_slot_policy_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::hashtable_debug_hooks #############

        set(abseil_absl_hashtable_debug_hooks_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_hashtable_debug_hooks_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_hashtable_debug_hooks_FRAMEWORKS_RELEASE}" "${abseil_absl_hashtable_debug_hooks_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_hashtable_debug_hooks_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_hashtable_debug_hooks_DEPS_TARGET)
            add_library(abseil_absl_hashtable_debug_hooks_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_hashtable_debug_hooks_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_hashtable_debug_hooks_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_hashtable_debug_hooks_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_hashtable_debug_hooks_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_hashtable_debug_hooks_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_hashtable_debug_hooks_LIBS_RELEASE}"
                              "${abseil_absl_hashtable_debug_hooks_LIB_DIRS_RELEASE}"
                              "${abseil_absl_hashtable_debug_hooks_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_hashtable_debug_hooks_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_hashtable_debug_hooks_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_hashtable_debug_hooks_DEPS_TARGET
                              abseil_absl_hashtable_debug_hooks_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_hashtable_debug_hooks"
                              "${abseil_absl_hashtable_debug_hooks_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::hashtable_debug_hooks
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_hashtable_debug_hooks_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_hashtable_debug_hooks_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_hashtable_debug_hooks_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::hashtable_debug_hooks
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_hashtable_debug_hooks_DEPS_TARGET)
        endif()

        set_property(TARGET absl::hashtable_debug_hooks APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_hashtable_debug_hooks_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::hashtable_debug_hooks APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_hashtable_debug_hooks_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::hashtable_debug_hooks APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_hashtable_debug_hooks_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::hashtable_debug_hooks APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_hashtable_debug_hooks_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::hashtable_debug_hooks APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_hashtable_debug_hooks_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::algorithm #############

        set(abseil_absl_algorithm_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_algorithm_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_algorithm_FRAMEWORKS_RELEASE}" "${abseil_absl_algorithm_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_algorithm_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_algorithm_DEPS_TARGET)
            add_library(abseil_absl_algorithm_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_algorithm_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_algorithm_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_algorithm_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_algorithm_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_algorithm_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_algorithm_LIBS_RELEASE}"
                              "${abseil_absl_algorithm_LIB_DIRS_RELEASE}"
                              "${abseil_absl_algorithm_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_algorithm_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_algorithm_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_algorithm_DEPS_TARGET
                              abseil_absl_algorithm_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_algorithm"
                              "${abseil_absl_algorithm_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::algorithm
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_algorithm_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_algorithm_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_algorithm_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::algorithm
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_algorithm_DEPS_TARGET)
        endif()

        set_property(TARGET absl::algorithm APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_algorithm_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::algorithm APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_algorithm_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::algorithm APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_algorithm_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::algorithm APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_algorithm_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::algorithm APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_algorithm_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::iterator_traits_test_helper_internal #############

        set(abseil_absl_iterator_traits_test_helper_internal_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_iterator_traits_test_helper_internal_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_iterator_traits_test_helper_internal_FRAMEWORKS_RELEASE}" "${abseil_absl_iterator_traits_test_helper_internal_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_iterator_traits_test_helper_internal_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_iterator_traits_test_helper_internal_DEPS_TARGET)
            add_library(abseil_absl_iterator_traits_test_helper_internal_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_iterator_traits_test_helper_internal_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_iterator_traits_test_helper_internal_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_iterator_traits_test_helper_internal_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_iterator_traits_test_helper_internal_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_iterator_traits_test_helper_internal_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_iterator_traits_test_helper_internal_LIBS_RELEASE}"
                              "${abseil_absl_iterator_traits_test_helper_internal_LIB_DIRS_RELEASE}"
                              "${abseil_absl_iterator_traits_test_helper_internal_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_iterator_traits_test_helper_internal_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_iterator_traits_test_helper_internal_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_iterator_traits_test_helper_internal_DEPS_TARGET
                              abseil_absl_iterator_traits_test_helper_internal_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_iterator_traits_test_helper_internal"
                              "${abseil_absl_iterator_traits_test_helper_internal_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::iterator_traits_test_helper_internal
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_iterator_traits_test_helper_internal_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_iterator_traits_test_helper_internal_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_iterator_traits_test_helper_internal_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::iterator_traits_test_helper_internal
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_iterator_traits_test_helper_internal_DEPS_TARGET)
        endif()

        set_property(TARGET absl::iterator_traits_test_helper_internal APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_iterator_traits_test_helper_internal_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::iterator_traits_test_helper_internal APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_iterator_traits_test_helper_internal_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::iterator_traits_test_helper_internal APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_iterator_traits_test_helper_internal_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::iterator_traits_test_helper_internal APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_iterator_traits_test_helper_internal_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::iterator_traits_test_helper_internal APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_iterator_traits_test_helper_internal_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::fast_type_id #############

        set(abseil_absl_fast_type_id_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_fast_type_id_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_fast_type_id_FRAMEWORKS_RELEASE}" "${abseil_absl_fast_type_id_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_fast_type_id_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_fast_type_id_DEPS_TARGET)
            add_library(abseil_absl_fast_type_id_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_fast_type_id_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_fast_type_id_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_fast_type_id_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_fast_type_id_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_fast_type_id_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_fast_type_id_LIBS_RELEASE}"
                              "${abseil_absl_fast_type_id_LIB_DIRS_RELEASE}"
                              "${abseil_absl_fast_type_id_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_fast_type_id_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_fast_type_id_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_fast_type_id_DEPS_TARGET
                              abseil_absl_fast_type_id_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_fast_type_id"
                              "${abseil_absl_fast_type_id_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::fast_type_id
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_fast_type_id_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_fast_type_id_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_fast_type_id_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::fast_type_id
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_fast_type_id_DEPS_TARGET)
        endif()

        set_property(TARGET absl::fast_type_id APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_fast_type_id_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::fast_type_id APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_fast_type_id_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::fast_type_id APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_fast_type_id_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::fast_type_id APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_fast_type_id_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::fast_type_id APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_fast_type_id_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::core_headers #############

        set(abseil_absl_core_headers_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_core_headers_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_core_headers_FRAMEWORKS_RELEASE}" "${abseil_absl_core_headers_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_core_headers_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_core_headers_DEPS_TARGET)
            add_library(abseil_absl_core_headers_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_core_headers_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_core_headers_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_core_headers_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_core_headers_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_core_headers_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_core_headers_LIBS_RELEASE}"
                              "${abseil_absl_core_headers_LIB_DIRS_RELEASE}"
                              "${abseil_absl_core_headers_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_core_headers_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_core_headers_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_core_headers_DEPS_TARGET
                              abseil_absl_core_headers_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_core_headers"
                              "${abseil_absl_core_headers_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::core_headers
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_core_headers_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_core_headers_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_core_headers_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::core_headers
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_core_headers_DEPS_TARGET)
        endif()

        set_property(TARGET absl::core_headers APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_core_headers_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::core_headers APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_core_headers_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::core_headers APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_core_headers_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::core_headers APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_core_headers_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::core_headers APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_core_headers_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::dynamic_annotations #############

        set(abseil_absl_dynamic_annotations_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_dynamic_annotations_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_dynamic_annotations_FRAMEWORKS_RELEASE}" "${abseil_absl_dynamic_annotations_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_dynamic_annotations_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_dynamic_annotations_DEPS_TARGET)
            add_library(abseil_absl_dynamic_annotations_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_dynamic_annotations_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_dynamic_annotations_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_dynamic_annotations_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_dynamic_annotations_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_dynamic_annotations_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_dynamic_annotations_LIBS_RELEASE}"
                              "${abseil_absl_dynamic_annotations_LIB_DIRS_RELEASE}"
                              "${abseil_absl_dynamic_annotations_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_dynamic_annotations_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_dynamic_annotations_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_dynamic_annotations_DEPS_TARGET
                              abseil_absl_dynamic_annotations_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_dynamic_annotations"
                              "${abseil_absl_dynamic_annotations_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::dynamic_annotations
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_dynamic_annotations_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_dynamic_annotations_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_dynamic_annotations_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::dynamic_annotations
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_dynamic_annotations_DEPS_TARGET)
        endif()

        set_property(TARGET absl::dynamic_annotations APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_dynamic_annotations_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::dynamic_annotations APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_dynamic_annotations_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::dynamic_annotations APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_dynamic_annotations_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::dynamic_annotations APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_dynamic_annotations_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::dynamic_annotations APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_dynamic_annotations_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::nullability #############

        set(abseil_absl_nullability_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_nullability_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_nullability_FRAMEWORKS_RELEASE}" "${abseil_absl_nullability_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_nullability_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_nullability_DEPS_TARGET)
            add_library(abseil_absl_nullability_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_nullability_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_nullability_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_nullability_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_nullability_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_nullability_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_nullability_LIBS_RELEASE}"
                              "${abseil_absl_nullability_LIB_DIRS_RELEASE}"
                              "${abseil_absl_nullability_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_nullability_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_nullability_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_nullability_DEPS_TARGET
                              abseil_absl_nullability_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_nullability"
                              "${abseil_absl_nullability_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::nullability
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_nullability_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_nullability_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_nullability_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::nullability
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_nullability_DEPS_TARGET)
        endif()

        set_property(TARGET absl::nullability APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_nullability_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::nullability APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_nullability_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::nullability APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_nullability_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::nullability APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_nullability_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::nullability APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_nullability_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::errno_saver #############

        set(abseil_absl_errno_saver_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_errno_saver_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_errno_saver_FRAMEWORKS_RELEASE}" "${abseil_absl_errno_saver_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_errno_saver_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_errno_saver_DEPS_TARGET)
            add_library(abseil_absl_errno_saver_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_errno_saver_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_errno_saver_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_errno_saver_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_errno_saver_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_errno_saver_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_errno_saver_LIBS_RELEASE}"
                              "${abseil_absl_errno_saver_LIB_DIRS_RELEASE}"
                              "${abseil_absl_errno_saver_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_errno_saver_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_errno_saver_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_errno_saver_DEPS_TARGET
                              abseil_absl_errno_saver_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_errno_saver"
                              "${abseil_absl_errno_saver_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::errno_saver
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_errno_saver_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_errno_saver_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_errno_saver_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::errno_saver
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_errno_saver_DEPS_TARGET)
        endif()

        set_property(TARGET absl::errno_saver APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_errno_saver_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::errno_saver APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_errno_saver_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::errno_saver APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_errno_saver_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::errno_saver APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_errno_saver_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::errno_saver APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_errno_saver_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::bad_variant_access #############

        set(abseil_absl_bad_variant_access_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_bad_variant_access_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_bad_variant_access_FRAMEWORKS_RELEASE}" "${abseil_absl_bad_variant_access_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_bad_variant_access_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_bad_variant_access_DEPS_TARGET)
            add_library(abseil_absl_bad_variant_access_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_bad_variant_access_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_bad_variant_access_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_bad_variant_access_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_bad_variant_access_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_bad_variant_access_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_bad_variant_access_LIBS_RELEASE}"
                              "${abseil_absl_bad_variant_access_LIB_DIRS_RELEASE}"
                              "${abseil_absl_bad_variant_access_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_bad_variant_access_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_bad_variant_access_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_bad_variant_access_DEPS_TARGET
                              abseil_absl_bad_variant_access_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_bad_variant_access"
                              "${abseil_absl_bad_variant_access_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::bad_variant_access
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_bad_variant_access_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_bad_variant_access_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_bad_variant_access_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::bad_variant_access
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_bad_variant_access_DEPS_TARGET)
        endif()

        set_property(TARGET absl::bad_variant_access APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_bad_variant_access_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::bad_variant_access APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_bad_variant_access_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::bad_variant_access APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_bad_variant_access_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::bad_variant_access APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_bad_variant_access_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::bad_variant_access APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_bad_variant_access_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::bad_optional_access #############

        set(abseil_absl_bad_optional_access_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_bad_optional_access_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_bad_optional_access_FRAMEWORKS_RELEASE}" "${abseil_absl_bad_optional_access_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_bad_optional_access_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_bad_optional_access_DEPS_TARGET)
            add_library(abseil_absl_bad_optional_access_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_bad_optional_access_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_bad_optional_access_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_bad_optional_access_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_bad_optional_access_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_bad_optional_access_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_bad_optional_access_LIBS_RELEASE}"
                              "${abseil_absl_bad_optional_access_LIB_DIRS_RELEASE}"
                              "${abseil_absl_bad_optional_access_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_bad_optional_access_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_bad_optional_access_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_bad_optional_access_DEPS_TARGET
                              abseil_absl_bad_optional_access_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_bad_optional_access"
                              "${abseil_absl_bad_optional_access_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::bad_optional_access
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_bad_optional_access_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_bad_optional_access_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_bad_optional_access_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::bad_optional_access
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_bad_optional_access_DEPS_TARGET)
        endif()

        set_property(TARGET absl::bad_optional_access APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_bad_optional_access_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::bad_optional_access APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_bad_optional_access_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::bad_optional_access APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_bad_optional_access_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::bad_optional_access APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_bad_optional_access_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::bad_optional_access APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_bad_optional_access_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::bad_any_cast #############

        set(abseil_absl_bad_any_cast_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_bad_any_cast_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_bad_any_cast_FRAMEWORKS_RELEASE}" "${abseil_absl_bad_any_cast_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_bad_any_cast_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_bad_any_cast_DEPS_TARGET)
            add_library(abseil_absl_bad_any_cast_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_bad_any_cast_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_bad_any_cast_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_bad_any_cast_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_bad_any_cast_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_bad_any_cast_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_bad_any_cast_LIBS_RELEASE}"
                              "${abseil_absl_bad_any_cast_LIB_DIRS_RELEASE}"
                              "${abseil_absl_bad_any_cast_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_bad_any_cast_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_bad_any_cast_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_bad_any_cast_DEPS_TARGET
                              abseil_absl_bad_any_cast_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_bad_any_cast"
                              "${abseil_absl_bad_any_cast_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::bad_any_cast
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_bad_any_cast_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_bad_any_cast_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_bad_any_cast_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::bad_any_cast
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_bad_any_cast_DEPS_TARGET)
        endif()

        set_property(TARGET absl::bad_any_cast APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_bad_any_cast_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::bad_any_cast APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_bad_any_cast_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::bad_any_cast APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_bad_any_cast_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::bad_any_cast APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_bad_any_cast_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::bad_any_cast APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_bad_any_cast_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::time_zone #############

        set(abseil_absl_time_zone_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_time_zone_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_time_zone_FRAMEWORKS_RELEASE}" "${abseil_absl_time_zone_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_time_zone_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_time_zone_DEPS_TARGET)
            add_library(abseil_absl_time_zone_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_time_zone_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_time_zone_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_time_zone_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_time_zone_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_time_zone_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_time_zone_LIBS_RELEASE}"
                              "${abseil_absl_time_zone_LIB_DIRS_RELEASE}"
                              "${abseil_absl_time_zone_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_time_zone_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_time_zone_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_time_zone_DEPS_TARGET
                              abseil_absl_time_zone_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_time_zone"
                              "${abseil_absl_time_zone_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::time_zone
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_time_zone_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_time_zone_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_time_zone_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::time_zone
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_time_zone_DEPS_TARGET)
        endif()

        set_property(TARGET absl::time_zone APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_time_zone_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::time_zone APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_time_zone_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::time_zone APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_time_zone_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::time_zone APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_time_zone_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::time_zone APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_time_zone_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::civil_time #############

        set(abseil_absl_civil_time_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_civil_time_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_civil_time_FRAMEWORKS_RELEASE}" "${abseil_absl_civil_time_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_civil_time_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_civil_time_DEPS_TARGET)
            add_library(abseil_absl_civil_time_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_civil_time_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_civil_time_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_civil_time_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_civil_time_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_civil_time_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_civil_time_LIBS_RELEASE}"
                              "${abseil_absl_civil_time_LIB_DIRS_RELEASE}"
                              "${abseil_absl_civil_time_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_civil_time_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_civil_time_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_civil_time_DEPS_TARGET
                              abseil_absl_civil_time_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_civil_time"
                              "${abseil_absl_civil_time_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::civil_time
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_civil_time_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_civil_time_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_civil_time_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::civil_time
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_civil_time_DEPS_TARGET)
        endif()

        set_property(TARGET absl::civil_time APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_civil_time_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::civil_time APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_civil_time_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::civil_time APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_civil_time_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::civil_time APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_civil_time_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::civil_time APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_civil_time_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::pretty_function #############

        set(abseil_absl_pretty_function_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_pretty_function_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_pretty_function_FRAMEWORKS_RELEASE}" "${abseil_absl_pretty_function_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_pretty_function_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_pretty_function_DEPS_TARGET)
            add_library(abseil_absl_pretty_function_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_pretty_function_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_pretty_function_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_pretty_function_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_pretty_function_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_pretty_function_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_pretty_function_LIBS_RELEASE}"
                              "${abseil_absl_pretty_function_LIB_DIRS_RELEASE}"
                              "${abseil_absl_pretty_function_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_pretty_function_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_pretty_function_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_pretty_function_DEPS_TARGET
                              abseil_absl_pretty_function_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_pretty_function"
                              "${abseil_absl_pretty_function_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::pretty_function
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_pretty_function_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_pretty_function_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_pretty_function_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::pretty_function
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_pretty_function_DEPS_TARGET)
        endif()

        set_property(TARGET absl::pretty_function APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_pretty_function_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::pretty_function APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_pretty_function_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::pretty_function APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_pretty_function_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::pretty_function APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_pretty_function_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::pretty_function APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_pretty_function_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT absl::config #############

        set(abseil_absl_config_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(abseil_absl_config_FRAMEWORKS_FOUND_RELEASE "${abseil_absl_config_FRAMEWORKS_RELEASE}" "${abseil_absl_config_FRAMEWORK_DIRS_RELEASE}")

        set(abseil_absl_config_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET abseil_absl_config_DEPS_TARGET)
            add_library(abseil_absl_config_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET abseil_absl_config_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_config_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_config_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_config_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'abseil_absl_config_DEPS_TARGET' to all of them
        conan_package_library_targets("${abseil_absl_config_LIBS_RELEASE}"
                              "${abseil_absl_config_LIB_DIRS_RELEASE}"
                              "${abseil_absl_config_BIN_DIRS_RELEASE}" # package_bindir
                              "${abseil_absl_config_LIBRARY_TYPE_RELEASE}"
                              "${abseil_absl_config_IS_HOST_WINDOWS_RELEASE}"
                              abseil_absl_config_DEPS_TARGET
                              abseil_absl_config_LIBRARIES_TARGETS
                              "_RELEASE"
                              "abseil_absl_config"
                              "${abseil_absl_config_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET absl::config
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${abseil_absl_config_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${abseil_absl_config_LIBRARIES_TARGETS}>
                     )

        if("${abseil_absl_config_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET absl::config
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         abseil_absl_config_DEPS_TARGET)
        endif()

        set_property(TARGET absl::config APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_config_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET absl::config APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_config_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET absl::config APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${abseil_absl_config_LIB_DIRS_RELEASE}>)
        set_property(TARGET absl::config APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${abseil_absl_config_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET absl::config APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${abseil_absl_config_COMPILE_OPTIONS_RELEASE}>)


    ########## AGGREGATED GLOBAL TARGET WITH THE COMPONENTS #####################
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::flags_parse)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::log_flags)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::flags_usage)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::log_internal_flags)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::flags_usage_internal)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::hashtable_profiler)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::flags)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::profile_builder)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::log_streamer)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::die_if_null)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::check)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::absl_check)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::flags_reflection)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::log)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::absl_log)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::log_internal_check_impl)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::linked_hash_map)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::linked_hash_set)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::node_hash_map)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::flat_hash_map)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::log_structured)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::log_internal_log_impl)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::log_internal_check_op)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::raw_hash_map)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::node_hash_set)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::flat_hash_set)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::statusor)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::log_internal_structured)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::log_internal_strip)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::raw_hash_set)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::hash_container_defaults)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::status)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::log_internal_message)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::hash_function_defaults)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::btree)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::cord)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::log_sink_registry)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::cordz_update_scope)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::cordz_sample_token)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::random_random)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::vlog_is_on)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::log_initialize)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::log_internal_log_sink_set)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::flags_internal)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::cordz_info)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::random_internal_nonsecure_base)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::random_seed_sequences)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::absl_vlog_is_on)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::log_globals)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::flags_config)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::hashtablez_sampler)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::cordz_handle)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::cordz_statistics)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::cord_internal)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::random_internal_entropy_pool)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::sample_recorder)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::vlog_config_internal)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::flags_program_name)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::failure_signal_handler)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::synchronization)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::debugging)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::examine_stack)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::crc_cord_state)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::generic_printer_internal)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::random_internal_distribution_test_util)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::log_sink)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::log_internal_format)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::flags_marshalling)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::symbolize)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::stacktrace)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::crc32c)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::hash_policy_traits)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::kernel_timeout_internal)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::str_format)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::random_internal_salted_seed_seq)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::log_internal_structured_proto)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::log_entry)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::log_internal_globals)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::flags_private_handle_accessor)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::demangle_internal)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::borrowed_fixup_buffer)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::chunked_queue)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::container_memory)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::time)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::str_format_internal)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::random_internal_randen_engine)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::random_internal_pcg_engine)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::random_internal_seed_material)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::random_distributions)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::log_internal_container)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::log_internal_fnmatch)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::log_internal_append_truncated)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::log_internal_nullstream)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::log_internal_proto)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::hash)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::flags_commandlineflag)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::flags_path_util)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::demangle_rust)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::layout)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::strings)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::random_internal_wide_multiply)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::random_internal_generate_real)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::random_internal_iostream_state_saver)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::numeric)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::decode_rust_punycode)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::random_internal_fastmath)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::int128)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::bounded_utf8_length_sequence)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::crc_internal)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::hashtable_control_bytes)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::graphcycles_internal)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::strings_internal)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::bits)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::city)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::function_ref)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::inlined_vector)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::poison)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::strings_append_and_overwrite)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::random_internal_randen)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::log_internal_conditions)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::any_invocable)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::crc_cpu_detect)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::inlined_vector_internal)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::tracing_internal)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::endian)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::malloc_internal)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::span)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::strings_resize_and_overwrite)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::random_internal_randen_hwaes)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::random_internal_mock_helpers)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::random_bit_gen_ref)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::bind_front)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::fixed_array)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::cleanup)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::base)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::variant)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::optional)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::any)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::cordz_functions)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::random_internal_distribution_caller)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::random_seed_gen_exception)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::memory)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::overload)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::debugging_internal)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::common_policy_traits)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::compressed_tuple)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::cleanup_internal)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::algorithm_container)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::scoped_set_env)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::throw_delegate)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::spinlock_wait)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::utility)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::compare)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::charset)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::random_internal_uniform_helper)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::periodic_sampler)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::meta)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::container_common)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::iterator_traits_internal)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::base_internal)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::raw_logging_internal)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::string_view)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::random_internal_randen_hwaes_impl)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::random_internal_randen_slow)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::exponential_biased)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::type_traits)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::log_internal_voidify)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::log_internal_nullguard)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::log_internal_config)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::flags_commandlineflag_internal)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::leak_check)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::non_temporal_memcpy)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::hashtable_debug)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::prefetch)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::strerror)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::nullability_traits_internal)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::no_destructor)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::log_severity)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::atomic_hook)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::cordz_update_tracker)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::has_ostream_operator)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::random_internal_platform)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::random_internal_fast_uniform_bits)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::random_internal_traits)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::numeric_representation)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::requires_internal)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::constexpr_testing_internal)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::weakly_mixed_integer)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::utf8_for_code_point)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::non_temporal_arm_intrinsics)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::raw_hash_set_resize_impl)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::node_slot_policy)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::hashtable_debug_hooks)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::algorithm)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::iterator_traits_test_helper_internal)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::fast_type_id)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::core_headers)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::dynamic_annotations)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::nullability)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::errno_saver)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::bad_variant_access)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::bad_optional_access)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::bad_any_cast)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::time_zone)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::civil_time)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::pretty_function)
    set_property(TARGET abseil::abseil APPEND PROPERTY INTERFACE_LINK_LIBRARIES absl::config)

########## For the modules (FindXXX)
set(abseil_LIBRARIES_RELEASE abseil::abseil)
