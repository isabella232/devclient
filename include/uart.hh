//
// Created by jakub on 24.10.2019.
//

#ifndef DEVCLIENT_UART_HH
#define DEVCLIENT_UART_HH

#include <thread>
#include <vector>
#include <giomm.h>
#include <ftdi.hpp>
#include <device.hh>

class Uart
{
public:
	Uart(const Device &device, const Glib::RefPtr<Gio::SocketAddress> &addr, int baudrate);
	virtual ~Uart();
	void start();
	void stop();

	sigc::signal<void, Glib::RefPtr<Gio::SocketAddress>> connected;
	sigc::signal<void, Glib::RefPtr<Gio::SocketAddress>> disconnected;

protected:
	virtual void usb_worker();
	virtual bool socket_worker(
	    const Glib::RefPtr<Gio::SocketConnection> &conn,
	    const Glib::RefPtr<Glib::Object> &source);

	Ftdi::Context m_context;
	Glib::RefPtr<Gio::ThreadedSocketService> m_socket_service;
	std::vector<Glib::RefPtr<Gio::SocketConnection>> m_connections;
	std::thread m_usb_worker;
	bool m_running;
};

#endif //DEVCLIENT_UART_HH
