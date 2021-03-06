/*----------------------------------------------------------------------------------------------
 *  Copyright (c) 2020 - present Alexander Voitenko
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *----------------------------------------------------------------------------------------------*/

#include "UTCommon.h"

#include <boost/filesystem.hpp>

#include <iostream>
#include <string>

std::string create_temp_test_directory() {
    auto path = boost::filesystem::temp_directory_path();
    path /= "uv_cpp";
    path /= "%%%%-%%%%-%%%%-%%%%-%%%%";
    path = boost::filesystem::unique_path(path);

    boost::filesystem::create_directories(path);

    return path.string();
}

#ifdef TARM_IO_PLATFORM_WINDOWS
    #include <windows.h>    //GetModuleFileNameW
#else // assuming Unix
    #include <unistd.h>     //readlink

    #ifdef TARM_IO_PLATFORM_MACOSX
        #include <sys/syslimits.h>
        #include <mach-o/dyld.h>
    #else
        #include <linux/limits.h>
    #endif
#endif

boost::filesystem::path exe_path() {
    boost::filesystem::path result;

#if defined(TARM_IO_PLATFORM_WINDOWS)
    wchar_t path[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, path, MAX_PATH);
    result = boost::filesystem::path(path).remove_filename();
#elif defined(TARM_IO_PLATFORM_MACOSX)
    char buf[PATH_MAX] = { 0 };
    uint32_t buf_size = PATH_MAX;
    if(!_NSGetExecutablePath(buf, &buf_size)) {
        result = boost::filesystem::path(std::string(buf, (buf_size > 0 ? buf_size : 0))).remove_filename();
    }
#elif defined(TARM_IO_PLATFORM_LINUX)
    char buf[PATH_MAX];
    ssize_t buf_size = readlink("/proc/self/exe", buf, PATH_MAX);
    result = boost::filesystem::path(std::string(buf, (buf_size > 0 ? buf_size : 0))).remove_filename();
#else
    #error Unknown platform.
#endif

    if (result.empty()) {
        std::cerr << "Fatal error: failed to get executable path." << std::endl;
        abort();
    }

    return result;
}

std::string create_empty_file(const std::string& path_where_create) {
    std::string file_path = path_where_create + "/empty";
    std::ofstream ofile(file_path);
    if (ofile.fail()) {
        return "";
    }
    ofile.close();

    return file_path;
}

std::string current_test_suite_name() {
#if TARM_IO_GTEST_VERSION_MAJOR >= 1 && TARM_IO_GTEST_VERSION_MINOR >= 10 && TARM_IO_GTEST_VERSION_PATCH >= 0
    const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
    return test_info ? test_info->test_suite_name() : "";
#else
    return ::testing::UnitTest::GetInstance()->current_test_case()->name();
#endif
}

std::string current_test_case_name() {
#if TARM_IO_GTEST_VERSION_MAJOR >= 1 && TARM_IO_GTEST_VERSION_MINOR >= 10 && TARM_IO_GTEST_VERSION_PATCH >= 0
    const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
    return test_info ? test_info->name() : "";
#else
    return ::testing::UnitTest::GetInstance()->current_test_info()->name();
#endif
}

namespace std {

void PrintTo(const std::chrono::minutes& duration, std::ostream* os) {
  *os << duration.count() << "m";
}

void PrintTo(const std::chrono::seconds& duration, std::ostream* os)  {
  *os << duration.count() << "s";
}

void PrintTo(const std::chrono::milliseconds& duration, std::ostream* os) {
  *os << duration.count() << "ms";
}

void PrintTo(const std::chrono::microseconds& duration, std::ostream* os) {
  *os << duration.count() << "us";
}

void PrintTo(const std::chrono::nanoseconds& duration, std::ostream* os) {
  *os << duration.count() << "ns";
}

} // namespace std
