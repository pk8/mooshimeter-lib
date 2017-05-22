#include <iostream>
#include <ctime>
#include "mooshimeter.h"

// Program updates mooshimeter clock.

//---------------------------------------------------------------------------
int main()
{
  try
  {
    // Look into mooshimeter.h for explanations of constructor.
    Mooshimeter m("88:4A:EA:8A:0B:0D", 0x0015, 0x0012,
      [&](const Measurement&) {}, [&](const Response&) {});
    // We are waiting for response to ensure command has been processed.
    m.cmd("TIME_UTC "+std::to_string(time(nullptr))).get();
  }
  catch (std::exception& ex)
  {
    StdOutExclusiveAccess guard;
    std::cerr << "main thread: " << ex.what() << std::endl;
  }
  return 0;
}
//---------------------------------------------------------------------------
