#ifndef Mapper_H
#define Mapper_H


#include "typedef.hpp"
#include <vector>
#include <string>



class Mapper 
{

public:
	Mapper(std::vector<u8> &prg, std::vector<u8> &chr, Mirroring mirroring) 
		: prg_rom {prg}, chr_rom {chr}, screen_mirroring {mirroring}
	{
	}

	virtual ~Mapper() {};

	virtual void CHR_write(u16 addr, u8 data);
	virtual u8 CHR_read(u16 addr);
	virtual u8 read(u16 addr) = 0;
	virtual void write(u16 addr, u8 data) = 0;

	virtual bool IRQ_Active();
	virtual void set_IRQ(bool);
	virtual std::vector<u8>& get_CHRrom();
	virtual Mirroring getMirroring();


protected:

	std::vector<u8> prg_rom {};
	std::vector<u8> chr_rom {};
	Mirroring screen_mirroring {};
	bool IRQ_request {};

};




#endif