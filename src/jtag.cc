//
// Created by jakub on 26.10.2019.
//

#include <vector>
#include <giomm.h>
#include <log.hh>
#include <jtag.hh>
#include <ftdi.hpp>

JtagServer::JtagServer(Glib::RefPtr<Gio::InetAddress>, uint16_t gdb_port,
    uint16_t ocd_port, const std::string &board_type):
    m_ocd_port(ocd_port),
    m_gdb_port(gdb_port)
{
}

void
JtagServer::start()
{
	int stdout_fd;
	int stderr_fd;
	Glib::Pid pid;
	std::vector<std::string> argv {
		"/usr/local/bin/openocd",
		"-f",
		"test.conf"
	};

	Glib::spawn_async_with_pipes("/tmp", argv,
	    Glib::SpawnFlags::SPAWN_DO_NOT_REAP_CHILD,
	    Glib::SlotSpawnChildSetup(), &pid, nullptr,
	    &stdout_fd, &stderr_fd);

	m_out = Gio::UnixInputStream::create(stdout_fd, true);
	m_err = Gio::UnixInputStream::create(stderr_fd, true);

	Glib::signal_io().connect(
	    sigc::bind(sigc::mem_fun(*this, &JtagServer::output_ready), m_out),
	    stdout_fd, Glib::IOCondition::IO_IN);

	Glib::signal_io().connect(
	    sigc::bind(sigc::mem_fun(*this, &JtagServer::output_ready), m_err),
	    stderr_fd, Glib::IOCondition::IO_IN);

	Glib::signal_child_watch().connect(
	    sigc::mem_fun(*this, &JtagServer::child_exited),
	    pid);
}

void
JtagServer::stop()
{

}

void
JtagServer::bypass(const Device &device)
{
	Ftdi::Context context;

	context.set_interface(INTERFACE_B);

	if (context.open(device.vid, device.pid, device.description,
	    device.serial) != 0)
		throw std::runtime_error("Failed to open device");

	if (context.set_bitmode(0xff, BITMODE_RESET) != 0)
		throw std::runtime_error("Failed to set bitmode");

	if (context.set_bitmode(0, BITMODE_BITBANG) != 0)
		throw std::runtime_error("Failed to set bitmode");

	Logger::info("Bypass mode enabled");
	context.close();
}

bool
JtagServer::output_ready(Glib::IOCondition cond,
    Glib::RefPtr<Gio::UnixInputStream> stream)
{
	char buffer[4096];
	size_t nread;

	nread = stream->read(buffer, sizeof(buffer));
	output_produced.emit(std::string(buffer, nread));

	return (true);
}

void
JtagServer::child_exited(Glib::Pid pid, int code)
{
	Logger::info("OpenOCD exited with code {} (pid {})", code, pid);
}
