//
// Created by jakub on 26.10.2019.
//

#include <fmt/format.h>
#include <gtkmm.h>
#include <device.hh>
#include <deviceselect.hh>

DeviceSelectDialog::DeviceSelectDialog():
    Gtk::Dialog("Select device", true),
    m_ok(Gtk::Stock::OK),
    m_cancel(Gtk::Stock::CANCEL)
{
	m_store = Gtk::ListStore::create(m_columns);
	m_treeview.set_model(m_store);
	m_treeview.append_column("VID", m_columns.m_vid);
	m_treeview.append_column("PID", m_columns.m_pid);
	m_treeview.append_column("Description", m_columns.m_description);
	m_treeview.append_column("Serial", m_columns.m_serial);

	for (const auto &i: DeviceEnumerator::enumerate()) {
		auto row = *(m_store->append());
		row[m_columns.m_vid] = fmt::format("{:#04x}", i.vid);
		row[m_columns.m_pid] = fmt::format("{:#04x}", i.pid);
		row[m_columns.m_description] = i.description;
		row[m_columns.m_serial] = i.serial;
		row[m_columns.m_device] = i;
	}

	m_ok.signal_clicked().connect(sigc::mem_fun(*this,
	    &DeviceSelectDialog::ok_clicked));
	m_cancel.signal_clicked().connect(sigc::mem_fun(*this,
	    &DeviceSelectDialog::cancel_clicked));

	get_content_area()->add(m_treeview);
	get_action_area()->add(m_ok);
	get_action_area()->add(m_cancel);
	set_size_request(400, 300);
	show_all_children();
}

std::optional<Device>
DeviceSelectDialog::get_selected_device()
{
	Glib::RefPtr<Gtk::TreeSelection> sel;
	Gtk::TreeModel::iterator it;

	sel = m_treeview.get_selection();
	if (!sel)
		return (std::nullopt);

	it = sel->get_selected();
	if (!it)
		return (std::nullopt);

	return (std::make_optional((*it)[m_columns.m_device]));
}

void
DeviceSelectDialog::ok_clicked()
{
	response(Gtk::ResponseType::RESPONSE_OK);
}

void
DeviceSelectDialog::cancel_clicked()
{
	response(Gtk::ResponseType::RESPONSE_CANCEL);
}

DeviceSelectDialog::ModelColumns::ModelColumns()
{
	add(m_vid);
	add(m_pid);
	add(m_description);
	add(m_serial);
	add(m_device);
}
