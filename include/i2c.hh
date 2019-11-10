//
// Created by jakub on 25.10.2019.
//

#ifndef DEVCLIENT_I2C_HH
#define DEVCLIENT_I2C_HH

#include <vector>
#include <ftdi.hpp>

#define SCL		(1u << 0)
#define SDA_OUT		(1u << 1)
#define SDA_IN		(1u << 2)
#define WP		(1u << 4)
#define OUT_PINS	(SCL | SDA_OUT | WP)

class I2C
{
public:
	I2C(const Device &device, int clock);
	virtual ~I2C();

	void start();
	void stop();
	void read(size_t nbytes, std::vector<uint8_t> &result);
	void write(const std::vector<uint8_t> &data);

protected:
	uint8_t read_byte(bool ack);
	void write_byte(uint8_t byte);

	Ftdi::Context m_context;
};

#endif //DEVCLIENT_I2C_HH
