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
 
#include <vector>
#include <giomm.h>
#include <gtkmm/main.h>
#include <ftdi.hpp>
#include <log.hh>
#include <jtag.hh>
#include <utils.hh>
#include <filesystem.hh>

#define BUFFER_SIZE	1024

JtagServer::JtagServer(const Device &device, Glib::RefPtr<Gio::InetAddress>,
    uint16_t gdb_port, uint16_t ocd_port, const std::string &board_script):
    m_device(device),
    m_ocd_port(ocd_port),
    m_gdb_port(gdb_port),
    m_board_script(board_script),
    m_running(false)
{
}

JtagServer::~JtagServer()
{
	stop();
}

void
JtagServer::start()
{
	int stdout_fd;
	int stderr_fd;
	std::vector<std::string> argv {
		executable_dir() + "/tools/bin/openocd",
		"-c", fmt::format("gdb_port {}", m_gdb_port),
		"-c", fmt::format("telnet_port {}", m_ocd_port),
		"-c", "tcl_port disabled",
		"-c", "interface ftdi",
		"-c", "transport select jtag",
		"-c", "adapter_khz 8000",
		"-c", "ftdi_channel 1",
		"-c", "ftdi_layout_init 0x0008 0x000b",
		"-c", "ftdi_layout_signal nTRST -data 0x10",
		"-c", "ftdi_layout_signal nSRST -oe 0x20 -data 0x20",
		"-c", "adapter_nsrst_delay 500",
		"-c", fmt::format("ftdi_serial \"{}\"", m_device.serial),
		"-c", fmt::format("ftdi_vid_pid {:#04x} {:#04x}", m_device.vid, m_device.pid),
		"-f", m_board_script
	};

	if (m_running)
		return;

	try {
		Glib::spawn_async_with_pipes("/tmp", argv,
		    Glib::SpawnFlags::SPAWN_DO_NOT_REAP_CHILD,
		    Glib::SlotSpawnChildSetup(), &m_pid, nullptr,
		    &stdout_fd, &stderr_fd);
	} catch (const Glib::Error &err) {
		Gtk::MessageDialog warning("Failed to run JTAG server: " + err.what());
		warning.set_position(Gtk::WIN_POS_CENTER_ALWAYS);
		warning.run();
		return;
	}

	m_out = Gio::UnixInputStream::create(stdout_fd, true);
	m_err = Gio::UnixInputStream::create(stderr_fd, true);

	m_out->read_bytes_async(BUFFER_SIZE, sigc::bind(sigc::mem_fun(
	    *this, &JtagServer::output_ready), m_out));

	m_err->read_bytes_async(BUFFER_SIZE, sigc::bind(sigc::mem_fun(
	    *this, &JtagServer::output_ready), m_err));

	Glib::signal_child_watch().connect(
	    sigc::mem_fun(*this, &JtagServer::child_exited),
	    m_pid);

	setpgid(m_pid, getpid());
	m_running = true;
}

void
JtagServer::stop()
{
	if (!m_running)
		return;

	kill(m_pid, SIGTERM);

	/* Crappy, there should be a better way to do this */
	while (m_running)
		Gtk::Main::iteration();
}

void
JtagServer::bypass(const Device &device)
{
	Ftdi::Context context;

	context.set_interface(INTERFACE_B);

	if (context.open(device.vid, device.pid, device.description,
	    device.serial) != 0)
	{
		Gtk::MessageDialog warning("Failed to open device.");
		warning.set_position(Gtk::WIN_POS_CENTER_ALWAYS);
		warning.run();
		return;
	}

	if (context.set_bitmode(0xff, BITMODE_RESET) != 0)
	{
		Gtk::MessageDialog warning("Failed to set BITMODE_RESET.");
		warning.set_position(Gtk::WIN_POS_CENTER_ALWAYS);
		warning.run();
		return;
	}

	if (context.set_bitmode(0, BITMODE_BITBANG) != 0)
	{
		Gtk::MessageDialog warning("Failed to set BITMODE_BITBANG.");
		warning.set_position(Gtk::WIN_POS_CENTER_ALWAYS);
		warning.run();
		return;
	}

	Logger::info("Bypass mode enabled");
	
	Gtk::MessageDialog info("Bypass mode enabled.");
	info.set_position(Gtk::WIN_POS_CENTER_ALWAYS);
	info.run();
	
	context.close();
}

void
JtagServer::output_ready(Glib::RefPtr<Gio::AsyncResult> &result,
    Glib::RefPtr<Gio::UnixInputStream> stream)
{
	Glib::RefPtr<Glib::Bytes> buffer;
	const char *ptr;
	size_t nread;

	buffer = stream->read_bytes_finish(result);
	if (buffer.get() == nullptr)
		return;

	ptr = static_cast<const char *>(buffer->get_data(nread));
	if (nread == 0)
		return;

	output_produced.emit(std::string(ptr, nread));
	stream->read_bytes_async(BUFFER_SIZE, sigc::bind(sigc::mem_fun(
	    *this, &JtagServer::output_ready), stream));
}

void
JtagServer::child_exited(Glib::Pid pid, int code)
{
	Logger::info("OpenOCD exited with code {} (pid {})", code, pid);
	m_running = false;
}
