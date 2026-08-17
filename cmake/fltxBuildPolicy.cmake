include_guard(GLOBAL)

set(FLTX_BUILD_POLICY_INCLUDED TRUE)

option(
    FLTX_DEVELOPER_BUILD
    "Build fltx developer targets: tests, examples, and isolated compile units."
    "${PROJECT_IS_TOP_LEVEL}"
)

set(
    FLTX_INTERNAL_FAST_MATH
    AUTO
    CACHE STRING "Fast-math mode for fltx compiled runtime sources: ON, OFF, or AUTO (validated platforms only)."
)
set_property(CACHE FLTX_INTERNAL_FAST_MATH PROPERTY STRINGS ON OFF AUTO)

option(
    FLTX_SIMD
    "Enable SIMD for fltx compiled sources and supported consuming translation units."
    ON
)

set(
    FLTX_FMA_MODE
    AUTO
    CACHE STRING "FMA policy for fltx and consuming translation units: AUTO, OFF, or ASSUME."
)
set_property(CACHE FLTX_FMA_MODE PROPERTY STRINGS AUTO OFF ASSUME)

option(
    FLTX_MSVC_TIMING_REPORTS
    "Emit MSVC compiler and linker timing reports for fltx developer targets."
    "${FLTX_DEVELOPER_BUILD}"
)

option(
    FLTX_MSVC_PARALLEL_COMPILE
    "Enable MSVC /MP multi-processor compilation for eligible fltx targets."
    "${FLTX_DEVELOPER_BUILD}"
)

option(
    FLTX_MSVC_DETAILED_TIMING_REPORTS
    "Emit detailed MSVC frontend and codegen timing reports for fltx developer targets."
    OFF
)

mark_as_advanced(
    FLTX_MSVC_TIMING_REPORTS
    FLTX_MSVC_PARALLEL_COMPILE
    FLTX_MSVC_DETAILED_TIMING_REPORTS
)

set(_FLTX_FAST_MATH_SUPPORTED OFF)
if(MSVC OR CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang|AppleClang)$")
    set(_FLTX_FAST_MATH_SUPPORTED ON)
endif()

# AUTO is an approval list, not a compiler-capability check. Add a platform
# here only after its required accuracy/special-value suite passes and its
# representative compiled-runtime benchmark set improves geometrically by 3%.
set(_FLTX_INTERNAL_FAST_MATH_AUTO_APPROVED OFF)

set(_FLTX_TARGET_PROCESSOR "${CMAKE_SYSTEM_PROCESSOR};${CMAKE_VS_PLATFORM_NAME}")
string(TOLOWER "${_FLTX_TARGET_PROCESSOR}" _FLTX_TARGET_PROCESSOR)

set(_FLTX_X86_FMA_TARGET_OPTIONS_SUPPORTED OFF)
if(_FLTX_TARGET_PROCESSOR MATCHES "(^|;)(x86_64|amd64|x64|win32|i[3-6]86)(;|$)")
    set(_FLTX_X86_FMA_TARGET_OPTIONS_SUPPORTED ON)
endif()

set(_FLTX_CLANG_CL OFF)
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND
   CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    set(_FLTX_CLANG_CL ON)
endif()

function(fltx_resolve_fast_math_mode _OUT _MODE)
    string(TOUPPER "${_MODE}" _FLTX_MODE)

    if(_FLTX_MODE STREQUAL "AUTO")
        set(${_OUT} ${_FLTX_FAST_MATH_SUPPORTED} PARENT_SCOPE)
    elseif(_FLTX_MODE STREQUAL "ON")
        if(_FLTX_FAST_MATH_SUPPORTED)
            set(${_OUT} ON PARENT_SCOPE)
        else()
            message(WARNING "fltx fast-math mode is ON, but no supported option set exists for ${CMAKE_CXX_COMPILER_ID}/${CMAKE_SYSTEM_NAME}; no fast-math options will be applied")
            set(${_OUT} OFF PARENT_SCOPE)
        endif()
    elseif(_FLTX_MODE STREQUAL "OFF")
        set(${_OUT} OFF PARENT_SCOPE)
    else()
        message(FATAL_ERROR "Invalid fltx fast-math mode '${_MODE}'. Expected AUTO, ON, or OFF.")
    endif()
endfunction()

function(fltx_resolve_internal_fast_math_mode _OUT _MODE)
    string(TOUPPER "${_MODE}" _FLTX_MODE)

    if(_FLTX_MODE STREQUAL "AUTO")
        set(${_OUT} ${_FLTX_INTERNAL_FAST_MATH_AUTO_APPROVED} PARENT_SCOPE)
        return()
    endif()

    fltx_resolve_fast_math_mode(_FLTX_EXPLICIT_FAST_MATH "${_MODE}")
    set(${_OUT} ${_FLTX_EXPLICIT_FAST_MATH} PARENT_SCOPE)
endfunction()

function(fltx_target_link_options_if_supported _TARGET)
    get_target_property(_TARGET_TYPE ${_TARGET} TYPE)

    if(_TARGET_TYPE STREQUAL "EXECUTABLE" OR
       _TARGET_TYPE STREQUAL "SHARED_LIBRARY" OR
       _TARGET_TYPE STREQUAL "MODULE_LIBRARY")
        target_link_options(${_TARGET} PRIVATE ${ARGN})
    endif()
endfunction()

function(fltx_apply_private_wasm_simd_options _TARGET)
    if(NOT EMSCRIPTEN OR NOT FLTX_SIMD)
        return()
    endif()

    target_compile_options(${_TARGET} PRIVATE -msimd128)
    fltx_target_link_options_if_supported(${_TARGET} -msimd128)
endfunction()

function(fltx_apply_private_fast_math_options _TARGET _MODE)
    fltx_resolve_fast_math_mode(_FLTX_ENABLE_TARGET_FAST_MATH "${_MODE}")
    if(NOT _FLTX_ENABLE_TARGET_FAST_MATH)
        return()
    endif()

    target_compile_definitions(${_TARGET} PRIVATE FLTX_FAST_MATH)

    if(MSVC)
        target_compile_options(${_TARGET} PRIVATE /fp:fast)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${_TARGET} PRIVATE
            -ffp-contract=fast
            -fno-math-errno
            -fno-trapping-math
        )
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "^(Clang|AppleClang)$")
        # Clang's plain "fast" mode can override source pragmas. This variant
        # keeps contraction fast outside protected error-free-transform blocks
        # while allowing their clang fp contract(off) directives to take effect.
        target_compile_definitions(${_TARGET} PRIVATE FLTX_DETAIL_FP_CONTRACT_FAST=1)
        target_compile_options(${_TARGET} PRIVATE
            -ffp-contract=fast-honor-pragmas
            -fno-math-errno
            -fno-trapping-math
        )
    endif()
endfunction()

function(fltx_configure_compiled_x86_fma_backend _TARGET)
    string(TOUPPER "${FLTX_FMA_MODE}" _FLTX_FMA_MODE_NORMALIZED)
    if(NOT _FLTX_FMA_MODE_NORMALIZED STREQUAL "AUTO" OR
       EMSCRIPTEN OR
       MSVC OR
       NOT _FLTX_X86_FMA_TARGET_OPTIONS_SUPPORTED OR
       NOT CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang|AppleClang)$")
        return()
    endif()

    get_target_property(_FLTX_TARGET_SOURCES ${_TARGET} SOURCES)
    foreach(_FLTX_SOURCE IN LISTS _FLTX_TARGET_SOURCES)
        if(_FLTX_SOURCE MATCHES "(^|[/\\\\])fma_x86\\.cpp$")
            set_property(SOURCE "${_FLTX_SOURCE}" APPEND PROPERTY COMPILE_OPTIONS -mfma)
            set_property(SOURCE "${_FLTX_SOURCE}" APPEND PROPERTY COMPILE_DEFINITIONS
                FLTX_BUILD_X86_FMA_BACKEND=1
            )
        endif()
    endforeach()
endfunction()

function(fltx_configure_public_header_contract _TARGET)
    set(_FLTX_VISIBILITY PUBLIC)
    if(ARGC GREATER 1)
        set(_FLTX_VISIBILITY "${ARGV1}")
    endif()

    string(TOUPPER "${FLTX_FMA_MODE}" _FLTX_FMA_MODE_NORMALIZED)
    if(NOT _FLTX_FMA_MODE_NORMALIZED MATCHES "^(AUTO|OFF|ASSUME)$")
        message(FATAL_ERROR "Invalid FLTX_FMA_MODE='${FLTX_FMA_MODE}'. Expected AUTO, OFF, or ASSUME.")
    endif()

    set(_FLTX_HEADER_DEFINITIONS)
    if(NOT FLTX_SIMD)
        list(APPEND _FLTX_HEADER_DEFINITIONS FLTX_HEADER_SIMD_OFF=1)
    endif()

    if(_FLTX_FMA_MODE_NORMALIZED STREQUAL "OFF")
        list(APPEND _FLTX_HEADER_DEFINITIONS FLTX_HEADER_FMA_OFF=1)
    elseif(_FLTX_FMA_MODE_NORMALIZED STREQUAL "ASSUME")
        list(APPEND _FLTX_HEADER_DEFINITIONS FLTX_HEADER_FMA_ASSUME=1)
    elseif(_FLTX_X86_FMA_TARGET_OPTIONS_SUPPORTED AND
           NOT EMSCRIPTEN AND
           NOT MSVC AND
           CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang|AppleClang)$")
        list(APPEND _FLTX_HEADER_DEFINITIONS FLTX_HAS_COMPILED_X86_FMA_BACKEND=1)
    endif()

    if(_FLTX_HEADER_DEFINITIONS)
        target_compile_definitions(${_TARGET} ${_FLTX_VISIBILITY} ${_FLTX_HEADER_DEFINITIONS})
    endif()

    if(_FLTX_FMA_MODE_NORMALIZED STREQUAL "ASSUME" AND
       _FLTX_X86_FMA_TARGET_OPTIONS_SUPPORTED AND
       NOT EMSCRIPTEN)
        if(_FLTX_CLANG_CL)
            target_compile_options(${_TARGET} ${_FLTX_VISIBILITY} /clang:-mfma)
        elseif(NOT MSVC AND CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang|AppleClang)$")
            target_compile_options(${_TARGET} ${_FLTX_VISIBILITY} -mfma)
        endif()
    endif()

    if(EMSCRIPTEN AND FLTX_SIMD)
        target_compile_options(${_TARGET} ${_FLTX_VISIBILITY} -msimd128)
        if(_FLTX_VISIBILITY STREQUAL "PUBLIC" OR _FLTX_VISIBILITY STREQUAL "INTERFACE")
            target_link_options(${_TARGET} INTERFACE -msimd128)
        endif()
    endif()
endfunction()

function(fltx_configure_internal_target _TARGET)
    cmake_parse_arguments(FLTX_INTERNAL "" "FAST_MATH_MODE" "" ${ARGN})

    # Consumer -ffast-math can enable process-wide FTZ/DAZ on MinGW, while
    # Emscripten may optimize the final linked module as a whole. Compile the
    # affected runtime implementations with their local evaluation barriers so
    # public APIs can retain the normal out-of-line dispatch boundary.
    if(MINGW OR EMSCRIPTEN)
        target_compile_definitions(${_TARGET} PRIVATE
            FLTX_DETAIL_COMPILED_RUNTIME_FP_BARRIERS=1
        )
    endif()

    if(DEFINED FLTX_INTERNAL_FAST_MATH_MODE)
        set(_FLTX_INTERNAL_FAST_MATH_MODE "${FLTX_INTERNAL_FAST_MATH_MODE}")
    else()
        set(_FLTX_INTERNAL_FAST_MATH_MODE "${FLTX_INTERNAL_FAST_MATH}")
    endif()

    string(TOUPPER "${_FLTX_INTERNAL_FAST_MATH_MODE}" _FLTX_INTERNAL_FAST_MATH_MODE_NORMALIZED)
    if(NOT _FLTX_INTERNAL_FAST_MATH_MODE_NORMALIZED MATCHES "^(AUTO|ON|OFF)$")
        message(FATAL_ERROR
            "Invalid internal fast-math mode '${_FLTX_INTERNAL_FAST_MATH_MODE}'. "
            "Expected AUTO, ON, or OFF."
        )
    endif()
    target_compile_definitions(${_TARGET} PRIVATE
        "FLTX_INTERNAL_FAST_MATH_REQUEST_${_FLTX_INTERNAL_FAST_MATH_MODE_NORMALIZED}=1"
    )

    fltx_resolve_internal_fast_math_mode(
        _FLTX_ENABLE_INTERNAL_FAST_MATH
        "${_FLTX_INTERNAL_FAST_MATH_MODE}"
    )
    if(_FLTX_ENABLE_INTERNAL_FAST_MATH)
        fltx_apply_private_fast_math_options(${_TARGET} ON)
    endif()

    if(NOT FLTX_SIMD)
        target_compile_definitions(${_TARGET} PRIVATE
            FLTX_HEADER_SIMD_OFF=1
            FLTX_F128_ENABLE_SIMD=0
            FLTX_F256_ENABLE_SIMD=0
            FLTX_F256_ENABLE_TRIG_SIMD=0
        )
    endif()

    fltx_apply_private_wasm_simd_options(${_TARGET})
    fltx_configure_compiled_x86_fma_backend(${_TARGET})
endfunction()

function(fltx_configure_library_target _TARGET)
    target_compile_features(${_TARGET} PUBLIC cxx_std_20)
    fltx_configure_public_header_contract(${_TARGET} PUBLIC)
    fltx_configure_internal_target(${_TARGET})
endfunction()

function(fltx_configure_consumer_fast_math _TARGET)
    cmake_parse_arguments(FLTX_CONSUMER "" "MODE" "" ${ARGN})
    if(NOT DEFINED FLTX_CONSUMER_MODE)
        set(FLTX_CONSUMER_MODE ON)
    endif()

    string(TOUPPER "${FLTX_CONSUMER_MODE}" _FLTX_CONSUMER_MODE_NORMALIZED)
    if(NOT _FLTX_CONSUMER_MODE_NORMALIZED MATCHES "^(ON|OFF)$")
        message(FATAL_ERROR
            "Invalid consumer fast-math mode '${FLTX_CONSUMER_MODE}'. "
            "Expected ON or OFF."
        )
    endif()

    if(MSVC)
        if(_FLTX_CONSUMER_MODE_NORMALIZED STREQUAL "ON")
            target_compile_options(${_TARGET} PRIVATE /fp:fast)
        else()
            target_compile_options(${_TARGET} PRIVATE /fp:precise)
        endif()
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang|AppleClang)$")
        if(_FLTX_CONSUMER_MODE_NORMALIZED STREQUAL "ON")
            target_compile_options(${_TARGET} PRIVATE -ffast-math)
            fltx_target_link_options_if_supported(${_TARGET} -ffast-math)
        else()
            target_compile_options(${_TARGET} PRIVATE -fno-fast-math)
            fltx_target_link_options_if_supported(${_TARGET} -fno-fast-math)
        endif()
    else()
        message(FATAL_ERROR
            "Consumer fast-math validation is unsupported for "
            "${CMAKE_CXX_COMPILER_ID}/${CMAKE_SYSTEM_NAME}."
        )
    endif()
endfunction()

function(fltx_assert_package_safe_interface _TARGET)
    foreach(_PROPERTY IN ITEMS INTERFACE_COMPILE_OPTIONS INTERFACE_LINK_OPTIONS)
        get_target_property(_VALUE ${_TARGET} ${_PROPERTY})
        if(NOT _VALUE)
            continue()
        endif()

        foreach(_FORBIDDEN IN ITEMS "/arch:AVX" "/arch:AVX2" "-mavx" "-mavx2" "/fp:fast" "-ffast-math")
            if("${_VALUE}" MATCHES "(^|;)${_FORBIDDEN}($|;)")
                message(FATAL_ERROR "${_TARGET} ${_PROPERTY} contains package-unsafe option '${_FORBIDDEN}': ${_VALUE}")
            endif()
        endforeach()

        string(TOUPPER "${FLTX_FMA_MODE}" _FLTX_FMA_MODE_NORMALIZED)
        if("${_VALUE}" MATCHES "(^|;)(-mfma|/clang:-mfma)($|;)" AND
           NOT _FLTX_FMA_MODE_NORMALIZED STREQUAL "ASSUME")
            message(FATAL_ERROR
                "${_TARGET} ${_PROPERTY} contains an FMA target option outside "
                "FLTX_FMA_MODE=ASSUME: ${_VALUE}"
            )
        endif()

        if("${_VALUE}" MATCHES "(^|;)-msimd128($|;)" AND NOT (EMSCRIPTEN AND FLTX_SIMD))
            message(FATAL_ERROR "${_TARGET} ${_PROPERTY} contains -msimd128 while Wasm SIMD is disabled: ${_VALUE}")
        endif()
    endforeach()
endfunction()
