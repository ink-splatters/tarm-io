find_package(GTest REQUIRED)

list(APPEND TESTS_LIST
    DisposableTest.cpp
    EventLoopTest.cpp
    FileTest.cpp
    TimerTest.cpp
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${GTEST_INCLUDE_DIRS}
)

set(RUN_TEST_ENV) # TODO: currently this is unused but may be utilized with sanitizers
set(TESTS_EXE_NAME tests)

add_executable(${TESTS_EXE_NAME} ${TESTS_LIST})
target_link_libraries(${TESTS_EXE_NAME}
    io
    GTest::GTest
    GTest::Main
    Boost::system
    Boost::filesystem
)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/tmp)

add_custom_target(RunTests
    DEPENDS ${TESTS_LIST}
    COMMAND ${CMAKE_COMMAND} -E env ${RUN_TEST_ENV} ./${TESTS_EXE_NAME}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
add_dependencies(RunTests ${TESTS_EXE_NAME})