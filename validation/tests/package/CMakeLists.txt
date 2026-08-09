cmake_minimum_required(VERSION 3.21)
project(fltx_package_consumers LANGUAGES CXX)

find_package(fltx CONFIG REQUIRED)

function(add_fltx_consumer target)
    add_executable(${target} consumer.cpp)
    target_link_libraries(${target} PRIVATE fltx::fltx)
    target_compile_features(${target} PRIVATE cxx_std_20)
    target_compile_definitions(${target} PRIVATE
        "FLTX_TESTS_COMPILER_ID=\"${CMAKE_CXX_COMPILER_ID}\""
        "FLTX_TESTS_COMPILER_VERSION=\"${CMAKE_CXX_COMPILER_VERSION}\""
        "FLTX_TESTS_BUILD_CONFIG=\"$<CONFIG>\""
        "FLTX_TESTS_SYSTEM_NAME=\"${CMAKE_SYSTEM_NAME}\""
        "FLTX_TESTS_SYSTEM_PROCESSOR=\"${CMAKE_SYSTEM_PROCESSOR}\""
        "FLTX_TESTS_BUILD_OPTIMIZED=$<IF:$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>,$<CONFIG:MinSizeRel>>,1,0>"
    )
endfunction()

add_fltx_consumer(fltx_package_strict)
add_fltx_consumer(fltx_package_fastmath)
add_fltx_consumer(fltx_package_no_fma)

target_compile_definitions(fltx_package_strict PRIVATE FLTX_TESTS_EXPECT_STRICT=1)
target_compile_definitions(fltx_package_fastmath PRIVATE FLTX_TESTS_EXPECT_FAST_MATH=1)

if(MSVC)
    target_compile_options(fltx_package_fastmath PRIVATE /fp:fast)
else()
    target_compile_options(fltx_package_fastmath PRIVATE -ffast-math)
endif()

# Exercise AUTO as a translation unit that cannot use local x86 FMA and whose
# test-only CPU probe reports that runtime FMA is unavailable.
target_compile_definitions(fltx_package_no_fma PRIVATE
    FLTX_TESTS_EXPECT_NO_LOCAL_FMA=1
    FLTX_TEST_RUNTIME_X86_FMA_AVAILABLE=0
    FLTX_TU_HAS_X86_FMA=0
    FLTX_HAS_X86_FMA=0
)

include(CTest)
if(BUILD_TESTING)
    add_test(NAME fltx.package.strict COMMAND fltx_package_strict)
    add_test(NAME fltx.package.fastmath COMMAND fltx_package_fastmath)
    add_test(NAME fltx.package.no_fma COMMAND fltx_package_no_fma)
endif()
