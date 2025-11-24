#ifndef PPU_H
#define PPU_H

#include "typedef.hpp"
#include <vector>

#include "Mapper.hpp"


class PPU
{
public:

    PPU(Mapper*, std::vector<u8>&);
    PPU() = default;


    // Data access
    u8 read(u16 addr);
    void write(u16 addr, u8 data);
    u8 read_PPU_data(u16 addr);
    void write_PPU_data(u16 addr, u8 data);
    u8 read_palette_data(u16);
    void write_palette_data(u16, u8);
    void writeDMA(u8);


    // Rendering
    void tick();


    // Getters and setters
    bool NMI_triggered();
    void set_NMI(bool);
    bool immediate_NMI_triggered();
    void set_immediate_NMI(bool);
    void resetCycles();
    u8 getCycles();
    u16 getDot();
    u16 getScanline();


    // Display Data
    void print_pattern(u8);
    void print_pallette();

private:

    // Storage
    std::vector<u8> vram {};
    std::vector<u8> OAMdata {};
    std::vector<u8> palette_table {};
    std::vector<u8> chr_rom {};
    std::vector<u8> secondaryOAM {};
    Mapper* rom {};


    // Registers 

    union {
        struct {
            unsigned int N : 2;
            unsigned int I : 1;
            unsigned int S : 1;
            unsigned int B : 1;
            unsigned int H : 1;
            unsigned int P : 1;
            unsigned int V : 1;
        };
        u8 val {};
    } ctrl {};

    union {
        struct {
            unsigned int G : 1;
            unsigned int m : 1;
            unsigned int M : 1;
            unsigned int b : 1;
            unsigned int s : 1;
            unsigned int R : 1;
            unsigned int Gr : 1;
            unsigned int B : 1;
        };
        u8 val {};
    } mask {};

    union {
        struct {
            unsigned int unused : 5;
            unsigned int O : 1;
            unsigned int S : 1;
            unsigned int V : 1;
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


    // Cycle Tracking
    u16 dot {};
    u16 scanline {}; 

    // OAM Functions
    u16 OAMpointer {}; 
    u8 SecondaryOAMpointer {};
    u8 OAMpointerData {};

    // Rendering latches
    u16 nametableLatch {};
    u16 attributeLatch {};
    u16 patternLowLatch {};
    u16 patternHighLatch {};

    // Rendering shift registers
    u16 patternLowRegister {};
    u16 patternHighRegister {};
    u16 attributeRegister1 {};
    u16 attributeRegister2 {};

    // Sprite Registers
    std::vector<u16> spriteRegisters {};
    u16 SpriteRegistersPointer {};

    // Sprite information
    u8 sprite_tile {};
    u8 sprite_attributes {};
    u16 sprite_y {};
    u16 sprite_x {};
    u16 spritePatternLow {};
    u16 spritePatternHigh {};
    bool sprite_priority { false };
    bool spriteOverflow { false };
    bool sprite_zero_hit { false };
    bool flipVertically { false };

    // Cycle Management
    bool NMI_enabled { false };
    bool immediate_NMI { false };
    int NMIcycles {};
    bool odd_cycle { true };
    u32 cycles {};
    bool skipVBlank { false };


    // Rendering 
    void render();
    void prerender_scanline();
    void visible_scanline();
    void postrender_scanline();

    // Sprite Eval
    void SpriteEvaluation();


    // Pixel output 
    void getPixelData();
    u8 get_background_pixel();
    u8 get_sprite_pixel();
    u8 choose_pixel(u8, u8);
    void fill_shift_registers();
    void pushPixel(u8);


    // Misc
    u16 getMirrorAddress(u16);
    void incrementCoarseY();
    void incrementCoarseX();
    u8 reverse(u8);

};


#endif // !PPU_H
