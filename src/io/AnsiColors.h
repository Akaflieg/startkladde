/*
 * AnsiColors.h
 *
 *  Created on: 27.02.2010
 *      Author: Martin Herrmann
 */

#ifndef ANSICOLORS_H_
#define ANSICOLORS_H_

#include <QString>
#include <QStringList>

/*
 * TODO:
 *   - must be able to store colors: AnsiColors errorColor=...
 *   - there should be a way to have a color output only if it changed
 */

/**
 * An ANSI color escape code generator
 *
 * Use:
 *   AnsiColors c;
 *   QString s=QString ("%1foo%2").arg (c.bold().red().on().yellow(), c.reset());
 *
 * This class was written with focus on usability rather than speed.
 */
class AnsiColors
{
	public:
		// *** Constants
		static const QString escapePrefix;
		static const QString escapeSuffix;

		static const int foreground=30;
		static const int background=40;

		static const int valueReset=0;

		static const int colorBlack  =0;
		static const int colorRed    =1;
		static const int colorGreen  =2;
		static const int colorYellow =3;
		static const int colorBlue   =4;
		static const int colorMagenta=5;
		static const int colorCyan   =6;
		static const int colorWhite  =7;
		static const int colorNone   =9;

		static const int attributeBold       =1;
		static const int attributeUnderline  =4;
		static const int attributeBlink      =5;
		static const int attributeInverse    =7;
		static const int attributeNoBold     =22;
		static const int attributeNoUnderline=24;
		static const int attributeNoBlink    =25;
		static const int attributeNoInverse  =27;


		// *** Construction
		AnsiColors ();

		// *** Formatting
		operator QString () const;

		// *** Values
		AnsiColors val (int v);
		AnsiColors on ();
		AnsiColors reset ();
		AnsiColors black  ();
		AnsiColors red    ();
		AnsiColors green  ();
		AnsiColors yellow ();
		AnsiColors blue   ();
		AnsiColors magenta();
		AnsiColors cyan   ();
		AnsiColors white  ();
		AnsiColors none   ();

		AnsiColors bold      ();
		AnsiColors underline ();
		AnsiColors blink     ();
		AnsiColors inverse   ();

		AnsiColors noBold      ();
		AnsiColors noUnderline ();
		AnsiColors noBlink     ();
		AnsiColors noInverse   ();


		// *** Properties
		QStringList values;
		int currentOffset;
};

#endif
