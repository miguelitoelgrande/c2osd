// console.h
// Source:  http://www.williamwilling.com/blog/?p=74   [Posted September 30th, 2007 by William Willing]
#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include <fstream>

class console
{
private:
  std::ofstream m_out;
  std::ofstream m_err;
  std::ifstream m_in;

  std::streambuf* m_old_cout;
  std::streambuf* m_old_cerr;
  std::streambuf* m_old_cin;

public:
  console();
  ~console();
};

#endif
