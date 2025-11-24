#include <stdio.h>
#include <stdlib.h>
#include "Item.h"
#include "ST.h"

int main(int argc, char *argv[])
{  int N, M, maxN = atoi(argv[1]), sw = atoi(argv[2]);
  Key v; Item item;
  STinit(maxN); srand(1);
  for (M = 0, N = 0; N < maxN; N++)
    {
      if (sw == 1) v = ITEMrand();
      else if (sw == 2) v = N+1;
      else if (ITEMscan(&v) == EOF) break;
      item = STsearch(v); if (item.key != NULLitem.key) continue;
      key(item) = v;
      STinsert(item); M++;
    }
  STsort(ITEMshow); printf("\n");
  printf("%d keys ", N);
  printf("%d distinct keys\n", STcount());
  return 0;
}
