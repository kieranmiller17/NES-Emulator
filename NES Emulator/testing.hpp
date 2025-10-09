#ifndef TEST_H
#define TEST_H

#include <vector>
#include "typedef.hpp"


namespace Test
{
	void testAllCodes();

	std::vector<u8> createTestRom(std::vector<u8>&&);

	void testBranchCycles();

	void test_BRK();
	void test_PHA();
	void test_PHP();
	void test_PLA();
	void test_PLP();
	void test_JSR();
	void test_BCC();
	void test_BCS();

	void test_ORA();
	void test_AND();
	void test_EOR();
	void test_ADC();
	void test_LDY();
	void test_LDX();
	void test_LDA();
	void test_CPY();
	void test_CMP();
	void test_BIT();

	void test_ASL();
	void test_LSR();
	void test_ROL();
	void test_ROR();

	void test_JMP_1();

};

#endif