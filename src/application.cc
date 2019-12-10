#include <application.hh>
#include <mainwindow.hh>

Devclient::Application *Devclient::Application::m_instance = nullptr;

Devclient::Application *Devclient::Application::instance()
{
	if (Devclient::Application::m_instance == nullptr)
		Devclient::Application::m_instance
			= new Devclient::Application();
	return Devclient::Application::m_instance;
}

Devclient::Application::Application() :
	m_app(Gtk::Application::create("pl.conclusive.devclient")),
	m_window(new MainWindow())
{}

Devclient::Application::~Application()
{
	delete m_window;
}

int Devclient::Application::run()
{
	return m_app->run(*m_window);
}

void Devclient::Application::close()
{
	Gtk::Main::quit();
}
