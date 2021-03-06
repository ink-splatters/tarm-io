/*----------------------------------------------------------------------------------------------
*  Copyright (c) 2020 - present Alexander Voitenko
*  Licensed under the MIT License. See License.txt in the project root for license information.
*----------------------------------------------------------------------------------------------*/

#include "Functions.h"

#include "detail/EventLoopHelpers.h"

#include <uv.h>

namespace tarm {
namespace io {
namespace fs {

namespace stat_detail {

struct StatRequest : public uv_fs_t {
    StatCallback callback;
};

void on_stat(uv_fs_t* req) {
    auto& request = *reinterpret_cast<StatRequest*>(req);
    Error error(req->result);
    if (request.callback) {
        request.callback(req->path, *reinterpret_cast<StatData*>(&request.statbuf), error);
    }
    uv_fs_req_cleanup(req);
    delete &request;
}

void stat_impl(EventLoop& loop, const Path& path, const StatCallback& callback) {
    auto request = new StatRequest;
    request->callback = callback;
    const Error stat_error = uv_fs_stat(reinterpret_cast<uv_loop_t*>(loop.raw_loop()),
                                        request,
                                        path.string().c_str(),
                                        on_stat);
    if (stat_error) {
        StatData data;
        if (callback) {
            callback(path, data, stat_error);
        }
        delete request;
        return;
    }
}

} // namespace stat_detail

void stat(EventLoop& loop, const Path& path, const StatCallback& callback) {
    ::tarm::io::detail::defer_execution_if_required(loop, stat_detail::stat_impl, loop, path, callback);
}

bool is_regular_file(const StatData& stat_data) {
    return (stat_data.mode & S_IFMT) == S_IFREG;
}

bool is_directory(const StatData& stat_data) {
    return (stat_data.mode & S_IFMT) == S_IFDIR;
}

bool is_symbolic_link(const StatData& stat_data) {
    return (stat_data.mode & S_IFMT) == S_IFLNK;
}

bool is_block_special(const StatData& stat_data) {
#ifdef TARM_IO_PLATFORM_WINDOWS
    return false;
#else
    return (stat_data.mode & S_IFMT) == S_IFBLK;
#endif
}

bool is_char_special(const StatData& stat_data) {
    return (stat_data.mode & S_IFMT) == S_IFCHR;
}

bool is_fifo(const StatData& stat_data) {
#ifdef TARM_IO_PLATFORM_WINDOWS
    return false;
#else
    return (stat_data.mode & S_IFMT) == S_IFIFO;
#endif
}

bool is_socket(const StatData& stat_data) {
#ifdef TARM_IO_PLATFORM_WINDOWS
    return false;
#else
    return (stat_data.mode & S_IFMT) == S_IFSOCK;
#endif
}

} // namespace fs
} // namespace io
} // namespace tarm
