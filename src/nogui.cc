#include <utils.hh>
#include <nogui.hh>
#include <log.hh>

#define BUFSIZE         4096


SerialCmdLine::SerialCmdLine(const Device &device, const Glib::RefPtr<Gio::SocketAddress> &addr, int baudrate) : main_loop(Glib::MainLoop::create())
{
	Glib::RefPtr<Gio::SocketAddress> retaddr;

	m_running = false;
	m_context.set_interface(INTERFACE_C);
	m_device = device;


	if (m_context.open(device.vid, device.pid, device.description,
	    device.serial) != 0) {
		Logger::error("Failed to open device.");
		return;
	}

	if (m_context.reset() != 0) {
		Logger::error("Failed to reset UART channel");
		return;
	}

	if (m_context.set_bitmode(0xff, BITMODE_RESET) != 0) {
		Logger::error("Failed to reset bitmode.");
		return;
	}

	if (m_context.bitbang_disable() != 0) {
		Logger::error("Failed to set bitbang_disable.");
		return;
	}

	if (m_context.set_baud_rate(baudrate) != 0) {
		Logger::error("Failed to set the baud rate.");
		return;
	}

	try {
		m_socket_service = Gio::ThreadedSocketService::create(10);
		m_socket_service->add_address(addr, Gio::SocketType::SOCKET_TYPE_STREAM, Gio::SocketProtocol::SOCKET_PROTOCOL_TCP, retaddr);
	} catch (const Glib::Exception &err) {
		throw std::runtime_error(err.what());
	}

	m_socket_service->signal_run().connect(sigc::mem_fun(*this, &SerialCmdLine::socket_worker));

	m_device = device;
	m_addr = addr;
	m_baudrate = baudrate;
}


void
SerialCmdLine::start(void)
{
	int baud;

	if (m_uart)
		return;

	try {
		m_socket_service->start();
		m_usb_worker = std::thread(&SerialCmdLine::usb_worker, this);
		m_running = true;
	} catch (const std::runtime_error &err) {
		Logger::warning("UART: I/O error: {}", err.what());
	}
}


bool
SerialCmdLine::socket_worker(const Glib::RefPtr<Gio::SocketConnection> &conn, const Glib::RefPtr<Glib::Object> &source)
{
	UartConnection uartconn;
	Glib::RefPtr<Gio::InputStream> istream;
	uint8_t buffer[BUFSIZE];
	ssize_t ret;
	int written;

	Logger::info("UART: accepted connection from {}", conn->get_remote_address()->to_string());

	uartconn.m_address = conn->get_remote_address();
	uartconn.m_address->reference();
	uartconn.m_cancel = Gio::Cancellable::create();
	uartconn.m_conn = conn;
	uartconn.m_ostream = conn->get_output_stream();

	istream = conn->get_input_stream();
	m_connections.push_back(uartconn);
	m_connected.emit(uartconn.m_address);

	/* Disable local echo */
	uartconn.m_ostream->write("\xFF\xFB\x01\xFF\xFB\x03");
	uartconn.m_ostream->write(fmt::format("==> Connected to {} {} <==\r\n",
	    m_device.description, m_device.serial));

	for (;;) {
		try {
			ret = istream->read(buffer, sizeof(buffer),
			    uartconn.m_cancel);
			if (ret <= 0)
				break;
		} catch (const Gio::Error &err) {
			Logger::warning("UART: I/O error: {}", err.what());
			break;
		}

		Logger::debug("UART: read {} bytes from socket", ret);

		written = m_context.write(buffer, ret);
		if (written != ret) {
			Logger::error("UART: read {} bytes, written {} bytes",
			    ret, written);
		}
	}

	Logger::info("UART: connection from {} ended",
	    uartconn.m_address->to_string());

	remove_connection(uartconn);
	return (false);
}


void
SerialCmdLine::remove_connection(const UartConnection &conn)
{
	auto it = std::find(m_connections.begin(),
	    m_connections.end(), conn);

	if (it != m_connections.end()) {
		m_disconnected.emit(conn.m_conn->get_remote_address());
		m_connections.erase(it);
	}
}


void
SerialCmdLine::usb_worker()
{
	uint8_t buffer[BUFSIZE];
	int ret;

	Logger::debug("UART: USB thread started");

	for (;;) {
		ret = m_context.read(buffer, sizeof(buffer));
		if (ret < 0 || !m_running) {
			fmt::print("ret: {:d}\n", ret);
			break;
		}
		if (ret == 0)
			continue;

		Logger::debug("read {} bytes from USB", ret);

		for (auto &i: m_connections) {
			try {
				i.m_ostream->write(buffer, ret);
			} catch (const Gio::Error &err) {
				Logger::warning(
				    "UART: error sending data to {}: {}",
				    i.m_conn->get_remote_address()->to_string(),
				    err.what());
				remove_connection(i);
				continue;
			}
		}
	}

	Logger::debug("UART: USB thread stopped");
}


void
JtagCmdLine::on_output_ready(const std::string &output)
{
	fmt::print("{:s}", output);
}


JtagCmdLine::JtagCmdLine(const Device &device, Glib::RefPtr<Gio::InetAddress> address, uint16_t gdb_port,  uint16_t ocd_port, const std::string &board_script) :
    m_address(address),
    m_device(device),
    m_ocd_port(ocd_port),
    m_gdb_port(gdb_port),
    m_board_script(board_script),
    m_running(false)
{
	m_server = std::make_shared<JtagServer>(device, address, gdb_port, ocd_port, board_script);
	m_server->on_output_produced.connect(sigc::mem_fun(*this, &JtagCmdLine::on_output_ready));
}


JtagCmdLine::JtagCmdLine(const Device &device) :
    m_device(device)
{
}


void
JtagCmdLine::bypass(const Device &device)
{
	Ftdi::Context context;

	context.set_interface(INTERFACE_B);

	if (context.open(device.vid, device.pid, device.description,
	    device.serial) != 0) {
		Logger::error("Failed to open device.");
		return;
	}

	if (context.reset() != 0) {
		Logger::error("Failed to reset channel");
		return;
	}

	if (context.set_bitmode(0xff, BITMODE_RESET) != 0) {
		Logger::error("Failed to set BITMODE_RESET");
		return;
	}

	if (context.set_bitmode(0, BITMODE_BITBANG) != 0)
	{
		Logger::error("Failed to set BITMODE_BITBANG");
		return;
	}

	Logger::info("Bypass mode enabled.");

	context.close();
}
