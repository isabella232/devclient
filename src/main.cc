#include <iostream>
#include <memory>
#include <sysexits.h>
#include <getopt.h>
#include <fmt/format.h>
#include <string>

#include <device.hh>
#include <uart.hh>
#include <mainwindow.hh>
#include <gtkmm/application.h>

static const struct option long_options[] = {
    { "list", no_argument, nullptr, 'l' },
    { "device", optional_argument, nullptr, 'd' },
    { "serial", required_argument, nullptr, 's' },
    { "help", no_argument, nullptr, 'h' },
    { nullptr, 0, nullptr, 0}
};

static void
usage(const std::string &argv0)
{

}

int
main(int argc, char *const argv[])
{
	Glib::RefPtr<Gtk::Application> app;
	Device dev;
	std::unique_ptr<Uart> uart;
	std::string uart_listen_addr;
	std::string serial;
	bool list = false;
	int ch;

	for (;;) {
		ch = getopt_long(argc, argv, "ld:s:h", long_options, nullptr);
		if (ch == -1)
			break;

		switch (ch) {
		case 'l':
			list = true;
			break;

		case 'd':
			serial = optarg;
			break;

		case 's':
			uart_listen_addr = optarg;
			break;

		case 'h':
			usage(argv[0]);
			return (EX_USAGE);

		default:
			fmt::print(stderr, "Unrecognized option: {}", ch);
			usage(argv[0]);
			return (EX_USAGE);
		}
	}

	Gio::init();

	if (list) {
		fmt::print("Available devices:\n");
		for (const auto &i: DeviceEnumerator::enumerate()) {
			fmt::print("{:#04x}:{:#04x} - {} ({})\n", i.vid, i.pid,
			    i.description, i.serial);
		}
	}

	if (!uart_listen_addr.empty()) {
		auto saddr = Gio::InetSocketAddress::create(
		    Gio::InetAddress::create("127.0.0.1"), 1234);

		dev = *DeviceEnumerator::find_by_serial(serial);
		uart = std::unique_ptr<Uart>(new Uart(dev, saddr, 115200));
	}

	app = Gtk::Application::create("pl.conclusive.devclient");

	MainWindow window;
	app->run(window);
	return 0;
}
