#include "textTest.h"

#include "src/text.h"

CPPUNIT_TEST_SUITE_REGISTRATION(textTest);


void textTest::testeintrag_ist_leer() {
	CPPUNIT_ASSERT(eintrag_ist_leer (""));
	CPPUNIT_ASSERT(eintrag_ist_leer ("-"));
	CPPUNIT_ASSERT(eintrag_ist_leer (" "));
	CPPUNIT_ASSERT(!eintrag_ist_leer ("Test"));
	CPPUNIT_ASSERT(!eintrag_ist_leer ("Eimer"));
	CPPUNIT_ASSERT(!eintrag_ist_leer ("Hallo"));
}

void textTest::testtrim() {
	CPPUNIT_ASSERT(true);
}

