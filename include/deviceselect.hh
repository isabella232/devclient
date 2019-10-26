//
// Created by jakub on 26.10.2019.
//

#ifndef DEVCLIENT_DEVICESELECT_HH
#define DEVCLIENT_DEVICESELECT_HH

#include <optional>
#include <gtkmm.h>
#include <device.hh>

class DeviceSelectDialog: public Gtk::Dialog
{
public:
	DeviceSelectDialog();
	std::optional<Device> get_selected_device();

protected:
	class ModelColumns: public Gtk::TreeModel::ColumnRecord
	{
	public:
		ModelColumns();

		Gtk::TreeModelColumn<Glib::ustring> m_vid;
		Gtk::TreeModelColumn<Glib::ustring> m_pid;
		Gtk::TreeModelColumn<Glib::ustring> m_description;
		Gtk::TreeModelColumn<Glib::ustring> m_serial;
		Gtk::TreeModelColumn<Device> m_device;
	};

	void ok_clicked();
	void cancel_clicked();

	Glib::RefPtr<Gtk::ListStore> m_store;
	Gtk::TreeView m_treeview;
	Gtk::Button m_ok;
	Gtk::Button m_cancel;
	ModelColumns m_columns;
};

#endif //DEVCLIENT_DEVICESELECT_HH
