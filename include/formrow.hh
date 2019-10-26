//
// Created by jakub on 26.10.2019.
//

#ifndef DEVCLIENT_FORMROW_HH
#define DEVCLIENT_FORMROW_HH

#include <gtkmm.h>

template <class T>
class FormRow: public Gtk::Box
{
public:
	FormRow(const Glib::ustring &label):
	    Gtk::Box(Gtk::Orientation::ORIENTATION_HORIZONTAL, 10),
	    m_label(label)
	{
		m_label.set_justify(Gtk::Justification::JUSTIFY_LEFT);
		m_label.set_size_request(250, -1);
		pack_start(m_label, false, true);
		pack_start(m_widget, true, true);
		show_all_children();
	}

	T &get_widget()
	{
		return (m_widget);
	}

protected:
	T m_widget;
	Gtk::Label m_label;
};

#endif //DEVCLIENT_FORMROW_HH
