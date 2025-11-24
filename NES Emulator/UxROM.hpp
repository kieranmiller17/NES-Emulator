#ifndef UxROM_H
#define UxROM_H

#include "Mapper.hpp"

class UxROM : public Mapper {

public:

	UxROM(std::vector<u8>& prg, std::vector<u8>& chr, Mirroring mirroring) : Mapper(prg, chr, mirroring) 
	{
		banks = prg.size() / 16384;
	}

	~UxROM() override = default;
	u8 read(u16 addr) override;
	void write(u16 addr, u8 data) override;


private:
	u8 bankSelect {};
	u8 banks {};
};

#endif