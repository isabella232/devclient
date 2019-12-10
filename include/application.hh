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
		static Application *instance();
		
		~Application();
		
		Application(Application const&) = delete;
		void operator=(Application const&) = delete;
		
		int run();
		void close();
	private:
		Application();
		
		static Application *m_instance;
		
		Glib::RefPtr<Gtk::Application> m_app;
		MainWindow *m_window;
	};
}

#endif //DEVCLIENT_APPLICATION_HH
