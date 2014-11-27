#ifndef COLORS_H_
#define COLORS_H_

#include "src/i18n/notr.h"

// TODO remove

#define c_1            notr ("\033[")
#define c_0            notr ("m")
#define	clr_default    notr ("0")
#define at_bold        notr ("1")
#define at_underline   notr ("4")
#define at_blink       notr ("5")
#define at_inverse     notr ("7")
#define at_nobold      notr ("22")
#define at_nounderline notr ("24")
#define at_noblink     notr ("25")
#define at_noinverse   notr ("27")

#define fg_black       notr ("30")
#define fg_red         notr ("31")
#define fg_greed       notr ("32")
#define fg_yellow      notr ("33")
#define fg_blue        notr ("34")
#define fg_magenta     notr ("35")
#define fg_cyan        notr ("36")
#define fg_white       notr ("37")
#define fg_default     notr ("39")

#define bg_black       notr ("40")
#define bg_red         notr ("41")
#define bg_greed       notr ("42")
#define bg_yellow      notr ("43")
#define bg_blue        notr ("44")
#define bg_magenta     notr ("45")
#define bg_cyan        notr ("46")
#define bg_white       notr ("47")
#define bg_default     notr ("49")

#define c_default c_1 clr_default c_0
#define c_error c_1 fg_red notr (";") at_bold c_0
#define c_message c_1 fg_yellow c_0
#define c_funclog c_1 fg_black notr (";") at_bold c_0
#define c_dbase c_1 fg_blue c_0


#endif

