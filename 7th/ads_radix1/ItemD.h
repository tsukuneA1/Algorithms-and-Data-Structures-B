typedef int Key;
typedef struct
{
  Key key;
  char information[10];
} Item;

#define key(A) (A.key)
#define less(A, B) (A < B)
#define exch(A, B) { Item t = A; A = B; B = t; }
#define compexch(A, B) if (less(B, A)) exch(A, B)
#define eq(A, B) (A == B)

#define bitsword 17
#define digit(A, B) (((A) >> (bitsword-((B)+1))) & 1)

#ifndef _ITEM_D_
extern Item NULLitem;
#endif

Key ITEMrand (void);
int ITEMscan (Key *);
void ITEMshow (Item);
