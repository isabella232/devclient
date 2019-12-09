#include <application.hh>
#include <mainwindow.hh>

std::unique_ptr<Devclient::Application> Devclient::Application::m_instance;
std::once_flag Devclient::Application::m_once;

Devclient::Application::Application() :
	m_app(Gtk::Application::create("pl.conclusive.devclient")),
	m_window(new MainWindow())
{}

Devclient::Application::~Application()
{
	delete m_window;
}

Devclient::Application& Devclient::Application::instance()
{
	std::call_once(
		Devclient::Application::m_once,
		[]{
			Devclient::Application::m_instance.reset(
				new Application);
		});
	return *Devclient::Application::m_instance.get();
}

int Devclient::Application::run()
{
	return m_app->run(*m_window);
}

void Devclient::Application::close()
{
	// Gtk::Main::quit();
}
