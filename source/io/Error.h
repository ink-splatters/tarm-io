/*----------------------------------------------------------------------------------------------
 *  Copyright (c) 2020 - present Alexander Voitenko
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *----------------------------------------------------------------------------------------------*/

#pragma once

#include "CommonMacros.h"
#include "Export.h"
#include "StatusCode.h"

#include <ostream>
#include <string>

namespace tarm {
namespace io {

class Error {
public:
    TARM_IO_ALLOW_COPY(Error);
    TARM_IO_ALLOW_MOVE(Error);

    TARM_IO_DLL_PUBLIC Error() = default; // StatusCode::OK
    TARM_IO_DLL_PUBLIC Error(std::int64_t libuv_code);
    TARM_IO_DLL_PUBLIC Error(StatusCode status_code);
    TARM_IO_DLL_PUBLIC Error(StatusCode status_code, const std::string& additional_info);

    TARM_IO_DLL_PUBLIC StatusCode code() const;
    TARM_IO_DLL_PUBLIC std::string string() const;
    TARM_IO_DLL_PUBLIC const std::string& additional_info() const;

    TARM_IO_DLL_PUBLIC operator bool() const;

private:
    std::int64_t m_libuv_code = 0;
    StatusCode m_status_code = StatusCode::OK;
    std::string m_additional_info;
};

TARM_IO_DLL_PUBLIC
bool operator==(const Error& s1, const Error& s2);

TARM_IO_DLL_PUBLIC
bool operator!=(const Error& s1, const Error& s2);

TARM_IO_DLL_PUBLIC
std::ostream& operator<<(std::ostream& os, const Error& err);

} // namespace io
} // namespace tarm
