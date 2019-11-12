//
// Created by jakub on 31.10.2019.
//

#ifndef DEVCLIENT_24C_HH
#define DEVCLIENT_24C_HH

#include <vector>
#include <log.hh>
#include <eeprom.hh>

#define EEPROM_24C256_ADDRESS_RD	0xa1
#define EEPROM_24C256_ADDRESS_WR	0xa0

class Eeprom24c: public Eeprom
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
		data.erase(data.begin());
	}

	void write(uint16_t offset, const std::vector<uint8_t> &data)
	{
		std::vector<uint8_t> slice;
		std::vector<uint8_t>::size_type i;

		for (i = 0; i < data.size(); i += 32) {
			Logger::debug("Writing to AT24C at offset {}", offset);
			m_i2c.start();
			m_i2c.write({
			    EEPROM_24C256_ADDRESS_WR,
			    static_cast<unsigned char>((offset >> 8) & 0xff),
			    static_cast<unsigned char>(offset & 0xff)
			});

			slice = std::vector<uint8_t>(data.begin() + i, data.begin() + i + 32);
			m_i2c.write(slice);
			m_i2c.stop();
			offset += 32;
			usleep(50000);
		}
	}

	void erase()
	{

	}
};

#endif //DEVCLIENT_24C_HH
