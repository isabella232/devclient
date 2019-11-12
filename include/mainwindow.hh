//
// Created by jakub on 26.10.2019.
//

#ifndef DEVCLIENT_MAINWINDOW_HH
#define DEVCLIENT_MAINWINDOW_HH

#include <gtkmm.h>
#include <formrow.hh>
#include <uart.hh>
#include <jtag.hh>
#include <gpio.hh>
#include <i2c.hh>
#include <dtb.hh>

class MainWindow;

class SerialTab: public Gtk::Box
{
public:
	SerialTab(MainWindow *parent, const Device &dev);

protected:
	void start_clicked();
	void stop_clicked();
	void launch_terminal_clicked();
	void client_connected(Glib::RefPtr<Gio::SocketAddress> addr);
	void client_disconnected(Glib::RefPtr<Gio::SocketAddress> addr);

	FormRow<Gtk::Entry> m_address_row;
	FormRow<Gtk::Entry> m_port_row;
	FormRow<Gtk::ComboBoxText> m_baud_row;
	FormRow<Gtk::Entry> m_status_row;
	Gtk::Separator m_separator;
	Gtk::Label m_label;
	Gtk::ScrolledWindow m_scroll;
	Gtk::ListViewText m_clients;
	Gtk::ButtonBox m_buttons;
	Gtk::Button m_start;
	Gtk::Button m_stop;
	Gtk::Button m_terminal;
	Uart *m_uart;
	MainWindow *m_parent;
	const Device &m_device;
};

class JtagTab: public Gtk::Box
{
public:
	JtagTab(MainWindow *parent, const Device &dev);

protected:
	void start_clicked();
	void stop_clicked();
	void bypass_clicked();
	void output_ready(const std::string &output);

	FormRow<Gtk::Entry> m_address_row;
	FormRow<Gtk::Entry> m_gdb_port_row;
	FormRow<Gtk::Entry> m_ocd_port_row;
	FormRow<Gtk::Entry> m_board_row;
	FormRow<Gtk::Entry> m_status_row;
	Glib::RefPtr<Gtk::TextBuffer> m_textbuffer;
	Gtk::ScrolledWindow m_scroll;
	Gtk::TextView m_textview;
	Gtk::ButtonBox m_buttons;
	Gtk::Button m_start;
	Gtk::Button m_stop;
	Gtk::Button m_bypass;
	JtagServer *m_server;
	MainWindow *m_parent;
	const Device &m_device;
};

class EepromTab: public Gtk::Box
{
public:
	EepromTab(MainWindow *parent, const Device &dev);

protected:
	void read_clicked();
	void write_clicked();
	void compile_done(bool ok, int size, const std::string &errors);
	void decompile_done(bool ok, int size, const std::string &errors);

	Glib::RefPtr<Gtk::TextBuffer> m_textbuffer;
	Gtk::ScrolledWindow m_scroll;
	Gtk::TextView m_textview;
	Gtk::ButtonBox m_buttons;
	Gtk::Button m_read;
	Gtk::Button m_write;
	Gtk::Button m_save;
	std::shared_ptr<DTB> m_dtb;
	std::shared_ptr<std::string> m_textual;
	std::shared_ptr<std::vector<uint8_t>> m_blob;
	MainWindow *m_parent;
	const Device &m_device;
};

class MainWindow: public Gtk::Window
{
public:
	MainWindow();
	virtual ~MainWindow();

	Gpio *m_gpio;
	I2C *m_i2c;

protected:
	Gtk::Notebook m_notebook;
	SerialTab m_uart_tab;
	JtagTab m_jtag_tab;
	EepromTab m_eeprom_tab;
	Device m_device;
};

#endif //DEVCLIENT_MAINWINDOW_HH
