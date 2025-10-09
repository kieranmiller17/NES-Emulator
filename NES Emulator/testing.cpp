#include "testing.hpp"
#include <iostream>
#include "CPU.hpp"
#include <vector>
#include <bitset>

void Test::testAllCodes() {

	test_BRK();
	test_PHA();
	test_PHP();
	test_PLA();
	test_PLP();
	test_JSR();
	test_BCC();
	test_BCS();
	test_ORA();
	test_AND();
	test_EOR();
	test_LDY();
	test_LDX();
	test_LDA();
	test_CPY();
	test_CMP();
	test_BIT();
	test_ASL();
	test_LSR();
	test_ROL();
	test_ROR();
	test_JMP_1();
};

std::vector<u8> Test::createTestRom(std::vector<u8>&& prg_rom)
{
	std::vector<u8> header { 0x4E, 0x45, 0x53, 0x1A, 0x02, 0x01, 0x31, 00, 00, 00, 00, 00, 00, 00, 00, 00 };
	prg_rom.resize(32768);
	prg_rom[0x7FFC] = (0x8000 & 0xFF); // initialise program_counter at 0x8000 (lower 8 bits)
	prg_rom[0x7FFD] = (0x8000 >> 8); // initialise program_counter at 0x8000 (upper 8 bits)
	prg_rom[0x7FFE] = (0x6000 & 0xFF); // arbitrary location for testing (lower 8 bits)
	prg_rom[0x7FFF] = (0x6000 >> 8); // arbitrary location for testing (upper 8 bits)
	std::vector<u8> chr_rom(8192);

	header.insert(header.end(), prg_rom.begin(), prg_rom.end());
	header.insert(header.end(), chr_rom.begin(), chr_rom.end());

	return header;
};

void Test::testBranchCycles() {
	CPU cpu {};
	cpu.reset();
	cpu.status = 0b00000001;
	cpu.program_counter = 0b11111000;
	cpu.write(cpu.program_counter, 0x7);
	cpu.tick(); // Normally called by execute function
	cpu.BCS();
	assert(cpu.cycles == 4);

	cpu.reset();
	cpu.status = 0b00000001;
	cpu.program_counter = 0b11111000;
	cpu.write(cpu.program_counter, 0x06);
	cpu.tick();  // Normally called by execute function
	cpu.BCS();
	assert(cpu.cycles == 3);

	cpu.reset();
	cpu.status = 0;
	cpu.program_counter = 0b11111000;
	cpu.write(cpu.program_counter, 0x02);
	cpu.tick();  // Normally called by execute function
	cpu.BCS();
	assert(cpu.cycles == 2);

	std::cout << "Branch Cycles OK!\n";

}

void Test::test_BRK() {

	std::vector<u8> test_rom = Test::createTestRom({ 0x00 });

	CPU cpu {};
	cpu.load(test_rom);
	cpu.reset();
	cpu.execute();

	assert(cpu.status == cpu.read(cpu.stack_pointer + 1)); // status to stack
	assert(cpu.status & 0b10000); // break flag set

	u8 test_counter = cpu.read(cpu.stack_pointer + 2);
	u8 high = cpu.read(cpu.stack_pointer + 3);

	assert((test_counter | (high << 8)) == 0x8002); // program_counter stored in stack
	assert(cpu.program_counter == 0x6000); // new program counter correctly collected
	std::cout << "BRK Passed!\n";

};

void Test::test_PHA() {

	std::vector<u8> test_rom = Test::createTestRom({ 0x48 });

	CPU cpu {};
	cpu.load(test_rom);
	cpu.reset();
	cpu.register_a = 0b10101111; // Set register_a non-zero for testing (zero is base value of stack elements)

	cpu.execute();
	assert(cpu.read(cpu.stack_pointer + 1) == cpu.register_a);
	std::cout << "PHA Passed!\n";

};

void Test::test_PHP() {

	std::vector<u8> test_rom = Test::createTestRom({ 0x08 });

	CPU cpu {};
	cpu.load(test_rom);
	cpu.reset();
	cpu.status = 0b11101100; // Set register_a non-zero for testing (zero is base value of stack elements)

	cpu.execute();
	assert(cpu.read(cpu.stack_pointer + 1) == cpu.status);
	std::cout << "PHP Passed!\n";

};

void Test::test_PLA() {

	std::vector<u8> test_rom = Test::createTestRom({ 0x68, 0x68 });

	CPU cpu {};
	cpu.load(test_rom);
	cpu.reset();
	cpu.write(cpu.stack_pointer, 0b10000000);
	cpu.stack_pointer--;

	cpu.execute();
	assert(cpu.register_a == 0b10000000);
	assert(cpu.status & 0b10000000);
	assert(!(cpu.status & 0b10));

	cpu.write(cpu.stack_pointer, 0);
	cpu.stack_pointer--;

	cpu.execute();
	assert(cpu.status & 0b10);
	assert(!(cpu.status & 0b10000000));
	std::cout << "PLA Passed!\n";

};

void Test::test_PLP() {

	std::vector<u8> test_rom = Test::createTestRom({ 0x28 });

	CPU cpu {};
	cpu.load(test_rom);
	cpu.reset();
	cpu.write(cpu.stack_pointer, 0b11001101);
	cpu.stack_pointer--;

	cpu.execute();
	assert(cpu.status == 0b11001101);
	std::cout << "PLP Passed!\n";
};

void Test::test_JSR() {

	std::vector<u8> test_rom = Test::createTestRom({ 0x20, 0x04, 0x60 });

	CPU cpu {};
	cpu.load(test_rom);
	cpu.reset();
	cpu.execute();

	assert(cpu.read(cpu.stack_pointer + 1) == 0x02);
	assert(cpu.read(cpu.stack_pointer + 2) == 0x80);
	assert(cpu.program_counter == 0x6004);

	std::cout << "JSR Passed!\n";

};

void Test::test_BCC() {
	std::vector<u8> test_rom = Test::createTestRom({ 0x90, 0xF0, 0x90, 0x71, 0x90, 0xE3 });

	CPU cpu {};
	cpu.load(test_rom);
	cpu.reset();
	cpu.execute();

	assert(cpu.program_counter == 0x7FF2);

	cpu.program_counter = 0x8002;
	cpu.execute();
	assert(cpu.program_counter == 0x8075);
	
	cpu.program_counter = 0x8004;
	cpu.status = 0b00000001;
	cpu.execute();
	assert(cpu.program_counter = 0x8004);

	std::cout << "BCC Passed!\n";
};

void Test::test_BCS() {
	std::vector<u8> test_rom = Test::createTestRom({ 0xB0, 0xF0, 0xB0, 0x71, 0xB0, 0xE3 });

	CPU cpu {};
	cpu.load(test_rom);
	cpu.reset();
	cpu.status = 0b00000001;
	cpu.execute();

	assert(cpu.program_counter == 0x7FF2);

	cpu.program_counter = 0x8002;
	cpu.execute();
	assert(cpu.program_counter == 0x8075);

	cpu.program_counter = 0x8004;
	cpu.status = 0;
	cpu.execute();
	assert(cpu.program_counter = 0x8004);

	std::cout << "BCS Passed!\n";
};

void Test::test_ORA() {
	std::vector<u8> test_rom = Test::createTestRom({ 0x09, 0b11000011, 0x09, 0x00 });

	CPU cpu {};
	cpu.load(test_rom);
	cpu.reset();
	cpu.status = 0x0F;
	cpu.register_a = 0x0F;
	cpu.execute();

	assert(cpu.register_a == 0b11001111);

	std::cout << "ORA Passed!\n";
}

void Test::test_AND() {
	std::vector<u8> test_rom = Test::createTestRom({ 0x29,  0b11001100});

	CPU cpu {};
	cpu.load(test_rom);
	cpu.reset();
	cpu.register_a = 0x0F;
	cpu.execute();

	assert(cpu.register_a == 0b00001100);

	std::cout << "AND Passed!\n";
}

void Test::test_EOR() {
	std::vector<u8> test_rom = Test::createTestRom({ 0x49,  0b11001100 });

	CPU cpu {};
	cpu.load(test_rom);
	cpu.reset();
	cpu.register_a = 0x0F;
	cpu.execute();

	assert(cpu.register_a == 0b11000011);

	std::cout << "EOR Passed!\n";
}


void Test::test_ADC() {
}

void Test::test_LDY() {
	std::vector<u8> test_rom = Test::createTestRom({ 0xA0,  0x53 });

	CPU cpu {};
	cpu.load(test_rom);
	cpu.reset();
	cpu.execute();

	assert(cpu.register_y == 0x53);

	std::cout << "LDY Passed!\n";
}


void Test::test_LDX() {
	std::vector<u8> test_rom = Test::createTestRom({ 0xA2,  0x53 });

	CPU cpu {};
	cpu.load(test_rom);
	cpu.reset();
	cpu.execute();

	assert(cpu.register_x == 0x53);

	std::cout << "LDX Passed!\n";
}


void Test::test_LDA() {
	std::vector<u8> test_rom = Test::createTestRom({ 0xA9,  0x53 });

	CPU cpu {};
	cpu.load(test_rom);
	cpu.reset();
	cpu.execute();

	assert(cpu.register_a == 0x53);

	std::cout << "LDA Passed!\n";
}


void Test::test_CPY() {
	std::vector<u8> test_rom = Test::createTestRom({ 0xC0,  19, 0xC0, 20, 0XC0, 21 });

	CPU cpu {};
	cpu.load(test_rom);
	cpu.reset();
	cpu.status = 0;
	cpu.register_y = 20;

	cpu.execute();
	assert(cpu.status == 1);

	cpu.status = 0;
	cpu.execute();
	assert(cpu.status == 0b11);

	cpu.status = 0;
	cpu.execute();
	assert(cpu.status == 0b10000000);

	std::cout << "CPY Passed!\n";
}

void Test::test_CMP() {
	std::vector<u8> test_rom = Test::createTestRom({ 0xC9,  19, 0xC9, 20, 0XC9, 21, 0xC9, 0x00 });

	CPU cpu {};
	cpu.load(test_rom);
	cpu.reset();
	cpu.status = 0;
	cpu.register_a = 20;

	cpu.execute();
	assert(cpu.status == 1);

	cpu.status = 0;
	cpu.execute();
	assert(cpu.status == 0b11);

	cpu.status = 0;
	cpu.execute();
	assert(cpu.status == 0b10000000);

	cpu.register_a = 250;
	cpu.status = 0;
	cpu.execute();
	assert(cpu.status == 0b10000001);

	std::cout << "CMP Passed!\n";
}

void Test::test_BIT() {
	std::vector<u8> test_rom = Test::createTestRom({ 0x2C, 0xF0, 0x2C, 0x0F });

	CPU cpu {};
	cpu.load(test_rom);
	cpu.reset();
	cpu.register_a = 0x0F;
	cpu.BIT(0xf0);
	assert(cpu.status & 0b10);
	assert(cpu.status & 0b10000000);
	assert(cpu.status & 0b01000000);

	cpu.BIT(0x0f);
	assert((cpu.status & 0b10) == 0);
	assert(!(cpu.status & 0b10000000));
	assert(!(cpu.status & 0b01000000));

	std::cout << "BIT Passed!\n";
}


void Test::test_ASL() {

	CPU cpu {};
	cpu.reset();

	u8 val = 0b11000101;
	u8 res = cpu.ASL(val);
	assert(res == 0b10001010);
	assert(cpu.status & 1);

	std::cout << "ASL Passed!\n";
}

void Test::test_LSR() {

	CPU cpu {};
	cpu.reset();

	u8 val = 0b11000101;
	u8 res = cpu.LSR(val);
	assert(res == 0b01100010);
	assert(cpu.status & 1);

	val = 0b11111110;
	res = cpu.LSR(val);
	assert(!(cpu.status & 1));

	std::cout << "LSR Passed!\n";
}


void Test::test_ROL() {

	CPU cpu {};
	cpu.reset();

	u8 val = 0b10101001;
	u8 res = cpu.ROL(val);
	assert(res == 0b01010010);
	assert(cpu.status & 1);
	res = cpu.ROL(res);
	assert(res == 0b10100101);
	assert(!(cpu.status & 1));
	std::cout << "ROL Passed!\n";
}


void Test::test_ROR() {

	CPU cpu {};
	cpu.reset();
	u8 val = 0b10101001;
	u8 res = cpu.ROR(val);
	assert(res == 0b01010100);
	assert(cpu.status & 1);
	res = cpu.ROR(res);
	assert(res == 0b10101010);
	assert(!(cpu.status & 1));
	std::cout << "ROR Passed\n";

}

void Test::test_JMP_1() {
	std::vector<u8> test_rom = Test::createTestRom({ 0x4C, 0x27, 0xDF });

	CPU cpu {};
	cpu.load(test_rom);
	cpu.reset();
	cpu.execute();
	assert(cpu.program_counter == 0xDF27);

	std::cout << "JMP 1 Passed!\n";
}
