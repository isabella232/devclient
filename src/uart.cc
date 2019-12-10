/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * Copyright (c) 2019 Conclusive Engineering
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <ftdi.hpp>
#include <log.hh>
#include <utils.hh>
#include <uart.hh>

#define BUFSIZE		4096

Uart::Uart(const Device &device, const Glib::RefPtr<Gio::SocketAddress> &addr,
    int baudrate)
{
	Glib::RefPtr<Gio::SocketAddress> retaddr;

	m_context.set_interface(INTERFACE_C);

	if (m_context.open(device.vid, device.pid, device.description,
	    device.serial) != 0)
	{
		Gtk::MessageDialog warning("Failed to open device.");
		warning.set_position(Gtk::WIN_POS_CENTER_ALWAYS);
		warning.run();
		return;
	}
	
	if (m_context.bitbang_disable() != 0)
	{
		Gtk::MessageDialog warning("Failed to disable bitbang mode.");
		warning.set_position(Gtk::WIN_POS_CENTER_ALWAYS);
		warning.run();
		return;
	}

	if (m_context.set_baud_rate(baudrate) != 0)
	{
		Gtk::MessageDialog warning("Failed to set the baud rate.");
		warning.set_position(Gtk::WIN_POS_CENTER_ALWAYS);
		warning.run();
		return;
	}

	m_socket_service = Gio::ThreadedSocketService::create(10);
	m_socket_service->add_address(
	    addr,
	    Gio::SocketType::SOCKET_TYPE_STREAM,
	    Gio::SocketProtocol::SOCKET_PROTOCOL_TCP,
	    retaddr);

	m_socket_service->signal_run().connect(sigc::mem_fun(*this,
	    &Uart::socket_worker));

	Logger::info("UART: listening on {}", addr->to_string());
}

Uart::~Uart()
{
	stop();
}

void
Uart::start()
{
	if (m_running)
		return;

	m_socket_service->start();
	m_usb_worker = std::thread(&Uart::usb_worker, this);
	m_running = true;
	Logger::debug("UART: started");
}

void
Uart::stop()
{
	if (!m_running)
		return;

	m_running = false;
	m_socket_service->stop();
	m_socket_service->close();
	m_context.close();
	m_usb_worker.join();
	Logger::debug("UART: stopped");
}

void
Uart::usb_worker()
{
	uint8_t buffer[BUFSIZE];
	size_t ret;

	for (;;) {
		ret = m_context.read(buffer, sizeof(buffer));
		if (ret < 0 || !m_running)
			break;

		if (ret > 0)
			Logger::debug("read {} bytes from USB", ret);

		for (auto &connection: m_connections)
		{
			if (connection != nullptr)
				connection
					->get_output_stream()
					->write(buffer, ret);
		}
		
	}
}

bool
Uart::socket_worker(
	const Glib::RefPtr<Gio::SocketConnection> &conn,
    	const Glib::RefPtr<Glib::Object> &source
){
	Glib::RefPtr<Gio::InputStream> istream;
	Glib::RefPtr<Gio::OutputStream> ostream;
	uint8_t buffer[BUFSIZE];
	ssize_t ret;
	int written;

	Logger::info("UART: accepted connection from {}",
	    conn->get_remote_address()->to_string());

	istream = conn->get_input_stream();
	ostream = conn->get_output_stream();
	m_connections.push_back(conn);
	connected.emit(conn->get_remote_address());

	/* Disable local echo */
	ostream->write("\xFF\xFB\x01\xFF\xFB\x03");

	for (;;) {
		ret = istream->read(buffer, sizeof(buffer));
		if (ret == 0)
			break;

		Logger::debug("UART: read {} bytes from socket", ret);

		written = m_context.write(buffer, ret);
		if (written != ret) {
			Logger::error("UART: read {} bytes, written {} bytes",
			    ret, written);
		}
	}

	disconnected.emit(conn->get_remote_address());
	Logger::info("UART: connection from {} ended",
	    conn->get_remote_address()->to_string());

	return (false);
}
