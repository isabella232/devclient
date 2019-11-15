//
// Created by jakub on 26.10.2019.
//

#ifndef DEVCLIENT_JTAG_HH
#define DEVCLIENT_JTAG_HH

#include <giomm.h>
#include <device.hh>

class JtagServer
{
public:
	JtagServer(const Device &device, Glib::RefPtr<Gio::InetAddress>,
	    uint16_t gdb_port, uint16_t ocd_port,
	    const std::string &board_type);
	void start();
	void stop();
	static void bypass(const Device &device);

	sigc::signal<void, const std::string &> output_produced;

protected:
	void child_exited(Glib::Pid pid, int code);
	bool output_ready(Glib::IOCondition cond,
	    Glib::RefPtr<Gio::UnixInputStream> stream);

	const Device &m_device;
	uint16_t m_ocd_port;
	uint16_t m_gdb_port;
	std::string m_board_type;
	Glib::Pid m_pid;
	Glib::RefPtr<Gio::UnixInputStream> m_out;
	Glib::RefPtr<Gio::UnixInputStream> m_err;
};

#endif //DEVCLIENT_JTAG_HH
