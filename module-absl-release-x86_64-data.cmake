########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

list(APPEND abseil_COMPONENT_NAMES absl::config absl::pretty_function absl::civil_time absl::time_zone absl::bad_any_cast absl::bad_optional_access absl::bad_variant_access absl::errno_saver absl::nullability absl::dynamic_annotations absl::core_headers absl::fast_type_id absl::iterator_traits_test_helper_internal absl::algorithm absl::hashtable_debug_hooks absl::node_slot_policy absl::raw_hash_set_resize_impl absl::non_temporal_arm_intrinsics absl::utf8_for_code_point absl::weakly_mixed_integer absl::constexpr_testing_internal absl::requires_internal absl::numeric_representation absl::random_internal_traits absl::random_internal_fast_uniform_bits absl::random_internal_platform absl::has_ostream_operator absl::cordz_update_tracker absl::atomic_hook absl::log_severity absl::no_destructor absl::nullability_traits_internal absl::strerror absl::prefetch absl::hashtable_debug absl::non_temporal_memcpy absl::leak_check absl::flags_commandlineflag_internal absl::log_internal_config absl::log_internal_nullguard absl::log_internal_voidify absl::type_traits absl::exponential_biased absl::random_internal_randen_slow absl::random_internal_randen_hwaes_impl absl::string_view absl::raw_logging_internal absl::base_internal absl::iterator_traits_internal absl::container_common absl::meta absl::periodic_sampler absl::random_internal_uniform_helper absl::charset absl::compare absl::utility absl::spinlock_wait absl::throw_delegate absl::scoped_set_env absl::algorithm_container absl::cleanup_internal absl::compressed_tuple absl::common_policy_traits absl::debugging_internal absl::overload absl::memory absl::random_seed_gen_exception absl::random_internal_distribution_caller absl::cordz_functions absl::any absl::optional absl::variant absl::base absl::cleanup absl::fixed_array absl::bind_front absl::random_bit_gen_ref absl::random_internal_mock_helpers absl::random_internal_randen_hwaes absl::strings_resize_and_overwrite absl::span absl::malloc_internal absl::endian absl::tracing_internal absl::inlined_vector_internal absl::crc_cpu_detect absl::any_invocable absl::log_internal_conditions absl::random_internal_randen absl::strings_append_and_overwrite absl::poison absl::inlined_vector absl::function_ref absl::city absl::bits absl::strings_internal absl::graphcycles_internal absl::hashtable_control_bytes absl::crc_internal absl::bounded_utf8_length_sequence absl::int128 absl::random_internal_fastmath absl::decode_rust_punycode absl::numeric absl::random_internal_iostream_state_saver absl::random_internal_generate_real absl::random_internal_wide_multiply absl::strings absl::layout absl::demangle_rust absl::flags_path_util absl::flags_commandlineflag absl::hash absl::log_internal_proto absl::log_internal_nullstream absl::log_internal_append_truncated absl::log_internal_fnmatch absl::log_internal_container absl::random_distributions absl::random_internal_seed_material absl::random_internal_pcg_engine absl::random_internal_randen_engine absl::str_format_internal absl::time absl::container_memory absl::chunked_queue absl::borrowed_fixup_buffer absl::demangle_internal absl::flags_private_handle_accessor absl::log_internal_globals absl::log_entry absl::log_internal_structured_proto absl::random_internal_salted_seed_seq absl::str_format absl::kernel_timeout_internal absl::hash_policy_traits absl::crc32c absl::stacktrace absl::symbolize absl::flags_marshalling absl::log_internal_format absl::log_sink absl::random_internal_distribution_test_util absl::generic_printer_internal absl::crc_cord_state absl::examine_stack absl::debugging absl::synchronization absl::failure_signal_handler absl::flags_program_name absl::vlog_config_internal absl::sample_recorder absl::random_internal_entropy_pool absl::cord_internal absl::cordz_statistics absl::cordz_handle absl::hashtablez_sampler absl::flags_config absl::log_globals absl::absl_vlog_is_on absl::random_seed_sequences absl::random_internal_nonsecure_base absl::cordz_info absl::flags_internal absl::log_internal_log_sink_set absl::log_initialize absl::vlog_is_on absl::random_random absl::cordz_sample_token absl::cordz_update_scope absl::log_sink_registry absl::cord absl::btree absl::hash_function_defaults absl::log_internal_message absl::status absl::hash_container_defaults absl::raw_hash_set absl::log_internal_strip absl::log_internal_structured absl::statusor absl::flat_hash_set absl::node_hash_set absl::raw_hash_map absl::log_internal_check_op absl::log_internal_log_impl absl::log_structured absl::flat_hash_map absl::node_hash_map absl::linked_hash_set absl::linked_hash_map absl::log_internal_check_impl absl::absl_log absl::log absl::flags_reflection absl::absl_check absl::check absl::die_if_null absl::log_streamer absl::profile_builder absl::flags absl::hashtable_profiler absl::flags_usage_internal absl::log_internal_flags absl::flags_usage absl::log_flags absl::flags_parse)
list(REMOVE_DUPLICATES abseil_COMPONENT_NAMES)
if(DEFINED abseil_FIND_DEPENDENCY_NAMES)
  list(APPEND abseil_FIND_DEPENDENCY_NAMES )
  list(REMOVE_DUPLICATES abseil_FIND_DEPENDENCY_NAMES)
else()
  set(abseil_FIND_DEPENDENCY_NAMES )
endif()

########### VARIABLES #######################################################################
#############################################################################################
set(abseil_PACKAGE_FOLDER_RELEASE "/root/.conan2/p/b/abseiacffe2dc619e8/p")
set(abseil_BUILD_MODULES_PATHS_RELEASE )


set(abseil_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_RES_DIRS_RELEASE )
set(abseil_DEFINITIONS_RELEASE )
set(abseil_SHARED_LINK_FLAGS_RELEASE )
set(abseil_EXE_LINK_FLAGS_RELEASE )
set(abseil_OBJECTS_RELEASE )
set(abseil_COMPILE_DEFINITIONS_RELEASE )
set(abseil_COMPILE_OPTIONS_C_RELEASE )
set(abseil_COMPILE_OPTIONS_CXX_RELEASE )
set(abseil_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_BIN_DIRS_RELEASE )
set(abseil_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_LIBS_RELEASE absl_flags_parse absl_log_flags absl_flags_usage absl_flags_usage_internal absl_hashtable_profiler absl_profile_builder absl_die_if_null absl_flags_reflection absl_log_internal_check_op absl_statusor absl_raw_hash_set absl_status absl_log_internal_message absl_cord absl_cordz_sample_token absl_log_initialize absl_log_internal_log_sink_set absl_flags_internal absl_cordz_info absl_random_seed_sequences absl_log_globals absl_flags_config absl_hashtablez_sampler absl_cordz_handle absl_cord_internal absl_random_internal_entropy_pool absl_vlog_config_internal absl_flags_program_name absl_failure_signal_handler absl_synchronization absl_examine_stack absl_crc_cord_state absl_generic_printer_internal absl_random_internal_distribution_test_util absl_log_sink absl_log_internal_format absl_flags_marshalling absl_symbolize absl_stacktrace absl_crc32c absl_kernel_timeout_internal absl_log_internal_structured_proto absl_log_entry absl_log_internal_globals absl_flags_private_handle_accessor absl_demangle_internal absl_borrowed_fixup_buffer absl_time absl_str_format_internal absl_random_internal_seed_material absl_random_distributions absl_log_internal_fnmatch absl_log_internal_proto absl_hash absl_flags_commandlineflag absl_demangle_rust absl_strings absl_decode_rust_punycode absl_int128 absl_crc_internal absl_graphcycles_internal absl_strings_internal absl_city absl_poison absl_random_internal_randen absl_log_internal_conditions absl_crc_cpu_detect absl_tracing_internal absl_malloc_internal absl_random_internal_randen_hwaes absl_base absl_cordz_functions absl_random_seed_gen_exception absl_debugging_internal absl_scoped_set_env absl_throw_delegate absl_spinlock_wait absl_periodic_sampler absl_raw_logging_internal absl_random_internal_randen_hwaes_impl absl_random_internal_randen_slow absl_exponential_biased absl_log_internal_nullguard absl_flags_commandlineflag_internal absl_leak_check absl_strerror absl_log_severity absl_random_internal_platform absl_utf8_for_code_point absl_time_zone absl_civil_time)
set(abseil_SYSTEM_LIBS_RELEASE pthread rt)
set(abseil_FRAMEWORK_DIRS_RELEASE )
set(abseil_FRAMEWORKS_RELEASE )
set(abseil_BUILD_DIRS_RELEASE )
set(abseil_NO_SONAME_MODE_RELEASE FALSE)


# COMPOUND VARIABLES
set(abseil_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_COMPILE_OPTIONS_C_RELEASE}>")
set(abseil_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_EXE_LINK_FLAGS_RELEASE}>")


set(abseil_COMPONENTS_RELEASE absl::config absl::pretty_function absl::civil_time absl::time_zone absl::bad_any_cast absl::bad_optional_access absl::bad_variant_access absl::errno_saver absl::nullability absl::dynamic_annotations absl::core_headers absl::fast_type_id absl::iterator_traits_test_helper_internal absl::algorithm absl::hashtable_debug_hooks absl::node_slot_policy absl::raw_hash_set_resize_impl absl::non_temporal_arm_intrinsics absl::utf8_for_code_point absl::weakly_mixed_integer absl::constexpr_testing_internal absl::requires_internal absl::numeric_representation absl::random_internal_traits absl::random_internal_fast_uniform_bits absl::random_internal_platform absl::has_ostream_operator absl::cordz_update_tracker absl::atomic_hook absl::log_severity absl::no_destructor absl::nullability_traits_internal absl::strerror absl::prefetch absl::hashtable_debug absl::non_temporal_memcpy absl::leak_check absl::flags_commandlineflag_internal absl::log_internal_config absl::log_internal_nullguard absl::log_internal_voidify absl::type_traits absl::exponential_biased absl::random_internal_randen_slow absl::random_internal_randen_hwaes_impl absl::string_view absl::raw_logging_internal absl::base_internal absl::iterator_traits_internal absl::container_common absl::meta absl::periodic_sampler absl::random_internal_uniform_helper absl::charset absl::compare absl::utility absl::spinlock_wait absl::throw_delegate absl::scoped_set_env absl::algorithm_container absl::cleanup_internal absl::compressed_tuple absl::common_policy_traits absl::debugging_internal absl::overload absl::memory absl::random_seed_gen_exception absl::random_internal_distribution_caller absl::cordz_functions absl::any absl::optional absl::variant absl::base absl::cleanup absl::fixed_array absl::bind_front absl::random_bit_gen_ref absl::random_internal_mock_helpers absl::random_internal_randen_hwaes absl::strings_resize_and_overwrite absl::span absl::malloc_internal absl::endian absl::tracing_internal absl::inlined_vector_internal absl::crc_cpu_detect absl::any_invocable absl::log_internal_conditions absl::random_internal_randen absl::strings_append_and_overwrite absl::poison absl::inlined_vector absl::function_ref absl::city absl::bits absl::strings_internal absl::graphcycles_internal absl::hashtable_control_bytes absl::crc_internal absl::bounded_utf8_length_sequence absl::int128 absl::random_internal_fastmath absl::decode_rust_punycode absl::numeric absl::random_internal_iostream_state_saver absl::random_internal_generate_real absl::random_internal_wide_multiply absl::strings absl::layout absl::demangle_rust absl::flags_path_util absl::flags_commandlineflag absl::hash absl::log_internal_proto absl::log_internal_nullstream absl::log_internal_append_truncated absl::log_internal_fnmatch absl::log_internal_container absl::random_distributions absl::random_internal_seed_material absl::random_internal_pcg_engine absl::random_internal_randen_engine absl::str_format_internal absl::time absl::container_memory absl::chunked_queue absl::borrowed_fixup_buffer absl::demangle_internal absl::flags_private_handle_accessor absl::log_internal_globals absl::log_entry absl::log_internal_structured_proto absl::random_internal_salted_seed_seq absl::str_format absl::kernel_timeout_internal absl::hash_policy_traits absl::crc32c absl::stacktrace absl::symbolize absl::flags_marshalling absl::log_internal_format absl::log_sink absl::random_internal_distribution_test_util absl::generic_printer_internal absl::crc_cord_state absl::examine_stack absl::debugging absl::synchronization absl::failure_signal_handler absl::flags_program_name absl::vlog_config_internal absl::sample_recorder absl::random_internal_entropy_pool absl::cord_internal absl::cordz_statistics absl::cordz_handle absl::hashtablez_sampler absl::flags_config absl::log_globals absl::absl_vlog_is_on absl::random_seed_sequences absl::random_internal_nonsecure_base absl::cordz_info absl::flags_internal absl::log_internal_log_sink_set absl::log_initialize absl::vlog_is_on absl::random_random absl::cordz_sample_token absl::cordz_update_scope absl::log_sink_registry absl::cord absl::btree absl::hash_function_defaults absl::log_internal_message absl::status absl::hash_container_defaults absl::raw_hash_set absl::log_internal_strip absl::log_internal_structured absl::statusor absl::flat_hash_set absl::node_hash_set absl::raw_hash_map absl::log_internal_check_op absl::log_internal_log_impl absl::log_structured absl::flat_hash_map absl::node_hash_map absl::linked_hash_set absl::linked_hash_map absl::log_internal_check_impl absl::absl_log absl::log absl::flags_reflection absl::absl_check absl::check absl::die_if_null absl::log_streamer absl::profile_builder absl::flags absl::hashtable_profiler absl::flags_usage_internal absl::log_internal_flags absl::flags_usage absl::log_flags absl::flags_parse)
########### COMPONENT absl::flags_parse VARIABLES ############################################

set(abseil_absl_flags_parse_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_flags_parse_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_flags_parse_BIN_DIRS_RELEASE )
set(abseil_absl_flags_parse_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_flags_parse_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_flags_parse_RES_DIRS_RELEASE )
set(abseil_absl_flags_parse_DEFINITIONS_RELEASE )
set(abseil_absl_flags_parse_OBJECTS_RELEASE )
set(abseil_absl_flags_parse_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_flags_parse_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_flags_parse_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_flags_parse_LIBS_RELEASE absl_flags_parse)
set(abseil_absl_flags_parse_SYSTEM_LIBS_RELEASE )
set(abseil_absl_flags_parse_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_flags_parse_FRAMEWORKS_RELEASE )
set(abseil_absl_flags_parse_DEPENDENCIES_RELEASE absl::algorithm_container absl::config absl::core_headers absl::flags_config absl::flags absl::flags_commandlineflag absl::flags_commandlineflag_internal absl::flags_internal absl::flags_private_handle_accessor absl::flags_program_name absl::flags_reflection absl::flags_usage absl::no_destructor absl::strings absl::synchronization)
set(abseil_absl_flags_parse_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_flags_parse_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_flags_parse_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_flags_parse_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_flags_parse_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_flags_parse_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_flags_parse_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_flags_parse_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_flags_parse_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_flags_parse_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::log_flags VARIABLES ############################################

set(abseil_absl_log_flags_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_log_flags_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_log_flags_BIN_DIRS_RELEASE )
set(abseil_absl_log_flags_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_log_flags_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_log_flags_RES_DIRS_RELEASE )
set(abseil_absl_log_flags_DEFINITIONS_RELEASE )
set(abseil_absl_log_flags_OBJECTS_RELEASE )
set(abseil_absl_log_flags_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_log_flags_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_log_flags_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_log_flags_LIBS_RELEASE absl_log_flags)
set(abseil_absl_log_flags_SYSTEM_LIBS_RELEASE )
set(abseil_absl_log_flags_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_log_flags_FRAMEWORKS_RELEASE )
set(abseil_absl_log_flags_DEPENDENCIES_RELEASE absl::config absl::core_headers absl::log_globals absl::log_severity absl::log_internal_config absl::log_internal_flags absl::flags absl::flags_marshalling absl::strings absl::vlog_config_internal)
set(abseil_absl_log_flags_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_log_flags_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_log_flags_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_log_flags_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_log_flags_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_log_flags_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_log_flags_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_log_flags_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_log_flags_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_log_flags_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::flags_usage VARIABLES ############################################

set(abseil_absl_flags_usage_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_flags_usage_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_flags_usage_BIN_DIRS_RELEASE )
set(abseil_absl_flags_usage_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_flags_usage_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_flags_usage_RES_DIRS_RELEASE )
set(abseil_absl_flags_usage_DEFINITIONS_RELEASE )
set(abseil_absl_flags_usage_OBJECTS_RELEASE )
set(abseil_absl_flags_usage_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_flags_usage_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_flags_usage_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_flags_usage_LIBS_RELEASE absl_flags_usage)
set(abseil_absl_flags_usage_SYSTEM_LIBS_RELEASE )
set(abseil_absl_flags_usage_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_flags_usage_FRAMEWORKS_RELEASE )
set(abseil_absl_flags_usage_DEPENDENCIES_RELEASE absl::config absl::core_headers absl::flags_usage_internal absl::no_destructor absl::raw_logging_internal absl::strings absl::synchronization)
set(abseil_absl_flags_usage_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_flags_usage_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_flags_usage_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_flags_usage_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_flags_usage_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_flags_usage_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_flags_usage_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_flags_usage_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_flags_usage_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_flags_usage_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::log_internal_flags VARIABLES ############################################

set(abseil_absl_log_internal_flags_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_log_internal_flags_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_log_internal_flags_BIN_DIRS_RELEASE )
set(abseil_absl_log_internal_flags_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_log_internal_flags_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_log_internal_flags_RES_DIRS_RELEASE )
set(abseil_absl_log_internal_flags_DEFINITIONS_RELEASE )
set(abseil_absl_log_internal_flags_OBJECTS_RELEASE )
set(abseil_absl_log_internal_flags_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_log_internal_flags_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_log_internal_flags_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_log_internal_flags_LIBS_RELEASE )
set(abseil_absl_log_internal_flags_SYSTEM_LIBS_RELEASE )
set(abseil_absl_log_internal_flags_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_log_internal_flags_FRAMEWORKS_RELEASE )
set(abseil_absl_log_internal_flags_DEPENDENCIES_RELEASE absl::flags)
set(abseil_absl_log_internal_flags_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_log_internal_flags_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_log_internal_flags_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_log_internal_flags_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_log_internal_flags_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_log_internal_flags_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_log_internal_flags_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_log_internal_flags_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_log_internal_flags_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_log_internal_flags_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::flags_usage_internal VARIABLES ############################################

set(abseil_absl_flags_usage_internal_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_flags_usage_internal_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_flags_usage_internal_BIN_DIRS_RELEASE )
set(abseil_absl_flags_usage_internal_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_flags_usage_internal_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_flags_usage_internal_RES_DIRS_RELEASE )
set(abseil_absl_flags_usage_internal_DEFINITIONS_RELEASE )
set(abseil_absl_flags_usage_internal_OBJECTS_RELEASE )
set(abseil_absl_flags_usage_internal_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_flags_usage_internal_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_flags_usage_internal_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_flags_usage_internal_LIBS_RELEASE absl_flags_usage_internal)
set(abseil_absl_flags_usage_internal_SYSTEM_LIBS_RELEASE )
set(abseil_absl_flags_usage_internal_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_flags_usage_internal_FRAMEWORKS_RELEASE )
set(abseil_absl_flags_usage_internal_DEPENDENCIES_RELEASE absl::config absl::flags_config absl::flags absl::flags_commandlineflag absl::flags_internal absl::flags_path_util absl::flags_private_handle_accessor absl::flags_program_name absl::flags_reflection absl::strings absl::synchronization)
set(abseil_absl_flags_usage_internal_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_flags_usage_internal_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_flags_usage_internal_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_flags_usage_internal_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_flags_usage_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_flags_usage_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_flags_usage_internal_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_flags_usage_internal_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_flags_usage_internal_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_flags_usage_internal_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::hashtable_profiler VARIABLES ############################################

set(abseil_absl_hashtable_profiler_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_hashtable_profiler_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_hashtable_profiler_BIN_DIRS_RELEASE )
set(abseil_absl_hashtable_profiler_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_hashtable_profiler_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_hashtable_profiler_RES_DIRS_RELEASE )
set(abseil_absl_hashtable_profiler_DEFINITIONS_RELEASE )
set(abseil_absl_hashtable_profiler_OBJECTS_RELEASE )
set(abseil_absl_hashtable_profiler_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_hashtable_profiler_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_hashtable_profiler_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_hashtable_profiler_LIBS_RELEASE absl_hashtable_profiler)
set(abseil_absl_hashtable_profiler_SYSTEM_LIBS_RELEASE )
set(abseil_absl_hashtable_profiler_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_hashtable_profiler_FRAMEWORKS_RELEASE )
set(abseil_absl_hashtable_profiler_DEPENDENCIES_RELEASE absl::profile_builder absl::config absl::core_headers absl::strings absl::span absl::hashtablez_sampler)
set(abseil_absl_hashtable_profiler_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_hashtable_profiler_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_hashtable_profiler_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_hashtable_profiler_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_hashtable_profiler_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_hashtable_profiler_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_hashtable_profiler_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_hashtable_profiler_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_hashtable_profiler_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_hashtable_profiler_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::flags VARIABLES ############################################

set(abseil_absl_flags_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_flags_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_flags_BIN_DIRS_RELEASE )
set(abseil_absl_flags_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_flags_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_flags_RES_DIRS_RELEASE )
set(abseil_absl_flags_DEFINITIONS_RELEASE )
set(abseil_absl_flags_OBJECTS_RELEASE )
set(abseil_absl_flags_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_flags_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_flags_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_flags_LIBS_RELEASE )
set(abseil_absl_flags_SYSTEM_LIBS_RELEASE )
set(abseil_absl_flags_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_flags_FRAMEWORKS_RELEASE )
set(abseil_absl_flags_DEPENDENCIES_RELEASE absl::config absl::flags_commandlineflag absl::flags_config absl::flags_internal absl::flags_reflection absl::core_headers absl::nullability absl::strings)
set(abseil_absl_flags_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_flags_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_flags_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_flags_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_flags_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_flags_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_flags_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_flags_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_flags_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_flags_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::profile_builder VARIABLES ############################################

set(abseil_absl_profile_builder_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_profile_builder_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_profile_builder_BIN_DIRS_RELEASE )
set(abseil_absl_profile_builder_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_profile_builder_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_profile_builder_RES_DIRS_RELEASE )
set(abseil_absl_profile_builder_DEFINITIONS_RELEASE )
set(abseil_absl_profile_builder_OBJECTS_RELEASE )
set(abseil_absl_profile_builder_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_profile_builder_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_profile_builder_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_profile_builder_LIBS_RELEASE absl_profile_builder)
set(abseil_absl_profile_builder_SYSTEM_LIBS_RELEASE )
set(abseil_absl_profile_builder_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_profile_builder_FRAMEWORKS_RELEASE )
set(abseil_absl_profile_builder_DEPENDENCIES_RELEASE absl::config absl::core_headers absl::raw_logging_internal absl::flat_hash_map absl::btree absl::strings absl::str_format absl::span)
set(abseil_absl_profile_builder_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_profile_builder_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_profile_builder_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_profile_builder_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_profile_builder_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_profile_builder_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_profile_builder_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_profile_builder_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_profile_builder_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_profile_builder_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::log_streamer VARIABLES ############################################

set(abseil_absl_log_streamer_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_log_streamer_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_log_streamer_BIN_DIRS_RELEASE )
set(abseil_absl_log_streamer_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_log_streamer_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_log_streamer_RES_DIRS_RELEASE )
set(abseil_absl_log_streamer_DEFINITIONS_RELEASE )
set(abseil_absl_log_streamer_OBJECTS_RELEASE )
set(abseil_absl_log_streamer_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_log_streamer_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_log_streamer_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_log_streamer_LIBS_RELEASE )
set(abseil_absl_log_streamer_SYSTEM_LIBS_RELEASE )
set(abseil_absl_log_streamer_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_log_streamer_FRAMEWORKS_RELEASE )
set(abseil_absl_log_streamer_DEPENDENCIES_RELEASE absl::config absl::absl_log absl::log_severity absl::optional absl::strings absl::strings_internal absl::utility)
set(abseil_absl_log_streamer_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_log_streamer_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_log_streamer_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_log_streamer_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_log_streamer_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_log_streamer_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_log_streamer_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_log_streamer_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_log_streamer_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_log_streamer_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::die_if_null VARIABLES ############################################

set(abseil_absl_die_if_null_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_die_if_null_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_die_if_null_BIN_DIRS_RELEASE )
set(abseil_absl_die_if_null_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_die_if_null_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_die_if_null_RES_DIRS_RELEASE )
set(abseil_absl_die_if_null_DEFINITIONS_RELEASE )
set(abseil_absl_die_if_null_OBJECTS_RELEASE )
set(abseil_absl_die_if_null_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_die_if_null_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_die_if_null_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_die_if_null_LIBS_RELEASE absl_die_if_null)
set(abseil_absl_die_if_null_SYSTEM_LIBS_RELEASE )
set(abseil_absl_die_if_null_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_die_if_null_FRAMEWORKS_RELEASE )
set(abseil_absl_die_if_null_DEPENDENCIES_RELEASE absl::config absl::core_headers absl::log absl::nullability absl::nullability_traits_internal absl::strings)
set(abseil_absl_die_if_null_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_die_if_null_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_die_if_null_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_die_if_null_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_die_if_null_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_die_if_null_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_die_if_null_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_die_if_null_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_die_if_null_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_die_if_null_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::check VARIABLES ############################################

set(abseil_absl_check_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_check_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_check_BIN_DIRS_RELEASE )
set(abseil_absl_check_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_check_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_check_RES_DIRS_RELEASE )
set(abseil_absl_check_DEFINITIONS_RELEASE )
set(abseil_absl_check_OBJECTS_RELEASE )
set(abseil_absl_check_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_check_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_check_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_check_LIBS_RELEASE )
set(abseil_absl_check_SYSTEM_LIBS_RELEASE )
set(abseil_absl_check_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_check_FRAMEWORKS_RELEASE )
set(abseil_absl_check_DEPENDENCIES_RELEASE absl::log_internal_check_impl absl::core_headers absl::log_internal_check_op absl::log_internal_conditions absl::log_internal_message absl::log_internal_strip)
set(abseil_absl_check_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_check_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_check_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_check_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_check_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_check_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_check_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_check_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_check_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_check_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::absl_check VARIABLES ############################################

set(abseil_absl_absl_check_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_absl_check_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_absl_check_BIN_DIRS_RELEASE )
set(abseil_absl_absl_check_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_absl_check_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_absl_check_RES_DIRS_RELEASE )
set(abseil_absl_absl_check_DEFINITIONS_RELEASE )
set(abseil_absl_absl_check_OBJECTS_RELEASE )
set(abseil_absl_absl_check_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_absl_check_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_absl_check_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_absl_check_LIBS_RELEASE )
set(abseil_absl_absl_check_SYSTEM_LIBS_RELEASE )
set(abseil_absl_absl_check_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_absl_check_FRAMEWORKS_RELEASE )
set(abseil_absl_absl_check_DEPENDENCIES_RELEASE absl::log_internal_check_impl)
set(abseil_absl_absl_check_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_absl_check_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_absl_check_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_absl_check_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_absl_check_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_absl_check_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_absl_check_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_absl_check_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_absl_check_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_absl_check_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::flags_reflection VARIABLES ############################################

set(abseil_absl_flags_reflection_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_flags_reflection_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_flags_reflection_BIN_DIRS_RELEASE )
set(abseil_absl_flags_reflection_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_flags_reflection_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_flags_reflection_RES_DIRS_RELEASE )
set(abseil_absl_flags_reflection_DEFINITIONS_RELEASE )
set(abseil_absl_flags_reflection_OBJECTS_RELEASE )
set(abseil_absl_flags_reflection_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_flags_reflection_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_flags_reflection_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_flags_reflection_LIBS_RELEASE absl_flags_reflection)
set(abseil_absl_flags_reflection_SYSTEM_LIBS_RELEASE )
set(abseil_absl_flags_reflection_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_flags_reflection_FRAMEWORKS_RELEASE )
set(abseil_absl_flags_reflection_DEPENDENCIES_RELEASE absl::config absl::fast_type_id absl::flags_commandlineflag absl::flags_private_handle_accessor absl::flags_config absl::strings absl::synchronization absl::flat_hash_map absl::no_destructor)
set(abseil_absl_flags_reflection_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_flags_reflection_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_flags_reflection_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_flags_reflection_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_flags_reflection_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_flags_reflection_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_flags_reflection_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_flags_reflection_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_flags_reflection_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_flags_reflection_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::log VARIABLES ############################################

set(abseil_absl_log_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_log_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_log_BIN_DIRS_RELEASE )
set(abseil_absl_log_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_log_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_log_RES_DIRS_RELEASE )
set(abseil_absl_log_DEFINITIONS_RELEASE )
set(abseil_absl_log_OBJECTS_RELEASE )
set(abseil_absl_log_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_log_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_log_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_log_LIBS_RELEASE )
set(abseil_absl_log_SYSTEM_LIBS_RELEASE )
set(abseil_absl_log_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_log_FRAMEWORKS_RELEASE )
set(abseil_absl_log_DEPENDENCIES_RELEASE absl::log_internal_log_impl absl::vlog_is_on)
set(abseil_absl_log_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_log_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_log_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_log_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_log_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_log_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_log_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_log_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_log_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_log_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::absl_log VARIABLES ############################################

set(abseil_absl_absl_log_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_absl_log_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_absl_log_BIN_DIRS_RELEASE )
set(abseil_absl_absl_log_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_absl_log_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_absl_log_RES_DIRS_RELEASE )
set(abseil_absl_absl_log_DEFINITIONS_RELEASE )
set(abseil_absl_absl_log_OBJECTS_RELEASE )
set(abseil_absl_absl_log_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_absl_log_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_absl_log_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_absl_log_LIBS_RELEASE )
set(abseil_absl_absl_log_SYSTEM_LIBS_RELEASE )
set(abseil_absl_absl_log_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_absl_log_FRAMEWORKS_RELEASE )
set(abseil_absl_absl_log_DEPENDENCIES_RELEASE absl::log_internal_log_impl)
set(abseil_absl_absl_log_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_absl_log_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_absl_log_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_absl_log_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_absl_log_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_absl_log_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_absl_log_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_absl_log_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_absl_log_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_absl_log_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::log_internal_check_impl VARIABLES ############################################

set(abseil_absl_log_internal_check_impl_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_log_internal_check_impl_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_log_internal_check_impl_BIN_DIRS_RELEASE )
set(abseil_absl_log_internal_check_impl_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_log_internal_check_impl_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_log_internal_check_impl_RES_DIRS_RELEASE )
set(abseil_absl_log_internal_check_impl_DEFINITIONS_RELEASE )
set(abseil_absl_log_internal_check_impl_OBJECTS_RELEASE )
set(abseil_absl_log_internal_check_impl_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_log_internal_check_impl_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_log_internal_check_impl_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_log_internal_check_impl_LIBS_RELEASE )
set(abseil_absl_log_internal_check_impl_SYSTEM_LIBS_RELEASE )
set(abseil_absl_log_internal_check_impl_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_log_internal_check_impl_FRAMEWORKS_RELEASE )
set(abseil_absl_log_internal_check_impl_DEPENDENCIES_RELEASE absl::core_headers absl::log_internal_check_op absl::log_internal_conditions absl::log_internal_strip)
set(abseil_absl_log_internal_check_impl_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_log_internal_check_impl_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_log_internal_check_impl_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_log_internal_check_impl_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_log_internal_check_impl_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_log_internal_check_impl_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_log_internal_check_impl_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_log_internal_check_impl_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_log_internal_check_impl_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_log_internal_check_impl_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::linked_hash_map VARIABLES ############################################

set(abseil_absl_linked_hash_map_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_linked_hash_map_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_linked_hash_map_BIN_DIRS_RELEASE )
set(abseil_absl_linked_hash_map_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_linked_hash_map_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_linked_hash_map_RES_DIRS_RELEASE )
set(abseil_absl_linked_hash_map_DEFINITIONS_RELEASE )
set(abseil_absl_linked_hash_map_OBJECTS_RELEASE )
set(abseil_absl_linked_hash_map_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_linked_hash_map_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_linked_hash_map_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_linked_hash_map_LIBS_RELEASE )
set(abseil_absl_linked_hash_map_SYSTEM_LIBS_RELEASE )
set(abseil_absl_linked_hash_map_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_linked_hash_map_FRAMEWORKS_RELEASE )
set(abseil_absl_linked_hash_map_DEPENDENCIES_RELEASE absl::container_common absl::config absl::core_headers absl::flat_hash_set absl::throw_delegate)
set(abseil_absl_linked_hash_map_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_linked_hash_map_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_linked_hash_map_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_linked_hash_map_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_linked_hash_map_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_linked_hash_map_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_linked_hash_map_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_linked_hash_map_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_linked_hash_map_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_linked_hash_map_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::linked_hash_set VARIABLES ############################################

set(abseil_absl_linked_hash_set_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_linked_hash_set_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_linked_hash_set_BIN_DIRS_RELEASE )
set(abseil_absl_linked_hash_set_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_linked_hash_set_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_linked_hash_set_RES_DIRS_RELEASE )
set(abseil_absl_linked_hash_set_DEFINITIONS_RELEASE )
set(abseil_absl_linked_hash_set_OBJECTS_RELEASE )
set(abseil_absl_linked_hash_set_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_linked_hash_set_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_linked_hash_set_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_linked_hash_set_LIBS_RELEASE )
set(abseil_absl_linked_hash_set_SYSTEM_LIBS_RELEASE )
set(abseil_absl_linked_hash_set_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_linked_hash_set_FRAMEWORKS_RELEASE )
set(abseil_absl_linked_hash_set_DEPENDENCIES_RELEASE absl::container_common absl::config absl::core_headers absl::flat_hash_set)
set(abseil_absl_linked_hash_set_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_linked_hash_set_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_linked_hash_set_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_linked_hash_set_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_linked_hash_set_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_linked_hash_set_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_linked_hash_set_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_linked_hash_set_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_linked_hash_set_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_linked_hash_set_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::node_hash_map VARIABLES ############################################

set(abseil_absl_node_hash_map_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_node_hash_map_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_node_hash_map_BIN_DIRS_RELEASE )
set(abseil_absl_node_hash_map_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_node_hash_map_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_node_hash_map_RES_DIRS_RELEASE )
set(abseil_absl_node_hash_map_DEFINITIONS_RELEASE )
set(abseil_absl_node_hash_map_OBJECTS_RELEASE )
set(abseil_absl_node_hash_map_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_node_hash_map_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_node_hash_map_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_node_hash_map_LIBS_RELEASE )
set(abseil_absl_node_hash_map_SYSTEM_LIBS_RELEASE )
set(abseil_absl_node_hash_map_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_node_hash_map_FRAMEWORKS_RELEASE )
set(abseil_absl_node_hash_map_DEPENDENCIES_RELEASE absl::container_memory absl::core_headers absl::hash_container_defaults absl::node_slot_policy absl::raw_hash_map absl::algorithm_container absl::memory absl::type_traits)
set(abseil_absl_node_hash_map_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_node_hash_map_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_node_hash_map_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_node_hash_map_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_node_hash_map_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_node_hash_map_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_node_hash_map_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_node_hash_map_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_node_hash_map_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_node_hash_map_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::flat_hash_map VARIABLES ############################################

set(abseil_absl_flat_hash_map_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_flat_hash_map_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_flat_hash_map_BIN_DIRS_RELEASE )
set(abseil_absl_flat_hash_map_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_flat_hash_map_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_flat_hash_map_RES_DIRS_RELEASE )
set(abseil_absl_flat_hash_map_DEFINITIONS_RELEASE )
set(abseil_absl_flat_hash_map_OBJECTS_RELEASE )
set(abseil_absl_flat_hash_map_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_flat_hash_map_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_flat_hash_map_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_flat_hash_map_LIBS_RELEASE )
set(abseil_absl_flat_hash_map_SYSTEM_LIBS_RELEASE )
set(abseil_absl_flat_hash_map_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_flat_hash_map_FRAMEWORKS_RELEASE )
set(abseil_absl_flat_hash_map_DEPENDENCIES_RELEASE absl::container_memory absl::core_headers absl::hash_container_defaults absl::raw_hash_map absl::algorithm_container absl::type_traits)
set(abseil_absl_flat_hash_map_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_flat_hash_map_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_flat_hash_map_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_flat_hash_map_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_flat_hash_map_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_flat_hash_map_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_flat_hash_map_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_flat_hash_map_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_flat_hash_map_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_flat_hash_map_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::log_structured VARIABLES ############################################

set(abseil_absl_log_structured_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_log_structured_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_log_structured_BIN_DIRS_RELEASE )
set(abseil_absl_log_structured_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_log_structured_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_log_structured_RES_DIRS_RELEASE )
set(abseil_absl_log_structured_DEFINITIONS_RELEASE )
set(abseil_absl_log_structured_OBJECTS_RELEASE )
set(abseil_absl_log_structured_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_log_structured_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_log_structured_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_log_structured_LIBS_RELEASE )
set(abseil_absl_log_structured_SYSTEM_LIBS_RELEASE )
set(abseil_absl_log_structured_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_log_structured_FRAMEWORKS_RELEASE )
set(abseil_absl_log_structured_DEPENDENCIES_RELEASE absl::config absl::core_headers absl::log_internal_structured absl::strings)
set(abseil_absl_log_structured_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_log_structured_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_log_structured_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_log_structured_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_log_structured_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_log_structured_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_log_structured_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_log_structured_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_log_structured_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_log_structured_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::log_internal_log_impl VARIABLES ############################################

set(abseil_absl_log_internal_log_impl_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_log_internal_log_impl_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_log_internal_log_impl_BIN_DIRS_RELEASE )
set(abseil_absl_log_internal_log_impl_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_log_internal_log_impl_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_log_internal_log_impl_RES_DIRS_RELEASE )
set(abseil_absl_log_internal_log_impl_DEFINITIONS_RELEASE )
set(abseil_absl_log_internal_log_impl_OBJECTS_RELEASE )
set(abseil_absl_log_internal_log_impl_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_log_internal_log_impl_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_log_internal_log_impl_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_log_internal_log_impl_LIBS_RELEASE )
set(abseil_absl_log_internal_log_impl_SYSTEM_LIBS_RELEASE )
set(abseil_absl_log_internal_log_impl_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_log_internal_log_impl_FRAMEWORKS_RELEASE )
set(abseil_absl_log_internal_log_impl_DEPENDENCIES_RELEASE absl::log_internal_conditions absl::log_internal_message absl::log_internal_strip absl::absl_vlog_is_on)
set(abseil_absl_log_internal_log_impl_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_log_internal_log_impl_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_log_internal_log_impl_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_log_internal_log_impl_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_log_internal_log_impl_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_log_internal_log_impl_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_log_internal_log_impl_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_log_internal_log_impl_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_log_internal_log_impl_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_log_internal_log_impl_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::log_internal_check_op VARIABLES ############################################

set(abseil_absl_log_internal_check_op_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_log_internal_check_op_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_log_internal_check_op_BIN_DIRS_RELEASE )
set(abseil_absl_log_internal_check_op_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_log_internal_check_op_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_log_internal_check_op_RES_DIRS_RELEASE )
set(abseil_absl_log_internal_check_op_DEFINITIONS_RELEASE )
set(abseil_absl_log_internal_check_op_OBJECTS_RELEASE )
set(abseil_absl_log_internal_check_op_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_log_internal_check_op_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_log_internal_check_op_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_log_internal_check_op_LIBS_RELEASE absl_log_internal_check_op)
set(abseil_absl_log_internal_check_op_SYSTEM_LIBS_RELEASE )
set(abseil_absl_log_internal_check_op_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_log_internal_check_op_FRAMEWORKS_RELEASE )
set(abseil_absl_log_internal_check_op_DEPENDENCIES_RELEASE absl::base absl::config absl::core_headers absl::has_ostream_operator absl::leak_check absl::log_internal_nullguard absl::log_internal_nullstream absl::log_internal_strip absl::nullability absl::strings)
set(abseil_absl_log_internal_check_op_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_log_internal_check_op_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_log_internal_check_op_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_log_internal_check_op_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_log_internal_check_op_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_log_internal_check_op_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_log_internal_check_op_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_log_internal_check_op_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_log_internal_check_op_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_log_internal_check_op_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::raw_hash_map VARIABLES ############################################

set(abseil_absl_raw_hash_map_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_raw_hash_map_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_raw_hash_map_BIN_DIRS_RELEASE )
set(abseil_absl_raw_hash_map_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_raw_hash_map_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_raw_hash_map_RES_DIRS_RELEASE )
set(abseil_absl_raw_hash_map_DEFINITIONS_RELEASE )
set(abseil_absl_raw_hash_map_OBJECTS_RELEASE )
set(abseil_absl_raw_hash_map_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_raw_hash_map_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_raw_hash_map_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_raw_hash_map_LIBS_RELEASE )
set(abseil_absl_raw_hash_map_SYSTEM_LIBS_RELEASE )
set(abseil_absl_raw_hash_map_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_raw_hash_map_FRAMEWORKS_RELEASE )
set(abseil_absl_raw_hash_map_DEPENDENCIES_RELEASE absl::config absl::common_policy_traits absl::container_memory absl::core_headers absl::raw_hash_set absl::type_traits absl::throw_delegate)
set(abseil_absl_raw_hash_map_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_raw_hash_map_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_raw_hash_map_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_raw_hash_map_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_raw_hash_map_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_raw_hash_map_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_raw_hash_map_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_raw_hash_map_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_raw_hash_map_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_raw_hash_map_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::node_hash_set VARIABLES ############################################

set(abseil_absl_node_hash_set_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_node_hash_set_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_node_hash_set_BIN_DIRS_RELEASE )
set(abseil_absl_node_hash_set_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_node_hash_set_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_node_hash_set_RES_DIRS_RELEASE )
set(abseil_absl_node_hash_set_DEFINITIONS_RELEASE )
set(abseil_absl_node_hash_set_OBJECTS_RELEASE )
set(abseil_absl_node_hash_set_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_node_hash_set_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_node_hash_set_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_node_hash_set_LIBS_RELEASE )
set(abseil_absl_node_hash_set_SYSTEM_LIBS_RELEASE )
set(abseil_absl_node_hash_set_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_node_hash_set_FRAMEWORKS_RELEASE )
set(abseil_absl_node_hash_set_DEPENDENCIES_RELEASE absl::container_memory absl::core_headers absl::hash_container_defaults absl::node_slot_policy absl::raw_hash_set absl::algorithm_container absl::memory absl::type_traits)
set(abseil_absl_node_hash_set_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_node_hash_set_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_node_hash_set_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_node_hash_set_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_node_hash_set_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_node_hash_set_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_node_hash_set_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_node_hash_set_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_node_hash_set_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_node_hash_set_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::flat_hash_set VARIABLES ############################################

set(abseil_absl_flat_hash_set_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_flat_hash_set_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_flat_hash_set_BIN_DIRS_RELEASE )
set(abseil_absl_flat_hash_set_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_flat_hash_set_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_flat_hash_set_RES_DIRS_RELEASE )
set(abseil_absl_flat_hash_set_DEFINITIONS_RELEASE )
set(abseil_absl_flat_hash_set_OBJECTS_RELEASE )
set(abseil_absl_flat_hash_set_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_flat_hash_set_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_flat_hash_set_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_flat_hash_set_LIBS_RELEASE )
set(abseil_absl_flat_hash_set_SYSTEM_LIBS_RELEASE )
set(abseil_absl_flat_hash_set_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_flat_hash_set_FRAMEWORKS_RELEASE )
set(abseil_absl_flat_hash_set_DEPENDENCIES_RELEASE absl::container_memory absl::hash_container_defaults absl::raw_hash_set absl::algorithm_container absl::core_headers absl::memory absl::type_traits)
set(abseil_absl_flat_hash_set_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_flat_hash_set_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_flat_hash_set_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_flat_hash_set_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_flat_hash_set_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_flat_hash_set_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_flat_hash_set_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_flat_hash_set_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_flat_hash_set_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_flat_hash_set_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::statusor VARIABLES ############################################

set(abseil_absl_statusor_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_statusor_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_statusor_BIN_DIRS_RELEASE )
set(abseil_absl_statusor_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_statusor_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_statusor_RES_DIRS_RELEASE )
set(abseil_absl_statusor_DEFINITIONS_RELEASE )
set(abseil_absl_statusor_OBJECTS_RELEASE )
set(abseil_absl_statusor_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_statusor_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_statusor_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_statusor_LIBS_RELEASE absl_statusor)
set(abseil_absl_statusor_SYSTEM_LIBS_RELEASE )
set(abseil_absl_statusor_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_statusor_FRAMEWORKS_RELEASE )
set(abseil_absl_statusor_DEPENDENCIES_RELEASE absl::base absl::config absl::core_headers absl::has_ostream_operator absl::nullability absl::raw_logging_internal absl::status absl::str_format absl::strings absl::type_traits absl::utility absl::variant)
set(abseil_absl_statusor_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_statusor_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_statusor_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_statusor_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_statusor_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_statusor_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_statusor_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_statusor_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_statusor_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_statusor_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::log_internal_structured VARIABLES ############################################

set(abseil_absl_log_internal_structured_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_log_internal_structured_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_log_internal_structured_BIN_DIRS_RELEASE )
set(abseil_absl_log_internal_structured_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_log_internal_structured_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_log_internal_structured_RES_DIRS_RELEASE )
set(abseil_absl_log_internal_structured_DEFINITIONS_RELEASE )
set(abseil_absl_log_internal_structured_OBJECTS_RELEASE )
set(abseil_absl_log_internal_structured_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_log_internal_structured_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_log_internal_structured_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_log_internal_structured_LIBS_RELEASE )
set(abseil_absl_log_internal_structured_SYSTEM_LIBS_RELEASE )
set(abseil_absl_log_internal_structured_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_log_internal_structured_FRAMEWORKS_RELEASE )
set(abseil_absl_log_internal_structured_DEPENDENCIES_RELEASE absl::any_invocable absl::config absl::core_headers absl::log_internal_message absl::log_internal_structured_proto absl::strings)
set(abseil_absl_log_internal_structured_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_log_internal_structured_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_log_internal_structured_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_log_internal_structured_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_log_internal_structured_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_log_internal_structured_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_log_internal_structured_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_log_internal_structured_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_log_internal_structured_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_log_internal_structured_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::log_internal_strip VARIABLES ############################################

set(abseil_absl_log_internal_strip_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_log_internal_strip_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_log_internal_strip_BIN_DIRS_RELEASE )
set(abseil_absl_log_internal_strip_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_log_internal_strip_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_log_internal_strip_RES_DIRS_RELEASE )
set(abseil_absl_log_internal_strip_DEFINITIONS_RELEASE )
set(abseil_absl_log_internal_strip_OBJECTS_RELEASE )
set(abseil_absl_log_internal_strip_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_log_internal_strip_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_log_internal_strip_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_log_internal_strip_LIBS_RELEASE )
set(abseil_absl_log_internal_strip_SYSTEM_LIBS_RELEASE )
set(abseil_absl_log_internal_strip_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_log_internal_strip_FRAMEWORKS_RELEASE )
set(abseil_absl_log_internal_strip_DEPENDENCIES_RELEASE absl::core_headers absl::log_internal_message absl::log_internal_nullstream absl::log_severity)
set(abseil_absl_log_internal_strip_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_log_internal_strip_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_log_internal_strip_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_log_internal_strip_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_log_internal_strip_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_log_internal_strip_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_log_internal_strip_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_log_internal_strip_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_log_internal_strip_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_log_internal_strip_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::raw_hash_set VARIABLES ############################################

set(abseil_absl_raw_hash_set_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_raw_hash_set_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_raw_hash_set_BIN_DIRS_RELEASE )
set(abseil_absl_raw_hash_set_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_raw_hash_set_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_raw_hash_set_RES_DIRS_RELEASE )
set(abseil_absl_raw_hash_set_DEFINITIONS_RELEASE )
set(abseil_absl_raw_hash_set_OBJECTS_RELEASE )
set(abseil_absl_raw_hash_set_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_raw_hash_set_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_raw_hash_set_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_raw_hash_set_LIBS_RELEASE absl_raw_hash_set)
set(abseil_absl_raw_hash_set_SYSTEM_LIBS_RELEASE )
set(abseil_absl_raw_hash_set_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_raw_hash_set_FRAMEWORKS_RELEASE )
set(abseil_absl_raw_hash_set_DEPENDENCIES_RELEASE absl::base absl::bits absl::common_policy_traits absl::compressed_tuple absl::config absl::container_common absl::container_memory absl::core_headers absl::dynamic_annotations absl::endian absl::function_ref absl::hash absl::hash_function_defaults absl::hash_policy_traits absl::hashtable_control_bytes absl::hashtable_debug_hooks absl::hashtablez_sampler absl::iterator_traits_internal absl::memory absl::meta absl::optional absl::prefetch absl::raw_logging_internal absl::utility absl::weakly_mixed_integer)
set(abseil_absl_raw_hash_set_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_raw_hash_set_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_raw_hash_set_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_raw_hash_set_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_raw_hash_set_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_raw_hash_set_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_raw_hash_set_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_raw_hash_set_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_raw_hash_set_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_raw_hash_set_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::hash_container_defaults VARIABLES ############################################

set(abseil_absl_hash_container_defaults_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_hash_container_defaults_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_hash_container_defaults_BIN_DIRS_RELEASE )
set(abseil_absl_hash_container_defaults_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_hash_container_defaults_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_hash_container_defaults_RES_DIRS_RELEASE )
set(abseil_absl_hash_container_defaults_DEFINITIONS_RELEASE )
set(abseil_absl_hash_container_defaults_OBJECTS_RELEASE )
set(abseil_absl_hash_container_defaults_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_hash_container_defaults_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_hash_container_defaults_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_hash_container_defaults_LIBS_RELEASE )
set(abseil_absl_hash_container_defaults_SYSTEM_LIBS_RELEASE )
set(abseil_absl_hash_container_defaults_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_hash_container_defaults_FRAMEWORKS_RELEASE )
set(abseil_absl_hash_container_defaults_DEPENDENCIES_RELEASE absl::config absl::hash_function_defaults)
set(abseil_absl_hash_container_defaults_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_hash_container_defaults_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_hash_container_defaults_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_hash_container_defaults_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_hash_container_defaults_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_hash_container_defaults_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_hash_container_defaults_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_hash_container_defaults_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_hash_container_defaults_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_hash_container_defaults_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::status VARIABLES ############################################

set(abseil_absl_status_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_status_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_status_BIN_DIRS_RELEASE )
set(abseil_absl_status_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_status_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_status_RES_DIRS_RELEASE )
set(abseil_absl_status_DEFINITIONS_RELEASE )
set(abseil_absl_status_OBJECTS_RELEASE )
set(abseil_absl_status_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_status_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_status_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_status_LIBS_RELEASE absl_status)
set(abseil_absl_status_SYSTEM_LIBS_RELEASE )
set(abseil_absl_status_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_status_FRAMEWORKS_RELEASE )
set(abseil_absl_status_DEPENDENCIES_RELEASE absl::atomic_hook absl::config absl::cord absl::core_headers absl::function_ref absl::inlined_vector absl::leak_check absl::memory absl::no_destructor absl::nullability absl::optional absl::raw_logging_internal absl::span absl::stacktrace absl::str_format absl::strerror absl::strings absl::symbolize)
set(abseil_absl_status_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_status_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_status_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_status_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_status_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_status_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_status_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_status_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_status_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_status_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::log_internal_message VARIABLES ############################################

set(abseil_absl_log_internal_message_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_log_internal_message_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_log_internal_message_BIN_DIRS_RELEASE )
set(abseil_absl_log_internal_message_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_log_internal_message_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_log_internal_message_RES_DIRS_RELEASE )
set(abseil_absl_log_internal_message_DEFINITIONS_RELEASE )
set(abseil_absl_log_internal_message_OBJECTS_RELEASE )
set(abseil_absl_log_internal_message_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_log_internal_message_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_log_internal_message_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_log_internal_message_LIBS_RELEASE absl_log_internal_message)
set(abseil_absl_log_internal_message_SYSTEM_LIBS_RELEASE )
set(abseil_absl_log_internal_message_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_log_internal_message_FRAMEWORKS_RELEASE )
set(abseil_absl_log_internal_message_DEPENDENCIES_RELEASE absl::base absl::config absl::core_headers absl::errno_saver absl::examine_stack absl::inlined_vector absl::log_internal_append_truncated absl::log_internal_format absl::log_internal_globals absl::log_internal_proto absl::log_internal_log_sink_set absl::log_internal_nullguard absl::log_internal_structured_proto absl::log_globals absl::log_entry absl::log_severity absl::log_sink absl::log_sink_registry absl::memory absl::nullability absl::raw_logging_internal absl::span absl::strerror absl::strings absl::strings_internal absl::time)
set(abseil_absl_log_internal_message_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_log_internal_message_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_log_internal_message_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_log_internal_message_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_log_internal_message_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_log_internal_message_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_log_internal_message_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_log_internal_message_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_log_internal_message_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_log_internal_message_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::hash_function_defaults VARIABLES ############################################

set(abseil_absl_hash_function_defaults_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_hash_function_defaults_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_hash_function_defaults_BIN_DIRS_RELEASE )
set(abseil_absl_hash_function_defaults_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_hash_function_defaults_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_hash_function_defaults_RES_DIRS_RELEASE )
set(abseil_absl_hash_function_defaults_DEFINITIONS_RELEASE )
set(abseil_absl_hash_function_defaults_OBJECTS_RELEASE )
set(abseil_absl_hash_function_defaults_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_hash_function_defaults_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_hash_function_defaults_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_hash_function_defaults_LIBS_RELEASE )
set(abseil_absl_hash_function_defaults_SYSTEM_LIBS_RELEASE )
set(abseil_absl_hash_function_defaults_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_hash_function_defaults_FRAMEWORKS_RELEASE )
set(abseil_absl_hash_function_defaults_DEPENDENCIES_RELEASE absl::config absl::container_common absl::cord absl::hash absl::strings absl::type_traits)
set(abseil_absl_hash_function_defaults_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_hash_function_defaults_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_hash_function_defaults_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_hash_function_defaults_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_hash_function_defaults_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_hash_function_defaults_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_hash_function_defaults_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_hash_function_defaults_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_hash_function_defaults_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_hash_function_defaults_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::btree VARIABLES ############################################

set(abseil_absl_btree_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_btree_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_btree_BIN_DIRS_RELEASE )
set(abseil_absl_btree_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_btree_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_btree_RES_DIRS_RELEASE )
set(abseil_absl_btree_DEFINITIONS_RELEASE )
set(abseil_absl_btree_OBJECTS_RELEASE )
set(abseil_absl_btree_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_btree_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_btree_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_btree_LIBS_RELEASE )
set(abseil_absl_btree_SYSTEM_LIBS_RELEASE )
set(abseil_absl_btree_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_btree_FRAMEWORKS_RELEASE )
set(abseil_absl_btree_DEPENDENCIES_RELEASE absl::common_policy_traits absl::compare absl::compressed_tuple absl::config absl::container_common absl::container_memory absl::cord absl::core_headers absl::layout absl::memory absl::raw_logging_internal absl::strings absl::throw_delegate absl::type_traits absl::weakly_mixed_integer)
set(abseil_absl_btree_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_btree_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_btree_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_btree_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_btree_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_btree_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_btree_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_btree_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_btree_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_btree_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::cord VARIABLES ############################################

set(abseil_absl_cord_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_cord_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_cord_BIN_DIRS_RELEASE )
set(abseil_absl_cord_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_cord_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_cord_RES_DIRS_RELEASE )
set(abseil_absl_cord_DEFINITIONS_RELEASE )
set(abseil_absl_cord_OBJECTS_RELEASE )
set(abseil_absl_cord_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_cord_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_cord_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_cord_LIBS_RELEASE absl_cord)
set(abseil_absl_cord_SYSTEM_LIBS_RELEASE )
set(abseil_absl_cord_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_cord_FRAMEWORKS_RELEASE )
set(abseil_absl_cord_DEPENDENCIES_RELEASE absl::base absl::config absl::cord_internal absl::cordz_functions absl::cordz_info absl::cordz_update_scope absl::cordz_update_tracker absl::core_headers absl::crc32c absl::crc_cord_state absl::endian absl::function_ref absl::inlined_vector absl::nullability absl::optional absl::raw_logging_internal absl::span absl::strings absl::strings_append_and_overwrite absl::strings_resize_and_overwrite absl::type_traits absl::weakly_mixed_integer)
set(abseil_absl_cord_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_cord_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_cord_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_cord_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_cord_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_cord_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_cord_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_cord_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_cord_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_cord_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::log_sink_registry VARIABLES ############################################

set(abseil_absl_log_sink_registry_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_log_sink_registry_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_log_sink_registry_BIN_DIRS_RELEASE )
set(abseil_absl_log_sink_registry_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_log_sink_registry_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_log_sink_registry_RES_DIRS_RELEASE )
set(abseil_absl_log_sink_registry_DEFINITIONS_RELEASE )
set(abseil_absl_log_sink_registry_OBJECTS_RELEASE )
set(abseil_absl_log_sink_registry_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_log_sink_registry_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_log_sink_registry_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_log_sink_registry_LIBS_RELEASE )
set(abseil_absl_log_sink_registry_SYSTEM_LIBS_RELEASE )
set(abseil_absl_log_sink_registry_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_log_sink_registry_FRAMEWORKS_RELEASE )
set(abseil_absl_log_sink_registry_DEPENDENCIES_RELEASE absl::config absl::log_sink absl::log_internal_log_sink_set absl::nullability)
set(abseil_absl_log_sink_registry_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_log_sink_registry_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_log_sink_registry_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_log_sink_registry_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_log_sink_registry_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_log_sink_registry_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_log_sink_registry_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_log_sink_registry_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_log_sink_registry_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_log_sink_registry_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::cordz_update_scope VARIABLES ############################################

set(abseil_absl_cordz_update_scope_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_cordz_update_scope_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_cordz_update_scope_BIN_DIRS_RELEASE )
set(abseil_absl_cordz_update_scope_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_cordz_update_scope_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_cordz_update_scope_RES_DIRS_RELEASE )
set(abseil_absl_cordz_update_scope_DEFINITIONS_RELEASE )
set(abseil_absl_cordz_update_scope_OBJECTS_RELEASE )
set(abseil_absl_cordz_update_scope_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_cordz_update_scope_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_cordz_update_scope_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_cordz_update_scope_LIBS_RELEASE )
set(abseil_absl_cordz_update_scope_SYSTEM_LIBS_RELEASE )
set(abseil_absl_cordz_update_scope_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_cordz_update_scope_FRAMEWORKS_RELEASE )
set(abseil_absl_cordz_update_scope_DEPENDENCIES_RELEASE absl::config absl::cord_internal absl::cordz_info absl::cordz_update_tracker absl::core_headers)
set(abseil_absl_cordz_update_scope_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_cordz_update_scope_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_cordz_update_scope_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_cordz_update_scope_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_cordz_update_scope_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_cordz_update_scope_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_cordz_update_scope_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_cordz_update_scope_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_cordz_update_scope_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_cordz_update_scope_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::cordz_sample_token VARIABLES ############################################

set(abseil_absl_cordz_sample_token_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_cordz_sample_token_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_cordz_sample_token_BIN_DIRS_RELEASE )
set(abseil_absl_cordz_sample_token_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_cordz_sample_token_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_cordz_sample_token_RES_DIRS_RELEASE )
set(abseil_absl_cordz_sample_token_DEFINITIONS_RELEASE )
set(abseil_absl_cordz_sample_token_OBJECTS_RELEASE )
set(abseil_absl_cordz_sample_token_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_cordz_sample_token_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_cordz_sample_token_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_cordz_sample_token_LIBS_RELEASE absl_cordz_sample_token)
set(abseil_absl_cordz_sample_token_SYSTEM_LIBS_RELEASE )
set(abseil_absl_cordz_sample_token_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_cordz_sample_token_FRAMEWORKS_RELEASE )
set(abseil_absl_cordz_sample_token_DEPENDENCIES_RELEASE absl::config absl::cordz_handle absl::cordz_info)
set(abseil_absl_cordz_sample_token_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_cordz_sample_token_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_cordz_sample_token_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_cordz_sample_token_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_cordz_sample_token_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_cordz_sample_token_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_cordz_sample_token_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_cordz_sample_token_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_cordz_sample_token_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_cordz_sample_token_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::random_random VARIABLES ############################################

set(abseil_absl_random_random_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_random_random_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_random_random_BIN_DIRS_RELEASE )
set(abseil_absl_random_random_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_random_random_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_random_random_RES_DIRS_RELEASE )
set(abseil_absl_random_random_DEFINITIONS_RELEASE )
set(abseil_absl_random_random_OBJECTS_RELEASE )
set(abseil_absl_random_random_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_random_random_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_random_random_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_random_random_LIBS_RELEASE )
set(abseil_absl_random_random_SYSTEM_LIBS_RELEASE )
set(abseil_absl_random_random_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_random_random_FRAMEWORKS_RELEASE )
set(abseil_absl_random_random_DEPENDENCIES_RELEASE absl::config absl::random_distributions absl::random_internal_nonsecure_base absl::random_internal_pcg_engine absl::random_internal_randen_engine absl::random_seed_sequences)
set(abseil_absl_random_random_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_random_random_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_random_random_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_random_random_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_random_random_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_random_random_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_random_random_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_random_random_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_random_random_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_random_random_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::vlog_is_on VARIABLES ############################################

set(abseil_absl_vlog_is_on_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_vlog_is_on_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_vlog_is_on_BIN_DIRS_RELEASE )
set(abseil_absl_vlog_is_on_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_vlog_is_on_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_vlog_is_on_RES_DIRS_RELEASE )
set(abseil_absl_vlog_is_on_DEFINITIONS_RELEASE )
set(abseil_absl_vlog_is_on_OBJECTS_RELEASE )
set(abseil_absl_vlog_is_on_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_vlog_is_on_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_vlog_is_on_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_vlog_is_on_LIBS_RELEASE )
set(abseil_absl_vlog_is_on_SYSTEM_LIBS_RELEASE )
set(abseil_absl_vlog_is_on_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_vlog_is_on_FRAMEWORKS_RELEASE )
set(abseil_absl_vlog_is_on_DEPENDENCIES_RELEASE absl::absl_vlog_is_on)
set(abseil_absl_vlog_is_on_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_vlog_is_on_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_vlog_is_on_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_vlog_is_on_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_vlog_is_on_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_vlog_is_on_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_vlog_is_on_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_vlog_is_on_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_vlog_is_on_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_vlog_is_on_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::log_initialize VARIABLES ############################################

set(abseil_absl_log_initialize_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_log_initialize_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_log_initialize_BIN_DIRS_RELEASE )
set(abseil_absl_log_initialize_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_log_initialize_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_log_initialize_RES_DIRS_RELEASE )
set(abseil_absl_log_initialize_DEFINITIONS_RELEASE )
set(abseil_absl_log_initialize_OBJECTS_RELEASE )
set(abseil_absl_log_initialize_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_log_initialize_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_log_initialize_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_log_initialize_LIBS_RELEASE absl_log_initialize)
set(abseil_absl_log_initialize_SYSTEM_LIBS_RELEASE )
set(abseil_absl_log_initialize_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_log_initialize_FRAMEWORKS_RELEASE )
set(abseil_absl_log_initialize_DEPENDENCIES_RELEASE absl::config absl::log_globals absl::log_internal_globals absl::time)
set(abseil_absl_log_initialize_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_log_initialize_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_log_initialize_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_log_initialize_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_log_initialize_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_log_initialize_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_log_initialize_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_log_initialize_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_log_initialize_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_log_initialize_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::log_internal_log_sink_set VARIABLES ############################################

set(abseil_absl_log_internal_log_sink_set_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_log_internal_log_sink_set_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_log_internal_log_sink_set_BIN_DIRS_RELEASE )
set(abseil_absl_log_internal_log_sink_set_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_log_internal_log_sink_set_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_log_internal_log_sink_set_RES_DIRS_RELEASE )
set(abseil_absl_log_internal_log_sink_set_DEFINITIONS_RELEASE )
set(abseil_absl_log_internal_log_sink_set_OBJECTS_RELEASE )
set(abseil_absl_log_internal_log_sink_set_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_log_internal_log_sink_set_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_log_internal_log_sink_set_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_log_internal_log_sink_set_LIBS_RELEASE absl_log_internal_log_sink_set)
set(abseil_absl_log_internal_log_sink_set_SYSTEM_LIBS_RELEASE )
set(abseil_absl_log_internal_log_sink_set_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_log_internal_log_sink_set_FRAMEWORKS_RELEASE )
set(abseil_absl_log_internal_log_sink_set_DEPENDENCIES_RELEASE absl::base absl::cleanup absl::config absl::core_headers absl::log_internal_config absl::log_internal_globals absl::log_globals absl::log_entry absl::log_severity absl::log_sink absl::no_destructor absl::raw_logging_internal absl::synchronization absl::span absl::strings)
set(abseil_absl_log_internal_log_sink_set_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_log_internal_log_sink_set_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_log_internal_log_sink_set_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_log_internal_log_sink_set_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_log_internal_log_sink_set_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_log_internal_log_sink_set_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_log_internal_log_sink_set_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_log_internal_log_sink_set_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_log_internal_log_sink_set_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_log_internal_log_sink_set_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::flags_internal VARIABLES ############################################

set(abseil_absl_flags_internal_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_flags_internal_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_flags_internal_BIN_DIRS_RELEASE )
set(abseil_absl_flags_internal_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_flags_internal_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_flags_internal_RES_DIRS_RELEASE )
set(abseil_absl_flags_internal_DEFINITIONS_RELEASE )
set(abseil_absl_flags_internal_OBJECTS_RELEASE )
set(abseil_absl_flags_internal_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_flags_internal_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_flags_internal_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_flags_internal_LIBS_RELEASE absl_flags_internal)
set(abseil_absl_flags_internal_SYSTEM_LIBS_RELEASE )
set(abseil_absl_flags_internal_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_flags_internal_FRAMEWORKS_RELEASE )
set(abseil_absl_flags_internal_DEPENDENCIES_RELEASE absl::base absl::config absl::fast_type_id absl::flags_commandlineflag absl::flags_commandlineflag_internal absl::flags_config absl::flags_marshalling absl::no_destructor absl::synchronization absl::meta absl::utility)
set(abseil_absl_flags_internal_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_flags_internal_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_flags_internal_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_flags_internal_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_flags_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_flags_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_flags_internal_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_flags_internal_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_flags_internal_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_flags_internal_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::cordz_info VARIABLES ############################################

set(abseil_absl_cordz_info_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_cordz_info_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_cordz_info_BIN_DIRS_RELEASE )
set(abseil_absl_cordz_info_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_cordz_info_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_cordz_info_RES_DIRS_RELEASE )
set(abseil_absl_cordz_info_DEFINITIONS_RELEASE )
set(abseil_absl_cordz_info_OBJECTS_RELEASE )
set(abseil_absl_cordz_info_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_cordz_info_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_cordz_info_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_cordz_info_LIBS_RELEASE absl_cordz_info)
set(abseil_absl_cordz_info_SYSTEM_LIBS_RELEASE )
set(abseil_absl_cordz_info_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_cordz_info_FRAMEWORKS_RELEASE )
set(abseil_absl_cordz_info_DEPENDENCIES_RELEASE absl::base absl::config absl::cord_internal absl::cordz_functions absl::cordz_handle absl::cordz_statistics absl::cordz_update_tracker absl::core_headers absl::no_destructor absl::inlined_vector absl::span absl::raw_logging_internal absl::stacktrace absl::synchronization absl::time)
set(abseil_absl_cordz_info_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_cordz_info_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_cordz_info_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_cordz_info_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_cordz_info_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_cordz_info_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_cordz_info_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_cordz_info_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_cordz_info_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_cordz_info_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::random_internal_nonsecure_base VARIABLES ############################################

set(abseil_absl_random_internal_nonsecure_base_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_random_internal_nonsecure_base_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_random_internal_nonsecure_base_BIN_DIRS_RELEASE )
set(abseil_absl_random_internal_nonsecure_base_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_random_internal_nonsecure_base_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_random_internal_nonsecure_base_RES_DIRS_RELEASE )
set(abseil_absl_random_internal_nonsecure_base_DEFINITIONS_RELEASE )
set(abseil_absl_random_internal_nonsecure_base_OBJECTS_RELEASE )
set(abseil_absl_random_internal_nonsecure_base_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_random_internal_nonsecure_base_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_random_internal_nonsecure_base_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_random_internal_nonsecure_base_LIBS_RELEASE )
set(abseil_absl_random_internal_nonsecure_base_SYSTEM_LIBS_RELEASE )
set(abseil_absl_random_internal_nonsecure_base_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_random_internal_nonsecure_base_FRAMEWORKS_RELEASE )
set(abseil_absl_random_internal_nonsecure_base_DEPENDENCIES_RELEASE absl::config absl::inlined_vector absl::random_internal_entropy_pool absl::random_internal_salted_seed_seq absl::random_internal_seed_material absl::type_traits)
set(abseil_absl_random_internal_nonsecure_base_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_random_internal_nonsecure_base_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_random_internal_nonsecure_base_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_random_internal_nonsecure_base_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_random_internal_nonsecure_base_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_random_internal_nonsecure_base_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_random_internal_nonsecure_base_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_random_internal_nonsecure_base_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_random_internal_nonsecure_base_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_random_internal_nonsecure_base_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::random_seed_sequences VARIABLES ############################################

set(abseil_absl_random_seed_sequences_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_random_seed_sequences_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_random_seed_sequences_BIN_DIRS_RELEASE )
set(abseil_absl_random_seed_sequences_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_random_seed_sequences_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_random_seed_sequences_RES_DIRS_RELEASE )
set(abseil_absl_random_seed_sequences_DEFINITIONS_RELEASE )
set(abseil_absl_random_seed_sequences_OBJECTS_RELEASE )
set(abseil_absl_random_seed_sequences_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_random_seed_sequences_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_random_seed_sequences_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_random_seed_sequences_LIBS_RELEASE absl_random_seed_sequences)
set(abseil_absl_random_seed_sequences_SYSTEM_LIBS_RELEASE )
set(abseil_absl_random_seed_sequences_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_random_seed_sequences_FRAMEWORKS_RELEASE )
set(abseil_absl_random_seed_sequences_DEPENDENCIES_RELEASE absl::config absl::inlined_vector absl::nullability absl::random_internal_entropy_pool absl::random_internal_salted_seed_seq absl::random_internal_seed_material absl::random_seed_gen_exception absl::span absl::string_view)
set(abseil_absl_random_seed_sequences_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_random_seed_sequences_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_random_seed_sequences_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_random_seed_sequences_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_random_seed_sequences_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_random_seed_sequences_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_random_seed_sequences_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_random_seed_sequences_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_random_seed_sequences_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_random_seed_sequences_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::absl_vlog_is_on VARIABLES ############################################

set(abseil_absl_absl_vlog_is_on_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_absl_vlog_is_on_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_absl_vlog_is_on_BIN_DIRS_RELEASE )
set(abseil_absl_absl_vlog_is_on_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_absl_vlog_is_on_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_absl_vlog_is_on_RES_DIRS_RELEASE )
set(abseil_absl_absl_vlog_is_on_DEFINITIONS_RELEASE )
set(abseil_absl_absl_vlog_is_on_OBJECTS_RELEASE )
set(abseil_absl_absl_vlog_is_on_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_absl_vlog_is_on_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_absl_vlog_is_on_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_absl_vlog_is_on_LIBS_RELEASE )
set(abseil_absl_absl_vlog_is_on_SYSTEM_LIBS_RELEASE )
set(abseil_absl_absl_vlog_is_on_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_absl_vlog_is_on_FRAMEWORKS_RELEASE )
set(abseil_absl_absl_vlog_is_on_DEPENDENCIES_RELEASE absl::vlog_config_internal absl::config absl::core_headers absl::strings)
set(abseil_absl_absl_vlog_is_on_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_absl_vlog_is_on_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_absl_vlog_is_on_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_absl_vlog_is_on_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_absl_vlog_is_on_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_absl_vlog_is_on_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_absl_vlog_is_on_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_absl_vlog_is_on_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_absl_vlog_is_on_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_absl_vlog_is_on_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::log_globals VARIABLES ############################################

set(abseil_absl_log_globals_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_log_globals_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_log_globals_BIN_DIRS_RELEASE )
set(abseil_absl_log_globals_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_log_globals_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_log_globals_RES_DIRS_RELEASE )
set(abseil_absl_log_globals_DEFINITIONS_RELEASE )
set(abseil_absl_log_globals_OBJECTS_RELEASE )
set(abseil_absl_log_globals_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_log_globals_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_log_globals_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_log_globals_LIBS_RELEASE absl_log_globals)
set(abseil_absl_log_globals_SYSTEM_LIBS_RELEASE )
set(abseil_absl_log_globals_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_log_globals_FRAMEWORKS_RELEASE )
set(abseil_absl_log_globals_DEPENDENCIES_RELEASE absl::atomic_hook absl::config absl::core_headers absl::hash absl::log_severity absl::raw_logging_internal absl::strings absl::vlog_config_internal)
set(abseil_absl_log_globals_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_log_globals_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_log_globals_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_log_globals_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_log_globals_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_log_globals_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_log_globals_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_log_globals_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_log_globals_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_log_globals_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::flags_config VARIABLES ############################################

set(abseil_absl_flags_config_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_flags_config_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_flags_config_BIN_DIRS_RELEASE )
set(abseil_absl_flags_config_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_flags_config_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_flags_config_RES_DIRS_RELEASE )
set(abseil_absl_flags_config_DEFINITIONS_RELEASE )
set(abseil_absl_flags_config_OBJECTS_RELEASE )
set(abseil_absl_flags_config_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_flags_config_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_flags_config_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_flags_config_LIBS_RELEASE absl_flags_config)
set(abseil_absl_flags_config_SYSTEM_LIBS_RELEASE )
set(abseil_absl_flags_config_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_flags_config_FRAMEWORKS_RELEASE )
set(abseil_absl_flags_config_DEPENDENCIES_RELEASE absl::config absl::flags_path_util absl::flags_program_name absl::core_headers absl::no_destructor absl::strings absl::synchronization)
set(abseil_absl_flags_config_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_flags_config_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_flags_config_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_flags_config_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_flags_config_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_flags_config_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_flags_config_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_flags_config_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_flags_config_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_flags_config_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::hashtablez_sampler VARIABLES ############################################

set(abseil_absl_hashtablez_sampler_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_hashtablez_sampler_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_hashtablez_sampler_BIN_DIRS_RELEASE )
set(abseil_absl_hashtablez_sampler_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_hashtablez_sampler_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_hashtablez_sampler_RES_DIRS_RELEASE )
set(abseil_absl_hashtablez_sampler_DEFINITIONS_RELEASE )
set(abseil_absl_hashtablez_sampler_OBJECTS_RELEASE )
set(abseil_absl_hashtablez_sampler_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_hashtablez_sampler_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_hashtablez_sampler_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_hashtablez_sampler_LIBS_RELEASE absl_hashtablez_sampler)
set(abseil_absl_hashtablez_sampler_SYSTEM_LIBS_RELEASE )
set(abseil_absl_hashtablez_sampler_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_hashtablez_sampler_FRAMEWORKS_RELEASE )
set(abseil_absl_hashtablez_sampler_DEPENDENCIES_RELEASE absl::base absl::config absl::exponential_biased absl::no_destructor absl::raw_logging_internal absl::sample_recorder absl::synchronization absl::time)
set(abseil_absl_hashtablez_sampler_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_hashtablez_sampler_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_hashtablez_sampler_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_hashtablez_sampler_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_hashtablez_sampler_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_hashtablez_sampler_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_hashtablez_sampler_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_hashtablez_sampler_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_hashtablez_sampler_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_hashtablez_sampler_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::cordz_handle VARIABLES ############################################

set(abseil_absl_cordz_handle_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_cordz_handle_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_cordz_handle_BIN_DIRS_RELEASE )
set(abseil_absl_cordz_handle_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_cordz_handle_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_cordz_handle_RES_DIRS_RELEASE )
set(abseil_absl_cordz_handle_DEFINITIONS_RELEASE )
set(abseil_absl_cordz_handle_OBJECTS_RELEASE )
set(abseil_absl_cordz_handle_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_cordz_handle_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_cordz_handle_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_cordz_handle_LIBS_RELEASE absl_cordz_handle)
set(abseil_absl_cordz_handle_SYSTEM_LIBS_RELEASE )
set(abseil_absl_cordz_handle_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_cordz_handle_FRAMEWORKS_RELEASE )
set(abseil_absl_cordz_handle_DEPENDENCIES_RELEASE absl::base absl::config absl::no_destructor absl::raw_logging_internal absl::synchronization)
set(abseil_absl_cordz_handle_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_cordz_handle_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_cordz_handle_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_cordz_handle_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_cordz_handle_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_cordz_handle_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_cordz_handle_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_cordz_handle_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_cordz_handle_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_cordz_handle_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::cordz_statistics VARIABLES ############################################

set(abseil_absl_cordz_statistics_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_cordz_statistics_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_cordz_statistics_BIN_DIRS_RELEASE )
set(abseil_absl_cordz_statistics_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_cordz_statistics_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_cordz_statistics_RES_DIRS_RELEASE )
set(abseil_absl_cordz_statistics_DEFINITIONS_RELEASE )
set(abseil_absl_cordz_statistics_OBJECTS_RELEASE )
set(abseil_absl_cordz_statistics_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_cordz_statistics_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_cordz_statistics_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_cordz_statistics_LIBS_RELEASE )
set(abseil_absl_cordz_statistics_SYSTEM_LIBS_RELEASE )
set(abseil_absl_cordz_statistics_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_cordz_statistics_FRAMEWORKS_RELEASE )
set(abseil_absl_cordz_statistics_DEPENDENCIES_RELEASE absl::config absl::core_headers absl::cordz_update_tracker absl::synchronization)
set(abseil_absl_cordz_statistics_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_cordz_statistics_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_cordz_statistics_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_cordz_statistics_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_cordz_statistics_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_cordz_statistics_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_cordz_statistics_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_cordz_statistics_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_cordz_statistics_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_cordz_statistics_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::cord_internal VARIABLES ############################################

set(abseil_absl_cord_internal_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_cord_internal_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_cord_internal_BIN_DIRS_RELEASE )
set(abseil_absl_cord_internal_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_cord_internal_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_cord_internal_RES_DIRS_RELEASE )
set(abseil_absl_cord_internal_DEFINITIONS_RELEASE )
set(abseil_absl_cord_internal_OBJECTS_RELEASE )
set(abseil_absl_cord_internal_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_cord_internal_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_cord_internal_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_cord_internal_LIBS_RELEASE absl_cord_internal)
set(abseil_absl_cord_internal_SYSTEM_LIBS_RELEASE )
set(abseil_absl_cord_internal_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_cord_internal_FRAMEWORKS_RELEASE )
set(abseil_absl_cord_internal_DEPENDENCIES_RELEASE absl::compressed_tuple absl::config absl::container_memory absl::compare absl::core_headers absl::crc_cord_state absl::endian absl::inlined_vector absl::layout absl::raw_logging_internal absl::strings absl::throw_delegate absl::type_traits)
set(abseil_absl_cord_internal_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_cord_internal_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_cord_internal_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_cord_internal_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_cord_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_cord_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_cord_internal_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_cord_internal_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_cord_internal_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_cord_internal_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::random_internal_entropy_pool VARIABLES ############################################

set(abseil_absl_random_internal_entropy_pool_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_random_internal_entropy_pool_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_random_internal_entropy_pool_BIN_DIRS_RELEASE )
set(abseil_absl_random_internal_entropy_pool_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_random_internal_entropy_pool_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_random_internal_entropy_pool_RES_DIRS_RELEASE )
set(abseil_absl_random_internal_entropy_pool_DEFINITIONS_RELEASE )
set(abseil_absl_random_internal_entropy_pool_OBJECTS_RELEASE )
set(abseil_absl_random_internal_entropy_pool_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_random_internal_entropy_pool_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_random_internal_entropy_pool_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_random_internal_entropy_pool_LIBS_RELEASE absl_random_internal_entropy_pool)
set(abseil_absl_random_internal_entropy_pool_SYSTEM_LIBS_RELEASE )
set(abseil_absl_random_internal_entropy_pool_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_random_internal_entropy_pool_FRAMEWORKS_RELEASE )
set(abseil_absl_random_internal_entropy_pool_DEPENDENCIES_RELEASE absl::base absl::config absl::core_headers absl::random_internal_platform absl::random_internal_randen absl::random_internal_seed_material absl::random_seed_gen_exception absl::span absl::synchronization)
set(abseil_absl_random_internal_entropy_pool_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_random_internal_entropy_pool_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_random_internal_entropy_pool_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_random_internal_entropy_pool_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_random_internal_entropy_pool_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_random_internal_entropy_pool_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_random_internal_entropy_pool_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_random_internal_entropy_pool_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_random_internal_entropy_pool_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_random_internal_entropy_pool_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::sample_recorder VARIABLES ############################################

set(abseil_absl_sample_recorder_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_sample_recorder_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_sample_recorder_BIN_DIRS_RELEASE )
set(abseil_absl_sample_recorder_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_sample_recorder_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_sample_recorder_RES_DIRS_RELEASE )
set(abseil_absl_sample_recorder_DEFINITIONS_RELEASE )
set(abseil_absl_sample_recorder_OBJECTS_RELEASE )
set(abseil_absl_sample_recorder_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_sample_recorder_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_sample_recorder_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_sample_recorder_LIBS_RELEASE )
set(abseil_absl_sample_recorder_SYSTEM_LIBS_RELEASE )
set(abseil_absl_sample_recorder_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_sample_recorder_FRAMEWORKS_RELEASE )
set(abseil_absl_sample_recorder_DEPENDENCIES_RELEASE absl::base absl::synchronization)
set(abseil_absl_sample_recorder_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_sample_recorder_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_sample_recorder_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_sample_recorder_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_sample_recorder_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_sample_recorder_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_sample_recorder_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_sample_recorder_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_sample_recorder_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_sample_recorder_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::vlog_config_internal VARIABLES ############################################

set(abseil_absl_vlog_config_internal_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_vlog_config_internal_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_vlog_config_internal_BIN_DIRS_RELEASE )
set(abseil_absl_vlog_config_internal_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_vlog_config_internal_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_vlog_config_internal_RES_DIRS_RELEASE )
set(abseil_absl_vlog_config_internal_DEFINITIONS_RELEASE )
set(abseil_absl_vlog_config_internal_OBJECTS_RELEASE )
set(abseil_absl_vlog_config_internal_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_vlog_config_internal_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_vlog_config_internal_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_vlog_config_internal_LIBS_RELEASE absl_vlog_config_internal)
set(abseil_absl_vlog_config_internal_SYSTEM_LIBS_RELEASE )
set(abseil_absl_vlog_config_internal_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_vlog_config_internal_FRAMEWORKS_RELEASE )
set(abseil_absl_vlog_config_internal_DEPENDENCIES_RELEASE absl::base absl::config absl::core_headers absl::log_internal_fnmatch absl::memory absl::no_destructor absl::nullability absl::strings absl::synchronization absl::optional)
set(abseil_absl_vlog_config_internal_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_vlog_config_internal_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_vlog_config_internal_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_vlog_config_internal_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_vlog_config_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_vlog_config_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_vlog_config_internal_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_vlog_config_internal_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_vlog_config_internal_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_vlog_config_internal_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::flags_program_name VARIABLES ############################################

set(abseil_absl_flags_program_name_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_flags_program_name_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_flags_program_name_BIN_DIRS_RELEASE )
set(abseil_absl_flags_program_name_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_flags_program_name_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_flags_program_name_RES_DIRS_RELEASE )
set(abseil_absl_flags_program_name_DEFINITIONS_RELEASE )
set(abseil_absl_flags_program_name_OBJECTS_RELEASE )
set(abseil_absl_flags_program_name_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_flags_program_name_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_flags_program_name_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_flags_program_name_LIBS_RELEASE absl_flags_program_name)
set(abseil_absl_flags_program_name_SYSTEM_LIBS_RELEASE )
set(abseil_absl_flags_program_name_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_flags_program_name_FRAMEWORKS_RELEASE )
set(abseil_absl_flags_program_name_DEPENDENCIES_RELEASE absl::config absl::core_headers absl::no_destructor absl::flags_path_util absl::strings absl::synchronization)
set(abseil_absl_flags_program_name_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_flags_program_name_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_flags_program_name_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_flags_program_name_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_flags_program_name_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_flags_program_name_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_flags_program_name_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_flags_program_name_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_flags_program_name_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_flags_program_name_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::failure_signal_handler VARIABLES ############################################

set(abseil_absl_failure_signal_handler_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_failure_signal_handler_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_failure_signal_handler_BIN_DIRS_RELEASE )
set(abseil_absl_failure_signal_handler_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_failure_signal_handler_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_failure_signal_handler_RES_DIRS_RELEASE )
set(abseil_absl_failure_signal_handler_DEFINITIONS_RELEASE )
set(abseil_absl_failure_signal_handler_OBJECTS_RELEASE )
set(abseil_absl_failure_signal_handler_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_failure_signal_handler_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_failure_signal_handler_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_failure_signal_handler_LIBS_RELEASE absl_failure_signal_handler)
set(abseil_absl_failure_signal_handler_SYSTEM_LIBS_RELEASE )
set(abseil_absl_failure_signal_handler_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_failure_signal_handler_FRAMEWORKS_RELEASE )
set(abseil_absl_failure_signal_handler_DEPENDENCIES_RELEASE absl::examine_stack absl::stacktrace absl::base absl::config absl::core_headers absl::raw_logging_internal)
set(abseil_absl_failure_signal_handler_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_failure_signal_handler_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_failure_signal_handler_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_failure_signal_handler_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_failure_signal_handler_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_failure_signal_handler_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_failure_signal_handler_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_failure_signal_handler_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_failure_signal_handler_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_failure_signal_handler_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::synchronization VARIABLES ############################################

set(abseil_absl_synchronization_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_synchronization_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_synchronization_BIN_DIRS_RELEASE )
set(abseil_absl_synchronization_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_synchronization_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_synchronization_RES_DIRS_RELEASE )
set(abseil_absl_synchronization_DEFINITIONS_RELEASE )
set(abseil_absl_synchronization_OBJECTS_RELEASE )
set(abseil_absl_synchronization_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_synchronization_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_synchronization_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_synchronization_LIBS_RELEASE absl_synchronization)
set(abseil_absl_synchronization_SYSTEM_LIBS_RELEASE pthread)
set(abseil_absl_synchronization_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_synchronization_FRAMEWORKS_RELEASE )
set(abseil_absl_synchronization_DEPENDENCIES_RELEASE absl::graphcycles_internal absl::kernel_timeout_internal absl::atomic_hook absl::base absl::base_internal absl::config absl::core_headers absl::dynamic_annotations absl::malloc_internal absl::meta absl::nullability absl::raw_logging_internal absl::stacktrace absl::symbolize absl::time absl::tracing_internal)
set(abseil_absl_synchronization_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_synchronization_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_synchronization_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_synchronization_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_synchronization_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_synchronization_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_synchronization_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_synchronization_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_synchronization_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_synchronization_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::debugging VARIABLES ############################################

set(abseil_absl_debugging_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_debugging_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_debugging_BIN_DIRS_RELEASE )
set(abseil_absl_debugging_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_debugging_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_debugging_RES_DIRS_RELEASE )
set(abseil_absl_debugging_DEFINITIONS_RELEASE )
set(abseil_absl_debugging_OBJECTS_RELEASE )
set(abseil_absl_debugging_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_debugging_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_debugging_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_debugging_LIBS_RELEASE )
set(abseil_absl_debugging_SYSTEM_LIBS_RELEASE )
set(abseil_absl_debugging_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_debugging_FRAMEWORKS_RELEASE )
set(abseil_absl_debugging_DEPENDENCIES_RELEASE absl::stacktrace absl::leak_check)
set(abseil_absl_debugging_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_debugging_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_debugging_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_debugging_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_debugging_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_debugging_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_debugging_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_debugging_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_debugging_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_debugging_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::examine_stack VARIABLES ############################################

set(abseil_absl_examine_stack_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_examine_stack_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_examine_stack_BIN_DIRS_RELEASE )
set(abseil_absl_examine_stack_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_examine_stack_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_examine_stack_RES_DIRS_RELEASE )
set(abseil_absl_examine_stack_DEFINITIONS_RELEASE )
set(abseil_absl_examine_stack_OBJECTS_RELEASE )
set(abseil_absl_examine_stack_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_examine_stack_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_examine_stack_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_examine_stack_LIBS_RELEASE absl_examine_stack)
set(abseil_absl_examine_stack_SYSTEM_LIBS_RELEASE )
set(abseil_absl_examine_stack_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_examine_stack_FRAMEWORKS_RELEASE )
set(abseil_absl_examine_stack_DEPENDENCIES_RELEASE absl::stacktrace absl::symbolize absl::config absl::core_headers absl::raw_logging_internal)
set(abseil_absl_examine_stack_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_examine_stack_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_examine_stack_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_examine_stack_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_examine_stack_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_examine_stack_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_examine_stack_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_examine_stack_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_examine_stack_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_examine_stack_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::crc_cord_state VARIABLES ############################################

set(abseil_absl_crc_cord_state_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_crc_cord_state_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_crc_cord_state_BIN_DIRS_RELEASE )
set(abseil_absl_crc_cord_state_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_crc_cord_state_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_crc_cord_state_RES_DIRS_RELEASE )
set(abseil_absl_crc_cord_state_DEFINITIONS_RELEASE )
set(abseil_absl_crc_cord_state_OBJECTS_RELEASE )
set(abseil_absl_crc_cord_state_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_crc_cord_state_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_crc_cord_state_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_crc_cord_state_LIBS_RELEASE absl_crc_cord_state)
set(abseil_absl_crc_cord_state_SYSTEM_LIBS_RELEASE )
set(abseil_absl_crc_cord_state_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_crc_cord_state_FRAMEWORKS_RELEASE )
set(abseil_absl_crc_cord_state_DEPENDENCIES_RELEASE absl::crc32c absl::config absl::strings absl::no_destructor)
set(abseil_absl_crc_cord_state_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_crc_cord_state_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_crc_cord_state_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_crc_cord_state_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_crc_cord_state_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_crc_cord_state_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_crc_cord_state_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_crc_cord_state_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_crc_cord_state_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_crc_cord_state_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::generic_printer_internal VARIABLES ############################################

set(abseil_absl_generic_printer_internal_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_generic_printer_internal_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_generic_printer_internal_BIN_DIRS_RELEASE )
set(abseil_absl_generic_printer_internal_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_generic_printer_internal_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_generic_printer_internal_RES_DIRS_RELEASE )
set(abseil_absl_generic_printer_internal_DEFINITIONS_RELEASE )
set(abseil_absl_generic_printer_internal_OBJECTS_RELEASE )
set(abseil_absl_generic_printer_internal_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_generic_printer_internal_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_generic_printer_internal_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_generic_printer_internal_LIBS_RELEASE absl_generic_printer_internal)
set(abseil_absl_generic_printer_internal_SYSTEM_LIBS_RELEASE )
set(abseil_absl_generic_printer_internal_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_generic_printer_internal_FRAMEWORKS_RELEASE )
set(abseil_absl_generic_printer_internal_DEPENDENCIES_RELEASE absl::config absl::strings absl::str_format absl::log_internal_container absl::requires_internal)
set(abseil_absl_generic_printer_internal_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_generic_printer_internal_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_generic_printer_internal_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_generic_printer_internal_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_generic_printer_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_generic_printer_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_generic_printer_internal_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_generic_printer_internal_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_generic_printer_internal_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_generic_printer_internal_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::random_internal_distribution_test_util VARIABLES ############################################

set(abseil_absl_random_internal_distribution_test_util_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_random_internal_distribution_test_util_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_random_internal_distribution_test_util_BIN_DIRS_RELEASE )
set(abseil_absl_random_internal_distribution_test_util_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_random_internal_distribution_test_util_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_random_internal_distribution_test_util_RES_DIRS_RELEASE )
set(abseil_absl_random_internal_distribution_test_util_DEFINITIONS_RELEASE )
set(abseil_absl_random_internal_distribution_test_util_OBJECTS_RELEASE )
set(abseil_absl_random_internal_distribution_test_util_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_random_internal_distribution_test_util_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_random_internal_distribution_test_util_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_random_internal_distribution_test_util_LIBS_RELEASE absl_random_internal_distribution_test_util)
set(abseil_absl_random_internal_distribution_test_util_SYSTEM_LIBS_RELEASE )
set(abseil_absl_random_internal_distribution_test_util_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_random_internal_distribution_test_util_FRAMEWORKS_RELEASE )
set(abseil_absl_random_internal_distribution_test_util_DEPENDENCIES_RELEASE absl::config absl::core_headers absl::raw_logging_internal absl::strings absl::str_format absl::span)
set(abseil_absl_random_internal_distribution_test_util_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_random_internal_distribution_test_util_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_random_internal_distribution_test_util_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_random_internal_distribution_test_util_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_random_internal_distribution_test_util_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_random_internal_distribution_test_util_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_random_internal_distribution_test_util_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_random_internal_distribution_test_util_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_random_internal_distribution_test_util_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_random_internal_distribution_test_util_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::log_sink VARIABLES ############################################

set(abseil_absl_log_sink_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_log_sink_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_log_sink_BIN_DIRS_RELEASE )
set(abseil_absl_log_sink_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_log_sink_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_log_sink_RES_DIRS_RELEASE )
set(abseil_absl_log_sink_DEFINITIONS_RELEASE )
set(abseil_absl_log_sink_OBJECTS_RELEASE )
set(abseil_absl_log_sink_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_log_sink_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_log_sink_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_log_sink_LIBS_RELEASE absl_log_sink)
set(abseil_absl_log_sink_SYSTEM_LIBS_RELEASE )
set(abseil_absl_log_sink_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_log_sink_FRAMEWORKS_RELEASE )
set(abseil_absl_log_sink_DEPENDENCIES_RELEASE absl::config absl::log_entry)
set(abseil_absl_log_sink_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_log_sink_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_log_sink_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_log_sink_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_log_sink_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_log_sink_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_log_sink_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_log_sink_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_log_sink_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_log_sink_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::log_internal_format VARIABLES ############################################

set(abseil_absl_log_internal_format_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_log_internal_format_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_log_internal_format_BIN_DIRS_RELEASE )
set(abseil_absl_log_internal_format_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_log_internal_format_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_log_internal_format_RES_DIRS_RELEASE )
set(abseil_absl_log_internal_format_DEFINITIONS_RELEASE )
set(abseil_absl_log_internal_format_OBJECTS_RELEASE )
set(abseil_absl_log_internal_format_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_log_internal_format_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_log_internal_format_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_log_internal_format_LIBS_RELEASE absl_log_internal_format)
set(abseil_absl_log_internal_format_SYSTEM_LIBS_RELEASE )
set(abseil_absl_log_internal_format_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_log_internal_format_FRAMEWORKS_RELEASE )
set(abseil_absl_log_internal_format_DEPENDENCIES_RELEASE absl::config absl::core_headers absl::log_internal_append_truncated absl::log_internal_config absl::log_internal_globals absl::log_severity absl::strings absl::str_format absl::time absl::span)
set(abseil_absl_log_internal_format_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_log_internal_format_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_log_internal_format_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_log_internal_format_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_log_internal_format_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_log_internal_format_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_log_internal_format_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_log_internal_format_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_log_internal_format_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_log_internal_format_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::flags_marshalling VARIABLES ############################################

set(abseil_absl_flags_marshalling_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_flags_marshalling_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_flags_marshalling_BIN_DIRS_RELEASE )
set(abseil_absl_flags_marshalling_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_flags_marshalling_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_flags_marshalling_RES_DIRS_RELEASE )
set(abseil_absl_flags_marshalling_DEFINITIONS_RELEASE )
set(abseil_absl_flags_marshalling_OBJECTS_RELEASE )
set(abseil_absl_flags_marshalling_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_flags_marshalling_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_flags_marshalling_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_flags_marshalling_LIBS_RELEASE absl_flags_marshalling)
set(abseil_absl_flags_marshalling_SYSTEM_LIBS_RELEASE )
set(abseil_absl_flags_marshalling_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_flags_marshalling_FRAMEWORKS_RELEASE )
set(abseil_absl_flags_marshalling_DEPENDENCIES_RELEASE absl::config absl::core_headers absl::log_severity absl::int128 absl::optional absl::strings absl::str_format)
set(abseil_absl_flags_marshalling_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_flags_marshalling_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_flags_marshalling_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_flags_marshalling_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_flags_marshalling_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_flags_marshalling_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_flags_marshalling_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_flags_marshalling_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_flags_marshalling_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_flags_marshalling_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::symbolize VARIABLES ############################################

set(abseil_absl_symbolize_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_symbolize_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_symbolize_BIN_DIRS_RELEASE )
set(abseil_absl_symbolize_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_symbolize_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_symbolize_RES_DIRS_RELEASE )
set(abseil_absl_symbolize_DEFINITIONS_RELEASE )
set(abseil_absl_symbolize_OBJECTS_RELEASE )
set(abseil_absl_symbolize_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_symbolize_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_symbolize_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_symbolize_LIBS_RELEASE absl_symbolize)
set(abseil_absl_symbolize_SYSTEM_LIBS_RELEASE )
set(abseil_absl_symbolize_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_symbolize_FRAMEWORKS_RELEASE )
set(abseil_absl_symbolize_DEPENDENCIES_RELEASE absl::debugging_internal absl::demangle_internal absl::base absl::config absl::core_headers absl::dynamic_annotations absl::malloc_internal absl::raw_logging_internal absl::strings)
set(abseil_absl_symbolize_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_symbolize_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_symbolize_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_symbolize_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_symbolize_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_symbolize_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_symbolize_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_symbolize_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_symbolize_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_symbolize_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::stacktrace VARIABLES ############################################

set(abseil_absl_stacktrace_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_stacktrace_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_stacktrace_BIN_DIRS_RELEASE )
set(abseil_absl_stacktrace_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_stacktrace_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_stacktrace_RES_DIRS_RELEASE )
set(abseil_absl_stacktrace_DEFINITIONS_RELEASE )
set(abseil_absl_stacktrace_OBJECTS_RELEASE )
set(abseil_absl_stacktrace_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_stacktrace_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_stacktrace_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_stacktrace_LIBS_RELEASE absl_stacktrace)
set(abseil_absl_stacktrace_SYSTEM_LIBS_RELEASE )
set(abseil_absl_stacktrace_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_stacktrace_FRAMEWORKS_RELEASE )
set(abseil_absl_stacktrace_DEPENDENCIES_RELEASE absl::borrowed_fixup_buffer absl::debugging_internal absl::config absl::core_headers absl::dynamic_annotations absl::malloc_internal absl::raw_logging_internal)
set(abseil_absl_stacktrace_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_stacktrace_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_stacktrace_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_stacktrace_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_stacktrace_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_stacktrace_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_stacktrace_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_stacktrace_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_stacktrace_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_stacktrace_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::crc32c VARIABLES ############################################

set(abseil_absl_crc32c_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_crc32c_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_crc32c_BIN_DIRS_RELEASE )
set(abseil_absl_crc32c_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_crc32c_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_crc32c_RES_DIRS_RELEASE )
set(abseil_absl_crc32c_DEFINITIONS_RELEASE )
set(abseil_absl_crc32c_OBJECTS_RELEASE )
set(abseil_absl_crc32c_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_crc32c_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_crc32c_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_crc32c_LIBS_RELEASE absl_crc32c)
set(abseil_absl_crc32c_SYSTEM_LIBS_RELEASE )
set(abseil_absl_crc32c_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_crc32c_FRAMEWORKS_RELEASE )
set(abseil_absl_crc32c_DEPENDENCIES_RELEASE absl::crc_cpu_detect absl::crc_internal absl::non_temporal_memcpy absl::config absl::core_headers absl::endian absl::prefetch absl::str_format absl::strings)
set(abseil_absl_crc32c_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_crc32c_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_crc32c_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_crc32c_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_crc32c_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_crc32c_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_crc32c_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_crc32c_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_crc32c_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_crc32c_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::hash_policy_traits VARIABLES ############################################

set(abseil_absl_hash_policy_traits_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_hash_policy_traits_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_hash_policy_traits_BIN_DIRS_RELEASE )
set(abseil_absl_hash_policy_traits_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_hash_policy_traits_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_hash_policy_traits_RES_DIRS_RELEASE )
set(abseil_absl_hash_policy_traits_DEFINITIONS_RELEASE )
set(abseil_absl_hash_policy_traits_OBJECTS_RELEASE )
set(abseil_absl_hash_policy_traits_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_hash_policy_traits_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_hash_policy_traits_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_hash_policy_traits_LIBS_RELEASE )
set(abseil_absl_hash_policy_traits_SYSTEM_LIBS_RELEASE )
set(abseil_absl_hash_policy_traits_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_hash_policy_traits_FRAMEWORKS_RELEASE )
set(abseil_absl_hash_policy_traits_DEPENDENCIES_RELEASE absl::container_memory absl::common_policy_traits absl::meta)
set(abseil_absl_hash_policy_traits_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_hash_policy_traits_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_hash_policy_traits_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_hash_policy_traits_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_hash_policy_traits_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_hash_policy_traits_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_hash_policy_traits_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_hash_policy_traits_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_hash_policy_traits_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_hash_policy_traits_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::kernel_timeout_internal VARIABLES ############################################

set(abseil_absl_kernel_timeout_internal_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_kernel_timeout_internal_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_kernel_timeout_internal_BIN_DIRS_RELEASE )
set(abseil_absl_kernel_timeout_internal_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_kernel_timeout_internal_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_kernel_timeout_internal_RES_DIRS_RELEASE )
set(abseil_absl_kernel_timeout_internal_DEFINITIONS_RELEASE )
set(abseil_absl_kernel_timeout_internal_OBJECTS_RELEASE )
set(abseil_absl_kernel_timeout_internal_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_kernel_timeout_internal_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_kernel_timeout_internal_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_kernel_timeout_internal_LIBS_RELEASE absl_kernel_timeout_internal)
set(abseil_absl_kernel_timeout_internal_SYSTEM_LIBS_RELEASE )
set(abseil_absl_kernel_timeout_internal_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_kernel_timeout_internal_FRAMEWORKS_RELEASE )
set(abseil_absl_kernel_timeout_internal_DEPENDENCIES_RELEASE absl::base absl::config absl::core_headers absl::raw_logging_internal absl::time)
set(abseil_absl_kernel_timeout_internal_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_kernel_timeout_internal_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_kernel_timeout_internal_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_kernel_timeout_internal_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_kernel_timeout_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_kernel_timeout_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_kernel_timeout_internal_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_kernel_timeout_internal_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_kernel_timeout_internal_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_kernel_timeout_internal_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::str_format VARIABLES ############################################

set(abseil_absl_str_format_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_str_format_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_str_format_BIN_DIRS_RELEASE )
set(abseil_absl_str_format_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_str_format_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_str_format_RES_DIRS_RELEASE )
set(abseil_absl_str_format_DEFINITIONS_RELEASE )
set(abseil_absl_str_format_OBJECTS_RELEASE )
set(abseil_absl_str_format_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_str_format_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_str_format_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_str_format_LIBS_RELEASE )
set(abseil_absl_str_format_SYSTEM_LIBS_RELEASE )
set(abseil_absl_str_format_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_str_format_FRAMEWORKS_RELEASE )
set(abseil_absl_str_format_DEPENDENCIES_RELEASE absl::config absl::core_headers absl::nullability absl::span absl::str_format_internal absl::string_view)
set(abseil_absl_str_format_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_str_format_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_str_format_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_str_format_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_str_format_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_str_format_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_str_format_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_str_format_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_str_format_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_str_format_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::random_internal_salted_seed_seq VARIABLES ############################################

set(abseil_absl_random_internal_salted_seed_seq_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_random_internal_salted_seed_seq_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_random_internal_salted_seed_seq_BIN_DIRS_RELEASE )
set(abseil_absl_random_internal_salted_seed_seq_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_random_internal_salted_seed_seq_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_random_internal_salted_seed_seq_RES_DIRS_RELEASE )
set(abseil_absl_random_internal_salted_seed_seq_DEFINITIONS_RELEASE )
set(abseil_absl_random_internal_salted_seed_seq_OBJECTS_RELEASE )
set(abseil_absl_random_internal_salted_seed_seq_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_random_internal_salted_seed_seq_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_random_internal_salted_seed_seq_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_random_internal_salted_seed_seq_LIBS_RELEASE )
set(abseil_absl_random_internal_salted_seed_seq_SYSTEM_LIBS_RELEASE )
set(abseil_absl_random_internal_salted_seed_seq_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_random_internal_salted_seed_seq_FRAMEWORKS_RELEASE )
set(abseil_absl_random_internal_salted_seed_seq_DEPENDENCIES_RELEASE absl::inlined_vector absl::optional absl::span absl::random_internal_seed_material absl::type_traits)
set(abseil_absl_random_internal_salted_seed_seq_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_random_internal_salted_seed_seq_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_random_internal_salted_seed_seq_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_random_internal_salted_seed_seq_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_random_internal_salted_seed_seq_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_random_internal_salted_seed_seq_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_random_internal_salted_seed_seq_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_random_internal_salted_seed_seq_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_random_internal_salted_seed_seq_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_random_internal_salted_seed_seq_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::log_internal_structured_proto VARIABLES ############################################

set(abseil_absl_log_internal_structured_proto_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_log_internal_structured_proto_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_log_internal_structured_proto_BIN_DIRS_RELEASE )
set(abseil_absl_log_internal_structured_proto_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_log_internal_structured_proto_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_log_internal_structured_proto_RES_DIRS_RELEASE )
set(abseil_absl_log_internal_structured_proto_DEFINITIONS_RELEASE )
set(abseil_absl_log_internal_structured_proto_OBJECTS_RELEASE )
set(abseil_absl_log_internal_structured_proto_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_log_internal_structured_proto_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_log_internal_structured_proto_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_log_internal_structured_proto_LIBS_RELEASE absl_log_internal_structured_proto)
set(abseil_absl_log_internal_structured_proto_SYSTEM_LIBS_RELEASE )
set(abseil_absl_log_internal_structured_proto_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_log_internal_structured_proto_FRAMEWORKS_RELEASE )
set(abseil_absl_log_internal_structured_proto_DEPENDENCIES_RELEASE absl::log_internal_proto absl::config absl::span absl::strings absl::variant)
set(abseil_absl_log_internal_structured_proto_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_log_internal_structured_proto_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_log_internal_structured_proto_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_log_internal_structured_proto_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_log_internal_structured_proto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_log_internal_structured_proto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_log_internal_structured_proto_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_log_internal_structured_proto_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_log_internal_structured_proto_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_log_internal_structured_proto_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::log_entry VARIABLES ############################################

set(abseil_absl_log_entry_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_log_entry_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_log_entry_BIN_DIRS_RELEASE )
set(abseil_absl_log_entry_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_log_entry_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_log_entry_RES_DIRS_RELEASE )
set(abseil_absl_log_entry_DEFINITIONS_RELEASE )
set(abseil_absl_log_entry_OBJECTS_RELEASE )
set(abseil_absl_log_entry_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_log_entry_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_log_entry_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_log_entry_LIBS_RELEASE absl_log_entry)
set(abseil_absl_log_entry_SYSTEM_LIBS_RELEASE )
set(abseil_absl_log_entry_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_log_entry_FRAMEWORKS_RELEASE )
set(abseil_absl_log_entry_DEPENDENCIES_RELEASE absl::config absl::core_headers absl::log_internal_config absl::log_internal_proto absl::log_severity absl::span absl::strings absl::time)
set(abseil_absl_log_entry_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_log_entry_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_log_entry_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_log_entry_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_log_entry_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_log_entry_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_log_entry_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_log_entry_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_log_entry_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_log_entry_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::log_internal_globals VARIABLES ############################################

set(abseil_absl_log_internal_globals_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_log_internal_globals_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_log_internal_globals_BIN_DIRS_RELEASE )
set(abseil_absl_log_internal_globals_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_log_internal_globals_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_log_internal_globals_RES_DIRS_RELEASE )
set(abseil_absl_log_internal_globals_DEFINITIONS_RELEASE )
set(abseil_absl_log_internal_globals_OBJECTS_RELEASE )
set(abseil_absl_log_internal_globals_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_log_internal_globals_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_log_internal_globals_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_log_internal_globals_LIBS_RELEASE absl_log_internal_globals)
set(abseil_absl_log_internal_globals_SYSTEM_LIBS_RELEASE )
set(abseil_absl_log_internal_globals_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_log_internal_globals_FRAMEWORKS_RELEASE )
set(abseil_absl_log_internal_globals_DEPENDENCIES_RELEASE absl::config absl::core_headers absl::log_severity absl::raw_logging_internal absl::strings absl::time)
set(abseil_absl_log_internal_globals_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_log_internal_globals_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_log_internal_globals_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_log_internal_globals_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_log_internal_globals_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_log_internal_globals_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_log_internal_globals_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_log_internal_globals_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_log_internal_globals_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_log_internal_globals_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::flags_private_handle_accessor VARIABLES ############################################

set(abseil_absl_flags_private_handle_accessor_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_flags_private_handle_accessor_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_flags_private_handle_accessor_BIN_DIRS_RELEASE )
set(abseil_absl_flags_private_handle_accessor_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_flags_private_handle_accessor_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_flags_private_handle_accessor_RES_DIRS_RELEASE )
set(abseil_absl_flags_private_handle_accessor_DEFINITIONS_RELEASE )
set(abseil_absl_flags_private_handle_accessor_OBJECTS_RELEASE )
set(abseil_absl_flags_private_handle_accessor_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_flags_private_handle_accessor_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_flags_private_handle_accessor_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_flags_private_handle_accessor_LIBS_RELEASE absl_flags_private_handle_accessor)
set(abseil_absl_flags_private_handle_accessor_SYSTEM_LIBS_RELEASE )
set(abseil_absl_flags_private_handle_accessor_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_flags_private_handle_accessor_FRAMEWORKS_RELEASE )
set(abseil_absl_flags_private_handle_accessor_DEPENDENCIES_RELEASE absl::config absl::flags_commandlineflag absl::flags_commandlineflag_internal absl::strings)
set(abseil_absl_flags_private_handle_accessor_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_flags_private_handle_accessor_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_flags_private_handle_accessor_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_flags_private_handle_accessor_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_flags_private_handle_accessor_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_flags_private_handle_accessor_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_flags_private_handle_accessor_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_flags_private_handle_accessor_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_flags_private_handle_accessor_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_flags_private_handle_accessor_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::demangle_internal VARIABLES ############################################

set(abseil_absl_demangle_internal_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_demangle_internal_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_demangle_internal_BIN_DIRS_RELEASE )
set(abseil_absl_demangle_internal_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_demangle_internal_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_demangle_internal_RES_DIRS_RELEASE )
set(abseil_absl_demangle_internal_DEFINITIONS_RELEASE )
set(abseil_absl_demangle_internal_OBJECTS_RELEASE )
set(abseil_absl_demangle_internal_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_demangle_internal_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_demangle_internal_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_demangle_internal_LIBS_RELEASE absl_demangle_internal)
set(abseil_absl_demangle_internal_SYSTEM_LIBS_RELEASE )
set(abseil_absl_demangle_internal_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_demangle_internal_FRAMEWORKS_RELEASE )
set(abseil_absl_demangle_internal_DEPENDENCIES_RELEASE absl::config absl::demangle_rust)
set(abseil_absl_demangle_internal_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_demangle_internal_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_demangle_internal_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_demangle_internal_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_demangle_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_demangle_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_demangle_internal_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_demangle_internal_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_demangle_internal_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_demangle_internal_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::borrowed_fixup_buffer VARIABLES ############################################

set(abseil_absl_borrowed_fixup_buffer_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_borrowed_fixup_buffer_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_borrowed_fixup_buffer_BIN_DIRS_RELEASE )
set(abseil_absl_borrowed_fixup_buffer_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_borrowed_fixup_buffer_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_borrowed_fixup_buffer_RES_DIRS_RELEASE )
set(abseil_absl_borrowed_fixup_buffer_DEFINITIONS_RELEASE )
set(abseil_absl_borrowed_fixup_buffer_OBJECTS_RELEASE )
set(abseil_absl_borrowed_fixup_buffer_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_borrowed_fixup_buffer_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_borrowed_fixup_buffer_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_borrowed_fixup_buffer_LIBS_RELEASE absl_borrowed_fixup_buffer)
set(abseil_absl_borrowed_fixup_buffer_SYSTEM_LIBS_RELEASE )
set(abseil_absl_borrowed_fixup_buffer_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_borrowed_fixup_buffer_FRAMEWORKS_RELEASE )
set(abseil_absl_borrowed_fixup_buffer_DEPENDENCIES_RELEASE absl::config absl::core_headers absl::hash absl::malloc_internal)
set(abseil_absl_borrowed_fixup_buffer_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_borrowed_fixup_buffer_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_borrowed_fixup_buffer_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_borrowed_fixup_buffer_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_borrowed_fixup_buffer_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_borrowed_fixup_buffer_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_borrowed_fixup_buffer_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_borrowed_fixup_buffer_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_borrowed_fixup_buffer_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_borrowed_fixup_buffer_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::chunked_queue VARIABLES ############################################

set(abseil_absl_chunked_queue_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_chunked_queue_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_chunked_queue_BIN_DIRS_RELEASE )
set(abseil_absl_chunked_queue_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_chunked_queue_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_chunked_queue_RES_DIRS_RELEASE )
set(abseil_absl_chunked_queue_DEFINITIONS_RELEASE )
set(abseil_absl_chunked_queue_OBJECTS_RELEASE )
set(abseil_absl_chunked_queue_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_chunked_queue_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_chunked_queue_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_chunked_queue_LIBS_RELEASE )
set(abseil_absl_chunked_queue_SYSTEM_LIBS_RELEASE )
set(abseil_absl_chunked_queue_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_chunked_queue_FRAMEWORKS_RELEASE )
set(abseil_absl_chunked_queue_DEPENDENCIES_RELEASE absl::config absl::core_headers absl::iterator_traits_internal absl::layout)
set(abseil_absl_chunked_queue_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_chunked_queue_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_chunked_queue_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_chunked_queue_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_chunked_queue_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_chunked_queue_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_chunked_queue_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_chunked_queue_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_chunked_queue_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_chunked_queue_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::container_memory VARIABLES ############################################

set(abseil_absl_container_memory_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_container_memory_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_container_memory_BIN_DIRS_RELEASE )
set(abseil_absl_container_memory_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_container_memory_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_container_memory_RES_DIRS_RELEASE )
set(abseil_absl_container_memory_DEFINITIONS_RELEASE )
set(abseil_absl_container_memory_OBJECTS_RELEASE )
set(abseil_absl_container_memory_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_container_memory_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_container_memory_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_container_memory_LIBS_RELEASE )
set(abseil_absl_container_memory_SYSTEM_LIBS_RELEASE )
set(abseil_absl_container_memory_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_container_memory_FRAMEWORKS_RELEASE )
set(abseil_absl_container_memory_DEPENDENCIES_RELEASE absl::config absl::hash absl::memory absl::type_traits absl::utility)
set(abseil_absl_container_memory_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_container_memory_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_container_memory_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_container_memory_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_container_memory_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_container_memory_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_container_memory_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_container_memory_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_container_memory_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_container_memory_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::time VARIABLES ############################################

set(abseil_absl_time_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_time_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_time_BIN_DIRS_RELEASE )
set(abseil_absl_time_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_time_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_time_RES_DIRS_RELEASE )
set(abseil_absl_time_DEFINITIONS_RELEASE )
set(abseil_absl_time_OBJECTS_RELEASE )
set(abseil_absl_time_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_time_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_time_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_time_LIBS_RELEASE absl_time)
set(abseil_absl_time_SYSTEM_LIBS_RELEASE )
set(abseil_absl_time_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_time_FRAMEWORKS_RELEASE )
set(abseil_absl_time_DEPENDENCIES_RELEASE absl::base absl::civil_time absl::core_headers absl::int128 absl::raw_logging_internal absl::strings absl::time_zone)
set(abseil_absl_time_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_time_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_time_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_time_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_time_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_time_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_time_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_time_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_time_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_time_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::str_format_internal VARIABLES ############################################

set(abseil_absl_str_format_internal_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_str_format_internal_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_str_format_internal_BIN_DIRS_RELEASE )
set(abseil_absl_str_format_internal_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_str_format_internal_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_str_format_internal_RES_DIRS_RELEASE )
set(abseil_absl_str_format_internal_DEFINITIONS_RELEASE )
set(abseil_absl_str_format_internal_OBJECTS_RELEASE )
set(abseil_absl_str_format_internal_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_str_format_internal_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_str_format_internal_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_str_format_internal_LIBS_RELEASE absl_str_format_internal)
set(abseil_absl_str_format_internal_SYSTEM_LIBS_RELEASE )
set(abseil_absl_str_format_internal_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_str_format_internal_FRAMEWORKS_RELEASE )
set(abseil_absl_str_format_internal_DEPENDENCIES_RELEASE absl::bits absl::strings absl::config absl::core_headers absl::fixed_array absl::inlined_vector absl::numeric_representation absl::type_traits absl::utility absl::int128 absl::span absl::strings_internal)
set(abseil_absl_str_format_internal_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_str_format_internal_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_str_format_internal_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_str_format_internal_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_str_format_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_str_format_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_str_format_internal_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_str_format_internal_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_str_format_internal_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_str_format_internal_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::random_internal_randen_engine VARIABLES ############################################

set(abseil_absl_random_internal_randen_engine_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_random_internal_randen_engine_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_random_internal_randen_engine_BIN_DIRS_RELEASE )
set(abseil_absl_random_internal_randen_engine_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_random_internal_randen_engine_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_random_internal_randen_engine_RES_DIRS_RELEASE )
set(abseil_absl_random_internal_randen_engine_DEFINITIONS_RELEASE )
set(abseil_absl_random_internal_randen_engine_OBJECTS_RELEASE )
set(abseil_absl_random_internal_randen_engine_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_random_internal_randen_engine_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_random_internal_randen_engine_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_random_internal_randen_engine_LIBS_RELEASE )
set(abseil_absl_random_internal_randen_engine_SYSTEM_LIBS_RELEASE )
set(abseil_absl_random_internal_randen_engine_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_random_internal_randen_engine_FRAMEWORKS_RELEASE )
set(abseil_absl_random_internal_randen_engine_DEPENDENCIES_RELEASE absl::endian absl::random_internal_iostream_state_saver absl::random_internal_randen absl::raw_logging_internal absl::type_traits)
set(abseil_absl_random_internal_randen_engine_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_random_internal_randen_engine_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_random_internal_randen_engine_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_random_internal_randen_engine_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_random_internal_randen_engine_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_random_internal_randen_engine_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_random_internal_randen_engine_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_random_internal_randen_engine_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_random_internal_randen_engine_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_random_internal_randen_engine_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::random_internal_pcg_engine VARIABLES ############################################

set(abseil_absl_random_internal_pcg_engine_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_random_internal_pcg_engine_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_random_internal_pcg_engine_BIN_DIRS_RELEASE )
set(abseil_absl_random_internal_pcg_engine_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_random_internal_pcg_engine_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_random_internal_pcg_engine_RES_DIRS_RELEASE )
set(abseil_absl_random_internal_pcg_engine_DEFINITIONS_RELEASE )
set(abseil_absl_random_internal_pcg_engine_OBJECTS_RELEASE )
set(abseil_absl_random_internal_pcg_engine_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_random_internal_pcg_engine_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_random_internal_pcg_engine_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_random_internal_pcg_engine_LIBS_RELEASE )
set(abseil_absl_random_internal_pcg_engine_SYSTEM_LIBS_RELEASE )
set(abseil_absl_random_internal_pcg_engine_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_random_internal_pcg_engine_FRAMEWORKS_RELEASE )
set(abseil_absl_random_internal_pcg_engine_DEPENDENCIES_RELEASE absl::config absl::int128 absl::random_internal_fastmath absl::random_internal_iostream_state_saver absl::type_traits)
set(abseil_absl_random_internal_pcg_engine_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_random_internal_pcg_engine_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_random_internal_pcg_engine_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_random_internal_pcg_engine_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_random_internal_pcg_engine_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_random_internal_pcg_engine_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_random_internal_pcg_engine_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_random_internal_pcg_engine_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_random_internal_pcg_engine_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_random_internal_pcg_engine_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::random_internal_seed_material VARIABLES ############################################

set(abseil_absl_random_internal_seed_material_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_random_internal_seed_material_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_random_internal_seed_material_BIN_DIRS_RELEASE )
set(abseil_absl_random_internal_seed_material_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_random_internal_seed_material_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_random_internal_seed_material_RES_DIRS_RELEASE )
set(abseil_absl_random_internal_seed_material_DEFINITIONS_RELEASE )
set(abseil_absl_random_internal_seed_material_OBJECTS_RELEASE )
set(abseil_absl_random_internal_seed_material_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_random_internal_seed_material_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_random_internal_seed_material_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_random_internal_seed_material_LIBS_RELEASE absl_random_internal_seed_material)
set(abseil_absl_random_internal_seed_material_SYSTEM_LIBS_RELEASE )
set(abseil_absl_random_internal_seed_material_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_random_internal_seed_material_FRAMEWORKS_RELEASE )
set(abseil_absl_random_internal_seed_material_DEPENDENCIES_RELEASE absl::config absl::optional absl::random_internal_fast_uniform_bits absl::raw_logging_internal absl::span absl::strings)
set(abseil_absl_random_internal_seed_material_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_random_internal_seed_material_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_random_internal_seed_material_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_random_internal_seed_material_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_random_internal_seed_material_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_random_internal_seed_material_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_random_internal_seed_material_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_random_internal_seed_material_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_random_internal_seed_material_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_random_internal_seed_material_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::random_distributions VARIABLES ############################################

set(abseil_absl_random_distributions_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_random_distributions_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_random_distributions_BIN_DIRS_RELEASE )
set(abseil_absl_random_distributions_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_random_distributions_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_random_distributions_RES_DIRS_RELEASE )
set(abseil_absl_random_distributions_DEFINITIONS_RELEASE )
set(abseil_absl_random_distributions_OBJECTS_RELEASE )
set(abseil_absl_random_distributions_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_random_distributions_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_random_distributions_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_random_distributions_LIBS_RELEASE absl_random_distributions)
set(abseil_absl_random_distributions_SYSTEM_LIBS_RELEASE )
set(abseil_absl_random_distributions_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_random_distributions_FRAMEWORKS_RELEASE )
set(abseil_absl_random_distributions_DEPENDENCIES_RELEASE absl::base_internal absl::config absl::core_headers absl::random_internal_generate_real absl::random_internal_distribution_caller absl::random_internal_fast_uniform_bits absl::random_internal_fastmath absl::random_internal_iostream_state_saver absl::random_internal_traits absl::random_internal_uniform_helper absl::random_internal_wide_multiply absl::strings absl::type_traits)
set(abseil_absl_random_distributions_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_random_distributions_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_random_distributions_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_random_distributions_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_random_distributions_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_random_distributions_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_random_distributions_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_random_distributions_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_random_distributions_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_random_distributions_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::log_internal_container VARIABLES ############################################

set(abseil_absl_log_internal_container_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_log_internal_container_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_log_internal_container_BIN_DIRS_RELEASE )
set(abseil_absl_log_internal_container_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_log_internal_container_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_log_internal_container_RES_DIRS_RELEASE )
set(abseil_absl_log_internal_container_DEFINITIONS_RELEASE )
set(abseil_absl_log_internal_container_OBJECTS_RELEASE )
set(abseil_absl_log_internal_container_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_log_internal_container_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_log_internal_container_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_log_internal_container_LIBS_RELEASE )
set(abseil_absl_log_internal_container_SYSTEM_LIBS_RELEASE )
set(abseil_absl_log_internal_container_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_log_internal_container_FRAMEWORKS_RELEASE )
set(abseil_absl_log_internal_container_DEPENDENCIES_RELEASE absl::config absl::requires_internal absl::strings)
set(abseil_absl_log_internal_container_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_log_internal_container_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_log_internal_container_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_log_internal_container_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_log_internal_container_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_log_internal_container_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_log_internal_container_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_log_internal_container_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_log_internal_container_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_log_internal_container_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::log_internal_fnmatch VARIABLES ############################################

set(abseil_absl_log_internal_fnmatch_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_log_internal_fnmatch_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_log_internal_fnmatch_BIN_DIRS_RELEASE )
set(abseil_absl_log_internal_fnmatch_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_log_internal_fnmatch_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_log_internal_fnmatch_RES_DIRS_RELEASE )
set(abseil_absl_log_internal_fnmatch_DEFINITIONS_RELEASE )
set(abseil_absl_log_internal_fnmatch_OBJECTS_RELEASE )
set(abseil_absl_log_internal_fnmatch_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_log_internal_fnmatch_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_log_internal_fnmatch_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_log_internal_fnmatch_LIBS_RELEASE absl_log_internal_fnmatch)
set(abseil_absl_log_internal_fnmatch_SYSTEM_LIBS_RELEASE )
set(abseil_absl_log_internal_fnmatch_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_log_internal_fnmatch_FRAMEWORKS_RELEASE )
set(abseil_absl_log_internal_fnmatch_DEPENDENCIES_RELEASE absl::config absl::strings)
set(abseil_absl_log_internal_fnmatch_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_log_internal_fnmatch_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_log_internal_fnmatch_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_log_internal_fnmatch_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_log_internal_fnmatch_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_log_internal_fnmatch_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_log_internal_fnmatch_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_log_internal_fnmatch_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_log_internal_fnmatch_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_log_internal_fnmatch_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::log_internal_append_truncated VARIABLES ############################################

set(abseil_absl_log_internal_append_truncated_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_log_internal_append_truncated_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_log_internal_append_truncated_BIN_DIRS_RELEASE )
set(abseil_absl_log_internal_append_truncated_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_log_internal_append_truncated_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_log_internal_append_truncated_RES_DIRS_RELEASE )
set(abseil_absl_log_internal_append_truncated_DEFINITIONS_RELEASE )
set(abseil_absl_log_internal_append_truncated_OBJECTS_RELEASE )
set(abseil_absl_log_internal_append_truncated_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_log_internal_append_truncated_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_log_internal_append_truncated_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_log_internal_append_truncated_LIBS_RELEASE )
set(abseil_absl_log_internal_append_truncated_SYSTEM_LIBS_RELEASE )
set(abseil_absl_log_internal_append_truncated_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_log_internal_append_truncated_FRAMEWORKS_RELEASE )
set(abseil_absl_log_internal_append_truncated_DEPENDENCIES_RELEASE absl::config absl::strings absl::strings_internal absl::span)
set(abseil_absl_log_internal_append_truncated_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_log_internal_append_truncated_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_log_internal_append_truncated_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_log_internal_append_truncated_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_log_internal_append_truncated_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_log_internal_append_truncated_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_log_internal_append_truncated_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_log_internal_append_truncated_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_log_internal_append_truncated_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_log_internal_append_truncated_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::log_internal_nullstream VARIABLES ############################################

set(abseil_absl_log_internal_nullstream_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_log_internal_nullstream_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_log_internal_nullstream_BIN_DIRS_RELEASE )
set(abseil_absl_log_internal_nullstream_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_log_internal_nullstream_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_log_internal_nullstream_RES_DIRS_RELEASE )
set(abseil_absl_log_internal_nullstream_DEFINITIONS_RELEASE )
set(abseil_absl_log_internal_nullstream_OBJECTS_RELEASE )
set(abseil_absl_log_internal_nullstream_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_log_internal_nullstream_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_log_internal_nullstream_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_log_internal_nullstream_LIBS_RELEASE )
set(abseil_absl_log_internal_nullstream_SYSTEM_LIBS_RELEASE )
set(abseil_absl_log_internal_nullstream_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_log_internal_nullstream_FRAMEWORKS_RELEASE )
set(abseil_absl_log_internal_nullstream_DEPENDENCIES_RELEASE absl::config absl::core_headers absl::log_severity absl::strings)
set(abseil_absl_log_internal_nullstream_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_log_internal_nullstream_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_log_internal_nullstream_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_log_internal_nullstream_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_log_internal_nullstream_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_log_internal_nullstream_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_log_internal_nullstream_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_log_internal_nullstream_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_log_internal_nullstream_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_log_internal_nullstream_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::log_internal_proto VARIABLES ############################################

set(abseil_absl_log_internal_proto_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_log_internal_proto_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_log_internal_proto_BIN_DIRS_RELEASE )
set(abseil_absl_log_internal_proto_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_log_internal_proto_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_log_internal_proto_RES_DIRS_RELEASE )
set(abseil_absl_log_internal_proto_DEFINITIONS_RELEASE )
set(abseil_absl_log_internal_proto_OBJECTS_RELEASE )
set(abseil_absl_log_internal_proto_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_log_internal_proto_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_log_internal_proto_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_log_internal_proto_LIBS_RELEASE absl_log_internal_proto)
set(abseil_absl_log_internal_proto_SYSTEM_LIBS_RELEASE )
set(abseil_absl_log_internal_proto_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_log_internal_proto_FRAMEWORKS_RELEASE )
set(abseil_absl_log_internal_proto_DEPENDENCIES_RELEASE absl::base absl::config absl::core_headers absl::strings absl::span)
set(abseil_absl_log_internal_proto_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_log_internal_proto_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_log_internal_proto_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_log_internal_proto_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_log_internal_proto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_log_internal_proto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_log_internal_proto_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_log_internal_proto_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_log_internal_proto_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_log_internal_proto_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::hash VARIABLES ############################################

set(abseil_absl_hash_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_hash_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_hash_BIN_DIRS_RELEASE )
set(abseil_absl_hash_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_hash_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_hash_RES_DIRS_RELEASE )
set(abseil_absl_hash_DEFINITIONS_RELEASE )
set(abseil_absl_hash_OBJECTS_RELEASE )
set(abseil_absl_hash_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_hash_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_hash_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_hash_LIBS_RELEASE absl_hash)
set(abseil_absl_hash_SYSTEM_LIBS_RELEASE )
set(abseil_absl_hash_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_hash_FRAMEWORKS_RELEASE )
set(abseil_absl_hash_DEPENDENCIES_RELEASE absl::bits absl::city absl::config absl::core_headers absl::endian absl::fixed_array absl::function_ref absl::meta absl::int128 absl::strings absl::optional absl::variant absl::utility absl::weakly_mixed_integer)
set(abseil_absl_hash_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_hash_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_hash_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_hash_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_hash_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_hash_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_hash_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_hash_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_hash_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_hash_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::flags_commandlineflag VARIABLES ############################################

set(abseil_absl_flags_commandlineflag_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_flags_commandlineflag_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_flags_commandlineflag_BIN_DIRS_RELEASE )
set(abseil_absl_flags_commandlineflag_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_flags_commandlineflag_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_flags_commandlineflag_RES_DIRS_RELEASE )
set(abseil_absl_flags_commandlineflag_DEFINITIONS_RELEASE )
set(abseil_absl_flags_commandlineflag_OBJECTS_RELEASE )
set(abseil_absl_flags_commandlineflag_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_flags_commandlineflag_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_flags_commandlineflag_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_flags_commandlineflag_LIBS_RELEASE absl_flags_commandlineflag)
set(abseil_absl_flags_commandlineflag_SYSTEM_LIBS_RELEASE )
set(abseil_absl_flags_commandlineflag_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_flags_commandlineflag_FRAMEWORKS_RELEASE )
set(abseil_absl_flags_commandlineflag_DEPENDENCIES_RELEASE absl::config absl::fast_type_id absl::flags_commandlineflag_internal absl::optional absl::strings)
set(abseil_absl_flags_commandlineflag_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_flags_commandlineflag_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_flags_commandlineflag_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_flags_commandlineflag_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_flags_commandlineflag_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_flags_commandlineflag_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_flags_commandlineflag_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_flags_commandlineflag_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_flags_commandlineflag_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_flags_commandlineflag_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::flags_path_util VARIABLES ############################################

set(abseil_absl_flags_path_util_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_flags_path_util_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_flags_path_util_BIN_DIRS_RELEASE )
set(abseil_absl_flags_path_util_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_flags_path_util_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_flags_path_util_RES_DIRS_RELEASE )
set(abseil_absl_flags_path_util_DEFINITIONS_RELEASE )
set(abseil_absl_flags_path_util_OBJECTS_RELEASE )
set(abseil_absl_flags_path_util_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_flags_path_util_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_flags_path_util_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_flags_path_util_LIBS_RELEASE )
set(abseil_absl_flags_path_util_SYSTEM_LIBS_RELEASE )
set(abseil_absl_flags_path_util_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_flags_path_util_FRAMEWORKS_RELEASE )
set(abseil_absl_flags_path_util_DEPENDENCIES_RELEASE absl::config absl::strings)
set(abseil_absl_flags_path_util_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_flags_path_util_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_flags_path_util_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_flags_path_util_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_flags_path_util_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_flags_path_util_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_flags_path_util_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_flags_path_util_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_flags_path_util_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_flags_path_util_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::demangle_rust VARIABLES ############################################

set(abseil_absl_demangle_rust_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_demangle_rust_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_demangle_rust_BIN_DIRS_RELEASE )
set(abseil_absl_demangle_rust_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_demangle_rust_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_demangle_rust_RES_DIRS_RELEASE )
set(abseil_absl_demangle_rust_DEFINITIONS_RELEASE )
set(abseil_absl_demangle_rust_OBJECTS_RELEASE )
set(abseil_absl_demangle_rust_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_demangle_rust_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_demangle_rust_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_demangle_rust_LIBS_RELEASE absl_demangle_rust)
set(abseil_absl_demangle_rust_SYSTEM_LIBS_RELEASE )
set(abseil_absl_demangle_rust_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_demangle_rust_FRAMEWORKS_RELEASE )
set(abseil_absl_demangle_rust_DEPENDENCIES_RELEASE absl::config absl::core_headers absl::decode_rust_punycode)
set(abseil_absl_demangle_rust_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_demangle_rust_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_demangle_rust_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_demangle_rust_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_demangle_rust_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_demangle_rust_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_demangle_rust_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_demangle_rust_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_demangle_rust_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_demangle_rust_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::layout VARIABLES ############################################

set(abseil_absl_layout_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_layout_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_layout_BIN_DIRS_RELEASE )
set(abseil_absl_layout_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_layout_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_layout_RES_DIRS_RELEASE )
set(abseil_absl_layout_DEFINITIONS_RELEASE )
set(abseil_absl_layout_OBJECTS_RELEASE )
set(abseil_absl_layout_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_layout_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_layout_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_layout_LIBS_RELEASE )
set(abseil_absl_layout_SYSTEM_LIBS_RELEASE )
set(abseil_absl_layout_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_layout_FRAMEWORKS_RELEASE )
set(abseil_absl_layout_DEPENDENCIES_RELEASE absl::config absl::core_headers absl::debugging_internal absl::meta absl::strings absl::span absl::utility)
set(abseil_absl_layout_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_layout_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_layout_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_layout_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_layout_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_layout_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_layout_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_layout_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_layout_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_layout_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::strings VARIABLES ############################################

set(abseil_absl_strings_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_strings_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_strings_BIN_DIRS_RELEASE )
set(abseil_absl_strings_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_strings_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_strings_RES_DIRS_RELEASE )
set(abseil_absl_strings_DEFINITIONS_RELEASE )
set(abseil_absl_strings_OBJECTS_RELEASE )
set(abseil_absl_strings_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_strings_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_strings_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_strings_LIBS_RELEASE absl_strings)
set(abseil_absl_strings_SYSTEM_LIBS_RELEASE )
set(abseil_absl_strings_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_strings_FRAMEWORKS_RELEASE )
set(abseil_absl_strings_DEPENDENCIES_RELEASE absl::string_view absl::strings_append_and_overwrite absl::strings_internal absl::strings_resize_and_overwrite absl::base absl::bits absl::charset absl::config absl::core_headers absl::endian absl::int128 absl::iterator_traits_internal absl::memory absl::nullability absl::raw_logging_internal absl::throw_delegate absl::type_traits)
set(abseil_absl_strings_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_strings_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_strings_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_strings_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_strings_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_strings_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_strings_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_strings_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_strings_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_strings_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::random_internal_wide_multiply VARIABLES ############################################

set(abseil_absl_random_internal_wide_multiply_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_random_internal_wide_multiply_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_random_internal_wide_multiply_BIN_DIRS_RELEASE )
set(abseil_absl_random_internal_wide_multiply_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_random_internal_wide_multiply_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_random_internal_wide_multiply_RES_DIRS_RELEASE )
set(abseil_absl_random_internal_wide_multiply_DEFINITIONS_RELEASE )
set(abseil_absl_random_internal_wide_multiply_OBJECTS_RELEASE )
set(abseil_absl_random_internal_wide_multiply_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_random_internal_wide_multiply_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_random_internal_wide_multiply_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_random_internal_wide_multiply_LIBS_RELEASE )
set(abseil_absl_random_internal_wide_multiply_SYSTEM_LIBS_RELEASE )
set(abseil_absl_random_internal_wide_multiply_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_random_internal_wide_multiply_FRAMEWORKS_RELEASE )
set(abseil_absl_random_internal_wide_multiply_DEPENDENCIES_RELEASE absl::bits absl::config absl::int128)
set(abseil_absl_random_internal_wide_multiply_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_random_internal_wide_multiply_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_random_internal_wide_multiply_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_random_internal_wide_multiply_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_random_internal_wide_multiply_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_random_internal_wide_multiply_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_random_internal_wide_multiply_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_random_internal_wide_multiply_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_random_internal_wide_multiply_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_random_internal_wide_multiply_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::random_internal_generate_real VARIABLES ############################################

set(abseil_absl_random_internal_generate_real_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_random_internal_generate_real_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_random_internal_generate_real_BIN_DIRS_RELEASE )
set(abseil_absl_random_internal_generate_real_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_random_internal_generate_real_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_random_internal_generate_real_RES_DIRS_RELEASE )
set(abseil_absl_random_internal_generate_real_DEFINITIONS_RELEASE )
set(abseil_absl_random_internal_generate_real_OBJECTS_RELEASE )
set(abseil_absl_random_internal_generate_real_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_random_internal_generate_real_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_random_internal_generate_real_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_random_internal_generate_real_LIBS_RELEASE )
set(abseil_absl_random_internal_generate_real_SYSTEM_LIBS_RELEASE )
set(abseil_absl_random_internal_generate_real_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_random_internal_generate_real_FRAMEWORKS_RELEASE )
set(abseil_absl_random_internal_generate_real_DEPENDENCIES_RELEASE absl::bits absl::random_internal_fastmath absl::random_internal_traits absl::type_traits)
set(abseil_absl_random_internal_generate_real_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_random_internal_generate_real_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_random_internal_generate_real_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_random_internal_generate_real_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_random_internal_generate_real_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_random_internal_generate_real_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_random_internal_generate_real_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_random_internal_generate_real_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_random_internal_generate_real_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_random_internal_generate_real_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::random_internal_iostream_state_saver VARIABLES ############################################

set(abseil_absl_random_internal_iostream_state_saver_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_random_internal_iostream_state_saver_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_random_internal_iostream_state_saver_BIN_DIRS_RELEASE )
set(abseil_absl_random_internal_iostream_state_saver_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_random_internal_iostream_state_saver_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_random_internal_iostream_state_saver_RES_DIRS_RELEASE )
set(abseil_absl_random_internal_iostream_state_saver_DEFINITIONS_RELEASE )
set(abseil_absl_random_internal_iostream_state_saver_OBJECTS_RELEASE )
set(abseil_absl_random_internal_iostream_state_saver_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_random_internal_iostream_state_saver_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_random_internal_iostream_state_saver_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_random_internal_iostream_state_saver_LIBS_RELEASE )
set(abseil_absl_random_internal_iostream_state_saver_SYSTEM_LIBS_RELEASE )
set(abseil_absl_random_internal_iostream_state_saver_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_random_internal_iostream_state_saver_FRAMEWORKS_RELEASE )
set(abseil_absl_random_internal_iostream_state_saver_DEPENDENCIES_RELEASE absl::config absl::int128 absl::type_traits)
set(abseil_absl_random_internal_iostream_state_saver_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_random_internal_iostream_state_saver_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_random_internal_iostream_state_saver_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_random_internal_iostream_state_saver_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_random_internal_iostream_state_saver_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_random_internal_iostream_state_saver_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_random_internal_iostream_state_saver_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_random_internal_iostream_state_saver_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_random_internal_iostream_state_saver_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_random_internal_iostream_state_saver_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::numeric VARIABLES ############################################

set(abseil_absl_numeric_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_numeric_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_numeric_BIN_DIRS_RELEASE )
set(abseil_absl_numeric_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_numeric_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_numeric_RES_DIRS_RELEASE )
set(abseil_absl_numeric_DEFINITIONS_RELEASE )
set(abseil_absl_numeric_OBJECTS_RELEASE )
set(abseil_absl_numeric_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_numeric_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_numeric_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_numeric_LIBS_RELEASE )
set(abseil_absl_numeric_SYSTEM_LIBS_RELEASE )
set(abseil_absl_numeric_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_numeric_FRAMEWORKS_RELEASE )
set(abseil_absl_numeric_DEPENDENCIES_RELEASE absl::int128)
set(abseil_absl_numeric_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_numeric_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_numeric_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_numeric_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_numeric_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_numeric_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_numeric_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_numeric_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_numeric_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_numeric_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::decode_rust_punycode VARIABLES ############################################

set(abseil_absl_decode_rust_punycode_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_decode_rust_punycode_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_decode_rust_punycode_BIN_DIRS_RELEASE )
set(abseil_absl_decode_rust_punycode_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_decode_rust_punycode_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_decode_rust_punycode_RES_DIRS_RELEASE )
set(abseil_absl_decode_rust_punycode_DEFINITIONS_RELEASE )
set(abseil_absl_decode_rust_punycode_OBJECTS_RELEASE )
set(abseil_absl_decode_rust_punycode_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_decode_rust_punycode_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_decode_rust_punycode_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_decode_rust_punycode_LIBS_RELEASE absl_decode_rust_punycode)
set(abseil_absl_decode_rust_punycode_SYSTEM_LIBS_RELEASE )
set(abseil_absl_decode_rust_punycode_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_decode_rust_punycode_FRAMEWORKS_RELEASE )
set(abseil_absl_decode_rust_punycode_DEPENDENCIES_RELEASE absl::bounded_utf8_length_sequence absl::config absl::nullability absl::utf8_for_code_point)
set(abseil_absl_decode_rust_punycode_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_decode_rust_punycode_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_decode_rust_punycode_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_decode_rust_punycode_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_decode_rust_punycode_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_decode_rust_punycode_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_decode_rust_punycode_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_decode_rust_punycode_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_decode_rust_punycode_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_decode_rust_punycode_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::random_internal_fastmath VARIABLES ############################################

set(abseil_absl_random_internal_fastmath_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_random_internal_fastmath_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_random_internal_fastmath_BIN_DIRS_RELEASE )
set(abseil_absl_random_internal_fastmath_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_random_internal_fastmath_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_random_internal_fastmath_RES_DIRS_RELEASE )
set(abseil_absl_random_internal_fastmath_DEFINITIONS_RELEASE )
set(abseil_absl_random_internal_fastmath_OBJECTS_RELEASE )
set(abseil_absl_random_internal_fastmath_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_random_internal_fastmath_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_random_internal_fastmath_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_random_internal_fastmath_LIBS_RELEASE )
set(abseil_absl_random_internal_fastmath_SYSTEM_LIBS_RELEASE )
set(abseil_absl_random_internal_fastmath_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_random_internal_fastmath_FRAMEWORKS_RELEASE )
set(abseil_absl_random_internal_fastmath_DEPENDENCIES_RELEASE absl::bits)
set(abseil_absl_random_internal_fastmath_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_random_internal_fastmath_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_random_internal_fastmath_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_random_internal_fastmath_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_random_internal_fastmath_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_random_internal_fastmath_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_random_internal_fastmath_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_random_internal_fastmath_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_random_internal_fastmath_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_random_internal_fastmath_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::int128 VARIABLES ############################################

set(abseil_absl_int128_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_int128_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_int128_BIN_DIRS_RELEASE )
set(abseil_absl_int128_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_int128_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_int128_RES_DIRS_RELEASE )
set(abseil_absl_int128_DEFINITIONS_RELEASE )
set(abseil_absl_int128_OBJECTS_RELEASE )
set(abseil_absl_int128_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_int128_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_int128_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_int128_LIBS_RELEASE absl_int128)
set(abseil_absl_int128_SYSTEM_LIBS_RELEASE )
set(abseil_absl_int128_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_int128_FRAMEWORKS_RELEASE )
set(abseil_absl_int128_DEPENDENCIES_RELEASE absl::compare absl::config absl::core_headers absl::bits)
set(abseil_absl_int128_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_int128_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_int128_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_int128_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_int128_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_int128_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_int128_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_int128_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_int128_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_int128_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::bounded_utf8_length_sequence VARIABLES ############################################

set(abseil_absl_bounded_utf8_length_sequence_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_bounded_utf8_length_sequence_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_bounded_utf8_length_sequence_BIN_DIRS_RELEASE )
set(abseil_absl_bounded_utf8_length_sequence_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_bounded_utf8_length_sequence_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_bounded_utf8_length_sequence_RES_DIRS_RELEASE )
set(abseil_absl_bounded_utf8_length_sequence_DEFINITIONS_RELEASE )
set(abseil_absl_bounded_utf8_length_sequence_OBJECTS_RELEASE )
set(abseil_absl_bounded_utf8_length_sequence_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_bounded_utf8_length_sequence_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_bounded_utf8_length_sequence_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_bounded_utf8_length_sequence_LIBS_RELEASE )
set(abseil_absl_bounded_utf8_length_sequence_SYSTEM_LIBS_RELEASE )
set(abseil_absl_bounded_utf8_length_sequence_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_bounded_utf8_length_sequence_FRAMEWORKS_RELEASE )
set(abseil_absl_bounded_utf8_length_sequence_DEPENDENCIES_RELEASE absl::bits absl::config)
set(abseil_absl_bounded_utf8_length_sequence_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_bounded_utf8_length_sequence_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_bounded_utf8_length_sequence_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_bounded_utf8_length_sequence_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_bounded_utf8_length_sequence_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_bounded_utf8_length_sequence_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_bounded_utf8_length_sequence_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_bounded_utf8_length_sequence_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_bounded_utf8_length_sequence_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_bounded_utf8_length_sequence_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::crc_internal VARIABLES ############################################

set(abseil_absl_crc_internal_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_crc_internal_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_crc_internal_BIN_DIRS_RELEASE )
set(abseil_absl_crc_internal_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_crc_internal_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_crc_internal_RES_DIRS_RELEASE )
set(abseil_absl_crc_internal_DEFINITIONS_RELEASE )
set(abseil_absl_crc_internal_OBJECTS_RELEASE )
set(abseil_absl_crc_internal_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_crc_internal_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_crc_internal_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_crc_internal_LIBS_RELEASE absl_crc_internal)
set(abseil_absl_crc_internal_SYSTEM_LIBS_RELEASE )
set(abseil_absl_crc_internal_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_crc_internal_FRAMEWORKS_RELEASE )
set(abseil_absl_crc_internal_DEPENDENCIES_RELEASE absl::crc_cpu_detect absl::config absl::core_headers absl::endian absl::prefetch absl::raw_logging_internal absl::memory absl::bits)
set(abseil_absl_crc_internal_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_crc_internal_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_crc_internal_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_crc_internal_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_crc_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_crc_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_crc_internal_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_crc_internal_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_crc_internal_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_crc_internal_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::hashtable_control_bytes VARIABLES ############################################

set(abseil_absl_hashtable_control_bytes_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_hashtable_control_bytes_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_hashtable_control_bytes_BIN_DIRS_RELEASE )
set(abseil_absl_hashtable_control_bytes_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_hashtable_control_bytes_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_hashtable_control_bytes_RES_DIRS_RELEASE )
set(abseil_absl_hashtable_control_bytes_DEFINITIONS_RELEASE )
set(abseil_absl_hashtable_control_bytes_OBJECTS_RELEASE )
set(abseil_absl_hashtable_control_bytes_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_hashtable_control_bytes_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_hashtable_control_bytes_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_hashtable_control_bytes_LIBS_RELEASE )
set(abseil_absl_hashtable_control_bytes_SYSTEM_LIBS_RELEASE )
set(abseil_absl_hashtable_control_bytes_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_hashtable_control_bytes_FRAMEWORKS_RELEASE )
set(abseil_absl_hashtable_control_bytes_DEPENDENCIES_RELEASE absl::bits absl::config absl::core_headers absl::endian)
set(abseil_absl_hashtable_control_bytes_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_hashtable_control_bytes_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_hashtable_control_bytes_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_hashtable_control_bytes_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_hashtable_control_bytes_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_hashtable_control_bytes_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_hashtable_control_bytes_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_hashtable_control_bytes_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_hashtable_control_bytes_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_hashtable_control_bytes_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::graphcycles_internal VARIABLES ############################################

set(abseil_absl_graphcycles_internal_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_graphcycles_internal_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_graphcycles_internal_BIN_DIRS_RELEASE )
set(abseil_absl_graphcycles_internal_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_graphcycles_internal_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_graphcycles_internal_RES_DIRS_RELEASE )
set(abseil_absl_graphcycles_internal_DEFINITIONS_RELEASE )
set(abseil_absl_graphcycles_internal_OBJECTS_RELEASE )
set(abseil_absl_graphcycles_internal_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_graphcycles_internal_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_graphcycles_internal_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_graphcycles_internal_LIBS_RELEASE absl_graphcycles_internal)
set(abseil_absl_graphcycles_internal_SYSTEM_LIBS_RELEASE )
set(abseil_absl_graphcycles_internal_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_graphcycles_internal_FRAMEWORKS_RELEASE )
set(abseil_absl_graphcycles_internal_DEPENDENCIES_RELEASE absl::base absl::base_internal absl::config absl::core_headers absl::malloc_internal absl::raw_logging_internal)
set(abseil_absl_graphcycles_internal_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_graphcycles_internal_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_graphcycles_internal_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_graphcycles_internal_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_graphcycles_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_graphcycles_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_graphcycles_internal_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_graphcycles_internal_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_graphcycles_internal_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_graphcycles_internal_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::strings_internal VARIABLES ############################################

set(abseil_absl_strings_internal_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_strings_internal_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_strings_internal_BIN_DIRS_RELEASE )
set(abseil_absl_strings_internal_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_strings_internal_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_strings_internal_RES_DIRS_RELEASE )
set(abseil_absl_strings_internal_DEFINITIONS_RELEASE )
set(abseil_absl_strings_internal_OBJECTS_RELEASE )
set(abseil_absl_strings_internal_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_strings_internal_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_strings_internal_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_strings_internal_LIBS_RELEASE absl_strings_internal)
set(abseil_absl_strings_internal_SYSTEM_LIBS_RELEASE )
set(abseil_absl_strings_internal_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_strings_internal_FRAMEWORKS_RELEASE )
set(abseil_absl_strings_internal_DEPENDENCIES_RELEASE absl::config absl::core_headers absl::endian absl::raw_logging_internal absl::strings_resize_and_overwrite absl::type_traits)
set(abseil_absl_strings_internal_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_strings_internal_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_strings_internal_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_strings_internal_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_strings_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_strings_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_strings_internal_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_strings_internal_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_strings_internal_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_strings_internal_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::bits VARIABLES ############################################

set(abseil_absl_bits_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_bits_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_bits_BIN_DIRS_RELEASE )
set(abseil_absl_bits_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_bits_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_bits_RES_DIRS_RELEASE )
set(abseil_absl_bits_DEFINITIONS_RELEASE )
set(abseil_absl_bits_OBJECTS_RELEASE )
set(abseil_absl_bits_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_bits_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_bits_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_bits_LIBS_RELEASE )
set(abseil_absl_bits_SYSTEM_LIBS_RELEASE )
set(abseil_absl_bits_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_bits_FRAMEWORKS_RELEASE )
set(abseil_absl_bits_DEPENDENCIES_RELEASE absl::config absl::core_headers absl::endian)
set(abseil_absl_bits_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_bits_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_bits_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_bits_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_bits_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_bits_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_bits_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_bits_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_bits_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_bits_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::city VARIABLES ############################################

set(abseil_absl_city_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_city_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_city_BIN_DIRS_RELEASE )
set(abseil_absl_city_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_city_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_city_RES_DIRS_RELEASE )
set(abseil_absl_city_DEFINITIONS_RELEASE )
set(abseil_absl_city_OBJECTS_RELEASE )
set(abseil_absl_city_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_city_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_city_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_city_LIBS_RELEASE absl_city)
set(abseil_absl_city_SYSTEM_LIBS_RELEASE )
set(abseil_absl_city_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_city_FRAMEWORKS_RELEASE )
set(abseil_absl_city_DEPENDENCIES_RELEASE absl::config absl::core_headers absl::endian)
set(abseil_absl_city_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_city_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_city_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_city_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_city_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_city_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_city_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_city_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_city_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_city_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::function_ref VARIABLES ############################################

set(abseil_absl_function_ref_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_function_ref_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_function_ref_BIN_DIRS_RELEASE )
set(abseil_absl_function_ref_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_function_ref_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_function_ref_RES_DIRS_RELEASE )
set(abseil_absl_function_ref_DEFINITIONS_RELEASE )
set(abseil_absl_function_ref_OBJECTS_RELEASE )
set(abseil_absl_function_ref_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_function_ref_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_function_ref_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_function_ref_LIBS_RELEASE )
set(abseil_absl_function_ref_SYSTEM_LIBS_RELEASE )
set(abseil_absl_function_ref_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_function_ref_FRAMEWORKS_RELEASE )
set(abseil_absl_function_ref_DEPENDENCIES_RELEASE absl::config absl::core_headers absl::any_invocable absl::meta absl::utility)
set(abseil_absl_function_ref_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_function_ref_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_function_ref_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_function_ref_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_function_ref_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_function_ref_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_function_ref_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_function_ref_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_function_ref_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_function_ref_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::inlined_vector VARIABLES ############################################

set(abseil_absl_inlined_vector_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_inlined_vector_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_inlined_vector_BIN_DIRS_RELEASE )
set(abseil_absl_inlined_vector_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_inlined_vector_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_inlined_vector_RES_DIRS_RELEASE )
set(abseil_absl_inlined_vector_DEFINITIONS_RELEASE )
set(abseil_absl_inlined_vector_OBJECTS_RELEASE )
set(abseil_absl_inlined_vector_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_inlined_vector_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_inlined_vector_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_inlined_vector_LIBS_RELEASE )
set(abseil_absl_inlined_vector_SYSTEM_LIBS_RELEASE )
set(abseil_absl_inlined_vector_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_inlined_vector_FRAMEWORKS_RELEASE )
set(abseil_absl_inlined_vector_DEPENDENCIES_RELEASE absl::algorithm absl::core_headers absl::inlined_vector_internal absl::throw_delegate absl::memory absl::type_traits absl::weakly_mixed_integer)
set(abseil_absl_inlined_vector_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_inlined_vector_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_inlined_vector_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_inlined_vector_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_inlined_vector_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_inlined_vector_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_inlined_vector_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_inlined_vector_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_inlined_vector_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_inlined_vector_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::poison VARIABLES ############################################

set(abseil_absl_poison_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_poison_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_poison_BIN_DIRS_RELEASE )
set(abseil_absl_poison_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_poison_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_poison_RES_DIRS_RELEASE )
set(abseil_absl_poison_DEFINITIONS_RELEASE )
set(abseil_absl_poison_OBJECTS_RELEASE )
set(abseil_absl_poison_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_poison_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_poison_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_poison_LIBS_RELEASE absl_poison)
set(abseil_absl_poison_SYSTEM_LIBS_RELEASE )
set(abseil_absl_poison_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_poison_FRAMEWORKS_RELEASE )
set(abseil_absl_poison_DEPENDENCIES_RELEASE absl::config absl::core_headers absl::malloc_internal)
set(abseil_absl_poison_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_poison_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_poison_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_poison_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_poison_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_poison_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_poison_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_poison_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_poison_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_poison_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::strings_append_and_overwrite VARIABLES ############################################

set(abseil_absl_strings_append_and_overwrite_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_strings_append_and_overwrite_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_strings_append_and_overwrite_BIN_DIRS_RELEASE )
set(abseil_absl_strings_append_and_overwrite_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_strings_append_and_overwrite_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_strings_append_and_overwrite_RES_DIRS_RELEASE )
set(abseil_absl_strings_append_and_overwrite_DEFINITIONS_RELEASE )
set(abseil_absl_strings_append_and_overwrite_OBJECTS_RELEASE )
set(abseil_absl_strings_append_and_overwrite_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_strings_append_and_overwrite_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_strings_append_and_overwrite_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_strings_append_and_overwrite_LIBS_RELEASE )
set(abseil_absl_strings_append_and_overwrite_SYSTEM_LIBS_RELEASE )
set(abseil_absl_strings_append_and_overwrite_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_strings_append_and_overwrite_FRAMEWORKS_RELEASE )
set(abseil_absl_strings_append_and_overwrite_DEPENDENCIES_RELEASE absl::config absl::core_headers absl::strings_resize_and_overwrite absl::throw_delegate)
set(abseil_absl_strings_append_and_overwrite_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_strings_append_and_overwrite_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_strings_append_and_overwrite_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_strings_append_and_overwrite_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_strings_append_and_overwrite_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_strings_append_and_overwrite_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_strings_append_and_overwrite_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_strings_append_and_overwrite_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_strings_append_and_overwrite_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_strings_append_and_overwrite_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::random_internal_randen VARIABLES ############################################

set(abseil_absl_random_internal_randen_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_random_internal_randen_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_random_internal_randen_BIN_DIRS_RELEASE )
set(abseil_absl_random_internal_randen_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_random_internal_randen_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_random_internal_randen_RES_DIRS_RELEASE )
set(abseil_absl_random_internal_randen_DEFINITIONS_RELEASE )
set(abseil_absl_random_internal_randen_OBJECTS_RELEASE )
set(abseil_absl_random_internal_randen_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_random_internal_randen_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_random_internal_randen_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_random_internal_randen_LIBS_RELEASE absl_random_internal_randen)
set(abseil_absl_random_internal_randen_SYSTEM_LIBS_RELEASE )
set(abseil_absl_random_internal_randen_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_random_internal_randen_FRAMEWORKS_RELEASE )
set(abseil_absl_random_internal_randen_DEPENDENCIES_RELEASE absl::random_internal_platform absl::random_internal_randen_hwaes absl::random_internal_randen_slow)
set(abseil_absl_random_internal_randen_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_random_internal_randen_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_random_internal_randen_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_random_internal_randen_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_random_internal_randen_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_random_internal_randen_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_random_internal_randen_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_random_internal_randen_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_random_internal_randen_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_random_internal_randen_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::log_internal_conditions VARIABLES ############################################

set(abseil_absl_log_internal_conditions_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_log_internal_conditions_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_log_internal_conditions_BIN_DIRS_RELEASE )
set(abseil_absl_log_internal_conditions_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_log_internal_conditions_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_log_internal_conditions_RES_DIRS_RELEASE )
set(abseil_absl_log_internal_conditions_DEFINITIONS_RELEASE )
set(abseil_absl_log_internal_conditions_OBJECTS_RELEASE )
set(abseil_absl_log_internal_conditions_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_log_internal_conditions_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_log_internal_conditions_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_log_internal_conditions_LIBS_RELEASE absl_log_internal_conditions)
set(abseil_absl_log_internal_conditions_SYSTEM_LIBS_RELEASE )
set(abseil_absl_log_internal_conditions_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_log_internal_conditions_FRAMEWORKS_RELEASE )
set(abseil_absl_log_internal_conditions_DEPENDENCIES_RELEASE absl::base absl::config absl::core_headers absl::log_internal_voidify)
set(abseil_absl_log_internal_conditions_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_log_internal_conditions_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_log_internal_conditions_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_log_internal_conditions_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_log_internal_conditions_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_log_internal_conditions_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_log_internal_conditions_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_log_internal_conditions_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_log_internal_conditions_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_log_internal_conditions_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::any_invocable VARIABLES ############################################

set(abseil_absl_any_invocable_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_any_invocable_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_any_invocable_BIN_DIRS_RELEASE )
set(abseil_absl_any_invocable_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_any_invocable_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_any_invocable_RES_DIRS_RELEASE )
set(abseil_absl_any_invocable_DEFINITIONS_RELEASE )
set(abseil_absl_any_invocable_OBJECTS_RELEASE )
set(abseil_absl_any_invocable_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_any_invocable_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_any_invocable_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_any_invocable_LIBS_RELEASE )
set(abseil_absl_any_invocable_SYSTEM_LIBS_RELEASE )
set(abseil_absl_any_invocable_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_any_invocable_FRAMEWORKS_RELEASE )
set(abseil_absl_any_invocable_DEPENDENCIES_RELEASE absl::base absl::config absl::core_headers absl::type_traits absl::utility)
set(abseil_absl_any_invocable_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_any_invocable_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_any_invocable_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_any_invocable_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_any_invocable_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_any_invocable_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_any_invocable_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_any_invocable_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_any_invocable_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_any_invocable_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::crc_cpu_detect VARIABLES ############################################

set(abseil_absl_crc_cpu_detect_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_crc_cpu_detect_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_crc_cpu_detect_BIN_DIRS_RELEASE )
set(abseil_absl_crc_cpu_detect_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_crc_cpu_detect_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_crc_cpu_detect_RES_DIRS_RELEASE )
set(abseil_absl_crc_cpu_detect_DEFINITIONS_RELEASE )
set(abseil_absl_crc_cpu_detect_OBJECTS_RELEASE )
set(abseil_absl_crc_cpu_detect_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_crc_cpu_detect_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_crc_cpu_detect_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_crc_cpu_detect_LIBS_RELEASE absl_crc_cpu_detect)
set(abseil_absl_crc_cpu_detect_SYSTEM_LIBS_RELEASE )
set(abseil_absl_crc_cpu_detect_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_crc_cpu_detect_FRAMEWORKS_RELEASE )
set(abseil_absl_crc_cpu_detect_DEPENDENCIES_RELEASE absl::base absl::config absl::optional)
set(abseil_absl_crc_cpu_detect_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_crc_cpu_detect_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_crc_cpu_detect_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_crc_cpu_detect_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_crc_cpu_detect_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_crc_cpu_detect_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_crc_cpu_detect_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_crc_cpu_detect_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_crc_cpu_detect_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_crc_cpu_detect_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::inlined_vector_internal VARIABLES ############################################

set(abseil_absl_inlined_vector_internal_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_inlined_vector_internal_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_inlined_vector_internal_BIN_DIRS_RELEASE )
set(abseil_absl_inlined_vector_internal_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_inlined_vector_internal_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_inlined_vector_internal_RES_DIRS_RELEASE )
set(abseil_absl_inlined_vector_internal_DEFINITIONS_RELEASE )
set(abseil_absl_inlined_vector_internal_OBJECTS_RELEASE )
set(abseil_absl_inlined_vector_internal_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_inlined_vector_internal_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_inlined_vector_internal_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_inlined_vector_internal_LIBS_RELEASE )
set(abseil_absl_inlined_vector_internal_SYSTEM_LIBS_RELEASE )
set(abseil_absl_inlined_vector_internal_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_inlined_vector_internal_FRAMEWORKS_RELEASE )
set(abseil_absl_inlined_vector_internal_DEPENDENCIES_RELEASE absl::base_internal absl::compressed_tuple absl::config absl::core_headers absl::memory absl::span absl::type_traits)
set(abseil_absl_inlined_vector_internal_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_inlined_vector_internal_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_inlined_vector_internal_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_inlined_vector_internal_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_inlined_vector_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_inlined_vector_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_inlined_vector_internal_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_inlined_vector_internal_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_inlined_vector_internal_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_inlined_vector_internal_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::tracing_internal VARIABLES ############################################

set(abseil_absl_tracing_internal_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_tracing_internal_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_tracing_internal_BIN_DIRS_RELEASE )
set(abseil_absl_tracing_internal_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_tracing_internal_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_tracing_internal_RES_DIRS_RELEASE )
set(abseil_absl_tracing_internal_DEFINITIONS_RELEASE )
set(abseil_absl_tracing_internal_OBJECTS_RELEASE )
set(abseil_absl_tracing_internal_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_tracing_internal_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_tracing_internal_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_tracing_internal_LIBS_RELEASE absl_tracing_internal)
set(abseil_absl_tracing_internal_SYSTEM_LIBS_RELEASE )
set(abseil_absl_tracing_internal_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_tracing_internal_FRAMEWORKS_RELEASE )
set(abseil_absl_tracing_internal_DEPENDENCIES_RELEASE absl::base)
set(abseil_absl_tracing_internal_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_tracing_internal_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_tracing_internal_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_tracing_internal_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_tracing_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_tracing_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_tracing_internal_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_tracing_internal_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_tracing_internal_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_tracing_internal_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::endian VARIABLES ############################################

set(abseil_absl_endian_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_endian_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_endian_BIN_DIRS_RELEASE )
set(abseil_absl_endian_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_endian_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_endian_RES_DIRS_RELEASE )
set(abseil_absl_endian_DEFINITIONS_RELEASE )
set(abseil_absl_endian_OBJECTS_RELEASE )
set(abseil_absl_endian_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_endian_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_endian_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_endian_LIBS_RELEASE )
set(abseil_absl_endian_SYSTEM_LIBS_RELEASE )
set(abseil_absl_endian_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_endian_FRAMEWORKS_RELEASE )
set(abseil_absl_endian_DEPENDENCIES_RELEASE absl::base absl::config absl::core_headers absl::nullability)
set(abseil_absl_endian_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_endian_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_endian_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_endian_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_endian_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_endian_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_endian_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_endian_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_endian_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_endian_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::malloc_internal VARIABLES ############################################

set(abseil_absl_malloc_internal_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_malloc_internal_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_malloc_internal_BIN_DIRS_RELEASE )
set(abseil_absl_malloc_internal_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_malloc_internal_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_malloc_internal_RES_DIRS_RELEASE )
set(abseil_absl_malloc_internal_DEFINITIONS_RELEASE )
set(abseil_absl_malloc_internal_OBJECTS_RELEASE )
set(abseil_absl_malloc_internal_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_malloc_internal_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_malloc_internal_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_malloc_internal_LIBS_RELEASE absl_malloc_internal)
set(abseil_absl_malloc_internal_SYSTEM_LIBS_RELEASE pthread)
set(abseil_absl_malloc_internal_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_malloc_internal_FRAMEWORKS_RELEASE )
set(abseil_absl_malloc_internal_DEPENDENCIES_RELEASE absl::base absl::base_internal absl::config absl::core_headers absl::dynamic_annotations absl::raw_logging_internal)
set(abseil_absl_malloc_internal_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_malloc_internal_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_malloc_internal_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_malloc_internal_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_malloc_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_malloc_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_malloc_internal_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_malloc_internal_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_malloc_internal_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_malloc_internal_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::span VARIABLES ############################################

set(abseil_absl_span_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_span_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_span_BIN_DIRS_RELEASE )
set(abseil_absl_span_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_span_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_span_RES_DIRS_RELEASE )
set(abseil_absl_span_DEFINITIONS_RELEASE )
set(abseil_absl_span_OBJECTS_RELEASE )
set(abseil_absl_span_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_span_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_span_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_span_LIBS_RELEASE )
set(abseil_absl_span_SYSTEM_LIBS_RELEASE )
set(abseil_absl_span_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_span_FRAMEWORKS_RELEASE )
set(abseil_absl_span_DEPENDENCIES_RELEASE absl::algorithm absl::config absl::core_headers absl::nullability absl::throw_delegate absl::type_traits absl::weakly_mixed_integer)
set(abseil_absl_span_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_span_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_span_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_span_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_span_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_span_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_span_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_span_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_span_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_span_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::strings_resize_and_overwrite VARIABLES ############################################

set(abseil_absl_strings_resize_and_overwrite_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_strings_resize_and_overwrite_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_strings_resize_and_overwrite_BIN_DIRS_RELEASE )
set(abseil_absl_strings_resize_and_overwrite_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_strings_resize_and_overwrite_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_strings_resize_and_overwrite_RES_DIRS_RELEASE )
set(abseil_absl_strings_resize_and_overwrite_DEFINITIONS_RELEASE )
set(abseil_absl_strings_resize_and_overwrite_OBJECTS_RELEASE )
set(abseil_absl_strings_resize_and_overwrite_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_strings_resize_and_overwrite_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_strings_resize_and_overwrite_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_strings_resize_and_overwrite_LIBS_RELEASE )
set(abseil_absl_strings_resize_and_overwrite_SYSTEM_LIBS_RELEASE )
set(abseil_absl_strings_resize_and_overwrite_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_strings_resize_and_overwrite_FRAMEWORKS_RELEASE )
set(abseil_absl_strings_resize_and_overwrite_DEPENDENCIES_RELEASE absl::config absl::core_headers absl::dynamic_annotations absl::throw_delegate)
set(abseil_absl_strings_resize_and_overwrite_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_strings_resize_and_overwrite_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_strings_resize_and_overwrite_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_strings_resize_and_overwrite_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_strings_resize_and_overwrite_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_strings_resize_and_overwrite_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_strings_resize_and_overwrite_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_strings_resize_and_overwrite_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_strings_resize_and_overwrite_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_strings_resize_and_overwrite_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::random_internal_randen_hwaes VARIABLES ############################################

set(abseil_absl_random_internal_randen_hwaes_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_random_internal_randen_hwaes_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_random_internal_randen_hwaes_BIN_DIRS_RELEASE )
set(abseil_absl_random_internal_randen_hwaes_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_random_internal_randen_hwaes_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_random_internal_randen_hwaes_RES_DIRS_RELEASE )
set(abseil_absl_random_internal_randen_hwaes_DEFINITIONS_RELEASE )
set(abseil_absl_random_internal_randen_hwaes_OBJECTS_RELEASE )
set(abseil_absl_random_internal_randen_hwaes_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_random_internal_randen_hwaes_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_random_internal_randen_hwaes_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_random_internal_randen_hwaes_LIBS_RELEASE absl_random_internal_randen_hwaes)
set(abseil_absl_random_internal_randen_hwaes_SYSTEM_LIBS_RELEASE )
set(abseil_absl_random_internal_randen_hwaes_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_random_internal_randen_hwaes_FRAMEWORKS_RELEASE )
set(abseil_absl_random_internal_randen_hwaes_DEPENDENCIES_RELEASE absl::random_internal_platform absl::random_internal_randen_hwaes_impl absl::config absl::optional)
set(abseil_absl_random_internal_randen_hwaes_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_random_internal_randen_hwaes_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_random_internal_randen_hwaes_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_random_internal_randen_hwaes_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_random_internal_randen_hwaes_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_random_internal_randen_hwaes_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_random_internal_randen_hwaes_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_random_internal_randen_hwaes_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_random_internal_randen_hwaes_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_random_internal_randen_hwaes_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::random_internal_mock_helpers VARIABLES ############################################

set(abseil_absl_random_internal_mock_helpers_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_random_internal_mock_helpers_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_random_internal_mock_helpers_BIN_DIRS_RELEASE )
set(abseil_absl_random_internal_mock_helpers_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_random_internal_mock_helpers_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_random_internal_mock_helpers_RES_DIRS_RELEASE )
set(abseil_absl_random_internal_mock_helpers_DEFINITIONS_RELEASE )
set(abseil_absl_random_internal_mock_helpers_OBJECTS_RELEASE )
set(abseil_absl_random_internal_mock_helpers_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_random_internal_mock_helpers_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_random_internal_mock_helpers_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_random_internal_mock_helpers_LIBS_RELEASE )
set(abseil_absl_random_internal_mock_helpers_SYSTEM_LIBS_RELEASE )
set(abseil_absl_random_internal_mock_helpers_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_random_internal_mock_helpers_FRAMEWORKS_RELEASE )
set(abseil_absl_random_internal_mock_helpers_DEPENDENCIES_RELEASE absl::config absl::fast_type_id absl::optional)
set(abseil_absl_random_internal_mock_helpers_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_random_internal_mock_helpers_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_random_internal_mock_helpers_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_random_internal_mock_helpers_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_random_internal_mock_helpers_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_random_internal_mock_helpers_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_random_internal_mock_helpers_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_random_internal_mock_helpers_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_random_internal_mock_helpers_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_random_internal_mock_helpers_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::random_bit_gen_ref VARIABLES ############################################

set(abseil_absl_random_bit_gen_ref_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_random_bit_gen_ref_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_random_bit_gen_ref_BIN_DIRS_RELEASE )
set(abseil_absl_random_bit_gen_ref_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_random_bit_gen_ref_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_random_bit_gen_ref_RES_DIRS_RELEASE )
set(abseil_absl_random_bit_gen_ref_DEFINITIONS_RELEASE )
set(abseil_absl_random_bit_gen_ref_OBJECTS_RELEASE )
set(abseil_absl_random_bit_gen_ref_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_random_bit_gen_ref_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_random_bit_gen_ref_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_random_bit_gen_ref_LIBS_RELEASE )
set(abseil_absl_random_bit_gen_ref_SYSTEM_LIBS_RELEASE )
set(abseil_absl_random_bit_gen_ref_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_random_bit_gen_ref_FRAMEWORKS_RELEASE )
set(abseil_absl_random_bit_gen_ref_DEPENDENCIES_RELEASE absl::config absl::core_headers absl::random_internal_distribution_caller absl::random_internal_fast_uniform_bits absl::type_traits)
set(abseil_absl_random_bit_gen_ref_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_random_bit_gen_ref_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_random_bit_gen_ref_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_random_bit_gen_ref_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_random_bit_gen_ref_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_random_bit_gen_ref_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_random_bit_gen_ref_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_random_bit_gen_ref_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_random_bit_gen_ref_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_random_bit_gen_ref_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::bind_front VARIABLES ############################################

set(abseil_absl_bind_front_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_bind_front_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_bind_front_BIN_DIRS_RELEASE )
set(abseil_absl_bind_front_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_bind_front_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_bind_front_RES_DIRS_RELEASE )
set(abseil_absl_bind_front_DEFINITIONS_RELEASE )
set(abseil_absl_bind_front_OBJECTS_RELEASE )
set(abseil_absl_bind_front_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_bind_front_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_bind_front_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_bind_front_LIBS_RELEASE )
set(abseil_absl_bind_front_SYSTEM_LIBS_RELEASE )
set(abseil_absl_bind_front_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_bind_front_FRAMEWORKS_RELEASE )
set(abseil_absl_bind_front_DEPENDENCIES_RELEASE absl::compressed_tuple)
set(abseil_absl_bind_front_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_bind_front_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_bind_front_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_bind_front_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_bind_front_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_bind_front_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_bind_front_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_bind_front_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_bind_front_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_bind_front_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::fixed_array VARIABLES ############################################

set(abseil_absl_fixed_array_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_fixed_array_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_fixed_array_BIN_DIRS_RELEASE )
set(abseil_absl_fixed_array_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_fixed_array_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_fixed_array_RES_DIRS_RELEASE )
set(abseil_absl_fixed_array_DEFINITIONS_RELEASE )
set(abseil_absl_fixed_array_OBJECTS_RELEASE )
set(abseil_absl_fixed_array_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_fixed_array_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_fixed_array_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_fixed_array_LIBS_RELEASE )
set(abseil_absl_fixed_array_SYSTEM_LIBS_RELEASE )
set(abseil_absl_fixed_array_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_fixed_array_FRAMEWORKS_RELEASE )
set(abseil_absl_fixed_array_DEPENDENCIES_RELEASE absl::compressed_tuple absl::algorithm absl::config absl::core_headers absl::dynamic_annotations absl::iterator_traits_internal absl::throw_delegate absl::memory absl::weakly_mixed_integer)
set(abseil_absl_fixed_array_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_fixed_array_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_fixed_array_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_fixed_array_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_fixed_array_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_fixed_array_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_fixed_array_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_fixed_array_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_fixed_array_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_fixed_array_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::cleanup VARIABLES ############################################

set(abseil_absl_cleanup_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_cleanup_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_cleanup_BIN_DIRS_RELEASE )
set(abseil_absl_cleanup_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_cleanup_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_cleanup_RES_DIRS_RELEASE )
set(abseil_absl_cleanup_DEFINITIONS_RELEASE )
set(abseil_absl_cleanup_OBJECTS_RELEASE )
set(abseil_absl_cleanup_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_cleanup_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_cleanup_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_cleanup_LIBS_RELEASE )
set(abseil_absl_cleanup_SYSTEM_LIBS_RELEASE )
set(abseil_absl_cleanup_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_cleanup_FRAMEWORKS_RELEASE )
set(abseil_absl_cleanup_DEPENDENCIES_RELEASE absl::cleanup_internal absl::config absl::core_headers)
set(abseil_absl_cleanup_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_cleanup_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_cleanup_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_cleanup_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_cleanup_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_cleanup_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_cleanup_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_cleanup_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_cleanup_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_cleanup_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::base VARIABLES ############################################

set(abseil_absl_base_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_base_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_base_BIN_DIRS_RELEASE )
set(abseil_absl_base_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_base_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_base_RES_DIRS_RELEASE )
set(abseil_absl_base_DEFINITIONS_RELEASE )
set(abseil_absl_base_OBJECTS_RELEASE )
set(abseil_absl_base_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_base_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_base_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_base_LIBS_RELEASE absl_base)
set(abseil_absl_base_SYSTEM_LIBS_RELEASE pthread rt)
set(abseil_absl_base_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_base_FRAMEWORKS_RELEASE )
set(abseil_absl_base_DEPENDENCIES_RELEASE absl::atomic_hook absl::base_internal absl::config absl::core_headers absl::dynamic_annotations absl::log_severity absl::nullability absl::raw_logging_internal absl::spinlock_wait absl::type_traits)
set(abseil_absl_base_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_base_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_base_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_base_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_base_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_base_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_base_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_base_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_base_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_base_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::variant VARIABLES ############################################

set(abseil_absl_variant_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_variant_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_variant_BIN_DIRS_RELEASE )
set(abseil_absl_variant_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_variant_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_variant_RES_DIRS_RELEASE )
set(abseil_absl_variant_DEFINITIONS_RELEASE )
set(abseil_absl_variant_OBJECTS_RELEASE )
set(abseil_absl_variant_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_variant_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_variant_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_variant_LIBS_RELEASE )
set(abseil_absl_variant_SYSTEM_LIBS_RELEASE )
set(abseil_absl_variant_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_variant_FRAMEWORKS_RELEASE )
set(abseil_absl_variant_DEPENDENCIES_RELEASE absl::config absl::utility)
set(abseil_absl_variant_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_variant_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_variant_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_variant_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_variant_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_variant_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_variant_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_variant_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_variant_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_variant_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::optional VARIABLES ############################################

set(abseil_absl_optional_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_optional_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_optional_BIN_DIRS_RELEASE )
set(abseil_absl_optional_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_optional_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_optional_RES_DIRS_RELEASE )
set(abseil_absl_optional_DEFINITIONS_RELEASE )
set(abseil_absl_optional_OBJECTS_RELEASE )
set(abseil_absl_optional_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_optional_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_optional_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_optional_LIBS_RELEASE )
set(abseil_absl_optional_SYSTEM_LIBS_RELEASE )
set(abseil_absl_optional_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_optional_FRAMEWORKS_RELEASE )
set(abseil_absl_optional_DEPENDENCIES_RELEASE absl::config absl::utility)
set(abseil_absl_optional_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_optional_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_optional_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_optional_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_optional_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_optional_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_optional_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_optional_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_optional_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_optional_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::any VARIABLES ############################################

set(abseil_absl_any_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_any_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_any_BIN_DIRS_RELEASE )
set(abseil_absl_any_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_any_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_any_RES_DIRS_RELEASE )
set(abseil_absl_any_DEFINITIONS_RELEASE )
set(abseil_absl_any_OBJECTS_RELEASE )
set(abseil_absl_any_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_any_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_any_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_any_LIBS_RELEASE )
set(abseil_absl_any_SYSTEM_LIBS_RELEASE )
set(abseil_absl_any_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_any_FRAMEWORKS_RELEASE )
set(abseil_absl_any_DEPENDENCIES_RELEASE absl::config absl::core_headers absl::utility)
set(abseil_absl_any_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_any_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_any_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_any_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_any_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_any_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_any_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_any_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_any_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_any_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::cordz_functions VARIABLES ############################################

set(abseil_absl_cordz_functions_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_cordz_functions_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_cordz_functions_BIN_DIRS_RELEASE )
set(abseil_absl_cordz_functions_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_cordz_functions_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_cordz_functions_RES_DIRS_RELEASE )
set(abseil_absl_cordz_functions_DEFINITIONS_RELEASE )
set(abseil_absl_cordz_functions_OBJECTS_RELEASE )
set(abseil_absl_cordz_functions_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_cordz_functions_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_cordz_functions_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_cordz_functions_LIBS_RELEASE absl_cordz_functions)
set(abseil_absl_cordz_functions_SYSTEM_LIBS_RELEASE )
set(abseil_absl_cordz_functions_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_cordz_functions_FRAMEWORKS_RELEASE )
set(abseil_absl_cordz_functions_DEPENDENCIES_RELEASE absl::config absl::core_headers absl::exponential_biased absl::raw_logging_internal)
set(abseil_absl_cordz_functions_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_cordz_functions_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_cordz_functions_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_cordz_functions_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_cordz_functions_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_cordz_functions_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_cordz_functions_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_cordz_functions_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_cordz_functions_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_cordz_functions_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::random_internal_distribution_caller VARIABLES ############################################

set(abseil_absl_random_internal_distribution_caller_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_random_internal_distribution_caller_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_random_internal_distribution_caller_BIN_DIRS_RELEASE )
set(abseil_absl_random_internal_distribution_caller_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_random_internal_distribution_caller_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_random_internal_distribution_caller_RES_DIRS_RELEASE )
set(abseil_absl_random_internal_distribution_caller_DEFINITIONS_RELEASE )
set(abseil_absl_random_internal_distribution_caller_OBJECTS_RELEASE )
set(abseil_absl_random_internal_distribution_caller_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_random_internal_distribution_caller_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_random_internal_distribution_caller_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_random_internal_distribution_caller_LIBS_RELEASE )
set(abseil_absl_random_internal_distribution_caller_SYSTEM_LIBS_RELEASE )
set(abseil_absl_random_internal_distribution_caller_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_random_internal_distribution_caller_FRAMEWORKS_RELEASE )
set(abseil_absl_random_internal_distribution_caller_DEPENDENCIES_RELEASE absl::config absl::utility absl::fast_type_id absl::type_traits)
set(abseil_absl_random_internal_distribution_caller_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_random_internal_distribution_caller_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_random_internal_distribution_caller_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_random_internal_distribution_caller_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_random_internal_distribution_caller_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_random_internal_distribution_caller_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_random_internal_distribution_caller_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_random_internal_distribution_caller_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_random_internal_distribution_caller_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_random_internal_distribution_caller_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::random_seed_gen_exception VARIABLES ############################################

set(abseil_absl_random_seed_gen_exception_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_random_seed_gen_exception_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_random_seed_gen_exception_BIN_DIRS_RELEASE )
set(abseil_absl_random_seed_gen_exception_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_random_seed_gen_exception_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_random_seed_gen_exception_RES_DIRS_RELEASE )
set(abseil_absl_random_seed_gen_exception_DEFINITIONS_RELEASE )
set(abseil_absl_random_seed_gen_exception_OBJECTS_RELEASE )
set(abseil_absl_random_seed_gen_exception_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_random_seed_gen_exception_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_random_seed_gen_exception_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_random_seed_gen_exception_LIBS_RELEASE absl_random_seed_gen_exception)
set(abseil_absl_random_seed_gen_exception_SYSTEM_LIBS_RELEASE )
set(abseil_absl_random_seed_gen_exception_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_random_seed_gen_exception_FRAMEWORKS_RELEASE )
set(abseil_absl_random_seed_gen_exception_DEPENDENCIES_RELEASE absl::config absl::raw_logging_internal)
set(abseil_absl_random_seed_gen_exception_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_random_seed_gen_exception_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_random_seed_gen_exception_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_random_seed_gen_exception_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_random_seed_gen_exception_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_random_seed_gen_exception_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_random_seed_gen_exception_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_random_seed_gen_exception_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_random_seed_gen_exception_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_random_seed_gen_exception_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::memory VARIABLES ############################################

set(abseil_absl_memory_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_memory_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_memory_BIN_DIRS_RELEASE )
set(abseil_absl_memory_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_memory_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_memory_RES_DIRS_RELEASE )
set(abseil_absl_memory_DEFINITIONS_RELEASE )
set(abseil_absl_memory_OBJECTS_RELEASE )
set(abseil_absl_memory_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_memory_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_memory_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_memory_LIBS_RELEASE )
set(abseil_absl_memory_SYSTEM_LIBS_RELEASE )
set(abseil_absl_memory_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_memory_FRAMEWORKS_RELEASE )
set(abseil_absl_memory_DEPENDENCIES_RELEASE absl::core_headers absl::meta)
set(abseil_absl_memory_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_memory_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_memory_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_memory_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_memory_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_memory_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_memory_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_memory_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_memory_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_memory_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::overload VARIABLES ############################################

set(abseil_absl_overload_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_overload_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_overload_BIN_DIRS_RELEASE )
set(abseil_absl_overload_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_overload_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_overload_RES_DIRS_RELEASE )
set(abseil_absl_overload_DEFINITIONS_RELEASE )
set(abseil_absl_overload_OBJECTS_RELEASE )
set(abseil_absl_overload_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_overload_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_overload_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_overload_LIBS_RELEASE )
set(abseil_absl_overload_SYSTEM_LIBS_RELEASE )
set(abseil_absl_overload_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_overload_FRAMEWORKS_RELEASE )
set(abseil_absl_overload_DEPENDENCIES_RELEASE absl::meta)
set(abseil_absl_overload_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_overload_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_overload_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_overload_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_overload_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_overload_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_overload_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_overload_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_overload_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_overload_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::debugging_internal VARIABLES ############################################

set(abseil_absl_debugging_internal_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_debugging_internal_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_debugging_internal_BIN_DIRS_RELEASE )
set(abseil_absl_debugging_internal_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_debugging_internal_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_debugging_internal_RES_DIRS_RELEASE )
set(abseil_absl_debugging_internal_DEFINITIONS_RELEASE )
set(abseil_absl_debugging_internal_OBJECTS_RELEASE )
set(abseil_absl_debugging_internal_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_debugging_internal_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_debugging_internal_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_debugging_internal_LIBS_RELEASE absl_debugging_internal)
set(abseil_absl_debugging_internal_SYSTEM_LIBS_RELEASE )
set(abseil_absl_debugging_internal_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_debugging_internal_FRAMEWORKS_RELEASE )
set(abseil_absl_debugging_internal_DEPENDENCIES_RELEASE absl::core_headers absl::config absl::dynamic_annotations absl::errno_saver absl::raw_logging_internal)
set(abseil_absl_debugging_internal_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_debugging_internal_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_debugging_internal_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_debugging_internal_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_debugging_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_debugging_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_debugging_internal_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_debugging_internal_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_debugging_internal_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_debugging_internal_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::common_policy_traits VARIABLES ############################################

set(abseil_absl_common_policy_traits_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_common_policy_traits_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_common_policy_traits_BIN_DIRS_RELEASE )
set(abseil_absl_common_policy_traits_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_common_policy_traits_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_common_policy_traits_RES_DIRS_RELEASE )
set(abseil_absl_common_policy_traits_DEFINITIONS_RELEASE )
set(abseil_absl_common_policy_traits_OBJECTS_RELEASE )
set(abseil_absl_common_policy_traits_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_common_policy_traits_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_common_policy_traits_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_common_policy_traits_LIBS_RELEASE )
set(abseil_absl_common_policy_traits_SYSTEM_LIBS_RELEASE )
set(abseil_absl_common_policy_traits_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_common_policy_traits_FRAMEWORKS_RELEASE )
set(abseil_absl_common_policy_traits_DEPENDENCIES_RELEASE absl::meta)
set(abseil_absl_common_policy_traits_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_common_policy_traits_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_common_policy_traits_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_common_policy_traits_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_common_policy_traits_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_common_policy_traits_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_common_policy_traits_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_common_policy_traits_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_common_policy_traits_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_common_policy_traits_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::compressed_tuple VARIABLES ############################################

set(abseil_absl_compressed_tuple_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_compressed_tuple_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_compressed_tuple_BIN_DIRS_RELEASE )
set(abseil_absl_compressed_tuple_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_compressed_tuple_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_compressed_tuple_RES_DIRS_RELEASE )
set(abseil_absl_compressed_tuple_DEFINITIONS_RELEASE )
set(abseil_absl_compressed_tuple_OBJECTS_RELEASE )
set(abseil_absl_compressed_tuple_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_compressed_tuple_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_compressed_tuple_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_compressed_tuple_LIBS_RELEASE )
set(abseil_absl_compressed_tuple_SYSTEM_LIBS_RELEASE )
set(abseil_absl_compressed_tuple_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_compressed_tuple_FRAMEWORKS_RELEASE )
set(abseil_absl_compressed_tuple_DEPENDENCIES_RELEASE absl::utility)
set(abseil_absl_compressed_tuple_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_compressed_tuple_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_compressed_tuple_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_compressed_tuple_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_compressed_tuple_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_compressed_tuple_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_compressed_tuple_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_compressed_tuple_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_compressed_tuple_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_compressed_tuple_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::cleanup_internal VARIABLES ############################################

set(abseil_absl_cleanup_internal_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_cleanup_internal_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_cleanup_internal_BIN_DIRS_RELEASE )
set(abseil_absl_cleanup_internal_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_cleanup_internal_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_cleanup_internal_RES_DIRS_RELEASE )
set(abseil_absl_cleanup_internal_DEFINITIONS_RELEASE )
set(abseil_absl_cleanup_internal_OBJECTS_RELEASE )
set(abseil_absl_cleanup_internal_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_cleanup_internal_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_cleanup_internal_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_cleanup_internal_LIBS_RELEASE )
set(abseil_absl_cleanup_internal_SYSTEM_LIBS_RELEASE )
set(abseil_absl_cleanup_internal_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_cleanup_internal_FRAMEWORKS_RELEASE )
set(abseil_absl_cleanup_internal_DEPENDENCIES_RELEASE absl::core_headers absl::utility)
set(abseil_absl_cleanup_internal_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_cleanup_internal_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_cleanup_internal_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_cleanup_internal_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_cleanup_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_cleanup_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_cleanup_internal_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_cleanup_internal_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_cleanup_internal_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_cleanup_internal_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::algorithm_container VARIABLES ############################################

set(abseil_absl_algorithm_container_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_algorithm_container_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_algorithm_container_BIN_DIRS_RELEASE )
set(abseil_absl_algorithm_container_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_algorithm_container_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_algorithm_container_RES_DIRS_RELEASE )
set(abseil_absl_algorithm_container_DEFINITIONS_RELEASE )
set(abseil_absl_algorithm_container_OBJECTS_RELEASE )
set(abseil_absl_algorithm_container_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_algorithm_container_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_algorithm_container_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_algorithm_container_LIBS_RELEASE )
set(abseil_absl_algorithm_container_SYSTEM_LIBS_RELEASE )
set(abseil_absl_algorithm_container_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_algorithm_container_FRAMEWORKS_RELEASE )
set(abseil_absl_algorithm_container_DEPENDENCIES_RELEASE absl::algorithm absl::config absl::core_headers absl::meta)
set(abseil_absl_algorithm_container_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_algorithm_container_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_algorithm_container_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_algorithm_container_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_algorithm_container_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_algorithm_container_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_algorithm_container_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_algorithm_container_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_algorithm_container_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_algorithm_container_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::scoped_set_env VARIABLES ############################################

set(abseil_absl_scoped_set_env_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_scoped_set_env_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_scoped_set_env_BIN_DIRS_RELEASE )
set(abseil_absl_scoped_set_env_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_scoped_set_env_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_scoped_set_env_RES_DIRS_RELEASE )
set(abseil_absl_scoped_set_env_DEFINITIONS_RELEASE )
set(abseil_absl_scoped_set_env_OBJECTS_RELEASE )
set(abseil_absl_scoped_set_env_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_scoped_set_env_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_scoped_set_env_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_scoped_set_env_LIBS_RELEASE absl_scoped_set_env)
set(abseil_absl_scoped_set_env_SYSTEM_LIBS_RELEASE )
set(abseil_absl_scoped_set_env_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_scoped_set_env_FRAMEWORKS_RELEASE )
set(abseil_absl_scoped_set_env_DEPENDENCIES_RELEASE absl::config absl::raw_logging_internal)
set(abseil_absl_scoped_set_env_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_scoped_set_env_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_scoped_set_env_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_scoped_set_env_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_scoped_set_env_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_scoped_set_env_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_scoped_set_env_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_scoped_set_env_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_scoped_set_env_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_scoped_set_env_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::throw_delegate VARIABLES ############################################

set(abseil_absl_throw_delegate_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_throw_delegate_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_throw_delegate_BIN_DIRS_RELEASE )
set(abseil_absl_throw_delegate_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_throw_delegate_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_throw_delegate_RES_DIRS_RELEASE )
set(abseil_absl_throw_delegate_DEFINITIONS_RELEASE )
set(abseil_absl_throw_delegate_OBJECTS_RELEASE )
set(abseil_absl_throw_delegate_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_throw_delegate_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_throw_delegate_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_throw_delegate_LIBS_RELEASE absl_throw_delegate)
set(abseil_absl_throw_delegate_SYSTEM_LIBS_RELEASE )
set(abseil_absl_throw_delegate_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_throw_delegate_FRAMEWORKS_RELEASE )
set(abseil_absl_throw_delegate_DEPENDENCIES_RELEASE absl::config absl::raw_logging_internal)
set(abseil_absl_throw_delegate_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_throw_delegate_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_throw_delegate_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_throw_delegate_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_throw_delegate_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_throw_delegate_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_throw_delegate_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_throw_delegate_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_throw_delegate_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_throw_delegate_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::spinlock_wait VARIABLES ############################################

set(abseil_absl_spinlock_wait_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_spinlock_wait_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_spinlock_wait_BIN_DIRS_RELEASE )
set(abseil_absl_spinlock_wait_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_spinlock_wait_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_spinlock_wait_RES_DIRS_RELEASE )
set(abseil_absl_spinlock_wait_DEFINITIONS_RELEASE )
set(abseil_absl_spinlock_wait_OBJECTS_RELEASE )
set(abseil_absl_spinlock_wait_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_spinlock_wait_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_spinlock_wait_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_spinlock_wait_LIBS_RELEASE absl_spinlock_wait)
set(abseil_absl_spinlock_wait_SYSTEM_LIBS_RELEASE )
set(abseil_absl_spinlock_wait_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_spinlock_wait_FRAMEWORKS_RELEASE )
set(abseil_absl_spinlock_wait_DEPENDENCIES_RELEASE absl::base_internal absl::core_headers absl::errno_saver)
set(abseil_absl_spinlock_wait_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_spinlock_wait_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_spinlock_wait_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_spinlock_wait_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_spinlock_wait_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_spinlock_wait_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_spinlock_wait_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_spinlock_wait_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_spinlock_wait_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_spinlock_wait_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::utility VARIABLES ############################################

set(abseil_absl_utility_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_utility_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_utility_BIN_DIRS_RELEASE )
set(abseil_absl_utility_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_utility_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_utility_RES_DIRS_RELEASE )
set(abseil_absl_utility_DEFINITIONS_RELEASE )
set(abseil_absl_utility_OBJECTS_RELEASE )
set(abseil_absl_utility_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_utility_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_utility_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_utility_LIBS_RELEASE )
set(abseil_absl_utility_SYSTEM_LIBS_RELEASE )
set(abseil_absl_utility_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_utility_FRAMEWORKS_RELEASE )
set(abseil_absl_utility_DEPENDENCIES_RELEASE absl::config absl::type_traits)
set(abseil_absl_utility_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_utility_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_utility_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_utility_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_utility_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_utility_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_utility_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_utility_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_utility_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_utility_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::compare VARIABLES ############################################

set(abseil_absl_compare_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_compare_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_compare_BIN_DIRS_RELEASE )
set(abseil_absl_compare_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_compare_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_compare_RES_DIRS_RELEASE )
set(abseil_absl_compare_DEFINITIONS_RELEASE )
set(abseil_absl_compare_OBJECTS_RELEASE )
set(abseil_absl_compare_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_compare_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_compare_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_compare_LIBS_RELEASE )
set(abseil_absl_compare_SYSTEM_LIBS_RELEASE )
set(abseil_absl_compare_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_compare_FRAMEWORKS_RELEASE )
set(abseil_absl_compare_DEPENDENCIES_RELEASE absl::config absl::core_headers absl::type_traits)
set(abseil_absl_compare_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_compare_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_compare_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_compare_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_compare_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_compare_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_compare_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_compare_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_compare_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_compare_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::charset VARIABLES ############################################

set(abseil_absl_charset_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_charset_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_charset_BIN_DIRS_RELEASE )
set(abseil_absl_charset_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_charset_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_charset_RES_DIRS_RELEASE )
set(abseil_absl_charset_DEFINITIONS_RELEASE )
set(abseil_absl_charset_OBJECTS_RELEASE )
set(abseil_absl_charset_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_charset_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_charset_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_charset_LIBS_RELEASE )
set(abseil_absl_charset_SYSTEM_LIBS_RELEASE )
set(abseil_absl_charset_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_charset_FRAMEWORKS_RELEASE )
set(abseil_absl_charset_DEPENDENCIES_RELEASE absl::config absl::string_view)
set(abseil_absl_charset_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_charset_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_charset_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_charset_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_charset_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_charset_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_charset_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_charset_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_charset_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_charset_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::random_internal_uniform_helper VARIABLES ############################################

set(abseil_absl_random_internal_uniform_helper_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_random_internal_uniform_helper_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_random_internal_uniform_helper_BIN_DIRS_RELEASE )
set(abseil_absl_random_internal_uniform_helper_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_random_internal_uniform_helper_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_random_internal_uniform_helper_RES_DIRS_RELEASE )
set(abseil_absl_random_internal_uniform_helper_DEFINITIONS_RELEASE )
set(abseil_absl_random_internal_uniform_helper_OBJECTS_RELEASE )
set(abseil_absl_random_internal_uniform_helper_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_random_internal_uniform_helper_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_random_internal_uniform_helper_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_random_internal_uniform_helper_LIBS_RELEASE )
set(abseil_absl_random_internal_uniform_helper_SYSTEM_LIBS_RELEASE )
set(abseil_absl_random_internal_uniform_helper_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_random_internal_uniform_helper_FRAMEWORKS_RELEASE )
set(abseil_absl_random_internal_uniform_helper_DEPENDENCIES_RELEASE absl::config absl::random_internal_traits absl::type_traits)
set(abseil_absl_random_internal_uniform_helper_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_random_internal_uniform_helper_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_random_internal_uniform_helper_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_random_internal_uniform_helper_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_random_internal_uniform_helper_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_random_internal_uniform_helper_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_random_internal_uniform_helper_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_random_internal_uniform_helper_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_random_internal_uniform_helper_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_random_internal_uniform_helper_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::periodic_sampler VARIABLES ############################################

set(abseil_absl_periodic_sampler_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_periodic_sampler_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_periodic_sampler_BIN_DIRS_RELEASE )
set(abseil_absl_periodic_sampler_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_periodic_sampler_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_periodic_sampler_RES_DIRS_RELEASE )
set(abseil_absl_periodic_sampler_DEFINITIONS_RELEASE )
set(abseil_absl_periodic_sampler_OBJECTS_RELEASE )
set(abseil_absl_periodic_sampler_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_periodic_sampler_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_periodic_sampler_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_periodic_sampler_LIBS_RELEASE absl_periodic_sampler)
set(abseil_absl_periodic_sampler_SYSTEM_LIBS_RELEASE )
set(abseil_absl_periodic_sampler_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_periodic_sampler_FRAMEWORKS_RELEASE )
set(abseil_absl_periodic_sampler_DEPENDENCIES_RELEASE absl::core_headers absl::exponential_biased)
set(abseil_absl_periodic_sampler_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_periodic_sampler_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_periodic_sampler_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_periodic_sampler_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_periodic_sampler_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_periodic_sampler_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_periodic_sampler_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_periodic_sampler_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_periodic_sampler_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_periodic_sampler_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::meta VARIABLES ############################################

set(abseil_absl_meta_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_meta_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_meta_BIN_DIRS_RELEASE )
set(abseil_absl_meta_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_meta_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_meta_RES_DIRS_RELEASE )
set(abseil_absl_meta_DEFINITIONS_RELEASE )
set(abseil_absl_meta_OBJECTS_RELEASE )
set(abseil_absl_meta_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_meta_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_meta_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_meta_LIBS_RELEASE )
set(abseil_absl_meta_SYSTEM_LIBS_RELEASE )
set(abseil_absl_meta_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_meta_FRAMEWORKS_RELEASE )
set(abseil_absl_meta_DEPENDENCIES_RELEASE absl::type_traits)
set(abseil_absl_meta_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_meta_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_meta_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_meta_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_meta_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_meta_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_meta_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_meta_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_meta_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_meta_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::container_common VARIABLES ############################################

set(abseil_absl_container_common_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_container_common_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_container_common_BIN_DIRS_RELEASE )
set(abseil_absl_container_common_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_container_common_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_container_common_RES_DIRS_RELEASE )
set(abseil_absl_container_common_DEFINITIONS_RELEASE )
set(abseil_absl_container_common_OBJECTS_RELEASE )
set(abseil_absl_container_common_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_container_common_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_container_common_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_container_common_LIBS_RELEASE )
set(abseil_absl_container_common_SYSTEM_LIBS_RELEASE )
set(abseil_absl_container_common_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_container_common_FRAMEWORKS_RELEASE )
set(abseil_absl_container_common_DEPENDENCIES_RELEASE absl::type_traits)
set(abseil_absl_container_common_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_container_common_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_container_common_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_container_common_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_container_common_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_container_common_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_container_common_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_container_common_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_container_common_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_container_common_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::iterator_traits_internal VARIABLES ############################################

set(abseil_absl_iterator_traits_internal_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_iterator_traits_internal_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_iterator_traits_internal_BIN_DIRS_RELEASE )
set(abseil_absl_iterator_traits_internal_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_iterator_traits_internal_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_iterator_traits_internal_RES_DIRS_RELEASE )
set(abseil_absl_iterator_traits_internal_DEFINITIONS_RELEASE )
set(abseil_absl_iterator_traits_internal_OBJECTS_RELEASE )
set(abseil_absl_iterator_traits_internal_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_iterator_traits_internal_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_iterator_traits_internal_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_iterator_traits_internal_LIBS_RELEASE )
set(abseil_absl_iterator_traits_internal_SYSTEM_LIBS_RELEASE )
set(abseil_absl_iterator_traits_internal_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_iterator_traits_internal_FRAMEWORKS_RELEASE )
set(abseil_absl_iterator_traits_internal_DEPENDENCIES_RELEASE absl::config absl::type_traits)
set(abseil_absl_iterator_traits_internal_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_iterator_traits_internal_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_iterator_traits_internal_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_iterator_traits_internal_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_iterator_traits_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_iterator_traits_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_iterator_traits_internal_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_iterator_traits_internal_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_iterator_traits_internal_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_iterator_traits_internal_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::base_internal VARIABLES ############################################

set(abseil_absl_base_internal_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_base_internal_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_base_internal_BIN_DIRS_RELEASE )
set(abseil_absl_base_internal_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_base_internal_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_base_internal_RES_DIRS_RELEASE )
set(abseil_absl_base_internal_DEFINITIONS_RELEASE )
set(abseil_absl_base_internal_OBJECTS_RELEASE )
set(abseil_absl_base_internal_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_base_internal_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_base_internal_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_base_internal_LIBS_RELEASE )
set(abseil_absl_base_internal_SYSTEM_LIBS_RELEASE )
set(abseil_absl_base_internal_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_base_internal_FRAMEWORKS_RELEASE )
set(abseil_absl_base_internal_DEPENDENCIES_RELEASE absl::config absl::type_traits)
set(abseil_absl_base_internal_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_base_internal_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_base_internal_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_base_internal_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_base_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_base_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_base_internal_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_base_internal_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_base_internal_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_base_internal_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::raw_logging_internal VARIABLES ############################################

set(abseil_absl_raw_logging_internal_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_raw_logging_internal_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_raw_logging_internal_BIN_DIRS_RELEASE )
set(abseil_absl_raw_logging_internal_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_raw_logging_internal_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_raw_logging_internal_RES_DIRS_RELEASE )
set(abseil_absl_raw_logging_internal_DEFINITIONS_RELEASE )
set(abseil_absl_raw_logging_internal_OBJECTS_RELEASE )
set(abseil_absl_raw_logging_internal_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_raw_logging_internal_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_raw_logging_internal_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_raw_logging_internal_LIBS_RELEASE absl_raw_logging_internal)
set(abseil_absl_raw_logging_internal_SYSTEM_LIBS_RELEASE )
set(abseil_absl_raw_logging_internal_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_raw_logging_internal_FRAMEWORKS_RELEASE )
set(abseil_absl_raw_logging_internal_DEPENDENCIES_RELEASE absl::atomic_hook absl::config absl::core_headers absl::errno_saver absl::log_severity)
set(abseil_absl_raw_logging_internal_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_raw_logging_internal_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_raw_logging_internal_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_raw_logging_internal_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_raw_logging_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_raw_logging_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_raw_logging_internal_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_raw_logging_internal_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_raw_logging_internal_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_raw_logging_internal_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::string_view VARIABLES ############################################

set(abseil_absl_string_view_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_string_view_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_string_view_BIN_DIRS_RELEASE )
set(abseil_absl_string_view_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_string_view_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_string_view_RES_DIRS_RELEASE )
set(abseil_absl_string_view_DEFINITIONS_RELEASE )
set(abseil_absl_string_view_OBJECTS_RELEASE )
set(abseil_absl_string_view_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_string_view_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_string_view_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_string_view_LIBS_RELEASE )
set(abseil_absl_string_view_SYSTEM_LIBS_RELEASE )
set(abseil_absl_string_view_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_string_view_FRAMEWORKS_RELEASE )
set(abseil_absl_string_view_DEPENDENCIES_RELEASE absl::config absl::core_headers absl::nullability)
set(abseil_absl_string_view_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_string_view_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_string_view_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_string_view_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_string_view_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_string_view_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_string_view_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_string_view_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_string_view_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_string_view_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::random_internal_randen_hwaes_impl VARIABLES ############################################

set(abseil_absl_random_internal_randen_hwaes_impl_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_random_internal_randen_hwaes_impl_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_random_internal_randen_hwaes_impl_BIN_DIRS_RELEASE )
set(abseil_absl_random_internal_randen_hwaes_impl_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_random_internal_randen_hwaes_impl_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_random_internal_randen_hwaes_impl_RES_DIRS_RELEASE )
set(abseil_absl_random_internal_randen_hwaes_impl_DEFINITIONS_RELEASE )
set(abseil_absl_random_internal_randen_hwaes_impl_OBJECTS_RELEASE )
set(abseil_absl_random_internal_randen_hwaes_impl_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_random_internal_randen_hwaes_impl_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_random_internal_randen_hwaes_impl_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_random_internal_randen_hwaes_impl_LIBS_RELEASE absl_random_internal_randen_hwaes_impl)
set(abseil_absl_random_internal_randen_hwaes_impl_SYSTEM_LIBS_RELEASE )
set(abseil_absl_random_internal_randen_hwaes_impl_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_random_internal_randen_hwaes_impl_FRAMEWORKS_RELEASE )
set(abseil_absl_random_internal_randen_hwaes_impl_DEPENDENCIES_RELEASE absl::random_internal_platform absl::config)
set(abseil_absl_random_internal_randen_hwaes_impl_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_random_internal_randen_hwaes_impl_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_random_internal_randen_hwaes_impl_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_random_internal_randen_hwaes_impl_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_random_internal_randen_hwaes_impl_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_random_internal_randen_hwaes_impl_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_random_internal_randen_hwaes_impl_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_random_internal_randen_hwaes_impl_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_random_internal_randen_hwaes_impl_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_random_internal_randen_hwaes_impl_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::random_internal_randen_slow VARIABLES ############################################

set(abseil_absl_random_internal_randen_slow_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_random_internal_randen_slow_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_random_internal_randen_slow_BIN_DIRS_RELEASE )
set(abseil_absl_random_internal_randen_slow_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_random_internal_randen_slow_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_random_internal_randen_slow_RES_DIRS_RELEASE )
set(abseil_absl_random_internal_randen_slow_DEFINITIONS_RELEASE )
set(abseil_absl_random_internal_randen_slow_OBJECTS_RELEASE )
set(abseil_absl_random_internal_randen_slow_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_random_internal_randen_slow_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_random_internal_randen_slow_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_random_internal_randen_slow_LIBS_RELEASE absl_random_internal_randen_slow)
set(abseil_absl_random_internal_randen_slow_SYSTEM_LIBS_RELEASE )
set(abseil_absl_random_internal_randen_slow_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_random_internal_randen_slow_FRAMEWORKS_RELEASE )
set(abseil_absl_random_internal_randen_slow_DEPENDENCIES_RELEASE absl::random_internal_platform absl::config)
set(abseil_absl_random_internal_randen_slow_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_random_internal_randen_slow_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_random_internal_randen_slow_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_random_internal_randen_slow_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_random_internal_randen_slow_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_random_internal_randen_slow_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_random_internal_randen_slow_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_random_internal_randen_slow_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_random_internal_randen_slow_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_random_internal_randen_slow_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::exponential_biased VARIABLES ############################################

set(abseil_absl_exponential_biased_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_exponential_biased_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_exponential_biased_BIN_DIRS_RELEASE )
set(abseil_absl_exponential_biased_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_exponential_biased_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_exponential_biased_RES_DIRS_RELEASE )
set(abseil_absl_exponential_biased_DEFINITIONS_RELEASE )
set(abseil_absl_exponential_biased_OBJECTS_RELEASE )
set(abseil_absl_exponential_biased_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_exponential_biased_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_exponential_biased_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_exponential_biased_LIBS_RELEASE absl_exponential_biased)
set(abseil_absl_exponential_biased_SYSTEM_LIBS_RELEASE )
set(abseil_absl_exponential_biased_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_exponential_biased_FRAMEWORKS_RELEASE )
set(abseil_absl_exponential_biased_DEPENDENCIES_RELEASE absl::config absl::core_headers)
set(abseil_absl_exponential_biased_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_exponential_biased_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_exponential_biased_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_exponential_biased_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_exponential_biased_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_exponential_biased_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_exponential_biased_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_exponential_biased_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_exponential_biased_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_exponential_biased_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::type_traits VARIABLES ############################################

set(abseil_absl_type_traits_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_type_traits_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_type_traits_BIN_DIRS_RELEASE )
set(abseil_absl_type_traits_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_type_traits_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_type_traits_RES_DIRS_RELEASE )
set(abseil_absl_type_traits_DEFINITIONS_RELEASE )
set(abseil_absl_type_traits_OBJECTS_RELEASE )
set(abseil_absl_type_traits_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_type_traits_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_type_traits_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_type_traits_LIBS_RELEASE )
set(abseil_absl_type_traits_SYSTEM_LIBS_RELEASE )
set(abseil_absl_type_traits_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_type_traits_FRAMEWORKS_RELEASE )
set(abseil_absl_type_traits_DEPENDENCIES_RELEASE absl::config absl::core_headers)
set(abseil_absl_type_traits_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_type_traits_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_type_traits_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_type_traits_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_type_traits_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_type_traits_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_type_traits_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_type_traits_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_type_traits_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_type_traits_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::log_internal_voidify VARIABLES ############################################

set(abseil_absl_log_internal_voidify_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_log_internal_voidify_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_log_internal_voidify_BIN_DIRS_RELEASE )
set(abseil_absl_log_internal_voidify_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_log_internal_voidify_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_log_internal_voidify_RES_DIRS_RELEASE )
set(abseil_absl_log_internal_voidify_DEFINITIONS_RELEASE )
set(abseil_absl_log_internal_voidify_OBJECTS_RELEASE )
set(abseil_absl_log_internal_voidify_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_log_internal_voidify_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_log_internal_voidify_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_log_internal_voidify_LIBS_RELEASE )
set(abseil_absl_log_internal_voidify_SYSTEM_LIBS_RELEASE )
set(abseil_absl_log_internal_voidify_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_log_internal_voidify_FRAMEWORKS_RELEASE )
set(abseil_absl_log_internal_voidify_DEPENDENCIES_RELEASE absl::config absl::core_headers)
set(abseil_absl_log_internal_voidify_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_log_internal_voidify_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_log_internal_voidify_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_log_internal_voidify_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_log_internal_voidify_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_log_internal_voidify_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_log_internal_voidify_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_log_internal_voidify_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_log_internal_voidify_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_log_internal_voidify_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::log_internal_nullguard VARIABLES ############################################

set(abseil_absl_log_internal_nullguard_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_log_internal_nullguard_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_log_internal_nullguard_BIN_DIRS_RELEASE )
set(abseil_absl_log_internal_nullguard_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_log_internal_nullguard_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_log_internal_nullguard_RES_DIRS_RELEASE )
set(abseil_absl_log_internal_nullguard_DEFINITIONS_RELEASE )
set(abseil_absl_log_internal_nullguard_OBJECTS_RELEASE )
set(abseil_absl_log_internal_nullguard_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_log_internal_nullguard_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_log_internal_nullguard_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_log_internal_nullguard_LIBS_RELEASE absl_log_internal_nullguard)
set(abseil_absl_log_internal_nullguard_SYSTEM_LIBS_RELEASE )
set(abseil_absl_log_internal_nullguard_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_log_internal_nullguard_FRAMEWORKS_RELEASE )
set(abseil_absl_log_internal_nullguard_DEPENDENCIES_RELEASE absl::config absl::core_headers)
set(abseil_absl_log_internal_nullguard_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_log_internal_nullguard_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_log_internal_nullguard_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_log_internal_nullguard_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_log_internal_nullguard_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_log_internal_nullguard_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_log_internal_nullguard_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_log_internal_nullguard_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_log_internal_nullguard_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_log_internal_nullguard_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::log_internal_config VARIABLES ############################################

set(abseil_absl_log_internal_config_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_log_internal_config_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_log_internal_config_BIN_DIRS_RELEASE )
set(abseil_absl_log_internal_config_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_log_internal_config_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_log_internal_config_RES_DIRS_RELEASE )
set(abseil_absl_log_internal_config_DEFINITIONS_RELEASE )
set(abseil_absl_log_internal_config_OBJECTS_RELEASE )
set(abseil_absl_log_internal_config_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_log_internal_config_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_log_internal_config_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_log_internal_config_LIBS_RELEASE )
set(abseil_absl_log_internal_config_SYSTEM_LIBS_RELEASE )
set(abseil_absl_log_internal_config_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_log_internal_config_FRAMEWORKS_RELEASE )
set(abseil_absl_log_internal_config_DEPENDENCIES_RELEASE absl::config absl::core_headers)
set(abseil_absl_log_internal_config_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_log_internal_config_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_log_internal_config_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_log_internal_config_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_log_internal_config_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_log_internal_config_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_log_internal_config_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_log_internal_config_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_log_internal_config_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_log_internal_config_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::flags_commandlineflag_internal VARIABLES ############################################

set(abseil_absl_flags_commandlineflag_internal_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_flags_commandlineflag_internal_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_flags_commandlineflag_internal_BIN_DIRS_RELEASE )
set(abseil_absl_flags_commandlineflag_internal_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_flags_commandlineflag_internal_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_flags_commandlineflag_internal_RES_DIRS_RELEASE )
set(abseil_absl_flags_commandlineflag_internal_DEFINITIONS_RELEASE )
set(abseil_absl_flags_commandlineflag_internal_OBJECTS_RELEASE )
set(abseil_absl_flags_commandlineflag_internal_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_flags_commandlineflag_internal_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_flags_commandlineflag_internal_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_flags_commandlineflag_internal_LIBS_RELEASE absl_flags_commandlineflag_internal)
set(abseil_absl_flags_commandlineflag_internal_SYSTEM_LIBS_RELEASE )
set(abseil_absl_flags_commandlineflag_internal_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_flags_commandlineflag_internal_FRAMEWORKS_RELEASE )
set(abseil_absl_flags_commandlineflag_internal_DEPENDENCIES_RELEASE absl::config absl::dynamic_annotations absl::fast_type_id)
set(abseil_absl_flags_commandlineflag_internal_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_flags_commandlineflag_internal_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_flags_commandlineflag_internal_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_flags_commandlineflag_internal_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_flags_commandlineflag_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_flags_commandlineflag_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_flags_commandlineflag_internal_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_flags_commandlineflag_internal_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_flags_commandlineflag_internal_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_flags_commandlineflag_internal_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::leak_check VARIABLES ############################################

set(abseil_absl_leak_check_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_leak_check_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_leak_check_BIN_DIRS_RELEASE )
set(abseil_absl_leak_check_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_leak_check_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_leak_check_RES_DIRS_RELEASE )
set(abseil_absl_leak_check_DEFINITIONS_RELEASE )
set(abseil_absl_leak_check_OBJECTS_RELEASE )
set(abseil_absl_leak_check_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_leak_check_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_leak_check_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_leak_check_LIBS_RELEASE absl_leak_check)
set(abseil_absl_leak_check_SYSTEM_LIBS_RELEASE )
set(abseil_absl_leak_check_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_leak_check_FRAMEWORKS_RELEASE )
set(abseil_absl_leak_check_DEPENDENCIES_RELEASE absl::config absl::core_headers)
set(abseil_absl_leak_check_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_leak_check_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_leak_check_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_leak_check_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_leak_check_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_leak_check_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_leak_check_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_leak_check_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_leak_check_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_leak_check_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::non_temporal_memcpy VARIABLES ############################################

set(abseil_absl_non_temporal_memcpy_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_non_temporal_memcpy_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_non_temporal_memcpy_BIN_DIRS_RELEASE )
set(abseil_absl_non_temporal_memcpy_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_non_temporal_memcpy_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_non_temporal_memcpy_RES_DIRS_RELEASE )
set(abseil_absl_non_temporal_memcpy_DEFINITIONS_RELEASE )
set(abseil_absl_non_temporal_memcpy_OBJECTS_RELEASE )
set(abseil_absl_non_temporal_memcpy_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_non_temporal_memcpy_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_non_temporal_memcpy_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_non_temporal_memcpy_LIBS_RELEASE )
set(abseil_absl_non_temporal_memcpy_SYSTEM_LIBS_RELEASE )
set(abseil_absl_non_temporal_memcpy_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_non_temporal_memcpy_FRAMEWORKS_RELEASE )
set(abseil_absl_non_temporal_memcpy_DEPENDENCIES_RELEASE absl::non_temporal_arm_intrinsics absl::config absl::core_headers)
set(abseil_absl_non_temporal_memcpy_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_non_temporal_memcpy_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_non_temporal_memcpy_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_non_temporal_memcpy_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_non_temporal_memcpy_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_non_temporal_memcpy_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_non_temporal_memcpy_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_non_temporal_memcpy_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_non_temporal_memcpy_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_non_temporal_memcpy_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::hashtable_debug VARIABLES ############################################

set(abseil_absl_hashtable_debug_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_hashtable_debug_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_hashtable_debug_BIN_DIRS_RELEASE )
set(abseil_absl_hashtable_debug_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_hashtable_debug_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_hashtable_debug_RES_DIRS_RELEASE )
set(abseil_absl_hashtable_debug_DEFINITIONS_RELEASE )
set(abseil_absl_hashtable_debug_OBJECTS_RELEASE )
set(abseil_absl_hashtable_debug_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_hashtable_debug_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_hashtable_debug_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_hashtable_debug_LIBS_RELEASE )
set(abseil_absl_hashtable_debug_SYSTEM_LIBS_RELEASE )
set(abseil_absl_hashtable_debug_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_hashtable_debug_FRAMEWORKS_RELEASE )
set(abseil_absl_hashtable_debug_DEPENDENCIES_RELEASE absl::hashtable_debug_hooks)
set(abseil_absl_hashtable_debug_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_hashtable_debug_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_hashtable_debug_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_hashtable_debug_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_hashtable_debug_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_hashtable_debug_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_hashtable_debug_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_hashtable_debug_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_hashtable_debug_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_hashtable_debug_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::prefetch VARIABLES ############################################

set(abseil_absl_prefetch_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_prefetch_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_prefetch_BIN_DIRS_RELEASE )
set(abseil_absl_prefetch_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_prefetch_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_prefetch_RES_DIRS_RELEASE )
set(abseil_absl_prefetch_DEFINITIONS_RELEASE )
set(abseil_absl_prefetch_OBJECTS_RELEASE )
set(abseil_absl_prefetch_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_prefetch_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_prefetch_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_prefetch_LIBS_RELEASE )
set(abseil_absl_prefetch_SYSTEM_LIBS_RELEASE )
set(abseil_absl_prefetch_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_prefetch_FRAMEWORKS_RELEASE )
set(abseil_absl_prefetch_DEPENDENCIES_RELEASE absl::config absl::core_headers)
set(abseil_absl_prefetch_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_prefetch_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_prefetch_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_prefetch_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_prefetch_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_prefetch_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_prefetch_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_prefetch_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_prefetch_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_prefetch_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::strerror VARIABLES ############################################

set(abseil_absl_strerror_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_strerror_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_strerror_BIN_DIRS_RELEASE )
set(abseil_absl_strerror_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_strerror_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_strerror_RES_DIRS_RELEASE )
set(abseil_absl_strerror_DEFINITIONS_RELEASE )
set(abseil_absl_strerror_OBJECTS_RELEASE )
set(abseil_absl_strerror_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_strerror_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_strerror_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_strerror_LIBS_RELEASE absl_strerror)
set(abseil_absl_strerror_SYSTEM_LIBS_RELEASE )
set(abseil_absl_strerror_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_strerror_FRAMEWORKS_RELEASE )
set(abseil_absl_strerror_DEPENDENCIES_RELEASE absl::config absl::core_headers absl::errno_saver)
set(abseil_absl_strerror_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_strerror_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_strerror_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_strerror_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_strerror_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_strerror_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_strerror_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_strerror_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_strerror_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_strerror_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::nullability_traits_internal VARIABLES ############################################

set(abseil_absl_nullability_traits_internal_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_nullability_traits_internal_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_nullability_traits_internal_BIN_DIRS_RELEASE )
set(abseil_absl_nullability_traits_internal_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_nullability_traits_internal_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_nullability_traits_internal_RES_DIRS_RELEASE )
set(abseil_absl_nullability_traits_internal_DEFINITIONS_RELEASE )
set(abseil_absl_nullability_traits_internal_OBJECTS_RELEASE )
set(abseil_absl_nullability_traits_internal_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_nullability_traits_internal_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_nullability_traits_internal_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_nullability_traits_internal_LIBS_RELEASE )
set(abseil_absl_nullability_traits_internal_SYSTEM_LIBS_RELEASE )
set(abseil_absl_nullability_traits_internal_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_nullability_traits_internal_FRAMEWORKS_RELEASE )
set(abseil_absl_nullability_traits_internal_DEPENDENCIES_RELEASE absl::config absl::nullability)
set(abseil_absl_nullability_traits_internal_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_nullability_traits_internal_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_nullability_traits_internal_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_nullability_traits_internal_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_nullability_traits_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_nullability_traits_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_nullability_traits_internal_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_nullability_traits_internal_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_nullability_traits_internal_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_nullability_traits_internal_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::no_destructor VARIABLES ############################################

set(abseil_absl_no_destructor_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_no_destructor_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_no_destructor_BIN_DIRS_RELEASE )
set(abseil_absl_no_destructor_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_no_destructor_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_no_destructor_RES_DIRS_RELEASE )
set(abseil_absl_no_destructor_DEFINITIONS_RELEASE )
set(abseil_absl_no_destructor_OBJECTS_RELEASE )
set(abseil_absl_no_destructor_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_no_destructor_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_no_destructor_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_no_destructor_LIBS_RELEASE )
set(abseil_absl_no_destructor_SYSTEM_LIBS_RELEASE )
set(abseil_absl_no_destructor_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_no_destructor_FRAMEWORKS_RELEASE )
set(abseil_absl_no_destructor_DEPENDENCIES_RELEASE absl::config absl::nullability)
set(abseil_absl_no_destructor_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_no_destructor_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_no_destructor_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_no_destructor_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_no_destructor_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_no_destructor_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_no_destructor_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_no_destructor_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_no_destructor_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_no_destructor_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::log_severity VARIABLES ############################################

set(abseil_absl_log_severity_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_log_severity_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_log_severity_BIN_DIRS_RELEASE )
set(abseil_absl_log_severity_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_log_severity_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_log_severity_RES_DIRS_RELEASE )
set(abseil_absl_log_severity_DEFINITIONS_RELEASE )
set(abseil_absl_log_severity_OBJECTS_RELEASE )
set(abseil_absl_log_severity_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_log_severity_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_log_severity_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_log_severity_LIBS_RELEASE absl_log_severity)
set(abseil_absl_log_severity_SYSTEM_LIBS_RELEASE )
set(abseil_absl_log_severity_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_log_severity_FRAMEWORKS_RELEASE )
set(abseil_absl_log_severity_DEPENDENCIES_RELEASE absl::config absl::core_headers)
set(abseil_absl_log_severity_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_log_severity_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_log_severity_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_log_severity_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_log_severity_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_log_severity_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_log_severity_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_log_severity_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_log_severity_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_log_severity_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::atomic_hook VARIABLES ############################################

set(abseil_absl_atomic_hook_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_atomic_hook_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_atomic_hook_BIN_DIRS_RELEASE )
set(abseil_absl_atomic_hook_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_atomic_hook_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_atomic_hook_RES_DIRS_RELEASE )
set(abseil_absl_atomic_hook_DEFINITIONS_RELEASE )
set(abseil_absl_atomic_hook_OBJECTS_RELEASE )
set(abseil_absl_atomic_hook_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_atomic_hook_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_atomic_hook_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_atomic_hook_LIBS_RELEASE )
set(abseil_absl_atomic_hook_SYSTEM_LIBS_RELEASE )
set(abseil_absl_atomic_hook_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_atomic_hook_FRAMEWORKS_RELEASE )
set(abseil_absl_atomic_hook_DEPENDENCIES_RELEASE absl::config absl::core_headers)
set(abseil_absl_atomic_hook_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_atomic_hook_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_atomic_hook_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_atomic_hook_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_atomic_hook_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_atomic_hook_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_atomic_hook_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_atomic_hook_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_atomic_hook_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_atomic_hook_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::cordz_update_tracker VARIABLES ############################################

set(abseil_absl_cordz_update_tracker_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_cordz_update_tracker_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_cordz_update_tracker_BIN_DIRS_RELEASE )
set(abseil_absl_cordz_update_tracker_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_cordz_update_tracker_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_cordz_update_tracker_RES_DIRS_RELEASE )
set(abseil_absl_cordz_update_tracker_DEFINITIONS_RELEASE )
set(abseil_absl_cordz_update_tracker_OBJECTS_RELEASE )
set(abseil_absl_cordz_update_tracker_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_cordz_update_tracker_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_cordz_update_tracker_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_cordz_update_tracker_LIBS_RELEASE )
set(abseil_absl_cordz_update_tracker_SYSTEM_LIBS_RELEASE )
set(abseil_absl_cordz_update_tracker_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_cordz_update_tracker_FRAMEWORKS_RELEASE )
set(abseil_absl_cordz_update_tracker_DEPENDENCIES_RELEASE absl::config)
set(abseil_absl_cordz_update_tracker_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_cordz_update_tracker_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_cordz_update_tracker_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_cordz_update_tracker_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_cordz_update_tracker_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_cordz_update_tracker_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_cordz_update_tracker_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_cordz_update_tracker_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_cordz_update_tracker_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_cordz_update_tracker_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::has_ostream_operator VARIABLES ############################################

set(abseil_absl_has_ostream_operator_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_has_ostream_operator_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_has_ostream_operator_BIN_DIRS_RELEASE )
set(abseil_absl_has_ostream_operator_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_has_ostream_operator_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_has_ostream_operator_RES_DIRS_RELEASE )
set(abseil_absl_has_ostream_operator_DEFINITIONS_RELEASE )
set(abseil_absl_has_ostream_operator_OBJECTS_RELEASE )
set(abseil_absl_has_ostream_operator_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_has_ostream_operator_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_has_ostream_operator_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_has_ostream_operator_LIBS_RELEASE )
set(abseil_absl_has_ostream_operator_SYSTEM_LIBS_RELEASE )
set(abseil_absl_has_ostream_operator_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_has_ostream_operator_FRAMEWORKS_RELEASE )
set(abseil_absl_has_ostream_operator_DEPENDENCIES_RELEASE absl::config)
set(abseil_absl_has_ostream_operator_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_has_ostream_operator_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_has_ostream_operator_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_has_ostream_operator_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_has_ostream_operator_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_has_ostream_operator_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_has_ostream_operator_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_has_ostream_operator_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_has_ostream_operator_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_has_ostream_operator_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::random_internal_platform VARIABLES ############################################

set(abseil_absl_random_internal_platform_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_random_internal_platform_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_random_internal_platform_BIN_DIRS_RELEASE )
set(abseil_absl_random_internal_platform_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_random_internal_platform_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_random_internal_platform_RES_DIRS_RELEASE )
set(abseil_absl_random_internal_platform_DEFINITIONS_RELEASE )
set(abseil_absl_random_internal_platform_OBJECTS_RELEASE )
set(abseil_absl_random_internal_platform_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_random_internal_platform_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_random_internal_platform_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_random_internal_platform_LIBS_RELEASE absl_random_internal_platform)
set(abseil_absl_random_internal_platform_SYSTEM_LIBS_RELEASE )
set(abseil_absl_random_internal_platform_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_random_internal_platform_FRAMEWORKS_RELEASE )
set(abseil_absl_random_internal_platform_DEPENDENCIES_RELEASE absl::config)
set(abseil_absl_random_internal_platform_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_random_internal_platform_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_random_internal_platform_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_random_internal_platform_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_random_internal_platform_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_random_internal_platform_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_random_internal_platform_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_random_internal_platform_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_random_internal_platform_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_random_internal_platform_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::random_internal_fast_uniform_bits VARIABLES ############################################

set(abseil_absl_random_internal_fast_uniform_bits_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_random_internal_fast_uniform_bits_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_random_internal_fast_uniform_bits_BIN_DIRS_RELEASE )
set(abseil_absl_random_internal_fast_uniform_bits_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_random_internal_fast_uniform_bits_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_random_internal_fast_uniform_bits_RES_DIRS_RELEASE )
set(abseil_absl_random_internal_fast_uniform_bits_DEFINITIONS_RELEASE )
set(abseil_absl_random_internal_fast_uniform_bits_OBJECTS_RELEASE )
set(abseil_absl_random_internal_fast_uniform_bits_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_random_internal_fast_uniform_bits_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_random_internal_fast_uniform_bits_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_random_internal_fast_uniform_bits_LIBS_RELEASE )
set(abseil_absl_random_internal_fast_uniform_bits_SYSTEM_LIBS_RELEASE )
set(abseil_absl_random_internal_fast_uniform_bits_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_random_internal_fast_uniform_bits_FRAMEWORKS_RELEASE )
set(abseil_absl_random_internal_fast_uniform_bits_DEPENDENCIES_RELEASE absl::config)
set(abseil_absl_random_internal_fast_uniform_bits_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_random_internal_fast_uniform_bits_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_random_internal_fast_uniform_bits_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_random_internal_fast_uniform_bits_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_random_internal_fast_uniform_bits_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_random_internal_fast_uniform_bits_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_random_internal_fast_uniform_bits_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_random_internal_fast_uniform_bits_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_random_internal_fast_uniform_bits_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_random_internal_fast_uniform_bits_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::random_internal_traits VARIABLES ############################################

set(abseil_absl_random_internal_traits_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_random_internal_traits_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_random_internal_traits_BIN_DIRS_RELEASE )
set(abseil_absl_random_internal_traits_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_random_internal_traits_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_random_internal_traits_RES_DIRS_RELEASE )
set(abseil_absl_random_internal_traits_DEFINITIONS_RELEASE )
set(abseil_absl_random_internal_traits_OBJECTS_RELEASE )
set(abseil_absl_random_internal_traits_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_random_internal_traits_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_random_internal_traits_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_random_internal_traits_LIBS_RELEASE )
set(abseil_absl_random_internal_traits_SYSTEM_LIBS_RELEASE )
set(abseil_absl_random_internal_traits_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_random_internal_traits_FRAMEWORKS_RELEASE )
set(abseil_absl_random_internal_traits_DEPENDENCIES_RELEASE absl::config)
set(abseil_absl_random_internal_traits_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_random_internal_traits_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_random_internal_traits_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_random_internal_traits_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_random_internal_traits_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_random_internal_traits_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_random_internal_traits_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_random_internal_traits_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_random_internal_traits_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_random_internal_traits_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::numeric_representation VARIABLES ############################################

set(abseil_absl_numeric_representation_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_numeric_representation_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_numeric_representation_BIN_DIRS_RELEASE )
set(abseil_absl_numeric_representation_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_numeric_representation_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_numeric_representation_RES_DIRS_RELEASE )
set(abseil_absl_numeric_representation_DEFINITIONS_RELEASE )
set(abseil_absl_numeric_representation_OBJECTS_RELEASE )
set(abseil_absl_numeric_representation_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_numeric_representation_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_numeric_representation_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_numeric_representation_LIBS_RELEASE )
set(abseil_absl_numeric_representation_SYSTEM_LIBS_RELEASE )
set(abseil_absl_numeric_representation_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_numeric_representation_FRAMEWORKS_RELEASE )
set(abseil_absl_numeric_representation_DEPENDENCIES_RELEASE absl::config)
set(abseil_absl_numeric_representation_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_numeric_representation_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_numeric_representation_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_numeric_representation_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_numeric_representation_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_numeric_representation_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_numeric_representation_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_numeric_representation_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_numeric_representation_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_numeric_representation_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::requires_internal VARIABLES ############################################

set(abseil_absl_requires_internal_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_requires_internal_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_requires_internal_BIN_DIRS_RELEASE )
set(abseil_absl_requires_internal_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_requires_internal_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_requires_internal_RES_DIRS_RELEASE )
set(abseil_absl_requires_internal_DEFINITIONS_RELEASE )
set(abseil_absl_requires_internal_OBJECTS_RELEASE )
set(abseil_absl_requires_internal_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_requires_internal_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_requires_internal_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_requires_internal_LIBS_RELEASE )
set(abseil_absl_requires_internal_SYSTEM_LIBS_RELEASE )
set(abseil_absl_requires_internal_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_requires_internal_FRAMEWORKS_RELEASE )
set(abseil_absl_requires_internal_DEPENDENCIES_RELEASE absl::config)
set(abseil_absl_requires_internal_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_requires_internal_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_requires_internal_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_requires_internal_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_requires_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_requires_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_requires_internal_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_requires_internal_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_requires_internal_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_requires_internal_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::constexpr_testing_internal VARIABLES ############################################

set(abseil_absl_constexpr_testing_internal_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_constexpr_testing_internal_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_constexpr_testing_internal_BIN_DIRS_RELEASE )
set(abseil_absl_constexpr_testing_internal_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_constexpr_testing_internal_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_constexpr_testing_internal_RES_DIRS_RELEASE )
set(abseil_absl_constexpr_testing_internal_DEFINITIONS_RELEASE )
set(abseil_absl_constexpr_testing_internal_OBJECTS_RELEASE )
set(abseil_absl_constexpr_testing_internal_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_constexpr_testing_internal_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_constexpr_testing_internal_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_constexpr_testing_internal_LIBS_RELEASE )
set(abseil_absl_constexpr_testing_internal_SYSTEM_LIBS_RELEASE )
set(abseil_absl_constexpr_testing_internal_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_constexpr_testing_internal_FRAMEWORKS_RELEASE )
set(abseil_absl_constexpr_testing_internal_DEPENDENCIES_RELEASE absl::config)
set(abseil_absl_constexpr_testing_internal_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_constexpr_testing_internal_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_constexpr_testing_internal_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_constexpr_testing_internal_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_constexpr_testing_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_constexpr_testing_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_constexpr_testing_internal_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_constexpr_testing_internal_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_constexpr_testing_internal_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_constexpr_testing_internal_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::weakly_mixed_integer VARIABLES ############################################

set(abseil_absl_weakly_mixed_integer_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_weakly_mixed_integer_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_weakly_mixed_integer_BIN_DIRS_RELEASE )
set(abseil_absl_weakly_mixed_integer_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_weakly_mixed_integer_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_weakly_mixed_integer_RES_DIRS_RELEASE )
set(abseil_absl_weakly_mixed_integer_DEFINITIONS_RELEASE )
set(abseil_absl_weakly_mixed_integer_OBJECTS_RELEASE )
set(abseil_absl_weakly_mixed_integer_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_weakly_mixed_integer_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_weakly_mixed_integer_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_weakly_mixed_integer_LIBS_RELEASE )
set(abseil_absl_weakly_mixed_integer_SYSTEM_LIBS_RELEASE )
set(abseil_absl_weakly_mixed_integer_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_weakly_mixed_integer_FRAMEWORKS_RELEASE )
set(abseil_absl_weakly_mixed_integer_DEPENDENCIES_RELEASE absl::config)
set(abseil_absl_weakly_mixed_integer_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_weakly_mixed_integer_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_weakly_mixed_integer_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_weakly_mixed_integer_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_weakly_mixed_integer_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_weakly_mixed_integer_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_weakly_mixed_integer_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_weakly_mixed_integer_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_weakly_mixed_integer_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_weakly_mixed_integer_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::utf8_for_code_point VARIABLES ############################################

set(abseil_absl_utf8_for_code_point_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_utf8_for_code_point_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_utf8_for_code_point_BIN_DIRS_RELEASE )
set(abseil_absl_utf8_for_code_point_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_utf8_for_code_point_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_utf8_for_code_point_RES_DIRS_RELEASE )
set(abseil_absl_utf8_for_code_point_DEFINITIONS_RELEASE )
set(abseil_absl_utf8_for_code_point_OBJECTS_RELEASE )
set(abseil_absl_utf8_for_code_point_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_utf8_for_code_point_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_utf8_for_code_point_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_utf8_for_code_point_LIBS_RELEASE absl_utf8_for_code_point)
set(abseil_absl_utf8_for_code_point_SYSTEM_LIBS_RELEASE )
set(abseil_absl_utf8_for_code_point_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_utf8_for_code_point_FRAMEWORKS_RELEASE )
set(abseil_absl_utf8_for_code_point_DEPENDENCIES_RELEASE absl::config)
set(abseil_absl_utf8_for_code_point_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_utf8_for_code_point_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_utf8_for_code_point_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_utf8_for_code_point_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_utf8_for_code_point_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_utf8_for_code_point_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_utf8_for_code_point_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_utf8_for_code_point_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_utf8_for_code_point_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_utf8_for_code_point_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::non_temporal_arm_intrinsics VARIABLES ############################################

set(abseil_absl_non_temporal_arm_intrinsics_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_non_temporal_arm_intrinsics_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_non_temporal_arm_intrinsics_BIN_DIRS_RELEASE )
set(abseil_absl_non_temporal_arm_intrinsics_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_non_temporal_arm_intrinsics_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_non_temporal_arm_intrinsics_RES_DIRS_RELEASE )
set(abseil_absl_non_temporal_arm_intrinsics_DEFINITIONS_RELEASE )
set(abseil_absl_non_temporal_arm_intrinsics_OBJECTS_RELEASE )
set(abseil_absl_non_temporal_arm_intrinsics_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_non_temporal_arm_intrinsics_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_non_temporal_arm_intrinsics_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_non_temporal_arm_intrinsics_LIBS_RELEASE )
set(abseil_absl_non_temporal_arm_intrinsics_SYSTEM_LIBS_RELEASE )
set(abseil_absl_non_temporal_arm_intrinsics_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_non_temporal_arm_intrinsics_FRAMEWORKS_RELEASE )
set(abseil_absl_non_temporal_arm_intrinsics_DEPENDENCIES_RELEASE absl::config)
set(abseil_absl_non_temporal_arm_intrinsics_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_non_temporal_arm_intrinsics_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_non_temporal_arm_intrinsics_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_non_temporal_arm_intrinsics_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_non_temporal_arm_intrinsics_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_non_temporal_arm_intrinsics_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_non_temporal_arm_intrinsics_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_non_temporal_arm_intrinsics_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_non_temporal_arm_intrinsics_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_non_temporal_arm_intrinsics_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::raw_hash_set_resize_impl VARIABLES ############################################

set(abseil_absl_raw_hash_set_resize_impl_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_raw_hash_set_resize_impl_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_raw_hash_set_resize_impl_BIN_DIRS_RELEASE )
set(abseil_absl_raw_hash_set_resize_impl_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_raw_hash_set_resize_impl_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_raw_hash_set_resize_impl_RES_DIRS_RELEASE )
set(abseil_absl_raw_hash_set_resize_impl_DEFINITIONS_RELEASE )
set(abseil_absl_raw_hash_set_resize_impl_OBJECTS_RELEASE )
set(abseil_absl_raw_hash_set_resize_impl_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_raw_hash_set_resize_impl_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_raw_hash_set_resize_impl_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_raw_hash_set_resize_impl_LIBS_RELEASE )
set(abseil_absl_raw_hash_set_resize_impl_SYSTEM_LIBS_RELEASE )
set(abseil_absl_raw_hash_set_resize_impl_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_raw_hash_set_resize_impl_FRAMEWORKS_RELEASE )
set(abseil_absl_raw_hash_set_resize_impl_DEPENDENCIES_RELEASE absl::config)
set(abseil_absl_raw_hash_set_resize_impl_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_raw_hash_set_resize_impl_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_raw_hash_set_resize_impl_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_raw_hash_set_resize_impl_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_raw_hash_set_resize_impl_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_raw_hash_set_resize_impl_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_raw_hash_set_resize_impl_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_raw_hash_set_resize_impl_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_raw_hash_set_resize_impl_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_raw_hash_set_resize_impl_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::node_slot_policy VARIABLES ############################################

set(abseil_absl_node_slot_policy_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_node_slot_policy_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_node_slot_policy_BIN_DIRS_RELEASE )
set(abseil_absl_node_slot_policy_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_node_slot_policy_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_node_slot_policy_RES_DIRS_RELEASE )
set(abseil_absl_node_slot_policy_DEFINITIONS_RELEASE )
set(abseil_absl_node_slot_policy_OBJECTS_RELEASE )
set(abseil_absl_node_slot_policy_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_node_slot_policy_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_node_slot_policy_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_node_slot_policy_LIBS_RELEASE )
set(abseil_absl_node_slot_policy_SYSTEM_LIBS_RELEASE )
set(abseil_absl_node_slot_policy_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_node_slot_policy_FRAMEWORKS_RELEASE )
set(abseil_absl_node_slot_policy_DEPENDENCIES_RELEASE absl::config)
set(abseil_absl_node_slot_policy_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_node_slot_policy_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_node_slot_policy_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_node_slot_policy_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_node_slot_policy_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_node_slot_policy_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_node_slot_policy_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_node_slot_policy_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_node_slot_policy_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_node_slot_policy_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::hashtable_debug_hooks VARIABLES ############################################

set(abseil_absl_hashtable_debug_hooks_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_hashtable_debug_hooks_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_hashtable_debug_hooks_BIN_DIRS_RELEASE )
set(abseil_absl_hashtable_debug_hooks_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_hashtable_debug_hooks_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_hashtable_debug_hooks_RES_DIRS_RELEASE )
set(abseil_absl_hashtable_debug_hooks_DEFINITIONS_RELEASE )
set(abseil_absl_hashtable_debug_hooks_OBJECTS_RELEASE )
set(abseil_absl_hashtable_debug_hooks_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_hashtable_debug_hooks_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_hashtable_debug_hooks_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_hashtable_debug_hooks_LIBS_RELEASE )
set(abseil_absl_hashtable_debug_hooks_SYSTEM_LIBS_RELEASE )
set(abseil_absl_hashtable_debug_hooks_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_hashtable_debug_hooks_FRAMEWORKS_RELEASE )
set(abseil_absl_hashtable_debug_hooks_DEPENDENCIES_RELEASE absl::config)
set(abseil_absl_hashtable_debug_hooks_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_hashtable_debug_hooks_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_hashtable_debug_hooks_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_hashtable_debug_hooks_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_hashtable_debug_hooks_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_hashtable_debug_hooks_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_hashtable_debug_hooks_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_hashtable_debug_hooks_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_hashtable_debug_hooks_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_hashtable_debug_hooks_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::algorithm VARIABLES ############################################

set(abseil_absl_algorithm_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_algorithm_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_algorithm_BIN_DIRS_RELEASE )
set(abseil_absl_algorithm_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_algorithm_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_algorithm_RES_DIRS_RELEASE )
set(abseil_absl_algorithm_DEFINITIONS_RELEASE )
set(abseil_absl_algorithm_OBJECTS_RELEASE )
set(abseil_absl_algorithm_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_algorithm_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_algorithm_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_algorithm_LIBS_RELEASE )
set(abseil_absl_algorithm_SYSTEM_LIBS_RELEASE )
set(abseil_absl_algorithm_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_algorithm_FRAMEWORKS_RELEASE )
set(abseil_absl_algorithm_DEPENDENCIES_RELEASE absl::config)
set(abseil_absl_algorithm_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_algorithm_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_algorithm_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_algorithm_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_algorithm_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_algorithm_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_algorithm_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_algorithm_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_algorithm_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_algorithm_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::iterator_traits_test_helper_internal VARIABLES ############################################

set(abseil_absl_iterator_traits_test_helper_internal_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_iterator_traits_test_helper_internal_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_iterator_traits_test_helper_internal_BIN_DIRS_RELEASE )
set(abseil_absl_iterator_traits_test_helper_internal_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_iterator_traits_test_helper_internal_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_iterator_traits_test_helper_internal_RES_DIRS_RELEASE )
set(abseil_absl_iterator_traits_test_helper_internal_DEFINITIONS_RELEASE )
set(abseil_absl_iterator_traits_test_helper_internal_OBJECTS_RELEASE )
set(abseil_absl_iterator_traits_test_helper_internal_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_iterator_traits_test_helper_internal_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_iterator_traits_test_helper_internal_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_iterator_traits_test_helper_internal_LIBS_RELEASE )
set(abseil_absl_iterator_traits_test_helper_internal_SYSTEM_LIBS_RELEASE )
set(abseil_absl_iterator_traits_test_helper_internal_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_iterator_traits_test_helper_internal_FRAMEWORKS_RELEASE )
set(abseil_absl_iterator_traits_test_helper_internal_DEPENDENCIES_RELEASE absl::config)
set(abseil_absl_iterator_traits_test_helper_internal_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_iterator_traits_test_helper_internal_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_iterator_traits_test_helper_internal_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_iterator_traits_test_helper_internal_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_iterator_traits_test_helper_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_iterator_traits_test_helper_internal_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_iterator_traits_test_helper_internal_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_iterator_traits_test_helper_internal_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_iterator_traits_test_helper_internal_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_iterator_traits_test_helper_internal_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::fast_type_id VARIABLES ############################################

set(abseil_absl_fast_type_id_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_fast_type_id_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_fast_type_id_BIN_DIRS_RELEASE )
set(abseil_absl_fast_type_id_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_fast_type_id_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_fast_type_id_RES_DIRS_RELEASE )
set(abseil_absl_fast_type_id_DEFINITIONS_RELEASE )
set(abseil_absl_fast_type_id_OBJECTS_RELEASE )
set(abseil_absl_fast_type_id_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_fast_type_id_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_fast_type_id_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_fast_type_id_LIBS_RELEASE )
set(abseil_absl_fast_type_id_SYSTEM_LIBS_RELEASE )
set(abseil_absl_fast_type_id_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_fast_type_id_FRAMEWORKS_RELEASE )
set(abseil_absl_fast_type_id_DEPENDENCIES_RELEASE absl::config)
set(abseil_absl_fast_type_id_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_fast_type_id_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_fast_type_id_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_fast_type_id_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_fast_type_id_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_fast_type_id_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_fast_type_id_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_fast_type_id_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_fast_type_id_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_fast_type_id_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::core_headers VARIABLES ############################################

set(abseil_absl_core_headers_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_core_headers_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_core_headers_BIN_DIRS_RELEASE )
set(abseil_absl_core_headers_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_core_headers_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_core_headers_RES_DIRS_RELEASE )
set(abseil_absl_core_headers_DEFINITIONS_RELEASE )
set(abseil_absl_core_headers_OBJECTS_RELEASE )
set(abseil_absl_core_headers_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_core_headers_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_core_headers_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_core_headers_LIBS_RELEASE )
set(abseil_absl_core_headers_SYSTEM_LIBS_RELEASE )
set(abseil_absl_core_headers_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_core_headers_FRAMEWORKS_RELEASE )
set(abseil_absl_core_headers_DEPENDENCIES_RELEASE absl::config)
set(abseil_absl_core_headers_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_core_headers_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_core_headers_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_core_headers_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_core_headers_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_core_headers_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_core_headers_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_core_headers_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_core_headers_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_core_headers_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::dynamic_annotations VARIABLES ############################################

set(abseil_absl_dynamic_annotations_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_dynamic_annotations_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_dynamic_annotations_BIN_DIRS_RELEASE )
set(abseil_absl_dynamic_annotations_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_dynamic_annotations_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_dynamic_annotations_RES_DIRS_RELEASE )
set(abseil_absl_dynamic_annotations_DEFINITIONS_RELEASE )
set(abseil_absl_dynamic_annotations_OBJECTS_RELEASE )
set(abseil_absl_dynamic_annotations_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_dynamic_annotations_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_dynamic_annotations_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_dynamic_annotations_LIBS_RELEASE )
set(abseil_absl_dynamic_annotations_SYSTEM_LIBS_RELEASE )
set(abseil_absl_dynamic_annotations_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_dynamic_annotations_FRAMEWORKS_RELEASE )
set(abseil_absl_dynamic_annotations_DEPENDENCIES_RELEASE absl::config)
set(abseil_absl_dynamic_annotations_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_dynamic_annotations_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_dynamic_annotations_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_dynamic_annotations_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_dynamic_annotations_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_dynamic_annotations_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_dynamic_annotations_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_dynamic_annotations_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_dynamic_annotations_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_dynamic_annotations_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::nullability VARIABLES ############################################

set(abseil_absl_nullability_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_nullability_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_nullability_BIN_DIRS_RELEASE )
set(abseil_absl_nullability_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_nullability_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_nullability_RES_DIRS_RELEASE )
set(abseil_absl_nullability_DEFINITIONS_RELEASE )
set(abseil_absl_nullability_OBJECTS_RELEASE )
set(abseil_absl_nullability_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_nullability_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_nullability_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_nullability_LIBS_RELEASE )
set(abseil_absl_nullability_SYSTEM_LIBS_RELEASE )
set(abseil_absl_nullability_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_nullability_FRAMEWORKS_RELEASE )
set(abseil_absl_nullability_DEPENDENCIES_RELEASE absl::config)
set(abseil_absl_nullability_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_nullability_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_nullability_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_nullability_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_nullability_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_nullability_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_nullability_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_nullability_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_nullability_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_nullability_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::errno_saver VARIABLES ############################################

set(abseil_absl_errno_saver_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_errno_saver_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_errno_saver_BIN_DIRS_RELEASE )
set(abseil_absl_errno_saver_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_errno_saver_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_errno_saver_RES_DIRS_RELEASE )
set(abseil_absl_errno_saver_DEFINITIONS_RELEASE )
set(abseil_absl_errno_saver_OBJECTS_RELEASE )
set(abseil_absl_errno_saver_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_errno_saver_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_errno_saver_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_errno_saver_LIBS_RELEASE )
set(abseil_absl_errno_saver_SYSTEM_LIBS_RELEASE )
set(abseil_absl_errno_saver_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_errno_saver_FRAMEWORKS_RELEASE )
set(abseil_absl_errno_saver_DEPENDENCIES_RELEASE absl::config)
set(abseil_absl_errno_saver_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_errno_saver_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_errno_saver_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_errno_saver_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_errno_saver_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_errno_saver_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_errno_saver_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_errno_saver_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_errno_saver_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_errno_saver_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::bad_variant_access VARIABLES ############################################

set(abseil_absl_bad_variant_access_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_bad_variant_access_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_bad_variant_access_BIN_DIRS_RELEASE )
set(abseil_absl_bad_variant_access_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_bad_variant_access_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_bad_variant_access_RES_DIRS_RELEASE )
set(abseil_absl_bad_variant_access_DEFINITIONS_RELEASE )
set(abseil_absl_bad_variant_access_OBJECTS_RELEASE )
set(abseil_absl_bad_variant_access_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_bad_variant_access_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_bad_variant_access_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_bad_variant_access_LIBS_RELEASE )
set(abseil_absl_bad_variant_access_SYSTEM_LIBS_RELEASE )
set(abseil_absl_bad_variant_access_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_bad_variant_access_FRAMEWORKS_RELEASE )
set(abseil_absl_bad_variant_access_DEPENDENCIES_RELEASE )
set(abseil_absl_bad_variant_access_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_bad_variant_access_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_bad_variant_access_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_bad_variant_access_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_bad_variant_access_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_bad_variant_access_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_bad_variant_access_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_bad_variant_access_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_bad_variant_access_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_bad_variant_access_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::bad_optional_access VARIABLES ############################################

set(abseil_absl_bad_optional_access_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_bad_optional_access_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_bad_optional_access_BIN_DIRS_RELEASE )
set(abseil_absl_bad_optional_access_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_bad_optional_access_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_bad_optional_access_RES_DIRS_RELEASE )
set(abseil_absl_bad_optional_access_DEFINITIONS_RELEASE )
set(abseil_absl_bad_optional_access_OBJECTS_RELEASE )
set(abseil_absl_bad_optional_access_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_bad_optional_access_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_bad_optional_access_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_bad_optional_access_LIBS_RELEASE )
set(abseil_absl_bad_optional_access_SYSTEM_LIBS_RELEASE )
set(abseil_absl_bad_optional_access_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_bad_optional_access_FRAMEWORKS_RELEASE )
set(abseil_absl_bad_optional_access_DEPENDENCIES_RELEASE )
set(abseil_absl_bad_optional_access_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_bad_optional_access_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_bad_optional_access_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_bad_optional_access_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_bad_optional_access_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_bad_optional_access_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_bad_optional_access_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_bad_optional_access_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_bad_optional_access_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_bad_optional_access_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::bad_any_cast VARIABLES ############################################

set(abseil_absl_bad_any_cast_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_bad_any_cast_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_bad_any_cast_BIN_DIRS_RELEASE )
set(abseil_absl_bad_any_cast_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_bad_any_cast_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_bad_any_cast_RES_DIRS_RELEASE )
set(abseil_absl_bad_any_cast_DEFINITIONS_RELEASE )
set(abseil_absl_bad_any_cast_OBJECTS_RELEASE )
set(abseil_absl_bad_any_cast_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_bad_any_cast_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_bad_any_cast_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_bad_any_cast_LIBS_RELEASE )
set(abseil_absl_bad_any_cast_SYSTEM_LIBS_RELEASE )
set(abseil_absl_bad_any_cast_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_bad_any_cast_FRAMEWORKS_RELEASE )
set(abseil_absl_bad_any_cast_DEPENDENCIES_RELEASE )
set(abseil_absl_bad_any_cast_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_bad_any_cast_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_bad_any_cast_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_bad_any_cast_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_bad_any_cast_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_bad_any_cast_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_bad_any_cast_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_bad_any_cast_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_bad_any_cast_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_bad_any_cast_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::time_zone VARIABLES ############################################

set(abseil_absl_time_zone_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_time_zone_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_time_zone_BIN_DIRS_RELEASE )
set(abseil_absl_time_zone_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_time_zone_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_time_zone_RES_DIRS_RELEASE )
set(abseil_absl_time_zone_DEFINITIONS_RELEASE )
set(abseil_absl_time_zone_OBJECTS_RELEASE )
set(abseil_absl_time_zone_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_time_zone_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_time_zone_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_time_zone_LIBS_RELEASE absl_time_zone)
set(abseil_absl_time_zone_SYSTEM_LIBS_RELEASE pthread)
set(abseil_absl_time_zone_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_time_zone_FRAMEWORKS_RELEASE )
set(abseil_absl_time_zone_DEPENDENCIES_RELEASE )
set(abseil_absl_time_zone_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_time_zone_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_time_zone_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_time_zone_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_time_zone_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_time_zone_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_time_zone_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_time_zone_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_time_zone_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_time_zone_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::civil_time VARIABLES ############################################

set(abseil_absl_civil_time_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_civil_time_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_civil_time_BIN_DIRS_RELEASE )
set(abseil_absl_civil_time_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_civil_time_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_civil_time_RES_DIRS_RELEASE )
set(abseil_absl_civil_time_DEFINITIONS_RELEASE )
set(abseil_absl_civil_time_OBJECTS_RELEASE )
set(abseil_absl_civil_time_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_civil_time_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_civil_time_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_civil_time_LIBS_RELEASE absl_civil_time)
set(abseil_absl_civil_time_SYSTEM_LIBS_RELEASE )
set(abseil_absl_civil_time_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_civil_time_FRAMEWORKS_RELEASE )
set(abseil_absl_civil_time_DEPENDENCIES_RELEASE )
set(abseil_absl_civil_time_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_civil_time_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_civil_time_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_civil_time_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_civil_time_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_civil_time_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_civil_time_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_civil_time_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_civil_time_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_civil_time_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::pretty_function VARIABLES ############################################

set(abseil_absl_pretty_function_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_pretty_function_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_pretty_function_BIN_DIRS_RELEASE )
set(abseil_absl_pretty_function_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_pretty_function_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_pretty_function_RES_DIRS_RELEASE )
set(abseil_absl_pretty_function_DEFINITIONS_RELEASE )
set(abseil_absl_pretty_function_OBJECTS_RELEASE )
set(abseil_absl_pretty_function_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_pretty_function_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_pretty_function_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_pretty_function_LIBS_RELEASE )
set(abseil_absl_pretty_function_SYSTEM_LIBS_RELEASE )
set(abseil_absl_pretty_function_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_pretty_function_FRAMEWORKS_RELEASE )
set(abseil_absl_pretty_function_DEPENDENCIES_RELEASE )
set(abseil_absl_pretty_function_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_pretty_function_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_pretty_function_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_pretty_function_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_pretty_function_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_pretty_function_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_pretty_function_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_pretty_function_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_pretty_function_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_pretty_function_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT absl::config VARIABLES ############################################

set(abseil_absl_config_INCLUDE_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/include")
set(abseil_absl_config_LIB_DIRS_RELEASE "${abseil_PACKAGE_FOLDER_RELEASE}/lib")
set(abseil_absl_config_BIN_DIRS_RELEASE )
set(abseil_absl_config_LIBRARY_TYPE_RELEASE STATIC)
set(abseil_absl_config_IS_HOST_WINDOWS_RELEASE 0)
set(abseil_absl_config_RES_DIRS_RELEASE )
set(abseil_absl_config_DEFINITIONS_RELEASE )
set(abseil_absl_config_OBJECTS_RELEASE )
set(abseil_absl_config_COMPILE_DEFINITIONS_RELEASE )
set(abseil_absl_config_COMPILE_OPTIONS_C_RELEASE "")
set(abseil_absl_config_COMPILE_OPTIONS_CXX_RELEASE "")
set(abseil_absl_config_LIBS_RELEASE )
set(abseil_absl_config_SYSTEM_LIBS_RELEASE )
set(abseil_absl_config_FRAMEWORK_DIRS_RELEASE )
set(abseil_absl_config_FRAMEWORKS_RELEASE )
set(abseil_absl_config_DEPENDENCIES_RELEASE )
set(abseil_absl_config_SHARED_LINK_FLAGS_RELEASE )
set(abseil_absl_config_EXE_LINK_FLAGS_RELEASE )
set(abseil_absl_config_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(abseil_absl_config_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${abseil_absl_config_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${abseil_absl_config_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${abseil_absl_config_EXE_LINK_FLAGS_RELEASE}>
)
set(abseil_absl_config_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${abseil_absl_config_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${abseil_absl_config_COMPILE_OPTIONS_C_RELEASE}>")