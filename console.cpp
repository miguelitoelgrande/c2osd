
// console.cpp
// Source:  http://www.williamwilling.com/blog/?p=74   [Posted September 30th, 2007 by William Willing]

#include "console.h"
#include <cstdio>
#include <iostream>
#include <windows.h>

console::console()
{
  // create a console window
  AllocConsole();

  //MM:
  	SetConsoleTitle("C2 OSD Console");
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED);

  // redirect std::cerr to our console window
  m_old_cerr = std::cerr.rdbuf();
  m_err.open("CONOUT$");
  std::cerr.rdbuf(m_err.rdbuf());

  // redirect std::cout to our console window
  m_old_cout = std::cout.rdbuf();
  m_out.open("CONOUT$");
  std::cout.rdbuf(m_out.rdbuf());

  // redirect std::cin to our console window
  m_old_cin = std::cin.rdbuf();
  m_in.open("CONIN$");
  std::cin.rdbuf(m_in.rdbuf());

  std::cout << "Console here\n";
}

console::~console()
{
  // reset the standard streams
  std::cin.rdbuf(m_old_cin);
  std::cerr.rdbuf(m_old_cerr);
  std::cout.rdbuf(m_old_cout);

  // remove the console window
  FreeConsole();
}