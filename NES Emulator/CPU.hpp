#ifndef CPU_H
#define CPU_H


#include "typedef.hpp"
#include <array>
#include <vector>
#include <functional>
#include <string>
#include "BUS.hpp"
#include "ROM.hpp"

class CPU
{

public:

	BUS bus {};

	void load(std::string);
	void reset();


	// Execute next opcode / instruction
	void execute();


	// Tick when cycle complete
	void tick();


    // Read and Write functionality
	u8 read(u16 addr);
	void write(u16 addr, u8 data);
	u8 readStack(u16 addr);
	void writeStack(u16 addr, u8 data);
	void writeDMA(u8 data);

	// Logging
	void log();
	void log_action();


private:


	// Registers
	u8 register_a { 0 };
	u8 register_x { 0 };
	u8 register_y { 0 };
	u8 status { 0b00100100 };


	// Counters / pointers
	u16 program_counter { 0x8000 };
	u8 stack_pointer { 0xFD };
	int cycles { 0 };


	// NMI Management
	bool NMI_found {};
	int NMIcycle {};


	// Record current opcode for use in debugging
	u8 op_code {};


	// Functions for setting status flags
	void setNegative(bool);
	void setOverflow(bool);
	void setUnused(bool);
	void setBreak(bool);
	void setDecimal(bool);
	void setInterrupt(bool);
	void setZero(bool);
	void setCarry(bool);
	void setNegativeAndZero(u8);


	// Addressing modes
	void relative_addressing(bool);
	void immediate_addressing(std::function<void (u8)>);
	void accumulator_addressing(std::function<u8 (u8)>);
	void implied_addressing(std::function<void ()>);
	void absolute_read_addressing(std::function<void (u8)>);
	void absolute_rmw_addressing(std::function<u8 (u8)>);
	void absolute_write_addressing(std::function<void (u16)>);
	void zero_page_read_addressing(std::function<void (u8)>);
	void zero_page_rmw_addressing(std::function<u8 (u8)>);
	void zero_page_write_addressing(std::function<void (u8)>);
	void zero_page_X_read_addressing(std::function<void (u8)>);
	void zero_page_X_rmw_addressing(std::function<u8 (u8)>);
	void zero_page_X_write_addressing(std::function<void (u8)>);
	void zero_page_Y_read_addressing(std::function<void (u8)>);
	void zero_page_Y_rmw_addressing(std::function<u8 (u8)>);
	void zero_page_Y_write_addressing(std::function<void (u8)>);
	void absolute_X_read_addressing(std::function<void (u8)>);
	void absolute_X_rmw_addressing(std::function<u8 (u8)>);
	void absolute_X_write_addressing(std::function<void (u16)>);
	void absolute_Y_read_addressing(std::function<void (u8)>);
	void absolute_Y_write_addressing(std::function<void (u16)>);
	void indirect_X_read_addressing(std::function<void (u8)>);
	void indirect_X_rmw_addressing(std::function<u8 (u8)>);
	void indirect_X_write_addressing(std::function<void (u16)>);
	void indirect_Y_read_addressing(std::function<void (u8)>);
	void indirect_Y_rmw_addressing(std::function<u8 (u8)>);
	void indirect_Y_write_addressing(std::function<void (u16)>);


	// Interrupt
	void BRK();
	void NMI();
	void IRQ();


	// Instructions Accessing Stack
	void RTI();
	void RTS();
	void PHA();
	void PHP();
	void PLA();
	void PLP();
	void JSR();


	// Branch Instructions
	void BCC();
	void BCS();
	void BNE();
	void BEQ();
	void BPL();
	void BMI();
	void BVC();
	void BVS();


	// Read instructions
	void LDA(u8);
	void LDX(u8);
	void LDY(u8);
	void EOR(u8);
	void AND(u8);
	void ORA(u8);
	void ADC(u8);
	void SBC(u8);
	void CMP(u8);
	void CPX(u8);
	void CPY(u8);
	void BIT(u8);


	// RMW Instructions
	u8 ASL(u8);
	u8 LSR(u8);
	u8 ROL(u8);
	u8 ROR(u8);
	u8 INC(u8);
	u8 DEC(u8);


	// Write Instructions
	void STX(u16);
	void STY(u16);
	void STA(u16);


	// Implied Instructions
	void CLC();
	void CLD();
	void CLI();
	void CLV();
	void DEX();
	void DEY();
	void INX();
	void INY();
	void NOP();
	void SEC();
	void SED();
	void SEI();
	void TAX();
	void TAY();
	void TSX();
	void TXA();
	void TXS();
	void TYA();


	// Jump Instructions
	void abs_addressing_JMP();
	void abs_indirect_addressing_JMP();

};

#endif