//
// Created by jakub on 26.10.2019.
//

#ifndef DEVCLIENT_JTAG_HH
#define DEVCLIENT_JTAG_HH

#include <giomm.h>

class JtagServer
{
public:
	JtagServer(Gio::InetAddress addr, uint16_t gdb_port,
	    uint16_t ocd_port, const std::string &board_type);
	void start();
	void stop();

	sigc::signal<void, const std::string &> output_produced;

protected:

};

#endif //DEVCLIENT_JTAG_HH
