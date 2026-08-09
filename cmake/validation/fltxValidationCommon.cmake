# Shared configuration for validation executables and libraries.

function(fltx_tests_build_configuration_identity config output_hash output_optimized)
    string(TOUPPER "${config}" config_upper)
    set(compile_flags
        "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${config_upper}}"
    )
    set(link_flags
        "${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_EXE_LINKER_FLAGS_${config_upper}}"
    )
    set(identity
        "compiler=${CMAKE_CXX_COMPILER}|id=${CMAKE_CXX_COMPILER_ID}|"
        "version=${CMAKE_CXX_COMPILER_VERSION}|generator=${CMAKE_GENERATOR}|"
        "config=${config}|cxx=${compile_flags}|link=${link_flags}|"
        "ipo=${CMAKE_INTERPROCEDURAL_OPTIMIZATION_${config_upper}}"
    )
    string(SHA256 flags_hash "${identity}")

    # Read the flags in command-line order so an explicit trailing /Od or -O0
    # cannot be published merely because the configuration is named Release.
    separate_arguments(flag_tokens NATIVE_COMMAND "${compile_flags}")
    set(optimized 0)
    foreach(flag IN LISTS flag_tokens)
        if(MSVC)
            if(flag MATCHES "^[-/]Od$")
                set(optimized 0)
            elseif(flag MATCHES "^[-/]O(1|2|x)$")
                set(optimized 1)
            endif()
        else()
            if(flag MATCHES "^-O0$" OR flag MATCHES "^-Og$")
                set(optimized 0)
            elseif(flag MATCHES "^-O([1-3s]|fast)$" OR flag STREQUAL "-O")
                set(optimized 1)
            endif()
        endif()
    endforeach()

    set(${output_hash} "${flags_hash}" PARENT_SCOPE)
    set(${output_optimized} "${optimized}" PARENT_SCOPE)
endfunction()

foreach(config Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER "${config}" config_upper)
    fltx_tests_build_configuration_identity(
        "${config}"
        "FLTX_TESTS_FLAGS_HASH_${config_upper}"
        "FLTX_TESTS_OPTIMIZED_${config_upper}"
    )
endforeach()

function(fltx_tests_add_build_identity target)
    string(CONCAT flags_hash
        "$<IF:$<CONFIG:Release>,${FLTX_TESTS_FLAGS_HASH_RELEASE},"
        "$<IF:$<CONFIG:RelWithDebInfo>,${FLTX_TESTS_FLAGS_HASH_RELWITHDEBINFO},"
        "$<IF:$<CONFIG:MinSizeRel>,${FLTX_TESTS_FLAGS_HASH_MINSIZEREL},"
        "${FLTX_TESTS_FLAGS_HASH_DEBUG}>>>"
    )
    string(CONCAT optimized
        "$<IF:$<CONFIG:Release>,${FLTX_TESTS_OPTIMIZED_RELEASE},"
        "$<IF:$<CONFIG:RelWithDebInfo>,${FLTX_TESTS_OPTIMIZED_RELWITHDEBINFO},"
        "$<IF:$<CONFIG:MinSizeRel>,${FLTX_TESTS_OPTIMIZED_MINSIZEREL},"
        "${FLTX_TESTS_OPTIMIZED_DEBUG}>>>"
    )
    target_compile_definitions(${target} PRIVATE
        "FLTX_TESTS_COMPILER_ID=\"${CMAKE_CXX_COMPILER_ID}\""
        "FLTX_TESTS_COMPILER_VERSION=\"${CMAKE_CXX_COMPILER_VERSION}\""
        "FLTX_TESTS_BUILD_CONFIG=\"$<CONFIG>\""
        "FLTX_TESTS_SYSTEM_NAME=\"${CMAKE_SYSTEM_NAME}\""
        "FLTX_TESTS_SYSTEM_PROCESSOR=\"${CMAKE_SYSTEM_PROCESSOR}\""
        "FLTX_TESTS_BUILD_FLAGS_HASH=\"${flags_hash}\""
        "FLTX_TESTS_BUILD_OPTIMIZED=${optimized}"
    )
endfunction()

function(fltx_tests_prepare_target target)
    target_compile_features(${target} PRIVATE cxx_std_20)
    fltx_tests_add_build_identity(${target})

    if(MSVC)
        target_compile_options(${target} PRIVATE /bigobj)
    elseif(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${target} PRIVATE -Wa,-mbig-obj)
    endif()

    fltx_configure_public_header_contract(${target} PRIVATE)
    fltx_apply_private_wasm_simd_options(${target})
    fltx_enable_msvc_parallel_compile(${target})
endfunction()

function(fltx_tests_configure_consumer_mode target mode)
    string(TOUPPER "${mode}" normalized_mode)
    if(normalized_mode STREQUAL "ON")
        set(expected_fast_math 1)
    elseif(normalized_mode STREQUAL "OFF")
        set(expected_fast_math 0)
    else()
        message(FATAL_ERROR
            "Invalid validation consumer fast-math mode '${mode}'."
        )
    endif()

    fltx_configure_consumer_fast_math(${target} MODE "${normalized_mode}")
    target_compile_definitions(${target} PRIVATE
        "FLTX_TESTS_EXPECT_CONSUMER_FAST_MATH=${expected_fast_math}"
    )
endfunction()

function(fltx_tests_prepare_test target label library)
    target_link_libraries(${target} PRIVATE ${library} Catch2::Catch2)
    target_include_directories(${target} PRIVATE "${FLTX_TESTS_SUPPORT_DIR}")
    target_compile_definitions(${target} PRIVATE "FLTX_TESTS_NAME=\"${label}\"")
    fltx_tests_prepare_target(${target})

    if(EMSCRIPTEN)
        # Catch2 must be able to observe public APIs which report errors by
        # throwing. Emscripten disables exception catching unless the final
        # application explicitly opts in.
        target_compile_options(${target} PRIVATE -fexceptions)
        target_link_options(${target} PRIVATE -fexceptions)
    endif()

    if(BUILD_TESTING)
        if(EMSCRIPTEN)
            add_test(NAME ${target} COMMAND ${target})
            set_tests_properties(${target} PROPERTIES
                LABELS "fltx;${label}"
            )
        else()
            catch_discover_tests(
                ${target}
                TEST_PREFIX "${target}::"
                # catch_discover_tests forwards PROPERTIES as a list. A
                # semicolon-separated label value is therefore emitted as
                # separate property arguments; keep the suite label singular
                # so `ctest -L` reliably selects these discovered cases.
                PROPERTIES LABELS "${label}"
                DISCOVERY_MODE POST_BUILD
            )
        endif()
    endif()
endfunction()

function(fltx_tests_prepare_runner target)
    set(library fltx::fltx)
    if(ARGC GREATER 1)
        set(library "${ARGV1}")
    endif()

    target_link_libraries(${target} PRIVATE ${library})
    fltx_tests_prepare_target(${target})

    if(EMSCRIPTEN)
        # Boost's libc++ integration currently uses deprecated allocator
        # members under modern standard modes. Keep that third-party noise out
        # of metrics builds without weakening warnings for the library itself.
        target_compile_options(${target} PRIVATE
            -fexceptions
            -Wno-deprecated-declarations
        )
        target_link_options(${target} PRIVATE
            -fexceptions
            -sNODERAWFS=1
            -sALLOW_MEMORY_GROWTH=1
            -sINITIAL_MEMORY=268435456
            -sMAXIMUM_MEMORY=1073741824
        )
    endif()
endfunction()
