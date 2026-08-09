# In-tree and installed-package consumer validation.

# This quick in-tree executable is useful while editing the consumer. The release
# target below runs the authoritative installed find_package fixture.
add_executable(fltx_package_smoke
    "${FLTX_VALIDATION_SOURCE_DIR}/tests/package/consumer.cpp"
)
target_link_libraries(fltx_package_smoke PRIVATE fltx::fltx)
target_compile_features(fltx_package_smoke PRIVATE cxx_std_20)
fltx_tests_add_build_identity(fltx_package_smoke)

if(BUILD_TESTING)
    add_test(NAME fltx.package.in_tree COMMAND fltx_package_smoke)
    set_tests_properties(fltx.package.in_tree PROPERTIES LABELS "fltx;package")
endif()

set(FLTX_TESTS_PACKAGE_INSTALL_DIR
    "${FLTX_VALIDATION_BINARY_DIR}/package_install"
)
string(MAKE_C_IDENTIFIER
    "${CMAKE_SYSTEM_NAME}_${CMAKE_CXX_COMPILER_ID}"
    FLTX_TESTS_PACKAGE_BUILD_ID
)
set(FLTX_TESTS_PACKAGE_BUILD_DIR
    "${FLTX_VALIDATION_BINARY_DIR}/package_build_${FLTX_TESTS_PACKAGE_BUILD_ID}"
)
set(FLTX_TESTS_PACKAGE_GENERATOR_ARGS -G "${CMAKE_GENERATOR}")
if(CMAKE_GENERATOR_PLATFORM)
    list(APPEND FLTX_TESTS_PACKAGE_GENERATOR_ARGS -A "${CMAKE_GENERATOR_PLATFORM}")
endif()
if(CMAKE_GENERATOR_TOOLSET)
    list(APPEND FLTX_TESTS_PACKAGE_GENERATOR_ARGS -T "${CMAKE_GENERATOR_TOOLSET}")
endif()

set(FLTX_TESTS_PACKAGE_TOOLCHAIN_ARGS)
if(EMSCRIPTEN)
    if(NOT VCPKG_CHAINLOAD_TOOLCHAIN_FILE)
        message(FATAL_ERROR
            "fltx package tests require the Emscripten chainload toolchain path."
        )
    endif()
    list(APPEND FLTX_TESTS_PACKAGE_TOOLCHAIN_ARGS
        "-DCMAKE_TOOLCHAIN_FILE=${VCPKG_CHAINLOAD_TOOLCHAIN_FILE}"
        "-DCMAKE_CROSSCOMPILING_EMULATOR=${CMAKE_CROSSCOMPILING_EMULATOR}"
    )
else()
    list(APPEND FLTX_TESTS_PACKAGE_TOOLCHAIN_ARGS -U CMAKE_TOOLCHAIN_FILE)
endif()
if(NOT EMSCRIPTEN AND NOT CMAKE_GENERATOR MATCHES "Visual Studio|Xcode")
    list(APPEND FLTX_TESTS_PACKAGE_TOOLCHAIN_ARGS
        "-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}"
    )
    if(CMAKE_MAKE_PROGRAM)
        list(APPEND FLTX_TESTS_PACKAGE_TOOLCHAIN_ARGS
            "-DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}"
        )
    endif()
endif()
set(FLTX_TESTS_PACKAGE_BUILD_TYPE_ARG)
if(NOT CMAKE_CONFIGURATION_TYPES)
    set(FLTX_TESTS_PACKAGE_BUILD_TYPE_ARG "-DCMAKE_BUILD_TYPE=$<CONFIG>")
endif()

add_custom_target(fltx_package_tests
    # These directories belong only to this fixture. Recreate both on every
    # invocation so removed headers, targets, and compile definitions cannot
    # survive in the install tree or nested CMake cache.
    COMMAND "${CMAKE_COMMAND}" -E rm -rf
        "${FLTX_TESTS_PACKAGE_INSTALL_DIR}"
        "${FLTX_TESTS_PACKAGE_BUILD_DIR}"
    COMMAND "${CMAKE_COMMAND}" --install "${CMAKE_BINARY_DIR}"
        --config "$<CONFIG>"
        --prefix "${FLTX_TESTS_PACKAGE_INSTALL_DIR}"
    COMMAND "${CMAKE_COMMAND}"
        -S "${FLTX_VALIDATION_SOURCE_DIR}/tests/package"
        -B "${FLTX_TESTS_PACKAGE_BUILD_DIR}"
        ${FLTX_TESTS_PACKAGE_GENERATOR_ARGS}
        ${FLTX_TESTS_PACKAGE_TOOLCHAIN_ARGS}
        "-DCMAKE_PREFIX_PATH=${FLTX_TESTS_PACKAGE_INSTALL_DIR}"
        "-Dfltx_DIR=${FLTX_TESTS_PACKAGE_INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}/cmake/fltx"
        ${FLTX_TESTS_PACKAGE_BUILD_TYPE_ARG}
        -DBUILD_TESTING=ON
    COMMAND "${CMAKE_COMMAND}" --build "${FLTX_TESTS_PACKAGE_BUILD_DIR}"
        --config "$<CONFIG>"
    COMMAND "${CMAKE_CTEST_COMMAND}"
        --test-dir "${FLTX_TESTS_PACKAGE_BUILD_DIR}"
        --build-config "$<CONFIG>"
        --output-on-failure
    DEPENDS fltx fltx_package_smoke
    USES_TERMINAL
    COMMENT "Testing installed fltx package consumers"
    VERBATIM
)
