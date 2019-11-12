//
// Created by jakub on 11.11.2019.
//

#include <vector>
#include <string>
#include <giomm.h>
#include <wait.h>
#include <log.hh>
#include <dtb.hh>

DTB::DTB( std::shared_ptr<std::string> &dts,
    std::shared_ptr<std::vector<uint8_t>> &dtb):
    m_dts(dts),
    m_dtb(dtb)
{
}

DTB::~DTB()
{
	Logger::debug("DTB destroyed");
}

void DTB::run_dtc(bool compile, const SlotDone &done)
{
	int stdin_fd;
	int stdout_fd;
	int stderr_fd;
	Glib::Pid pid;

	m_done = done;
	m_compile = compile;

	std::string errors;
	std::vector<std::string> argv {
		"/usr/bin/dtc",
		"-I", compile ? "dts" : "dtb",
		"-O", compile ? "dtb" : "dts"
	};

	Glib::spawn_async_with_pipes("/tmp", argv,
	    Glib::SpawnFlags::SPAWN_DO_NOT_REAP_CHILD,
	    Glib::SlotSpawnChildSetup(),
	    &pid, &stdin_fd, &stdout_fd, &stderr_fd);

	m_in = Gio::UnixOutputStream::create(stdin_fd, true);
	m_out = Gio::UnixInputStream::create(stdout_fd, true);
	m_err = Gio::UnixInputStream::create(stderr_fd, true);

	Glib::signal_io().connect(
	    sigc::mem_fun(*this, &DTB::output_ready),
	    stdout_fd, Glib::IOCondition::IO_IN);

	Glib::signal_io().connect(
	    sigc::mem_fun(*this, &DTB::error_ready),
	    stderr_fd, Glib::IOCondition::IO_IN);

	Glib::signal_child_watch().connect(
	    sigc::mem_fun(*this, &DTB::child_exited),
	    pid);

	if (compile)
		m_in->write(*m_dts);
	else
		m_in->write(m_dtb->data(), m_dtb->size());

	m_in->flush();
	m_in->close();
}

void
DTB::compile(const DTB::SlotDone &done)
{
	run_dtc(true, done);
}

void
DTB::decompile(const DTB::SlotDone &done)
{
	run_dtc(false, done);
}

bool
DTB::output_ready(Glib::IOCondition condition)
{
	ssize_t ret;
	uint8_t buffer[1024];

	ret = m_out->read(buffer, sizeof(buffer));
	if (ret == 0)
		return (false);

	if (ret < 0)
		return (false);

	if (m_compile)
		m_dtb->insert(m_dtb->end(), &buffer[0], &buffer[ret]);
	else
		m_dts->insert(m_dts->end(), &buffer[0], &buffer[ret]);

	return (true);
}

bool
DTB::error_ready(Glib::IOCondition condition)
{
	ssize_t ret;
	uint8_t buffer[1024];

	ret = m_err->read(buffer, sizeof(buffer));
	if (ret == 0)
		return (false);

	if (ret < 0)
		return (false);

	m_errors.insert(m_errors.end(), &buffer[0], &buffer[ret]);
	return (true);
}

void
DTB::child_exited(Glib::Pid pid, int code)
{
	Logger::debug("Child exited, status: {}", code);
	m_done(code == 0, m_compile ? m_dtb->size() : m_dts->size(), m_errors);
}
