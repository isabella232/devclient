#ifndef DEVCLIENT_APPLICATION_HH
#define DEVCLIENT_APPLICATION_HH

#include <memory>
#include <mutex>
#include <gtkmm.h>
#include <mainwindow.hh>

namespace Devclient
{
	class Application
	{
	public:
		static Devclient::Application& instance();
		
		~Application();
		
		Application(Application const&) = delete;
		void operator=(Application const&) = delete;
		
		int run();
		void close();
	private:
		Application();
		
		static std::unique_ptr<Devclient::Application> m_instance;
		static std::once_flag m_once;
		
		Glib::RefPtr<Gtk::Application> m_app;
		MainWindow *m_window;
	};
}

#endif //DEVCLIENT_APPLICATION_HH
