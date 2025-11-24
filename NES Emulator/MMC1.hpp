#ifndef MMC1_H
#define MMC1_H

#include <array>

#include "Mapper.hpp"


class MMC1 : public Mapper {

public:

	MMC1(std::vector<u8>& prg, std::vector<u8>& chr, Mirroring mirroring) : Mapper(prg, chr, mirroring)
	{
        controlRegister.val = 0xF;
	}

	~MMC1() override = default;
	u8 read(u16 addr) override;
	void write(u16 addr, u8 data) override;
    void CHR_write(u16 addr, u8 data) override;
    u8 CHR_read(u16 addr) override;

private:
	
	u8 shiftRegister { 0x10 };

    union {
        struct {
            unsigned int M : 2;
            unsigned int P : 2;
            unsigned int C : 1;
            unsigned int u : 3;

        };
        u8 val {};
    } controlRegister;

    u8 chrBank_0 {};
    u8 chrBank_1 {};
    u8 prgBank {};

    std::array<u8, 8192> wram {};


};


#endif