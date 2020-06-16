/*----------------------------------------------------------------------------------------------
 *  Copyright (c) 2020 - present Alexander Voitenko
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *----------------------------------------------------------------------------------------------*/

#pragma once

#include "BufferSizeResult.h"
#include "CommonMacros.h"
#include "DataChunk.h"
#include "Endpoint.h"
#include "Error.h"
#include "EventLoop.h"
#include "Export.h"
#include "Forward.h"
#include "Removable.h"
#include "UserDataHolder.h"

#include <memory>
#include <functional>

namespace tarm {
namespace io {

class UdpClient : public Removable,
                  public UserDataHolder {
public:
    using EndSendCallback = std::function<void(UdpClient&, const Error&)>;
    using DataReceivedCallback = std::function<void(UdpClient&, const DataChunk&, const Error&)>;
    using TimeoutCallback = std::function<void(UdpClient&, const Error&)>;

    IO_FORBID_COPY(UdpClient);
    IO_FORBID_MOVE(UdpClient);

    IO_DLL_PUBLIC UdpClient(EventLoop& loop);

    IO_DLL_PUBLIC Error start_receive(DataReceivedCallback receive_callback);
    IO_DLL_PUBLIC Error start_receive(DataReceivedCallback receive_callback,
                                      std::size_t timeout_ms,
                                      TimeoutCallback timeout_callback);

    IO_DLL_PUBLIC Error set_destination(const Endpoint& endpoint);
    IO_DLL_PUBLIC const Endpoint& endpoint() const;
    // Returns 0 on error
    IO_DLL_PUBLIC std::uint16_t bound_port() const;

    IO_DLL_PUBLIC  bool is_open() const;

    IO_DLL_PUBLIC BufferSizeResult receive_buffer_size() const;
    IO_DLL_PUBLIC BufferSizeResult send_buffer_size() const;
    IO_DLL_PUBLIC Error set_receive_buffer_size(std::size_t size);
    IO_DLL_PUBLIC Error set_send_buffer_size(std::size_t size);

    IO_DLL_PUBLIC void send_data(const char* c_str, std::uint32_t size, EndSendCallback callback = nullptr);
    IO_DLL_PUBLIC void send_data(std::shared_ptr<const char> buffer, std::uint32_t size, EndSendCallback callback = nullptr);
    IO_DLL_PUBLIC void send_data(const std::string& message, EndSendCallback callback = nullptr);
    IO_DLL_PUBLIC void send_data(std::string&& message, EndSendCallback callback = nullptr);

    IO_DLL_PUBLIC void schedule_removal() override;

    // TODO: implement close with incativity timeout like for UdpPeer

protected:
    IO_DLL_PUBLIC ~UdpClient();

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace io
} // namespace tarm
