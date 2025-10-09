// CPU.cpp Contains code for CPU

#include "typedef.hpp"
#include <array>
#include <vector>
#include <cassert>
#include <iostream>
#include <bitset>
#include "BUS.hpp"
#include "CPU.hpp"
#include <functional>
#include <string>

void CPU::load(std::string path) {
	bus.rom = ROM(path);
}

void CPU::load(std::vector<u8>& cartridge) {
	bus.rom = ROM(cartridge);
}

void CPU::reset() {
	program_counter = 0;
	cycles = 0;
	status = 0b00100100;
	program_counter |= read(0xFFFC);
	program_counter |= (read(0xFFFD) << 8);
}

void CPU::tick() {
	cycles += 1;
};

u8 CPU::read(u16 addr) {
	return bus.read(addr);
};

void CPU::write(u16 addr, u8 data) {
	bus.write(addr, data);
};

u8 CPU::readStack(u16 addr) {
	return bus.read(0x100 + addr);
}

void CPU::writeStack(u16 addr, u8 data) {
	bus.write(0x100 + addr, data);
}

void CPU::log() {

	u8 prev_counter {};
	cycles = 7;

	for (int i { 0 }; i <= 6000; i++) {
		
		prev_counter = program_counter;
		std::cout << std::hex << (int)program_counter << "  ";

		for (int i { 0 }; i <= 2; i++) {
			std::cout << std::hex << (int)read(program_counter + i) << " ";
		}

		std::cout << " A:" << std::hex << (int)register_a << " ";
		std::cout << " X:" << std::hex << (int)register_x << " ";
		std::cout << " Y:" << std::hex << (int)register_y << " ";
		std::cout << " P:" << std::hex << (int)status << " ";
		std::cout << " SP:" << std::hex << (int)stack_pointer << " ";
		std::cout << " CYC:" << std::dec << (int)cycles << "\n";
		execute();

	}
}

void CPU::setNegative(bool val) {
	if (val) {
		status |= 0b10000000;
	}
	else {
		status &= 0b01111111;
	}
};

void CPU::setOverflow(bool val) {
	if (val) {
		status |= 0b1000000;
	}
	else {
		status &= 0b10111111;
	}
};

void CPU::setUnused(bool val) {
	if (val) {
		status |= 0b100000;
	}
	else {
		status &= 0b11011111;
	}
}

void CPU::setBreak(bool val) {
	if (val) {
		status |= 0b10000;
	}
	else {
		status &= 0b11101111;
	}
};

void CPU::setDecimal(bool val) {
	if (val) {
		status |= 0b1000;
	}
	else {
		status &= 0b11110111;
	}
};

void CPU::setInterrupt(bool val) {
	if (val) {
		status |= 0b100;
	}
	else {
		status &= 0b11111011;
	}
};

void CPU::setZero(bool val) {
	if (val) {
		status |= 0b10;
	}
	else {
		status &= 0b11111101;
	}
};

void CPU::setCarry(bool val) {
	if (val) {
		status |= 1;
	}
	else {
		status &= 0b11111110;
	}
};

void CPU::setNegativeAndZero(u8 val) {
	setNegative(val & 0x80);
	setZero(val == 0);
};


// Addressing Modes

void CPU::relative_addressing(bool branch) {
	s8 op { static_cast<s8>(read(program_counter)) }; program_counter++; tick(); // Tick 2

	if (!branch) { return; }; // Branch not taken

	u16 old_counter { program_counter };
	program_counter = old_counter + op;
	tick(); // Tick 3
	if (((old_counter & 0xFF) + op) > 0xFF) { tick(); }; // Tick 4 (if new page)
};

void CPU::immediate_addressing(std::function<void (u8)> func ) {
	func(read(program_counter++)); tick(); // Tick 2
};

void CPU::accumulator_addressing(std::function<u8 (u8)> func) {
	register_a = func(register_a); tick(); // Tick 2
}

void CPU::implied_addressing(std::function<void ()> func) {
	func(); tick(); // Tick 2
}

void CPU::absolute_read_addressing(std::function<void (u8)> func ) {
	u16 addr = read(program_counter++); tick(); // Tick 2
	addr |= (read(program_counter++) << 8); tick(); // Tick 3
	func(read(addr)); tick(); // Tick 4
};

void CPU::absolute_rmw_addressing(std::function<u8 (u8)> func ) {
	u16 addr = read(program_counter++); tick(); // Tick 2
	addr |= (read(program_counter++) << 8); tick(); // Tick 3
	u8 val = read(addr); tick(); // Tick 4
	write(addr, val); // (Probably pointless)
	u8 res = func(val); tick(); //Tick 5
	write(addr, res); tick(); // Tick 6;
};

void CPU::absolute_write_addressing(std::function<void (u16)> func ) {
	u16 addr = read(program_counter++); tick(); // Tick 2
	addr |= (read(program_counter++) << 8); tick(); // Tick 3
	func(addr); tick(); // Tick 4
}

void CPU::zero_page_read_addressing(std::function<void (u8)> func ) {
	u8 addr = read(program_counter++); tick();  // Tick 2
	func(read(addr)); tick();  // Tick 3
}

void CPU::zero_page_rmw_addressing(std::function<u8 (u8)> func) {
	u8 addr = read(program_counter++); tick();  // Tick 2
	u8 val = read(addr); tick(); // Tick 3
	write(addr, val); // (Probably pointless)
	u8 res = func(val); tick();  // Tick 4
	write(addr, res); tick(); // Tick 5
};

void CPU::zero_page_write_addressing(std::function<void (u8)> func) {
	u8 addr = read(program_counter++); tick(); // Tick 2
	func(addr); tick(); // Tick 3
}

void CPU::zero_page_X_read_addressing(std::function<void (u8)> func) {
	u8 addr = read(program_counter++); tick(); // Tick 2
	addr += register_x; tick(); // Tick 3
	func(read(addr)); tick(); // Tick 4
}

void CPU::zero_page_X_rmw_addressing(std::function<u8 (u8)> func) {
	u8 addr = read(program_counter++); tick(); // Tick 2
	addr += register_x; tick(); // Tick 3
	u8 val = read(addr); tick(); // Tick 4
	write(addr, val); 
	u8 res = func(val); tick(); // Tick 5
	write(addr, res); tick(); // Tick 6
}

void CPU::zero_page_X_write_addressing(std::function<void (u8)> func) {
	u8 addr = read(program_counter++); tick(); // Tick 2
	addr += register_x; tick(); // Tick 3
	func(addr); tick(); // Tick 4
}

void CPU::zero_page_Y_read_addressing(std::function<void (u8)> func) {
	u8 addr = read(program_counter++); tick(); // Tick 2
	addr += register_y; tick(); // Tick 3
	func(read(addr)); tick(); // Tick 4
}

void CPU::zero_page_Y_rmw_addressing(std::function<u8 (u8)> func) {
	u8 addr = read(program_counter++); tick(); // Tick 2
	addr += register_y; tick(); // Tick 3
	u8 val = read(addr); tick(); // Tick 4
	write(addr, val);
	u8 res = func(val); tick(); // Tick 5
}

void CPU::zero_page_Y_write_addressing(std::function<void (u8)> func) {
	u8 addr = read(program_counter++); tick(); // Tick 2
	addr += register_y; tick(); // Tick 3
	func(addr); tick(); // Tick 4
}

void CPU::absolute_X_read_addressing(std::function<void (u8)> func) {
	u16 addr = read(program_counter++); tick(); // Tick 2
	addr |= (read(program_counter++) << 8); tick(); // Tick 3
	u16 newaddr = addr + register_x;
	if (((addr & 0xFF) + register_x) > 0xFF) { tick(); }; // Possible Tick 4
	func(read(newaddr)); tick(); // Tick 4/5
}

void CPU::absolute_X_rmw_addressing(std::function<u8 (u8)> func) {
	u16 addr = read(program_counter++); tick(); // Tick 2
	addr |= (read(program_counter++) << 8); tick(); // Tick 3
	addr += register_x; tick(); // Tick 4
	u8 val = read(addr); tick(); // Tick 5
	write(addr, val); 
	u8 res = func(val); tick(); // Tick 6
	write(addr, res); tick(); //Tick 7
}

void CPU::absolute_X_write_addressing(std::function<void (u16)> func) {
	u16 addr = read(program_counter++); tick(); // Tick 2
	addr |= (read(program_counter++) << 8); tick(); // Tick 3
	addr += register_x; tick(); // Tick 4
	func(addr); tick();
}

void CPU::absolute_Y_read_addressing(std::function<void (u8)> func) {
	u16 addr = read(program_counter++); tick(); // Tick 2
	addr |= (read(program_counter++) << 8); tick(); // Tick 3
	u16 newaddr = addr + register_y;
	if (((addr & 0xFF) + register_y) > 0xFF) { tick(); }; // Possible Tick 4
	func(read(newaddr)); tick(); // Tick 4/5
}

void CPU::absolute_Y_write_addressing(std::function<void (u16)> func) {
	u16 addr = read(program_counter++); tick(); // Tick 2
	addr |= (read(program_counter++) << 8); tick(); // Tick 3
	addr += register_y; tick(); // Tick 4
	func(addr); tick();
}

void CPU::indirect_X_read_addressing(std::function<void (u8)> func) {
	u8 pointer = read(program_counter++); tick(); // Tick 2
	pointer += register_x; tick(); // Tick 3
	u16 addr = read(pointer++); tick(); // Tick 4
	addr |= (read(pointer) << 8); tick(); // Tick 5
	func(read(addr)); tick(); // Tick 6
}

void CPU::indirect_X_rmw_addressing(std::function<u8 (u8)> func) {
	u8 pointer = read(program_counter++); tick(); // Tick 2
	pointer += register_x; tick(); // Tick 3
	u16 addr = read(pointer++); tick(); // Tick 4
	addr |= (read(pointer) << 8); tick(); // Tick 5
	u8 val = read(addr); tick(); // Tick 6
	write(addr, val);
	u8 res = func(val); tick(); // Tick 7
	write(addr, res); // Tick 8
}

void CPU::indirect_X_write_addressing(std::function<void (u16)> func) {
	u8 pointer = read(program_counter++); tick(); // Tick 2
	pointer += register_x; tick(); // Tick 3
	u16 addr = read(pointer++); tick(); // Tick 4
	addr |= (read(pointer) << 8); tick(); // Tick 5
	func(addr); tick(); // Tick 6
}

void CPU::indirect_Y_read_addressing(std::function<void (u8)> func) {
	u8 pointer = read(program_counter++); tick(); // Tick 2
	u16 addr = read(pointer++); tick(); // Tick 3
	addr |= (read(pointer) << 8);
	u16 newaddr = addr + register_y; tick(); // Tick 4
	if (((addr & 0xFF) + register_y) > 0xFF) { tick(); }; // Tick 5
	func(read(newaddr)); tick(); // Tick 6
}

void CPU::indirect_Y_rmw_addressing(std::function<u8 (u8)> func) {
	u8 pointer = read(program_counter++); tick(); // Tick 2
	u16 addr = read(pointer++); tick(); // Tick 3
	addr |= (read(pointer) << 8); tick(); // Tick 4
	u16 newaddr = addr + register_y; tick(); // Tick 5
	u8 val = read(newaddr); tick(); // Tick 6
	write(newaddr, val);
	u8 res = func(val); tick(); // Tick 7
	write(newaddr, res); tick(); // Tick 8
}

void CPU::indirect_Y_write_addressing(std::function<void (u16)> func) {
	u8 pointer = read(program_counter++); tick(); // Tick 2
	u16 addr = read(pointer++); tick(); // Tick 3
	addr |= (read(pointer) << 8); tick(); // Tick 4
	u16 newaddr = addr + register_y; tick(); // Tick 5
	func(newaddr); tick(); // Tick 6

}


// Instructions Accesing Stack

void CPU::BRK() {
	program_counter++; tick(); // Tick 2
	writeStack(stack_pointer--, program_counter >> 8); tick(); // Tick 3
	setBreak(true);
	writeStack(stack_pointer--, program_counter & 0xFF); tick(); // Tick 4
	writeStack(stack_pointer--, status); tick(); // Tick 5
	program_counter = 0;
	program_counter |= read(0xFFFE); tick(); // Tick 6
	program_counter |= (read(0xFFFF) << 8); tick(); // Tick 7
};

void CPU::NMI() {
	tick(); tick(); // Tick 1, 2
	writeStack(stack_pointer--, program_counter >> 8); tick(); // Tick 3
	writeStack(stack_pointer--, program_counter & 0xFF); tick(); // Tick 4
	setBreak(false);
	writeStack(stack_pointer--, status); tick(); // Tick 5
	program_counter = 0;
	program_counter |= read(0xFFFA); setInterrupt(true); tick(); // Tick 6
	program_counter |= (read(0xFFFB) << 8); tick(); // Tick 7
}

void CPU::RTI() { // Not Tested
	tick(); // Tick 2
	stack_pointer++; tick(); // Tick 3
	status = readStack(stack_pointer++); 
	setBreak(false);
	setUnused(true);
	tick(); // Tick 4
	program_counter = 0;
	program_counter |= readStack(stack_pointer++); tick(); // Tick 5
	program_counter |= (readStack(stack_pointer) << 8); tick(); // Tick 6
};

void CPU::RTS() { // Not Tested
	tick(); // Tick 2
	stack_pointer++; tick(); // Tick 3
	program_counter = 0;
	program_counter |= readStack(stack_pointer++); tick(); // Tick 4
	program_counter |= (readStack(stack_pointer) << 8); tick(); // Tick 5
	program_counter++; tick(); // Tick 6
}; 

void CPU::PHA() { 
	tick(); // Tick 2
	writeStack(stack_pointer--, register_a); tick(); // Tick 3
};

void CPU::PHP() { 
	tick(); // Tick 2
	writeStack(stack_pointer--, status | (0b10000)); tick(); // Tick 3
};

void CPU::PLA() {
	tick(); // Tick 2
	stack_pointer++; tick(); // Tick 3
	register_a = readStack(stack_pointer);
	setZero(register_a == 0);
	setNegative((register_a >> 7) & 1);
	tick(); // Tick 4
};

void CPU::PLP() { 
	tick(); // Tick 2
	stack_pointer++; tick(); // Tick 3
	status = readStack(stack_pointer);
	setBreak(false);
	setUnused(true);
	tick(); // Tick 4
};

void CPU::JSR() { 
	const u8 lo { read(program_counter++) }; tick(); // Tick 2
	tick(); // Tick 3
	writeStack(stack_pointer--, program_counter >> 8); tick(); // Tick 4
	writeStack(stack_pointer--, program_counter & 0xFF); tick(); // Tick 5
	const u8 hi = read(program_counter);
	program_counter = 0;
	program_counter |= lo;
	program_counter |= (hi << 8); tick(); // Tick 6
};



// Branch Instructions

void CPU::BCC() {
	relative_addressing(!(status & 1));
};

void CPU::BCS() {
	relative_addressing(status & 1);
};

void CPU::BNE() {
	relative_addressing(!((status >> 1) & 1));
};

void CPU::BEQ() {
	relative_addressing((status >> 1) & 1);
};

void CPU::BPL() {
	relative_addressing(!((status >> 7) & 1));
};

void CPU::BMI() {
	relative_addressing((status >> 7) & 1);
};

void CPU::BVC() {
	relative_addressing(!((status >> 6) & 1));
};

void CPU::BVS() {
	relative_addressing((status >> 6) & 1);
};




// Read Instructions

void CPU::LDA(u8 val) { 
	register_a = val;
	setNegativeAndZero(register_a);
};

void CPU::LDX(u8 val) { 
	register_x = val;
	setNegativeAndZero(register_x);
};

void CPU::LDY(u8 val) { 
	register_y = val;
	setNegativeAndZero(register_y);
};

void CPU::EOR(u8 val) {
	register_a ^= val;
	setNegativeAndZero(register_a);
};

void CPU::AND(u8 val) {
	register_a &= val;
	setNegativeAndZero(register_a);
};

void CPU::ORA(u8 val) {
	register_a |= val;
	setNegativeAndZero(register_a);
};

void CPU::ADC(u8 val) { // Not tested
	u8 carry = status & 1;
	u16 res = val + register_a + carry;
	u8 overflow = (register_a ^ res) & (val ^ res) & 0x80;
	setCarry(res > 0xFF);
	setOverflow(overflow);
	register_a = res;
	setNegativeAndZero(register_a);
};

void CPU::SBC(u8 val) {
	ADC(~val);
};

void CPU::CMP(u8 val) { 
	setNegativeAndZero(register_a - val);
	setCarry(val <= register_a);
};

void CPU::CPX(u8 val) { 
	setNegativeAndZero(register_x - val);
	setCarry(val <= register_x);
};

void CPU::CPY(u8 val) { 
	setNegativeAndZero(register_y - val);
	setCarry(val <= register_y);
};

void CPU::BIT(u8 val) {
	setZero((register_a & val) == 0);
	setOverflow((val >> 6) & 1);
	setNegative((val >> 7) & 1);
};



// RMW Instructions

u8 CPU::ASL(u8 val) { 
	setCarry((val >> 7) == 1);
	val <<= 1;
	setNegativeAndZero(val);
	return val;
};

u8 CPU::LSR(u8 val) { 
	setCarry((val & 1) == 1);
	val >>= 1;
	setNegativeAndZero(val);
	return val;
};

u8 CPU::ROL(u8 val) { 
	u8 bit = (val >> 7);
	val <<= 1;
	val |= (status & 1);
	status &= 0xFE;
	status |= bit;
	setNegativeAndZero(val);
	return val;
};

u8 CPU::ROR(u8 val) { 
	u8 carry = val & 1;
	val >>= 1;
	val |= ((status & 1) << 7);
	status &= 0xFE;
	status |= carry;
	setNegativeAndZero(val);
	return val;
};

u8 CPU::INC(u8 val) {
	val += 1;
	setNegativeAndZero(val);
	return val;
};

u8 CPU::DEC(u8 val) {
	val -= 1;
	setNegativeAndZero(val);
	return val;
};

// Write instructiona

void CPU::STX(u16 addr) {
	write(addr, register_x);
};

void CPU::STY(u16 addr) {
	write(addr, register_y);
};

void CPU::STA(u16 addr) {
	write(addr, register_a);
};


// Implied Instructions

void CPU::CLC() {
	status &= 0b11111110; tick();
}

void CPU::CLD() {
	status &= 0b11110111; tick();
}

void CPU::CLI() {
	status &= 0b11111011; tick();
}

void CPU::CLV() {
	status &= 0b10111111; tick();
}

void CPU::DEX() {
	register_x -= 1;
	setNegativeAndZero(register_x);
	tick();
}

void CPU::DEY() {
	register_y -= 1;
	setNegativeAndZero(register_y);
	tick();
}

void CPU::INX() {
	register_x += 1;
	setNegativeAndZero(register_x);
	tick();
}

void CPU::INY() {
	register_y += 1; 
	setNegativeAndZero(register_y);
	tick();
}

void CPU::NOP() {
	tick();
	return;
}

void CPU::SEC() {
	status |= 1; tick();
}

void CPU::SED() {
	status |= 0b1000; tick();
}

void CPU::SEI() {
	status |= 0b100; tick();
}

void CPU::TAX() {
	register_x = register_a;
	setNegativeAndZero(register_x); 
	tick();
}

void CPU::TAY() {
	register_y = register_a;
	setNegativeAndZero(register_y);
	tick();
}

void CPU::TSX() {
	register_x = stack_pointer;
	setNegativeAndZero(register_x);
	tick();
}

void CPU::TXA() {
	register_a = register_x;
	setNegativeAndZero(register_a);
	tick();
}

void CPU::TXS() {
	stack_pointer = register_x;
	tick();
}

void CPU::TYA() {
	register_a = register_y;
	setNegativeAndZero(register_a);
	tick();
}

// Jump Instructions

void CPU::abs_addressing_JMP() {
	u16 addr = read(program_counter++); tick(); // Tick 2
	addr |= (read(program_counter) << 8);
	program_counter = addr; tick(); // Tick 3
}

void CPU::abs_indirect_addressing_JMP() { // Not Tested
	u16 pointer = read(program_counter++); tick(); // Tick 2
	pointer |= (read(program_counter++) << 8); tick(); // Tick 3
	u8 latch = read(pointer); tick(); // Tick 4
	program_counter = (read((pointer & 0xFF00) | (pointer+1) & 0xFF) << 8) | latch; tick(); // Tick 5
}


void CPU::execute() {

	using namespace std::placeholders;

	if (NMI_enabled) {
		NMI();
		return;
	};

	u8 op_code = read(program_counter++);
	tick();

	switch (op_code)
	{
	case 0x00: 
		BRK(); 
		break;
	case 0x01:
		indirect_X_read_addressing(std::bind(&CPU::ORA, this, _1));
		break;
	case 0x05:
		zero_page_read_addressing(std::bind(&CPU::ORA, this, _1));
		break;
	case 0x06:
		zero_page_rmw_addressing(std::bind(&CPU::ASL, this, _1));
		break;
	case 0x08:
		PHP();
		break;
	case 0x09:
		immediate_addressing(std::bind(&CPU::ORA, this, _1));
		break;
	case 0x0A:
		accumulator_addressing(std::bind(&CPU::ASL, this, _1));
		break;
	case 0x0D:
		absolute_read_addressing(std::bind(&CPU::ORA, this, _1));
		break;
	case 0x0E:
		absolute_rmw_addressing(std::bind(&CPU::ASL, this, _1));
		break;
	case 0x10:
		BPL();
		break;
	case 0x11:
		indirect_Y_read_addressing(std::bind(&CPU::ORA, this, _1));
		break;
	case 0x15:
		zero_page_X_read_addressing(std::bind(&CPU::ORA, this, _1));
		break;
	case 0x16:
		zero_page_X_rmw_addressing(std::bind(&CPU::ASL, this, _1));
		break;
	case 0x18:
		CLC();
		break;
	case 0x19:
		absolute_Y_read_addressing(std::bind(&CPU::ORA, this, _1));
		break;
	case 0x1D:
		absolute_X_read_addressing(std::bind(&CPU::ORA, this, _1));
		break;
	case 0x1E:
		absolute_X_rmw_addressing(std::bind(&CPU::ASL, this, _1));
		break;
	case 0x20:
		JSR();
		break;
	case 0x21:
		indirect_X_read_addressing(std::bind(&CPU::AND, this, _1));
		break;
	case 0x24:
		zero_page_read_addressing(std::bind(&CPU::BIT, this, _1));
		break;
	case 0x25:
		zero_page_read_addressing(std::bind(&CPU::AND, this, _1));
		break;
	case 0x26:
		zero_page_rmw_addressing(std::bind(&CPU::ROL, this, _1));
		break;
	case 0x28:
		PLP();
		break;
	case 0x29:
		immediate_addressing(std::bind(&CPU::AND, this, _1));
		break;
	case 0x2A:
		accumulator_addressing(std::bind(&CPU::ROL, this, _1));
		break;
	case 0x2C:
		absolute_read_addressing(std::bind(&CPU::BIT, this, _1));
		break;
	case 0x2D:
		absolute_read_addressing(std::bind(&CPU::AND, this, _1));
		break;
	case 0x2E:
		absolute_rmw_addressing(std::bind(&CPU::ROL, this, _1));
		break;
	case 0x30:
		BMI();
		break;
	case 0x31:
		indirect_Y_read_addressing(std::bind(&CPU::AND, this, _1));
		break;
	case 0x35:
		zero_page_X_read_addressing(std::bind(&CPU::AND, this, _1));
		break;
	case 0x36:
		zero_page_X_rmw_addressing(std::bind(&CPU::ROL, this, _1));
		break;
	case 0x38:
		SEC();
		break;
	case 0x39:
		absolute_Y_read_addressing(std::bind(&CPU::AND, this, _1));
		break;
	case 0x3D:
		absolute_X_read_addressing(std::bind(&CPU::AND, this, _1));
		break;
	case 0x3E:
		absolute_X_rmw_addressing(std::bind(&CPU::ROL, this, _1));
		break;
	case 0x40:
		RTI();
		break;
	case 0x41:
		indirect_X_read_addressing(std::bind(&CPU::EOR, this, _1));
		break;
	case 0x45:
		zero_page_read_addressing(std::bind(&CPU::EOR, this, _1));
		break;
	case 0x46:
		zero_page_rmw_addressing(std::bind(&CPU::LSR, this, _1));
		break;
	case 0x48:
		PHA();
		break;
	case 0x49:
		immediate_addressing(std::bind(&CPU::EOR, this, _1));
		break;
	case 0x4A:
		accumulator_addressing(std::bind(&CPU::LSR, this, _1));
		break;
	case 0x4C:
		abs_addressing_JMP();
		break;
	case 0x4D:
		absolute_read_addressing(std::bind(&CPU::EOR, this, _1));
		break;
	case 0x4E:
		absolute_rmw_addressing(std::bind(&CPU::LSR, this, _1));
		break;
	case 0x50:
		BVC();
		break;
	case 0x51:
		indirect_Y_read_addressing(std::bind(&CPU::EOR, this, _1));
		break;
	case 0x55:
		zero_page_X_read_addressing(std::bind(&CPU::EOR, this, _1));
		break;
	case 0x56:
		zero_page_X_rmw_addressing(std::bind(&CPU::LSR, this, _1));
		break;
	case 0x58:
		CLI();
		break;
	case 0x59:
		absolute_Y_read_addressing(std::bind(&CPU::EOR, this, _1));
		break;
	case 0x5D:
		absolute_X_read_addressing(std::bind(&CPU::EOR, this, _1));
		break;
	case 0x5E:
		absolute_X_rmw_addressing(std::bind(&CPU::LSR, this, _1));
		break;
	case 0x60:
		RTS();
		break;
	case 0x61:
		indirect_X_read_addressing(std::bind(&CPU::ADC, this, _1));
		break;
	case 0x65:
		zero_page_read_addressing(std::bind(&CPU::ADC, this, _1));
		break;
	case 0x66:
		zero_page_rmw_addressing(std::bind(&CPU::ROR, this, _1));
		break;
	case 0x68:
		PLA();
		break;
	case 0x69:
		immediate_addressing(std::bind(&CPU::ADC, this, _1));
		break;
	case 0x6A:
		accumulator_addressing(std::bind(&CPU::ROR, this, _1));
		break;
	case 0x6C:
		abs_indirect_addressing_JMP();
		break;
	case 0x6D:
		absolute_read_addressing(std::bind(&CPU::ADC, this, _1));
		break;
	case 0x6E:
		absolute_rmw_addressing(std::bind(&CPU::ROR, this, _1));
		break;
	case 0x70:
		BVS();
		break;
	case 0x71:
		indirect_Y_read_addressing(std::bind(&CPU::ADC, this, _1));
		break;
	case 0x75:
		zero_page_X_read_addressing(std::bind(&CPU::ADC, this, _1));
		break;
	case 0x76:
		zero_page_X_rmw_addressing(std::bind(&CPU::ROR, this, _1));
		break;
	case 0x78:
		SEI();
		break;
	case 0x79:
		absolute_Y_read_addressing(std::bind(&CPU::ADC, this, _1));
		break;
	case 0x7D:
		absolute_X_read_addressing(std::bind(&CPU::ADC, this, _1));
		break;
	case 0x7E:
		absolute_X_rmw_addressing(std::bind(&CPU::ROR, this, _1));
		break;
	case 0x81:
		indirect_X_write_addressing(std::bind(&CPU::STA, this, _1));
		break;
	case 0x84:
		zero_page_write_addressing(std::bind(&CPU::STY, this, _1));
		break;
	case 0x85:
		zero_page_write_addressing(std::bind(&CPU::STA, this, _1));
		break;
	case 0x86:
		zero_page_write_addressing(std::bind(&CPU::STX, this, _1));
		break;
	case 0x88:
		DEY();
		break;
	case 0x8A:
		TXA();
		break;
	case 0x8C:
		absolute_write_addressing(std::bind(&CPU::STY, this, _1));
		break;
	case 0x8D:
		absolute_write_addressing(std::bind(&CPU::STA, this, _1));
		break;
	case 0x8E:
		absolute_write_addressing(std::bind(&CPU::STX, this, _1));
		break;
	case 0x90:
		BCC();
		break;
	case 0x91:
		indirect_Y_write_addressing(std::bind(&CPU::STA, this, _1));
		break;
	case 0x94:
		zero_page_X_write_addressing(std::bind(&CPU::STY, this, _1));
		break;
	case 0x95:
		zero_page_X_write_addressing(std::bind(&CPU::STA, this, _1));
		break;
	case 0x96:
		zero_page_Y_write_addressing(std::bind(&CPU::STX, this, _1));
		break;
	case 0x98:
		TYA();
		break;
	case 0x99:
		absolute_Y_write_addressing(std::bind(&CPU::STA, this, _1));
		break;
	case 0x9A:
		TXS();
		break;
	case 0x9D:
		absolute_X_write_addressing(std::bind(&CPU::STA, this, _1));
		break;
	case 0xA0:
		immediate_addressing(std::bind(&CPU::LDY, this, _1));
		break;
	case 0xA1:
		indirect_X_read_addressing(std::bind(&CPU::LDA, this, _1));
		break;
	case 0xA2:
		immediate_addressing(std::bind(&CPU::LDX, this, _1));
		break;
	case 0xA4:
		zero_page_read_addressing(std::bind(&CPU::LDY, this, _1));
		break;
	case 0xA5:
		zero_page_read_addressing(std::bind(&CPU::LDA, this, _1));
		break;
	case 0xA6:
		zero_page_read_addressing(std::bind(&CPU::LDX, this, _1));
		break;
	case 0xA8:
		TAY();
		break;
	case 0xA9:
		immediate_addressing(std::bind(&CPU::LDA, this, _1));
		break;
	case 0xAA:
		TAX();
		break;
	case 0xAC:
		absolute_read_addressing(std::bind(&CPU::LDY, this, _1));
		break;
	case 0xAD:
		absolute_read_addressing(std::bind(&CPU::LDA, this, _1));
		break;
	case 0xAE:
		absolute_read_addressing(std::bind(&CPU::LDX, this, _1));
		break;
	case 0xB0:
		BCS();
		break;
	case 0xB1:
		indirect_Y_read_addressing(std::bind(&CPU::LDA, this, _1));
		break;
	case 0xB4:
		zero_page_X_read_addressing(std::bind(&CPU::LDY, this, _1));
		break;
	case 0xB5:
		zero_page_X_read_addressing(std::bind(&CPU::LDA, this, _1));
		break;
	case 0xB6:
		zero_page_Y_read_addressing(std::bind(&CPU::LDX, this, _1));
		break;
	case 0xB8:
		CLV();
		break;
	case 0xB9:
		absolute_Y_read_addressing(std::bind(&CPU::LDA, this, _1));
		break;
	case 0xBA:
		TSX();
		break;
	case 0xBC:
		absolute_X_read_addressing(std::bind(&CPU::LDY, this, _1));
		break;
	case 0xBD:
		absolute_X_read_addressing(std::bind(&CPU::LDA, this, _1));
		break;
	case 0xBE:
		absolute_Y_read_addressing(std::bind(&CPU::LDX, this, _1));
		break;
	case 0xC0:
		immediate_addressing(std::bind(&CPU::CPY, this, _1));
		break;
	case 0xC1:
		indirect_X_read_addressing(std::bind(&CPU::CMP, this, _1));
		break;
	case 0xC4:
		zero_page_read_addressing(std::bind(&CPU::CPY, this, _1));
		break;
	case 0xC5:
		zero_page_read_addressing(std::bind(&CPU::CMP, this, _1));
		break;
	case 0xC6:
		zero_page_rmw_addressing(std::bind(&CPU::DEC, this, _1));
		break;
	case 0xC8:
		INY();
		break;
	case 0xC9:
		immediate_addressing(std::bind(&CPU::CMP, this, _1));
		break;
	case 0xCA:
		DEX();
		break;
	case 0xCC:
		absolute_read_addressing(std::bind(&CPU::CPY, this, _1));
		break;
	case 0xCD:
		absolute_read_addressing(std::bind(&CPU::CMP, this, _1));
		break;
	case 0xCE:
		absolute_rmw_addressing(std::bind(&CPU::DEC, this, _1));
		break;
	case 0xD0:
		BNE();
		break;
	case 0xD1:
		indirect_Y_read_addressing(std::bind(&CPU::CMP, this, _1));
		break;
	case 0xD5:
		zero_page_X_read_addressing(std::bind(&CPU::CMP, this, _1));
		break;
	case 0xD6:
		zero_page_X_rmw_addressing(std::bind(&CPU::DEC, this, _1));
		break;
	case 0xD8:
		CLD();
		break;
	case 0xD9:
		absolute_Y_read_addressing(std::bind(&CPU::CMP, this, _1));
		break;
	case 0xDD:
		absolute_X_read_addressing(std::bind(&CPU::CMP, this, _1));
		break;
	case 0xDE:
		absolute_X_rmw_addressing(std::bind(&CPU::DEC, this, _1));
		break;
	case 0xE0:
		immediate_addressing(std::bind(&CPU::CPX, this, _1));
		break;
	case 0xE1:
		indirect_X_read_addressing(std::bind(&CPU::SBC, this, _1));
		break;
	case 0xE4:
		zero_page_read_addressing(std::bind(&CPU::CPX, this, _1));
		break;
	case 0xE5:
		zero_page_read_addressing(std::bind(&CPU::SBC, this, _1));
		break;
	case 0xE6:
		zero_page_rmw_addressing(std::bind(&CPU::INC, this, _1));
		break;
	case 0xE8:
		INX();
		break;
	case 0xE9:
		immediate_addressing(std::bind(&CPU::SBC, this, _1));
		break;
	case 0xEA:
		NOP();
		break;
	case 0xEC:
		absolute_read_addressing(std::bind(&CPU::CPX, this, _1));
		break;
	case 0xED:
		absolute_read_addressing(std::bind(&CPU::SBC, this, _1));
		break;
	case 0xEE:
		absolute_rmw_addressing(std::bind(&CPU::INC, this, _1));
		break;
	case 0xF0:
		BEQ();
		break;
	case 0xF1:
		indirect_Y_read_addressing(std::bind(&CPU::SBC, this, _1));
		break;
	case 0xF9:
		absolute_Y_read_addressing(std::bind(&CPU::SBC, this, _1));
		break;
	case 0xF5:
		zero_page_X_read_addressing(std::bind(&CPU::SBC, this, _1));
		break;
	case 0xF6:
		zero_page_X_rmw_addressing(std::bind(&CPU::INC, this, _1));
		break;
	case 0xF8:
		SED();
		break;
	case 0xFD:
		absolute_X_read_addressing(std::bind(&CPU::SBC, this, _1));
		break;
	case 0xFE:
		absolute_X_rmw_addressing(std::bind(&CPU::INC, this, _1));
		break;
	default:
		return;
	}
};