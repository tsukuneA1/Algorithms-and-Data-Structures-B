#include <stdio.h>
#define bitsword 32
#define digit(A, B) (((A) >> (bitsword-((B)+1))) & 1)

int
main (void)
{
  int b, d;

  d = 16777215;
  b = digit (d, 12);
  printf ("12th bit of %d = %d\n", d, b);

  return 0;
}
