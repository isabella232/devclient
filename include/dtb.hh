//
// Created by jakub on 11.11.2019.
//

#ifndef DEVCLIENT_DTB_HH
#define DEVCLIENT_DTB_HH

#include <vector>
#include <string>
#include <condition_variable>

class DTB
{
public:
	using SlotDone = sigc::slot<void, bool, int, std::string>;

	DTB(std::shared_ptr<std::string> &dts, std::shared_ptr<std::vector<uint8_t>> &dtb);
	virtual ~DTB();
	void compile(const SlotDone &done);
	void decompile(const SlotDone &done);

protected:
	void run_dtc(bool compile, const SlotDone &done);
	void output_ready(Glib::RefPtr<Gio::AsyncResult> &result);
	void error_ready(Glib::RefPtr<Gio::AsyncResult> &result);
	void child_exited(Glib::Pid pid, int code);

	Glib::RefPtr<Gio::UnixOutputStream> m_in;
	Glib::RefPtr<Gio::UnixInputStream> m_out;
	Glib::RefPtr<Gio::UnixInputStream> m_err;
	std::shared_ptr<std::vector<uint8_t>> m_dtb;
	std::shared_ptr<std::string> m_dts;
	std::string m_errors;
	SlotDone m_done;
	bool m_compile;
};

#endif //DEVCLIENT_DTB_HH
