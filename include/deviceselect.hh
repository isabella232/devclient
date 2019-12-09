/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * Copyright (c) 2019 Conclusive Engineering
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

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

	Glib::RefPtr<Gtk::ListStore> m_store;
	Gtk::TreeView m_treeview;
	Gtk::Button m_ok;
	ModelColumns m_columns;
};

#endif //DEVCLIENT_DEVICESELECT_HH
