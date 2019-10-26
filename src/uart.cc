//
// Created by jakub on 24.10.2019.
//

#include <ftdi.hpp>
#include <log.hh>
#include <utils.hh>
#include <uart.hh>

#define BUFSIZE		4096

Uart::Uart(const Device &device, const Glib::RefPtr<Gio::SocketAddress> &addr,
    int baudrate)
{
	Glib::RefPtr<Gio::SocketAddress> retaddr;

	m_context.set_interface(INTERFACE_A);

	if (m_context.open(device.vid, device.pid, device.description,
	    device.serial) != 0)
		throw std::runtime_error("Failed to open device");

	if (m_context.bitbang_disable() != 0)
		throw std::runtime_error("Failed to disable bitbang mode");

	if (m_context.set_baud_rate(baudrate) != 0)
		throw std::runtime_error("Failed to set the baud rate");

	m_socket_service = Gio::ThreadedSocketService::create(10);
	m_socket_service->reference();
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
	m_socket_service->unreference();
}

void
Uart::start()
{
	if (m_running)
		return;

	m_running = true;
	m_socket_service->start();
	m_usb_worker = std::thread(&Uart::usb_worker, this);
}

void
Uart::stop()
{
	if (!m_running)
		return;

	m_running = false;
	m_socket_service->stop();
	m_usb_worker.join();
	m_context.close();
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

		for (auto &i: m_connections)
			i->get_output_stream()->write(buffer, ret);
	}
}

bool
Uart::socket_worker(const Glib::RefPtr<Gio::SocketConnection> &conn,
    const Glib::RefPtr<Glib::Object> &source)
{
	Glib::RefPtr<Gio::InputStream> istream;
	uint8_t buffer[BUFSIZE];
	ssize_t ret;
	int written;

	Logger::info("UART: accepted connection from {}",
	    conn->get_remote_address()->to_string());

	istream = conn->get_input_stream();
	m_connections.push_back(conn);
	connected.emit(conn->get_remote_address());

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
