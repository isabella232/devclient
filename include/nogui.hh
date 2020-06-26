#ifndef NOGUI_HH
#define NOGUI_HH

#include <uart.hh>
#include <jtag.hh>

class SerialCmdLine
{
public:
	SerialCmdLine(const Device &device, const Glib::RefPtr<Gio::SocketAddress> &addr, int baudrate);

	std::shared_ptr<Uart> m_uart;
	Glib::RefPtr<Gio::ThreadedSocketService> m_socket_service;
	sigc::signal<void, Glib::RefPtr<Gio::SocketAddress>> m_connected;
	sigc::signal<void, Glib::RefPtr<Gio::SocketAddress>> m_disconnected;
	Glib::RefPtr<Glib::MainLoop> main_loop;
	void start();

private:
	bool socket_worker(const Glib::RefPtr<Gio::SocketConnection> &conn, const Glib::RefPtr<Glib::Object> &source);
	std::vector<UartConnection> m_connections;
	void usb_worker();
	void remove_connection(const UartConnection &conn);
	Device m_device;
	Ftdi::Context m_context;
	void client_connected(Glib::RefPtr<Gio::SocketAddress> addr);
	void client_disconnected(Glib::RefPtr<Gio::SocketAddress> addr);
	Glib::RefPtr<Gio::SocketAddress> m_addr;
	int m_baudrate;
	std::thread m_usb_worker;
	bool m_running;
};

class JtagCmdLine
{
public:
	bool gui;

	JtagCmdLine(void);
	JtagCmdLine(const Device &device, Glib::RefPtr<Gio::InetAddress> address, uint16_t gdb_port,  uint16_t ocd_port, const std::string &board_script);
	JtagCmdLine(const Device &device);
	std::shared_ptr<JtagServer> m_server;
	void bypass(const Device &device);
	void on_output_ready(const std::string &output);
	void on_server_start();
	void on_server_exit();

private:
	const Device &m_device;
	Glib::RefPtr<Gio::InetAddress> m_address;
	uint16_t m_ocd_port;
	uint16_t m_gdb_port;
	std::string m_board_script;
	bool m_running;
};

#endif // NOGUI_HH

