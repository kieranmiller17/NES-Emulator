#ifndef MMC3_H
#define MMC3_H

#include <array>

#include "Mapper.hpp"

class MMC3 : public Mapper {

public:

    MMC3(std::vector<u8>& prg, std::vector<u8>& chr, Mirroring mirroring) : Mapper(prg, chr, mirroring) {
        prg_size = prg_rom.size() / 8192;
    }

    ~MMC3() override = default;
    u8 read(u16 addr) override;
    void write(u16 addr, u8 data) override;
    u8 CHR_read(u16 addr) override;



private:

    union {
        struct {
            unsigned int R : 3;
            unsigned int u : 3;
            unsigned int P : 1;
            unsigned int C : 1;

        };
        u8 val {};
    } bankSelect;

    u8 IRQ_counter {};
    u8 IRQ_reload_val {};
    bool IRQ_reload {};
    bool IRQ_enabled {};

    u8 A12_prev_value {}; // for scanline counting

    u8 CHRbank_2kb_1 {};
    u8 CHRbank_2kb_2 {};

    u8 CHRbank_1kb_1 {};
    u8 CHRbank_1kb_2 {};
    u8 CHRbank_1kb_3 {};
    u8 CHRbank_1kb_4 {};

    u8 PRGbank_1 {};
    u8 PRGbank_2 {};
    int prg_size {};

    std::array<u8, 8192> wram {};
};


#endif