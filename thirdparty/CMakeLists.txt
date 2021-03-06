#----------------------------------------------------------------------------------------------
#  Copyright (c) 2020 - present Alexander Voitenko
#  Licensed under the MIT License. See License.txt in the project root for license information.
#----------------------------------------------------------------------------------------------

include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)

if (WIN32)
  list(APPEND LIBUV_LIB_DEPS
       advapi32
       iphlpapi
       psapi
       shell32
       user32
       userenv
       ws2_32)
else()
    if (NOT CMAKE_SYSTEM_NAME STREQUAL "Android")
        # Android has pthread as part of its c library, not as a separate
        # libpthread.so.
        list(APPEND LIBUV_LIB_DEPS pthread)
    endif()
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "AIX")
    list(APPEND LIBUV_LIB_DEPS perfstat)
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Android")
    list(APPEND uv_libs dl)
endif()

if (CMAKE_SYSTEM_NAME MATCHES "DragonFly|FreeBSD|NetBSD|OpenBSD")
    list(APPEND LIBUV_LIB_DEPS kvm)
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    list(APPEND LIBUV_LIB_DEPS dl rt)
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "SunOS")
    list(APPEND LIBUV_LIB_DEPS kstat nsl sendfile socket)
endif()

set(LIBUV_ADDITIONAL_FLAGS "")
if (DEFINED IO_SANITIZER)
    set(LIBUV_ADDITIONAL_FLAGS "${LIBUV_ADDITIONAL_FLAGS} -fsanitize=${IO_SANITIZER}")
endif()

# Promoting LIBUV_LIB_DEPS to parent scope
set(LIBUV_LIB_DEPS "${LIBUV_LIB_DEPS}" PARENT_SCOPE)

if (TARM_IO_USE_EXTERNAL_LIBUV)
    return()
endif()

if (NOT DEFINED CMAKE_INSTALL_LIBDIR)
    set(CMAKE_INSTALL_LIBDIR "lib")
endif()

set(LIBUV_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/libuv")
set(LIBUV_STATIC_LIB_PATH "${LIBUV_PREFIX}/install/${CMAKE_INSTALL_LIBDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}uv_a${CMAKE_STATIC_LIBRARY_SUFFIX}")
set(LIBUV_DYNAMIC_LIB_PATH "${LIBUV_PREFIX}/install/${CMAKE_INSTALL_LIBDIR}/${CMAKE_SHARED_LIBRARY_PREFIX}uv${CMAKE_SHARED_LIBRARY_SUFFIX}")
set(LIBUV_INCLUDE_PATH "${LIBUV_PREFIX}/install/include")

# Workaround for error about non-existent directory in INTERFACE_INCLUDE_DIRECTORIES during CMake configuration step
# https://gitlab.kitware.com/cmake/cmake/-/issues/15052
file(MAKE_DIRECTORY "${LIBUV_INCLUDE_PATH}")

set(LIBUV_CMAKE_ARGS
    -Dlibuv_buildtests=OFF
    -DCMAKE_INSTALL_PREFIX=${LIBUV_PREFIX}/install
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON
    -DCMAKE_C_FLAGS=${LIBUV_ADDITIONAL_FLAGS}
    -DCMAKE_SHARED_LINKER_FLAGS=${LIBUV_ADDITIONAL_FLAGS}
)

if (MSVC)
    list(APPEND LIBUV_CMAKE_ARGS -DCMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE=${CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE})
endif()

set(LIBUV_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libuv-${TARM_IO_LIBUV_VERSION}")

# Starting from CMake-3.20 it is impossible to pass --config option into CMAKE_ARGS in the ExternalProject_Add,
# so 2 separate versions were added.
if (CMAKE_CONFIGURATION_TYPES)
    # This branch is called for build systems which support multiple configurations in the same directory,
    # like XCode, MSVC and "Ninja Multi-Config".
    ExternalProject_Add(libuv-bundled
        SOURCE_DIR "${LIBUV_SOURCE_DIR}"
        PREFIX "${LIBUV_PREFIX}"
        BINARY_DIR "${LIBUV_PREFIX}/build"
        INSTALL_DIR "${LIBUV_PREFIX}/install"
        CMAKE_ARGS "${LIBUV_CMAKE_ARGS}"
        BUILD_COMMAND cmake --build . --config $<CONFIG>
        BUILD_BYPRODUCTS ${LIBUV_STATIC_LIB_PATH}
        BUILD_BYPRODUCTS ${LIBUV_DYNAMIC_LIB_PATH}
        BUILD_BYPRODUCTS ${LIBUV_INCLUDE_PATH}
    )
else()
    list(APPEND LIBUV_CMAKE_ARGS -DCMAKE_BUILD_TYPE=$<CONFIG>)

    ExternalProject_Add(libuv-bundled
        SOURCE_DIR "${LIBUV_SOURCE_DIR}"
        PREFIX "${LIBUV_PREFIX}"
        BINARY_DIR "${LIBUV_PREFIX}/build"
        INSTALL_DIR "${LIBUV_PREFIX}/install"
        CMAKE_ARGS "${LIBUV_CMAKE_ARGS}"
        BUILD_BYPRODUCTS ${LIBUV_STATIC_LIB_PATH}
        BUILD_BYPRODUCTS ${LIBUV_DYNAMIC_LIB_PATH}
        BUILD_BYPRODUCTS ${LIBUV_INCLUDE_PATH}
    )
endif()

# IMPORTED target is required for Ninja generator because it needs all rules to be defined at configuration step of CMake
if (NOT DEFINED IO_SANITIZER)
    add_library(LibUV::LibUV STATIC IMPORTED GLOBAL)
    set_target_properties(LibUV::LibUV PROPERTIES
        IMPORTED_LOCATION "${LIBUV_STATIC_LIB_PATH}"
        INTERFACE_INCLUDE_DIRECTORIES "${LIBUV_INCLUDE_PATH}")
else()
    add_library(LibUV::LibUV SHARED IMPORTED GLOBAL)
    set_target_properties(LibUV::LibUV PROPERTIES
        IMPORTED_LOCATION "${LIBUV_DYNAMIC_LIB_PATH}"
        INTERFACE_INCLUDE_DIRECTORIES "${LIBUV_INCLUDE_PATH}")
endif()

add_dependencies(LibUV::LibUV libuv-bundled)
