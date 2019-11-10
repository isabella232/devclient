//
// Created by jakub on 27.10.2019.
//

#include <fmt/format.h>
#include <ftdi.hpp>
#include <device.hh>
#include <gpio.hh>

Gpio::Gpio(const Device &device)
{
	m_context.set_interface(INTERFACE_D);

	if (m_context.open(device.vid, device.pid, device.description,
	    device.serial) != 0) {
		throw std::runtime_error(fmt::format(
		    "Failed to open device: {}",
		    m_context.error_string()));
	}

	if (m_context.set_bitmode(0xff, BITMODE_RESET) != 0)
		throw std::runtime_error("Failed to set bitmode");

	if (m_context.set_bitmode(0xff, BITMODE_BITBANG) != 0)
		throw std::runtime_error("Failed to set bitmode");
}

Gpio::~Gpio()
{
	m_context.close();
}

uint8_t
Gpio::get()
{
	uint8_t rd;

	m_context.read(&rd, 1);
	return (rd);
}

void
Gpio::set(uint8_t mask)
{
	m_context.write(&mask, 1);
}
