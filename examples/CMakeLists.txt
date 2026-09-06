cmake_minimum_required(VERSION 3.21)
project(fltx LANGUAGES CXX)

if(NOT COMMAND fltx_enable_msvc_parallel_compile)
    include("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/fltxMsvcParallel.cmake")
endif()

function(fltx_add_example example_name)
    add_executable(${example_name} ${ARGN})
    target_link_libraries(${example_name} PRIVATE fltx::fltx)
    target_compile_definitions(${example_name} PRIVATE FLTX_ENABLE_FQD_EXPRESSIONS=1)

    target_compile_options(${example_name} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/constexpr:steps10000000>
        $<$<CXX_COMPILER_ID:MSVC>:/constexpr:backtrace100>
        $<$<CXX_COMPILER_ID:Clang,AppleClang>:-fconstexpr-steps=10000000>
        $<$<CXX_COMPILER_ID:Clang,AppleClang>:-fconstexpr-backtrace-limit=100>
    )

    target_compile_features(${example_name} PRIVATE cxx_std_20)
    fltx_enable_msvc_parallel_compile(${example_name})
    set_target_properties(${example_name} PROPERTIES FOLDER "fltx/examples")

    if(MSVC)
        target_compile_options(${example_name} PRIVATE /Bt+)
        target_link_options(${example_name} PRIVATE /TIME)
    endif()
endfunction()

fltx_add_example("01_example_basic"                  example_basic.cpp)
fltx_add_example("02_example_constexpr_io"           example_constexpr_io.cpp)
fltx_add_example("03_example_dispatch"               example_dispatch.cpp)
fltx_add_example("04_example_mandelbrot"             example_mandelbrot.cpp helpers/bmp_writer.h)
fltx_add_example("05_example_consteval_ellipse"      example_consteval_ellipse.cpp)
fltx_add_example("06_example_consteval_library_sweep" example_consteval_library_sweep.cpp)
fltx_add_example("07_example_random"                 example_random.cpp)
fltx_add_example("08_example_pow"                    example_pow.cpp)
fltx_add_example("09_example_newton_solver"          example_newton_solver.cpp)
fltx_add_example("10_example_charconv"              example_charconv.cpp)
fltx_add_example("11_example_std_integration"       example_std_integration.cpp)
fltx_add_example("12_example_storage_interop"       example_storage_interop.cpp)
fltx_add_example("13_example_special_values"        example_special_values.cpp)
fltx_add_example("14_example_polynomial"            example_polynomial.cpp)
fltx_add_example("15_example_consteval_coefficients" example_consteval_coefficients.cpp)
