#ifndef PPU_H
#define PPU_H

#include "typedef.hpp"
#include <vector>
#include "ROM.hpp"


class PPU
{
public:

    // Storage
    std::vector<u8> vram {};
    std::vector<u8> OAMdata {};
    std::vector<u8> palette_table {};
    std::vector<u8> chr_rom {};
    std::vector<u8> secondaryOAM {};

    Mirroring mirroring {};

    // Registers 
    union {
        struct {
            unsigned int V : 1;
            unsigned int P : 1;
            unsigned int H : 1;
            unsigned int B : 1;
            unsigned int S : 1;
            unsigned int I : 1;
            unsigned int N2 : 1;
            unsigned int N1 : 1;
        };
        u8 val {};
    } ctrl {};

    union {
        struct {
            unsigned int B : 1;
            unsigned int Gr : 1;
            unsigned int R : 1;
            unsigned int s : 1;
            unsigned int b : 1;
            unsigned int M : 1;
            unsigned int m : 1;
            unsigned int G : 1;
        };
        u8 val {};
    } mask {};

    union {
        struct {
            unsigned int unused : 5;
            unsigned int V : 1;
            unsigned int S : 1;
            unsigned int O : 1;
        };
        u8 val {};
    } status {};

    u8 OAMaddr {};
    u8 scroll {};
    u8 OAMdma {};


    // Internal Registers

    union {
        struct {
            u16 X : 5; 
            u16 Y : 5;
            u16 N : 2;
            u16 y : 3;
            u16 u : 1; 
        };
        u16 val;
    } register_v {};


    union {
        struct {
            u16 X : 5;
            u16 Y : 5; // Coarse Y
            u16 N : 2;
            u16 y : 3; // Fine y
            u16 u : 1;
        };
        u16 val;
    } register_t {};


    u8 register_x {};
    u8 register_w {};

    u8 read_buffer {};

    u8 dot {}; // Cycle Tracking
    u8 OAMpointer {}; // Pointer for OAM during Sprite Evaluation
    u8 SecondaryOAMpointer {};
    u8 OAMpointerData {};
    bool spriteOverflow {false}; // For use in EvaluateSprite function

    u8 scanline {}; // Current Scanline

    // Rendering latches
    u8 nametableLatch {};
    u8 attributeLatch {};
    u8 patternLowLatch {};
    u8 patternHighLatch {};


    // Rendering shift registers
    u16 patternLowRegister {};
    u16 patternHighRegister {};
    u16 attributeRegister1 {};
    u16 attributeRegister2 {};

    PPU(std::vector<u8>&, Mirroring);
    PPU() = default;

    u8 read(u16 addr);
    void write(u16 addr, u8 data);
    u8 read_PPU_data(u16 addr);
    void write_PPU_data(u16 addr, u8 data);

    void tick();

    void render_visible_scanline();
    void pushPixel();
    void fill_shift_registers();
    u8 get_background_pixel();
    void getPixelData();

    u16 getMirrorAddress(u16);
    //void SpriteEvaluation();

    void incrementCoarseY();
    void incrementCoarseX();
};


#endif // !PPU_H
