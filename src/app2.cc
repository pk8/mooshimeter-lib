#include <iostream>
#include <fstream>
#include <iomanip>
#include <unistd.h>
#include "mooshimeter.h"

// Program configures the mooshimeter (channel 1: voltage 600V, channel 2:
// current 10A), takes a single measurement and creates a plot using gnuplot.

//---------------------------------------------------------------------------
int main()
{
  try
  {
    std::condition_variable cv;
    // Look into mooshimeter.h for explanations of constructor.
    Mooshimeter m("88:4A:EA:8A:0B:0D", 0x0015, 0x0012,
      //---------------------------------------------------------------------
      [&](const Measurement& m)
      {
        unsigned n = m.ch1.buf.size();
        if (n != m.ch2.buf.size() || !n)
        {
          return;
        }
        std::ofstream f("gp.dat");
        for (unsigned i=0; i<n; ++i)
        {
          f
            << i/8000. << "\t"
            << m.ch1.buf[i] << "\t"
            << m.ch2.buf[i] << std::endl;
        }
        f.close();
        std::ofstream g("gp.txt");
        g
          << "set grid" << std::endl
          << "set xrange [0:255./8000.]" << std::endl
          << "set xlabel 't [s]'" << std::endl
          << "set ylabel 'U [V]'" << std::endl
          << "set y2label 'I [A]'" << std::endl
          << "set ytics nomirror" << std::endl
          << "set y2tics" << std::endl
          << "f(x)=a*sin(b*x+c)" << std::endl
          << "a=300." << std::endl
          << "b=314." << std::endl
          << "c=1." << std::endl
          << "fit f(x) \"gp.dat\" using 1:3 via a,b,c" << std::endl
          <<
            "set title 'Fitted sin: '.sprintf('RMS = %.2f V, "
            "f = %.2f Hz', abs(a)/sqrt(2.),b/2./pi)"
          << std::endl
          <<
            "plot \"gp.dat\" using 1:3 with lines title 'U', "
            "\"gp.dat\" using 1:2 with lines axes x1y2 title 'I', "
            "f(x) title 'sin'"
          << std::endl;
        g.close();
        system("gnuplot -persist gp.txt");
        unlink("gp.txt");
        unlink("gp.dat");
        unlink("fit.log");

        // We are done.
        cv.notify_one();
      },
      //---------------------------------------------------------------------
      [&](const Response&) {});
      //---------------------------------------------------------------------

    m.cmd("CH1:MAPPING CURRENT");
    m.cmd("CH1:RANGE_I 10");
    m.cmd("CH2:MAPPING VOLTAGE");
    m.cmd("CH2:RANGE_I 600");
    m.cmd("SAMPLING:RATE 8000");
    m.cmd("SAMPLING:DEPTH 256");
    m.cmd("CH1:ANALYSIS BUFFER");
    m.cmd("CH2:ANALYSIS BUFFER");
    m.cmd("SAMPLING:TRIGGER SINGLE");

    // Wait for gnuplot.
    std::mutex mtx;
    std::unique_lock<std::mutex> guard(mtx);
    cv.wait(guard);
  }
  catch (std::exception& ex)
  {
    StdOutExclusiveAccess guard;
    std::cerr << "main thread: " << ex.what() << std::endl;
  }
  return 0;
}
//---------------------------------------------------------------------------
