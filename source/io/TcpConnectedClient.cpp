#include "TcpConnectedClient.h"

#include "ByteSwap.h"
#include "Common.h"
#include "TcpServer.h"
#include "detail/TcpClientImplBase.h"

#include <assert.h>

namespace io {
class TcpConnectedClient::Impl : public detail::TcpClientImplBase<TcpConnectedClient, TcpConnectedClient::Impl> {
public:
    Impl(EventLoop& loop, TcpServer& server, TcpConnectedClient& parent);
    ~Impl();

    void close();

    void set_close_callback(CloseCallback callback);

    void shutdown();

    void start_read(DataReceiveCallback data_receive_callback);
    uv_tcp_t* tcp_client_stream();

protected:
    // statics
    static void on_shutdown(uv_shutdown_t* req, int status);
    static void on_close(uv_handle_t* handle);
    static void on_read(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf);

private:
    TcpServer* m_server = nullptr;

    DataReceiveCallback m_receive_callback = nullptr;

    CloseCallback m_close_callback = nullptr;
};


TcpConnectedClient::Impl::Impl(EventLoop& loop, TcpServer& server, TcpConnectedClient& parent) :
    TcpClientImplBase(loop, parent),
    m_server(&server) {
    init_stream();
    m_is_open = true;
}

TcpConnectedClient::Impl::~Impl() {
    IO_LOG(m_loop, TRACE, this, "_");

    // TODO: move to base class
    assert(m_tcp_stream != nullptr);
    delete m_tcp_stream;
}

void TcpConnectedClient::Impl::shutdown() {
    if (!is_open()) {
        return;
    }

    m_is_open = false;

    auto shutdown_req = new uv_shutdown_t;
    shutdown_req->data = this;
    uv_shutdown(shutdown_req, reinterpret_cast<uv_stream_t*>(m_tcp_stream), on_shutdown);
}

void TcpConnectedClient::Impl::set_close_callback(CloseCallback callback) {
    m_close_callback = callback;
}

uv_tcp_t* TcpConnectedClient::Impl::tcp_client_stream() {
    return m_tcp_stream;
}

void TcpConnectedClient::Impl::close() {
    if (!is_open()) {
        return;
    }

    IO_LOG(m_loop, TRACE, "address:", ip4_addr_to_string(m_ipv4_addr), ":", port());

    m_is_open = false;

    //if (!uv_is_closing(reinterpret_cast<uv_handle_t*>(m_tcp_stream))) {
        uv_close(reinterpret_cast<uv_handle_t*>(m_tcp_stream), on_close);
        //m_tcp_stream->data = nullptr;
        //m_tcp_stream = nullptr;
    //}
}

void TcpConnectedClient::Impl::start_read(DataReceiveCallback data_receive_callback) {
    m_receive_callback = data_receive_callback;

    if (m_receive_callback) {
        // TODO: handle return status code
        uv_read_start(reinterpret_cast<uv_stream_t*>(m_tcp_stream),
                      alloc_read_buffer,
                      on_read);
    }

    //TODO: set ip and port
    set_ipv4_addr(0);
    set_port(0);
}

////////////////////////////////////////////// static //////////////////////////////////////////////

void TcpConnectedClient::Impl::on_shutdown(uv_shutdown_t* req, int status) {
    auto& this_ = *reinterpret_cast<TcpConnectedClient::Impl*>(req->data);

    IO_LOG(this_.m_loop, TRACE, this_.m_parent, "address:", io::ip4_addr_to_string(this_.m_ipv4_addr), ":", this_.port());

    // TODO: close callback call on_shutdown???

    if (this_.m_close_callback) {
        this_.m_close_callback(*this_.m_parent, Error(status));
        this_.m_close_callback = nullptr; // TODO: looks like a hack
    }

    this_.m_server->remove_client_connection(this_.m_parent);

    uv_close(reinterpret_cast<uv_handle_t*>(req->handle), on_close);
    delete req;
}

void TcpConnectedClient::Impl::on_close(uv_handle_t* handle) {
    auto loop_ptr = reinterpret_cast<EventLoop*>(handle->loop->data);
    IO_LOG(loop_ptr, TRACE, "");

    auto& this_ = *reinterpret_cast<TcpConnectedClient::Impl*>(handle->data);
    this_.m_server->remove_client_connection(this_.m_parent);

    if (this_.m_close_callback) {
        this_.m_close_callback(*this_.m_parent, Error(0));
    }

    this_.m_port = 0;
    this_.m_ipv4_addr = 0;

    this_.m_parent->schedule_removal();

    //delete reinterpret_cast<uv_tcp_t*>(handle);
}

void TcpConnectedClient::Impl::on_read(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf) {
    auto& this_ = *reinterpret_cast<TcpConnectedClient::Impl*>(handle->data);

    if (nread >= 0) {
        IO_LOG(this_.m_loop, TRACE, "Received data, size:", nread);
    } else {
        IO_LOG(this_.m_loop, TRACE, "Receive error:", uv_strerror(nread));
    }

    Error error(nread);
    if (!error) {
        if (this_.m_receive_callback) {
            this_.m_receive_callback(*this_.m_server, *this_.m_parent, buf->base, nread);
        }
    } else {
        // TODO: this is common code with TcpCLient. Extract to TcpClientImplBase???
        //---------------------------------------------
        IO_LOG(this_.m_loop, DEBUG, "connection end address:", ip4_addr_to_string(this_.ipv4_addr()), ":", this_.port(), "reason:", error.string());

        if (this_.m_close_callback) {
            if (error.code() == io::StatusCode::END_OF_FILE) {
                this_.m_close_callback(*this_.m_parent, Error(0)); // OK
            } else {
                // Could be CONNECTION_RESET_BY_PEER (ECONNRESET), for example
                this_.m_close_callback(*this_.m_parent, error);
            }
        }
        //---------------------------------------------

        this_.m_close_callback = nullptr; // TODO: looks like a hack
        this_.close();
        //this_.m_server->remove_client_connection(this_.m_parent);
        //this_.m_parent->schedule_removal();
    }
}

///////////////////////////////////////// implementation ///////////////////////////////////////////

TcpConnectedClient::TcpConnectedClient(EventLoop& loop, TcpServer& server) :
    Disposable(loop),
    m_impl(new Impl(loop, server, *this)) {
}

TcpConnectedClient::~TcpConnectedClient() {
}

void TcpConnectedClient::schedule_removal() {
    Disposable::schedule_removal();
}

std::uint32_t TcpConnectedClient::ipv4_addr() const {
    return m_impl->ipv4_addr();
}

std::uint16_t TcpConnectedClient::port() const {
    return m_impl->port();
}

void TcpConnectedClient::close() {
    return m_impl->close();
}

bool TcpConnectedClient::is_open() const {
    return m_impl->is_open();
}

void TcpConnectedClient::send_data(std::shared_ptr<const char> buffer, std::uint32_t size, EndSendCallback callback) {
    return m_impl->send_data(buffer, size, callback);
}

void TcpConnectedClient::send_data(const std::string& message, EndSendCallback callback) {
    return m_impl->send_data(message, callback);
}

void TcpConnectedClient::set_close_callback(CloseCallback callback) {
    return m_impl->set_close_callback(callback);
}

std::size_t TcpConnectedClient::pending_write_requesets() const {
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

void TcpConnectedClient::set_ipv4_addr(std::uint32_t value) {
    return m_impl->set_ipv4_addr(value);
}

void TcpConnectedClient::set_port(std::uint16_t value) {
    return m_impl->set_port(value);
}

} // namespace io
