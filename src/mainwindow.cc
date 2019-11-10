//
// Created by jakub on 26.10.2019.
//

#include <string>
#include <fmt/format.h>
#include <formrow.hh>
#include <utils.hh>
#include <deviceselect.hh>
#include <mainwindow.hh>

MainWindow::MainWindow():
    m_uart_tab(m_device),
    m_jtag_tab(m_device),
    m_eeprom_tab(m_device)
{
	DeviceSelectDialog dialog;

	set_title("Conclusive developer cable client");
	set_size_request(640, 480);

	m_notebook.append_page(m_uart_tab, "Serial console");
	m_notebook.append_page(m_jtag_tab, "JTAG");
	m_notebook.append_page(m_eeprom_tab, "EEPROM");
	add(m_notebook);
	show_all_children();

	if (dialog.run() == Gtk::ResponseType::RESPONSE_CANCEL)
		throw std::runtime_error("No device selected");

	m_device = dialog.get_selected_device().value();
	m_i2c = new I2C(m_device, 100000);
	m_gpio = new Gpio(m_device);
	m_gpio->set(0);
}

MainWindow::~MainWindow()
{

}

SerialTab::SerialTab(const Device &dev):
    Gtk::Box(Gtk::Orientation::ORIENTATION_VERTICAL),
    m_device(dev),
    m_address_row("Listen address"),
    m_port_row("Listen port"),
    m_baud_row("Port baud rate"),
    m_status_row("Status"),
    m_clients(1),
    m_start("Start"),
    m_stop("Stop"),
    m_terminal("Launch terminal"),
    m_label("Connected clients:")
{

	m_address_row.get_widget().set_text("127.0.0.1");
	m_port_row.get_widget().set_text("2222");
	m_baud_row.get_widget().append("9600");
	m_baud_row.get_widget().append("19200");
	m_baud_row.get_widget().append("38400");
	m_baud_row.get_widget().append("57600");
	m_baud_row.get_widget().append("115200");
	m_baud_row.get_widget().set_active_text("115200");
	m_status_row.get_widget().set_editable(false);
	m_clients.set_column_title(0, "Client address");
	m_scroll.add(m_clients);

	m_start.signal_clicked().connect(sigc::mem_fun(*this,
	    &SerialTab::start_clicked));

	m_stop.signal_clicked().connect(sigc::mem_fun(*this,
	    &SerialTab::stop_clicked));

	m_terminal.signal_clicked().connect(sigc::mem_fun(*this,
	    &SerialTab::launch_terminal_clicked));

	m_buttons.pack_start(m_start);
	m_buttons.pack_start(m_stop);
	m_buttons.pack_start(m_terminal);

	set_border_width(5);
	pack_start(m_address_row, false, true);
	pack_start(m_port_row, false, true);
	pack_start(m_baud_row, false, true);
	pack_start(m_status_row, false, true);
	pack_start(m_separator, false, true);
	pack_start(m_label, false, true);
	pack_start(m_scroll, true, true);
	pack_start(m_buttons, false, true);
}

void
SerialTab::start_clicked()
{
	Glib::RefPtr<Gio::SocketAddress> addr;
	int baud;

	baud = std::stoi(m_baud_row.get_widget().get_active_text());
	addr = Gio::InetSocketAddress::create(
	    Gio::InetAddress::create(m_address_row.get_widget().get_text()),
	    std::stoi(m_port_row.get_widget().get_text()));

	m_uart = new Uart(m_device, addr, baud);
	m_uart->connected.connect(sigc::mem_fun(*this,
	    &SerialTab::client_connected));
	m_uart->disconnected.connect(sigc::mem_fun(*this,
	    &SerialTab::client_disconnected));
	m_uart->start();
	m_status_row.get_widget().set_text("Running");
}

void
SerialTab::stop_clicked()
{
	m_uart->stop();
	delete m_uart;

	m_status_row.get_widget().set_text("Stopped");
}

void
SerialTab::launch_terminal_clicked()
{
	std::vector<std::string> argv {
		"x-terminal-emulator",
		"-e",
		fmt::format("telnet 127.0.0.1 {}", m_port_row.get_widget().get_text())
	};

	Glib::spawn_async("/", argv, Glib::SpawnFlags::SPAWN_SEARCH_PATH);
}

void
SerialTab::client_connected(Glib::RefPtr<Gio::SocketAddress> addr)
{
	m_clients.append(addr->to_string());
}

void
SerialTab::client_disconnected(Glib::RefPtr<Gio::SocketAddress> addr)
{
	Glib::RefPtr<Gtk::TreeModel> model = m_clients.get_model();
	Glib::ustring addr_s = addr->to_string();
	Gtk::ListStore *store;

	store = dynamic_cast<Gtk::ListStore *>(model.get());
	store->foreach_iter([=] (const Gtk::TreeIter &it) -> bool {
		Glib::ustring result;

		it->get_value(0, result);
		if (result == addr_s)
			store->erase(it);
	});
}

JtagTab::JtagTab(const Device &dev):
    Gtk::Box(Gtk::Orientation::ORIENTATION_VERTICAL),
    m_device(dev),
    m_address_row("Listen address"),
    m_gdb_port_row("GDB server listen port"),
    m_ocd_port_row("OpenOCD listen port"),
    m_board_row("Board type"),
    m_status_row("Status"),
    m_start("Start"),
    m_stop("Stop"),
    m_bypass("J-Link bypass mode")
{
	m_address_row.get_widget().set_text("127.0.0.1");
	m_ocd_port_row.get_widget().set_text("4444");
	m_gdb_port_row.get_widget().set_text("3333");

	m_textbuffer = Gtk::TextBuffer::create();
	m_textview.set_editable(false);
	m_textview.set_buffer(m_textbuffer);
	m_textview.set_monospace(true);
	m_scroll.add(m_textview);
	m_buttons.pack_start(m_start);
	m_buttons.pack_start(m_stop);
	m_buttons.pack_start(m_bypass);

	m_start.signal_clicked().connect(sigc::mem_fun(*this,
	    &JtagTab::start_clicked));
	m_stop.signal_clicked().connect(sigc::mem_fun(*this,
	    &JtagTab::stop_clicked));
	m_bypass.signal_clicked().connect(sigc::mem_fun(*this,
	    &JtagTab::bypass_clicked));

	set_border_width(5);
	pack_start(m_address_row, false, true);
	pack_start(m_gdb_port_row, false, true);
	pack_start(m_ocd_port_row, false, true);
	pack_start(m_board_row, false, true);
	pack_start(m_status_row, false, true);
	pack_start(m_scroll, true, true);
	pack_start(m_buttons, false, true);
}

void
JtagTab::output_ready(const std::string &output)
{
	Glib::ustring text = m_textbuffer->get_text();

	text += output;
	m_textbuffer->set_text(text);
	m_textview.scroll_to(m_textbuffer->get_insert());
}

void
JtagTab::start_clicked()
{
	Glib::RefPtr<Gio::InetAddress> addr;

	addr = Gio::InetAddress::create(m_address_row.get_widget().get_text());
	m_server = new JtagServer(addr, 1234, 1235, "foo");
	m_server->output_produced.connect(sigc::mem_fun(*this, &JtagTab::output_ready));
	m_server->start();
}

void
JtagTab::stop_clicked()
{
}

void
JtagTab::bypass_clicked()
{
	JtagServer::bypass(m_device);
}

EepromTab::EepromTab(const Device &dev):
    Gtk::Box(Gtk::Orientation::ORIENTATION_VERTICAL),
    m_device(dev),
    m_read("Read"),
    m_write("Write"),
    m_save("Save buffer to file")
{
	m_scroll.add(m_textview);
	m_buttons.pack_start(m_read);
	m_buttons.pack_start(m_write);
	m_buttons.pack_start(m_save);

	set_border_width(5);
	pack_start(m_scroll, true, true);
	pack_start(m_buttons, false, true);
}
