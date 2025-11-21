typedef int Key;

#ifdef BTREE_ITEM
// B木用のItem定義（ポインタベース）
typedef void *Item;
#define key(A) (*(Key *)A)
#define less(A, B) (A < B)
#define eq(A, B) (A == B)

#ifndef _ITEM_C_
extern Item NULLitem;
#endif

Key ITEMrand(void);
int ITEMscan(Key *);
void ITEMshow(Item);
Item ITEMmake(Key k);
void ITEMfree(Item item);

#else
// 通常のItem定義（構造体ベース）
typedef struct { Key key; char information[10]; } Item;
#define key(A) (A.key)

#define less(A, B) (A < B)
#define exch(A, B) { Item t = A; A = B; B = t; }
#define compexch(A, B) if (less(B, A)) exch(A, B)
#define eq(A, B) (A == B)

#ifndef _ITEM_C_
extern Item NULLitem;
#endif

Key ITEMrand(void);
int ITEMscan(Key *);
void ITEMshow(Item);
#endif
