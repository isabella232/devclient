//
// Created by jakub on 11.11.2019.
//

#include <vector>
#include <string>
#include <giomm.h>
#include <wait.h>
#include <log.hh>
#include <utils.hh>
#include <dtb.hh>

#define BUFFER_SIZE	1024

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
		executable_dir() / "tools/bin/dtc",
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

	m_out->read_bytes_async(BUFFER_SIZE,
	    sigc::mem_fun(*this, &DTB::output_ready));

	m_err->read_bytes_async(BUFFER_SIZE,
	    sigc::mem_fun(*this, &DTB::error_ready));

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

void
DTB::output_ready(Glib::RefPtr<Gio::AsyncResult> &result)
{
	Glib::RefPtr<Glib::Bytes> data;
	const uint8_t *buffer;
	size_t len;

	data = m_out->read_bytes_finish(result);
	buffer = static_cast<const uint8_t *>(data->get_data(len));

	if (len == 0)
		return;

	if (m_compile)
		m_dtb->insert(m_dtb->end(), &buffer[0], &buffer[len]);
	else
		m_dts->insert(m_dts->end(), &buffer[0], &buffer[len]);

	m_out->read_bytes_async(1024, sigc::mem_fun(*this, &DTB::output_ready));
}

void
DTB::error_ready(Glib::RefPtr<Gio::AsyncResult> &result)
{
	Glib::RefPtr<Glib::Bytes> data;
	const uint8_t *buffer;
	size_t len;

	data = m_err->read_bytes_finish(result);
	buffer = static_cast<const uint8_t *>(data->get_data(len));

	if (len == 0)
		return;

	m_errors.insert(m_errors.end(), &buffer[0], &buffer[len]);
	m_err->read_bytes_async(1024, sigc::mem_fun(*this, &DTB::error_ready));
}

void
DTB::child_exited(Glib::Pid pid, int code)
{
	Logger::debug("Child exited, status: {}", code);
	m_done(code == 0, m_compile ? m_dtb->size() : m_dts->size(), m_errors);
}
