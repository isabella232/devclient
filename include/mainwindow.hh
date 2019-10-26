//
// Created by jakub on 26.10.2019.
//

#ifndef DEVCLIENT_MAINWINDOW_HH
#define DEVCLIENT_MAINWINDOW_HH

#include <gtkmm.h>
#include <formrow.hh>
#include <uart.hh>

class SerialTab: public Gtk::Box
{
public:
	SerialTab(const Device &dev);

protected:
	void start_clicked();
	void stop_clicked();
	void client_connected(Glib::RefPtr<Gio::SocketAddress> addr);
	void client_disconnected(Glib::RefPtr<Gio::SocketAddress> addr);

	FormRow<Gtk::Entry> m_address_row;
	FormRow<Gtk::Entry> m_port_row;
	FormRow<Gtk::ComboBoxText> m_baud_row;
	FormRow<Gtk::Label> m_status_row;
	Gtk::ScrolledWindow m_scroll;
	Gtk::ListViewText m_clients;
	Gtk::ButtonBox m_buttons;
	Gtk::Button m_start;
	Gtk::Button m_stop;
	Uart *m_uart;
	const Device &m_device;
};

class JtagTab: public Gtk::Box
{
public:
	JtagTab(const Device &dev);

protected:
	FormRow<Gtk::Entry> m_address_row;
	FormRow<Gtk::Entry> m_gdb_port_row;
	FormRow<Gtk::Entry> m_ocd_port_row;
	FormRow<Gtk::Entry> m_board_row;
	FormRow<Gtk::Label> m_status_row;
	Glib::RefPtr<Gtk::TextBuffer> m_textbuffer;
	Gtk::ScrolledWindow m_scroll;
	Gtk::TextView m_textview;
	Gtk::ButtonBox m_buttons;
	Gtk::Button m_start;
	Gtk::Button m_stop;
	const Device &m_device;
};

class EepromTab: public Gtk::Box
{
public:
	EepromTab(const Device &dev);

protected:
	Gtk::ScrolledWindow m_scroll;
	Gtk::TextView m_textview;
	Gtk::ButtonBox m_buttons;
	Gtk::Button m_read;
	Gtk::Button m_write;
	Gtk::Button m_save;
	const Device &m_device;
};

class MainWindow: public Gtk::Window
{
public:
	MainWindow();
	virtual ~MainWindow();

protected:
	Gtk::Notebook m_notebook;
	SerialTab m_uart_tab;
	JtagTab m_jtag_tab;
	EepromTab m_eeprom_tab;
	Device m_device;
};

#endif //DEVCLIENT_MAINWINDOW_HH
