
#ifndef ROM_H
#define ROM_H

#include <vector>
#include "typedef.hpp"
#include <string>


class ROM 
{
public:
	std::vector<u8> prg_rom {};
	std::vector<u8> chr_rom {};
	u8 mapper {};
	Mirroring screen_mirroring {};

	ROM();
	ROM(std::vector<u8>&);
	ROM(std::string);

	u8 read(u16);
	void write(u16, u8);
};


#endif