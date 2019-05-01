#include "headerFuncs.h"


// Dr. Butler's class

double timestamp()
{
  struct timeval timer;

  gettimeofday( &timer, ( struct timezone * ) 0 );
  return ( timer.tv_sec + (timer.tv_usec / 1000000.0) );
}
