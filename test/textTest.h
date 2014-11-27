#ifndef TEST_textTest
#define TEST_textTest
#include "cppunit/extensions/HelperMacros.h"

class textTest : public CppUnit::TestFixture {
public:
	void testeintrag_ist_leer();
	void testtrim();
	CPPUNIT_TEST_SUITE(textTest);
	CPPUNIT_TEST(testeintrag_ist_leer);
	CPPUNIT_TEST(testtrim);
	CPPUNIT_TEST_SUITE_END();
};
#endif
