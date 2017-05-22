#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <unistd.h>
#include "mooshimeter.h"

// Program configures the mooshimeter (channel 1: voltage 600V, channel 2:
// current 10A), computes and prints values of arithmetic mean, root square
// mean, real power for 15 seconds.

//---------------------------------------------------------------------------
// Function can calculate arithmetic and root square means. f has to be a
// function that returns the value of i-th sample. g has to be a function
// calculating the mean value from the sum.
//
// Program configures the mooshimeter to measure s (256) samples with rate
// 8000 samples/sec. Frequency of alternating current is equal to 50 Hz in my
// country and corresponds to n (160) samples.
template <class F, class G>
double period_avg(F f, G g, int n, int s)
{
  double s1 = 0., s2 = 0.;
  for (int i=0; i<n; ++i)
  {
    s1 += f(i);
  }
  s2 += g(s1, n);
  for (int i=0; i<s-n; ++i)
  {
    s1 -= f(i);
    s1 += f(i+n);
    s2 += g(s1, n);
  }
  s2 /= s-n+1;
  return s2;
}
//---------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const Measurement::Time& t)
{
  tm* ptm = localtime(&t.s);
  char buf[64];
  sprintf(buf, "%04d-%02d-%02d %2d:%02d:%02d.%03d",
    ptm->tm_year+1900, ptm->tm_mon, ptm->tm_mday,
    ptm->tm_hour, ptm->tm_min, ptm->tm_sec, t.ms);
  os << buf;
  return os;
}
//---------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os,
                         const Measurement::Channel& c)
{
  if (c.buf.size())
  {
    constexpr int N = 160;
    os
      << std::setw(9)
      // arithmetic mean
      << period_avg(
        [&](int i){ return c.buf[i]; },
        [](double s, int n){ return s/n; },
        N, c.buf.size())
      << std::setw(9)
      // root mean square
      << period_avg(
        [&](int i){ return pow(c.buf[i], 2); },
        [](double s, int n){ return sqrt(s/n); },
        N, c.buf.size());
  }
  else
  {
    os << std::setw(9) << c.value;
  }
  return os;
}
//---------------------------------------------------------------------------
int main()
{
  try
  {
    double bat_v = NAN;
    bool header = true;

    // Look into mooshimeter.h for explanations of constructor.
    Mooshimeter m("88:4A:EA:8A:0B:0D", 0x0015, 0x0012,
      //---------------------------------------------------------------------
      [&](const Measurement& m)
      {
        constexpr int N = 160;
        std::ostringstream os;
        if (header)
        {
          header = false;
          os
            << "   DATE        TIME             I [A]              U [V]          POWER     V_BAT"
            << std::endl
            << "                            MEAN      RMS      MEAN      RMS"
            << std::endl;
        }
        os
          << std::fixed << std::setprecision(3)
          << m.t
          << " " << m.ch1
          << " " << m.ch2
          << " " << std::setw(9)
          // real power
          << period_avg(
            [&](int i){ return m.ch1.buf[i] * m.ch2.buf[i]; },
            [](double s, int n){ return s/n; },
            N, m.ch1.buf.size())
          << " " << std::setw(9) << bat_v
          << std::endl;
        {
          StdOutExclusiveAccess guard;
          std::cout << os.str();
        }
      },
      //---------------------------------------------------------------------
      [&](const Response& r)
      {
        if (r.name=="BAT_V")
        {
          bat_v = std::stod(r.value);
        }
      });
      //---------------------------------------------------------------------

    m.cmd("CH1:MAPPING CURRENT");
    m.cmd("CH1:RANGE_I 10");
    m.cmd("CH2:MAPPING VOLTAGE");
    m.cmd("CH2:RANGE_I 600");
    m.cmd("SAMPLING:RATE 8000");
    m.cmd("SAMPLING:DEPTH 256");
    m.cmd("CH1:ANALYSIS BUFFER");
    m.cmd("CH2:ANALYSIS BUFFER");
    m.cmd("SAMPLING:TRIGGER CONTINUOUS");

    std::this_thread::sleep_for(std::chrono::seconds(15));
  }
  catch (std::exception& ex)
  {
    StdOutExclusiveAccess guard;
    std::cerr << "main thread: " << ex.what() << std::endl;
  }
  return 0;
}
//---------------------------------------------------------------------------
