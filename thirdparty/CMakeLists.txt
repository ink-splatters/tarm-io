include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)

if(WIN32)
  list(APPEND LIBUV_LIB_DEPS
       advapi32
       iphlpapi
       psapi
       shell32
       user32
       userenv
       ws2_32)
else()
    if(NOT CMAKE_SYSTEM_NAME STREQUAL "Android")
        # Android has pthread as part of its c library, not as a separate
        # libpthread.so.
        list(APPEND LIBUV_LIB_DEPS pthread)
    endif()
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "AIX")
    list(APPEND LIBUV_LIB_DEPS perfstat)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Android")
    list(APPEND uv_libs dl)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "DragonFly|FreeBSD|NetBSD|OpenBSD")
    list(APPEND LIBUV_LIB_DEPS kvm)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    list(APPEND LIBUV_LIB_DEPS dl rt)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
    list(APPEND LIBUV_LIB_DEPS kstat nsl sendfile socket)
endif()

set(LIBUV_ADDITIONAL_FLAGS "")
if (DEFINED IO_SANITIZER)
    set(LIBUV_ADDITIONAL_FLAGS "${LIBUV_ADDITIONAL_FLAGS} -fsanitize=${IO_SANITIZER}")
endif()

# Promoting LIBUV_LIB_DEPS to parent scope
set(LIBUV_LIB_DEPS "${LIBUV_LIB_DEPS}" PARENT_SCOPE)

set(LIBUV_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/libuv")
set(LIBUV_STATIC_LIB_PATH "${LIBUV_PREFIX}/install/lib/${CMAKE_STATIC_LIBRARY_PREFIX}uv_a${CMAKE_STATIC_LIBRARY_SUFFIX}" PARENT_SCOPE)
set(LIBUV_DYNAMIC_LIB_PATH "${LIBUV_PREFIX}/install/lib/${CMAKE_SHARED_LIBRARY_PREFIX}uv${CMAKE_SHARED_LIBRARY_SUFFIX}" PARENT_SCOPE)
set(LIBUV_INCLUDE_PATH "${LIBUV_PREFIX}/install/include" PARENT_SCOPE)
ExternalProject_Add(libuv
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libuv-${IO_LIBUV_VERSION}"
    PREFIX "${LIBUV_PREFIX}"
    BINARY_DIR "${LIBUV_PREFIX}/build"
    INSTALL_DIR "${LIBUV_PREFIX}/install"
    CMAKE_ARGS -Dlibuv_buildtests=OFF
               -DCMAKE_INSTALL_PREFIX=${LIBUV_PREFIX}/install
               -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
               -DCMAKE_C_FLAGS=-fPIC\ ${LIBUV_ADDITIONAL_FLAGS}
               -DCMAKE_SHARED_LINKER_FLAGS=${LIBUV_ADDITIONAL_FLAGS}
)