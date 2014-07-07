# ifndef NANOSEC_H_
# define NANOSEC_H_

# if defined WIN32
#   if defined(_MSC_VER) && _MSC_VER >= 1600 // VS2010+
#     include <stdint.h>
#   elif defined(_MSC_VER) && _MSC_VER >= 1500 // VS2008
      extern "C"
      {
#       include "./thirdparty/oslec/spandsp-0.0.3/src/msvc/inttypes.h"
      }
#   else
#     error "Supported only VS2008+"
#   endif

#   include <windows.h>
# elif defined __MACH__
#   include <mach/mach_time.h>
# else
#   include <time.h>
# endif

// Time scale:
// 1 byte:
// 2 bytes:
// 4 bytes:
// 8 bytes:

// The calculations are in nanoseconds because of hardware progress
// on simultaneous packets processing and network interfaces very large speed.
// The time is preferably tuned to be absolute, global intergalaxy time - NTP etc.)))
// but may be drift about +-15 minutes)))
// 64-bit integer supports 292 years addressing epoch, so small for future generations.
// So, we'll use a timecode in frames or even keyframes - but not in packets.
// 32 bits are about 2 seconds with nanosecond granularity.
// With 1 000 000 packets/sec granularity error is 0.1% with nanosecond resolution.

class Nanosec
{
public:
  // singletone-like startup initializer and getter
  // should be called from main()
  static uint64_t Now()
  {
    static Nanosec timeSrc;
    return timeSrc.CurrTime();
  }

private:

  Nanosec()
  {
# if defined WIN32
    LARGE_INTEGER pf = {0};
    ::QueryPerformanceFrequency(&pf);
    m_frequency = pf.QuadPart;

# elif defined __MACH__
    mach_timebase_info(&m_timebase);
# endif
  }

  uint64_t CurrTime()
  {
# if defined WIN32
    LARGE_INTEGER pc = {0};
    ::QueryPerformanceCounter(&pc);
    return (uint64_t)(pc.QuadPart) * 1000000000ULL / m_frequency;
# elif defined __MACH__
    return mach_absolute_time() * m_timebase.numer / m_timebase.denom;
# else // LINUX/ANDROID
    timespec now = {0};
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (uint64_t) now.tv_sec * 1000000000ULL + now.tv_nsec;
# endif
  }

private:

# if defined WIN32
  uint64_t m_frequency;
# elif defined __MACH__
  mach_timebase_info_data_t m_timebase;
# endif
};

# endif