/*
 * AnsiColors.cpp
 *
 *  Created on: 27.02.2010
 *      Author: Martin Herrmann
 */

#include "AnsiColors.h"
#include "src/i18n/notr.h"

const QString AnsiColors::escapePrefix=notr ("\033[");
const QString AnsiColors::escapeSuffix=notr ("m");

AnsiColors::AnsiColors ():
	currentOffset (foreground)
{
}

AnsiColors::operator QString () const
{
	if (values.empty())
		return escapePrefix+QString::number (valueReset)+escapeSuffix;
	else
		return escapePrefix+values.join (notr (";"))+escapeSuffix;
}

AnsiColors AnsiColors::val (int v)
{
	AnsiColors result (*this);
	result.values+=QString::number (v);
	return result;
}

AnsiColors AnsiColors::on ()
{
	AnsiColors result (*this);
	result.currentOffset=background;
	return result;
}

AnsiColors AnsiColors::reset ()
{
	return AnsiColors ();
}

AnsiColors AnsiColors::black   () { return val (colorBlack  +currentOffset); }
AnsiColors AnsiColors::red     () { return val (colorRed    +currentOffset); }
AnsiColors AnsiColors::green   () { return val (colorGreen  +currentOffset); }
AnsiColors AnsiColors::yellow  () { return val (colorYellow +currentOffset); }
AnsiColors AnsiColors::blue    () { return val (colorBlue   +currentOffset); }
AnsiColors AnsiColors::magenta () { return val (colorMagenta+currentOffset); }
AnsiColors AnsiColors::cyan    () { return val (colorCyan   +currentOffset); }
AnsiColors AnsiColors::white   () { return val (colorWhite  +currentOffset); }
AnsiColors AnsiColors::none    () { return val (colorNone   +currentOffset); }

AnsiColors AnsiColors::bold      () { return val (attributeBold     ); }
AnsiColors AnsiColors::underline () { return val (attributeUnderline); }
AnsiColors AnsiColors::blink     () { return val (attributeBlink    ); }
AnsiColors AnsiColors::inverse   () { return val (attributeInverse  ); }

AnsiColors AnsiColors::noBold      () { return val (attributeNoBold     ); }
AnsiColors AnsiColors::noUnderline () { return val (attributeNoUnderline); }
AnsiColors AnsiColors::noBlink     () { return val (attributeNoBlink    ); }
AnsiColors AnsiColors::noInverse   () { return val (attributeNoInverse  ); }
