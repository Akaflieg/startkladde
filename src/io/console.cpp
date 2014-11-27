//#include "console.h"
//
//#include <iostream>
//
//#include <unistd.h>
//#include <termios.h>
//
//#include "src/util/qString.h"
//
//QString read_password ()
//	/*
//	 * Reads a QString (up to \n) without echo when on a terminal.
//	 */
//{
//	// Find out if we are reading from a terminal.
//	bool tty=(isatty (STDIN_FILENO)==1);
//	struct termios term_org;
//
//	if (tty)
//	{
//		// Save the current terminal state
//		tcgetattr (STDIN_FILENO, &term_org);
//
//		// Set the terminal state to no echo
//		struct termios term_new=term_org;
//		term_new.c_lflag&=~ECHO;
//		tcsetattr (STDIN_FILENO, TCSANOW, &term_new);
//	}
//
//	// Read the text
//	std::string r;
//	getline (std::cin, r);
////	cin >> r;
//
//	if (tty)
//	{
//		// Reset the terminal to its original state.
//		tcsetattr (STDIN_FILENO, TCSANOW, &term_org);
//
//		// Output the endl that was lost during input.
//		std::cout << std::endl;
//	}
//
//	return std2q (r);
//}
//
//

