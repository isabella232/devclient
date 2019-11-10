//
// Created by jakub on 31.10.2019.
//

#ifndef DEVCLIENT_24C256_HH
#define DEVCLIENT_24C256_HH

#include <vector>
#include <eeprom.hh>

#define EEPROM_24C256_ADDRESS_RD	0x
#define EEPROM_24C256_ADDRESS_WR	0xa0
#define EEPROM_SIZE			32768

class Eeprom24c256: public Eeprom
{
public:
	using Eeprom::Eeprom;

	void read(uint16_t offset, size_t length, std::vector<uint8_t> &data)
	{
		m_i2c.start();
		m_i2c.write({
		    EEPROM_24C256_ADDRESS_WR,
		    static_cast<unsigned char>((offset >> 8) & 0xff),
		    static_cast<unsigned char>(offset & 0xff)
		});

		m_i2c.start();
		m_i2c.write({ EEPROM_24C256_ADDRESS_RD });
		m_i2c.read(length, data);
		m_i2c.stop();
	}

	void write(uint16_t offset, const std::vector<uint8_t> &data)
	{
		m_i2c.start();
		m_i2c.write({
		    EEPROM_24C256_ADDRESS_WR,
		    static_cast<unsigned char>((offset >> 8) & 0xff),
		    static_cast<unsigned char>(offset & 0xff)
		});

		m_i2c.write(data);
		m_i2c.stop();
	}

	void erase()
	{

	}
};

#endif //DEVCLIENT_24C256_HH
