//
// Created by jakub on 27.10.2019.
//

#ifndef DEVCLIENT_GPIO_HH
#define DEVCLIENT_GPIO_HH

#include <ftdi.hpp>
#include <device.hh>

class Gpio
{
public:
	Gpio(const Device &device);
	virtual ~Gpio();

	uint8_t get();
	void set(uint8_t mask);

protected:
	Ftdi::Context m_context;
};

#endif //DEVCLIENT_GPIO_HH
