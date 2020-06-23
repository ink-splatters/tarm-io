/*----------------------------------------------------------------------------------------------
 *  Copyright (c) 2020 - present Alexander Voitenko
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *----------------------------------------------------------------------------------------------*/

#include "TcpConnectedClient.h"

#include "ByteSwap.h"
#include "detail/Common.h"
#include "Convert.h"
#include "TcpServer.h"
#include "detail/TcpClientImplBase.h"

#include <assert.h>

namespace tarm {
namespace io {
class TcpConnectedClient::Impl : public detail::TcpClientImplBase<TcpConnectedClient, TcpConnectedClient::Impl> {
public:
    Impl(EventLoop& loop, TcpServer& server, TcpConnectedClient& parent, CloseCallback cloase_callback);
    ~Impl();

    void set_endpoint(const Endpoint& endpoint);

    void close();

    void shutdown();

    void start_read(DataReceiveCallback data_receive_callback);
    uv_tcp_t* tcp_client_stream();

    TcpServer& server();
    const TcpServer& server() const;

protected:
    // statics
    static void on_shutdown(uv_shutdown_t* req, int status);
    static void on_close(uv_handle_t* handle);
    static void on_read(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf);

private:
    TcpServer* m_server = nullptr;

    DataReceiveCallback m_receive_callback = nullptr;
};

TcpConnectedClient::Impl::Impl(EventLoop& loop, TcpServer& server, TcpConnectedClient& parent, CloseCallback cloase_callback) :
    TcpClientImplBase(loop, parent),
    m_server(&server) {
    m_close_callback = cloase_callback;
}

TcpConnectedClient::Impl::~Impl() {
    IO_LOG(m_loop, TRACE, m_parent, "");

    assert(m_tcp_stream != nullptr);
    delete m_tcp_stream;
}

void TcpConnectedClient::Impl::set_endpoint(const Endpoint& endpoint) {
    m_destination_endpoint = endpoint;
    m_is_open = true;
}

void TcpConnectedClient::Impl::shutdown() {
    if (!is_open()) {
        return;
    }

    IO_LOG(m_loop, TRACE, m_parent, "endpoint:", this->endpoint());

    m_is_open = false;

    auto shutdown_req = new uv_shutdown_t;
    shutdown_req->data = this;
    uv_shutdown(shutdown_req, reinterpret_cast<uv_stream_t*>(m_tcp_stream), on_shutdown);
}

uv_tcp_t* TcpConnectedClient::Impl::tcp_client_stream() {
    return m_tcp_stream;
}

void TcpConnectedClient::Impl::close() {
    if (!is_open()) {
        return;
    }

    IO_LOG(m_loop, TRACE, m_parent, "endpoint:", this->endpoint());

    m_is_open = false;

    uv_close(reinterpret_cast<uv_handle_t*>(m_tcp_stream), on_close);
}

void TcpConnectedClient::Impl::start_read(DataReceiveCallback data_receive_callback) {
    m_receive_callback = data_receive_callback;

    const Error read_error = uv_read_start(reinterpret_cast<uv_stream_t*>(m_tcp_stream),
                                           alloc_read_buffer,
                                           on_read);
    if (read_error) {
        // TODO: call close callback instead of read? Because when we have error during read, we call close callback.
        //       and close connection.
        m_loop->schedule_callback([this, read_error](io::EventLoop&) {
            if (m_receive_callback) {
                m_receive_callback(*m_parent, {nullptr, 0}, read_error);
            }
        });
    }
}

TcpServer& TcpConnectedClient::Impl::server() {
    return *m_server;
}

const TcpServer& TcpConnectedClient::Impl::server() const {
    return *m_server;
}

////////////////////////////////////////////// static //////////////////////////////////////////////

void TcpConnectedClient::Impl::on_shutdown(uv_shutdown_t* req, int status) {
    auto& this_ = *reinterpret_cast<TcpConnectedClient::Impl*>(req->data);

    IO_LOG(this_.m_loop, TRACE, this_.m_parent, "endpoint:", this_.endpoint());

    uv_close(reinterpret_cast<uv_handle_t*>(req->handle), on_close);
    delete req;
}

void TcpConnectedClient::Impl::on_close(uv_handle_t* handle) {
    auto loop_ptr = reinterpret_cast<EventLoop*>(handle->loop->data);
    auto& this_ = *reinterpret_cast<TcpConnectedClient::Impl*>(handle->data);

    IO_LOG(loop_ptr, TRACE, this_.m_parent);

    this_.m_server->remove_client_connection(this_.m_parent);

    if (this_.m_close_callback) {
        this_.m_close_callback(*this_.m_parent, Error(0));
    }

    this_.m_destination_endpoint.clear();

    this_.m_parent->schedule_removal();
}

void TcpConnectedClient::Impl::on_read(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf) {
    auto& this_ = *reinterpret_cast<TcpConnectedClient::Impl*>(handle->data);

    if (nread >= 0) {
        IO_LOG(this_.m_loop, TRACE, this_.m_parent, "Received data, size:", nread);
    } else {
        IO_LOG(this_.m_loop, TRACE, this_.m_parent, "Receive error:", Error(nread));
    }

    Error error(nread);
    if (!error) {
        if (this_.m_receive_callback) {
            const auto prev_use_count = this_.m_read_buf.use_count();
            this_.m_receive_callback(*this_.m_parent, {this_.m_read_buf,  std::size_t(nread), this_.m_data_offset}, Error(0));
            if (prev_use_count != this_.m_read_buf.use_count()) { // user made a copy
                this_.m_read_buf.reset(); // will reallocate new one on demand
            }
        }

        this_.m_data_offset += static_cast<std::size_t>(nread);
    } else {
        this_.on_read_error(error);

        // Reseting close callback before close() call as we already called it
        this_.m_close_callback = nullptr;
        this_.close();
    }
}

///////////////////////////////////////// implementation ///////////////////////////////////////////

TcpConnectedClient::TcpConnectedClient(EventLoop& loop, TcpServer& server, CloseCallback cloase_callback) :
    Removable(loop),
    m_impl(new Impl(loop, server, *this, cloase_callback)) {
}

TcpConnectedClient::~TcpConnectedClient() {
}

void TcpConnectedClient::schedule_removal() {
    Removable::schedule_removal();
}

const Endpoint& TcpConnectedClient::endpoint() const {
    return m_impl->endpoint();
}

void TcpConnectedClient::close() {
    return m_impl->close();
}

bool TcpConnectedClient::is_open() const {
    return m_impl->is_open();
}

void TcpConnectedClient::send_data(const char* c_str, std::uint32_t size, EndSendCallback callback)  {
    return m_impl->send_data(c_str, size, callback);
}

void TcpConnectedClient::send_data(std::shared_ptr<const char> buffer, std::uint32_t size, EndSendCallback callback) {
    return m_impl->send_data(buffer, size, callback);
}

void TcpConnectedClient::send_data(const std::string& message, EndSendCallback callback) {
    return m_impl->send_data(message, callback);
}

void TcpConnectedClient::send_data(std::string&& message, EndSendCallback callback) {
    return m_impl->send_data(std::move(message), callback);
}

std::size_t TcpConnectedClient::pending_send_requesets() const {
    return m_impl->pending_write_requests();
}

void TcpConnectedClient::shutdown() {
    return m_impl->shutdown();
}

void TcpConnectedClient::start_read(DataReceiveCallback data_receive_callback) {
    return m_impl->start_read(data_receive_callback);
}

void* TcpConnectedClient::tcp_client_stream() {
    return m_impl->tcp_client_stream();
}

void TcpConnectedClient::delay_send(bool enabled) {
    return m_impl->delay_send(enabled);
}

bool TcpConnectedClient::is_delay_send() const {
    return m_impl->is_delay_send();
}

TcpServer& TcpConnectedClient::server() {
    return m_impl->server();
}

const TcpServer& TcpConnectedClient::server() const {
    return m_impl->server();
}

void TcpConnectedClient::set_endpoint(const Endpoint& endpoint) {
    return m_impl->set_endpoint(endpoint);
}

Error TcpConnectedClient::init_stream() {
    return m_impl->init_stream();
}

} // namespace io
} // namespace tarm
