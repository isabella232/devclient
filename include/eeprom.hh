//
// Created by jakub on 31.10.2019.
//

#ifndef DEVCLIENT_EEPROM_HH
#define DEVCLIENT_EEPROM_HH

#include <i2c.hh>
#include <eeprom.hh>

class Eeprom
{
public:
	Eeprom(I2C &i2c): m_i2c(i2c) {}
	virtual ~Eeprom() {}

	virtual void read(uint16_t offset, size_t length,
	    std::vector<uint8_t> &data) = 0;
	virtual void write(uint16_t offset,
	    const std::vector<uint8_t> &data) = 0;
	virtual void erase() = 0;

protected:
	I2C &m_i2c;
};

#endif //DEVCLIENT_EEPROM_HH
