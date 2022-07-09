###############################################################################
#
# cmake settings
#
###############################################################################

cmake_minimum_required (VERSION 3.8)
cmake_policy(SET CMP0091 NEW)

# In-source builds are disabled
if ("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    message(FATAL_ERROR
        "CMake generation is not possible within the source directory!"
        "\n Remove the CMakeCache.txt file and try again from another folder, e.g.:"
        "\n "
        "\n rm CMakeCache.txt"
        "\n mkdir build"
        "\n cd build"
        "\n cmake .."
    )
endif()

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${EXECUTABLE_OUTPUT_PATH}")

###############################################################################
#
# project settings
#
###############################################################################

set(VCPKG_TARGET_TRIPLET "x64-windows-static")

project(ocrServer C CXX)

set(ocrServer_src src/main.cpp src/ocr_options.cpp src/ocr_engine.cpp src/ocr_server.cpp)

add_executable(ocrServer ${ocrServer_src} ${ocrServer_src})
set_property(TARGET ocrServer PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

find_package(OpenSSL REQUIRED)
find_package(Tesseract CONFIG REQUIRED)
target_link_libraries(ocrServer PRIVATE libtesseract PRIVATE OpenSSL::SSL OpenSSL::Crypto)

add_definitions(-DCPPHTTPLIB_OPENSSL_SUPPORT)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ocrServer.config.json ${EXECUTABLE_OUTPUT_PATH}/ocrServer.config.json COPYONLY)

###############################################################################
#
# compiler and linker
#
###############################################################################

# Check for C++ standard to use
get_property(known_features GLOBAL PROPERTY CMAKE_CXX_KNOWN_FEATURES)
if (cxx_std_17 IN_LIST known_features)
    set(CMAKE_CXX_STANDARD 17)
elseif (cxx_std_14 IN_LIST known_features)
    set(CMAKE_CXX_STANDARD 14)
else()  # minimum required standard
    set(CMAKE_CXX_STANDARD 11)
endif()

# workaround for bad info in CMAKE_CXX_KNOWN_FEATURES for g++ 4.8.4
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    set(CMAKE_CXX_STANDARD 11)
    if (${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER_EQUAL "6.1.0")
        set(CMAKE_CXX_STANDARD 14)
    endif()
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(LIBRARY_TYPE STATIC)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CLANG 1)
endif()

if (WIN32)
    if (MSVC)
        add_definitions(-D_CRT_SECURE_NO_WARNINGS)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
        if (NOT CLANG)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
        endif()
    endif()
    set(LIB_Ws2_32 Ws2_32)
endif()

if (OPENMP_BUILD)
    if (MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /openmp")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
    endif()
endif()

if (CYGWIN)
    add_definitions(-D__CYGWIN__)
endif()

if (UNIX)
    if (NOT ANDROID)
        set(LIB_pthread pthread)
    endif()
endif()

###############################################################################
#
# configure
#
###############################################################################

# auto optimize
include(OptimizeForArchitecture)
AutodetectHostArchitecture()
OptimizeForArchitecture()
foreach(flag ${Vc_ARCHITECTURE_FLAGS})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
endforeach()

set(AVX_OPT OFF)
set(AVX2_OPT OFF)
set(SSE41_OPT OFF)
set(MARCH_NATIVE_OPT OFF)
foreach(flag ${_enable_vector_unit_list})  # from OptimizeForArchitecture()
    string(TOUPPER "${flag}" flag)
    string(REPLACE "\." "_" flag "${flag}")
    set(sim_flags "${sim_flags} -D${flag}")
    string(REPLACE "_" "" flag "${flag}")
    if("${flag}" MATCHES "AVX|AVX2|SSE41")
        set("${flag}_OPT" ON)
    endif()
endforeach(flag)

set(AUTOCONFIG_SRC ${CMAKE_CURRENT_BINARY_DIR}/config_auto.h.in)
set(AUTOCONFIG ${CMAKE_CURRENT_BINARY_DIR}/config_auto.h)
add_definitions(-DHAVE_CONFIG_H)

include(Configure)

configure_file(${AUTOCONFIG_SRC} ${AUTOCONFIG} @ONLY)

# show summary of configuration
message( STATUS )
message( STATUS "General configuration for OcrServer")
message( STATUS "--------------------------------------------------------")
message( STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message( STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message( STATUS "Used standard: C++${CMAKE_CXX_STANDARD}")
message( STATUS "CXX compiler options: ${CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE_UP}} ${CMAKE_CXX_FLAGS}")
message( STATUS "Linker options: ${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_EXE_LINKER_FLAGS_${CMAKE_BUILD_TYPE_UP}}")
message( STATUS "Install directory: ${CMAKE_INSTALL_PREFIX}")
message( STATUS "Architecture flags: ${Vc_ARCHITECTURE_FLAGS}")
message( STATUS "Vector unit list: ${_enable_vector_unit_list}")
message( STATUS "AVX_OPT: ${AVX_OPT}")
message( STATUS "AVX2_OPT: ${AVX2_OPT}")
message( STATUS "SSE41_OPT: ${SSE41_OPT}")
message( STATUS "MARCH_NATIVE_OPT: ${MARCH_NATIVE_OPT}")
message( STATUS "sim_flags: ${sim_flags}")
message( STATUS "--------------------------------------------------------")
message( STATUS )

###############################################################################
#
# build
#
###############################################################################

include(BuildFunctions)
include(SourceGroups)

add_definitions(-D_SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS=1)

include_directories(${Leptonica_INCLUDE_DIRS})
include_directories(${Tesseract_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

if(ANDROID_TOOLCHAIN)
  include_directories(${ANDROID_TOOLCHAIN}/sysroot/usr/include)
  add_compile_definitions(__ANDROID_API_FUTURE__)
endif()

########################################
# uninstall target
########################################

if(NOT TARGET uninstall)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY)

    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()

###############################################################################
