function(fltx_add_size_probe target precision case_name repeats)
    add_executable(${target} EXCLUDE_FROM_ALL probe.cpp)
    target_link_libraries(${target} PRIVATE fltx::fltx)
    target_compile_features(${target} PRIVATE cxx_std_20)
    target_compile_definitions(${target} PRIVATE
        FLTX_ENABLE_FQD_EXPRESSIONS=$<BOOL:${FLTX_VALIDATION_FQD_EXPRESSIONS}>
        FLTX_BENCHMARK_SIZE_PRECISION=${precision}
        FLTX_BENCHMARK_SIZE_CASE=${case_name}
        FLTX_BENCHMARK_SIZE_REPEATS=${repeats}
    )

    if(MSVC)
        target_compile_options(${target} PRIVATE $<$<CONFIG:Release>:/Gy> $<$<CONFIG:Release>:/Gw>)
        target_link_options(${target} PRIVATE $<$<CONFIG:Release>:/OPT:REF> $<$<CONFIG:Release>:/OPT:ICF>)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target} PRIVATE
            $<$<CONFIG:Release>:-ffunction-sections>
            $<$<CONFIG:Release>:-fdata-sections>
        )
        if(NOT EMSCRIPTEN)
            target_link_options(${target} PRIVATE $<$<CONFIG:Release>:-Wl,--gc-sections>)
        endif()
    endif()
endfunction()

fltx_add_size_probe(fltx_size_dd_baseline 128 0 0)
fltx_add_size_probe(fltx_size_dd_arithmetic 128 1 32)
fltx_add_size_probe(fltx_size_dd_math 128 2 16)
fltx_add_size_probe(fltx_size_dd_io 128 3 8)
fltx_add_size_probe(fltx_size_dd_random 128 4 16)
fltx_add_size_probe(fltx_size_qd_baseline 256 0 0)
fltx_add_size_probe(fltx_size_qd_arithmetic 256 1 32)
fltx_add_size_probe(fltx_size_qd_math 256 2 16)
fltx_add_size_probe(fltx_size_qd_io 256 3 8)
fltx_add_size_probe(fltx_size_qd_random 256 4 16)

set(FLTX_BENCHMARK_SIZE_TARGETS
    fltx_size_dd_baseline
    fltx_size_dd_arithmetic
    fltx_size_dd_math
    fltx_size_dd_io
    fltx_size_dd_random
    fltx_size_qd_baseline
    fltx_size_qd_arithmetic
    fltx_size_qd_math
    fltx_size_qd_io
    fltx_size_qd_random
)

set(FLTX_BENCHMARK_SIZE_REPORT_ARGS)
foreach(target IN LISTS FLTX_BENCHMARK_SIZE_TARGETS)
    list(APPEND FLTX_BENCHMARK_SIZE_REPORT_ARGS --file "${target}=$<TARGET_FILE:${target}>")
endforeach()

add_custom_target(fltx_binary_size
    COMMAND "${Python3_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/report.py"
        --output "${CMAKE_CURRENT_BINARY_DIR}/binary_size.csv"
        ${FLTX_BENCHMARK_SIZE_REPORT_ARGS}
    DEPENDS ${FLTX_BENCHMARK_SIZE_TARGETS}
    USES_TERMINAL
    VERBATIM
    COMMENT "Reporting fltx binary-size probes"
)
