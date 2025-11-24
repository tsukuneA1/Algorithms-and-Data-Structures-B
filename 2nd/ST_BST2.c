#include <stdlib.h>
#include "Item.h"
typedef struct STnode* link;
struct STnode { Item item; link l, r; int N; };
static link head, z;
link NEW(Item item, link l, link r, int N)
{ link x = malloc(sizeof *x); 
  x->item = item; x->l = l; x->r = r; x->N = N;
  return x;
}
void STinit()
{ head = (z = NEW(NULLitem, 0, 0, 0)); }
int STcount(void) { return head->N; }
Item searchR(link h, Key v)
{ Key t = key(h->item);
  if (h == z) return NULLitem;
  if eq(v, t) return h->item;
  if less(v, t) return searchR(h->l, v);
  else return searchR(h->r, v);
}
Item STsearch(Key v) 
{ return searchR(head, v); } 

link rotR(link h)
{ link x = h->l; h->l = x->r; x->r = h; 
  return x; }
link rotL(link h)
{ link x = h->r; h->r = x->l; x->l = h; 
  return x; }

link insertT(link h, Item item)
{ Key v = key(item);
  if (h == z) return NEW(item, z, z, 1); 
  if (less(v, key(h->item))) 
    { h->l = insertT(h->l, item); h = rotR(h); }
  else
    { h->r = insertT(h->r, item); h = rotL(h); }
  return h;
}
void STinsert(Item item)
{ head = insertT(head, item); }



void sortR(link h, void (*visit)(Item))
{ 
  if (h == z) return;
  sortR(h->l, visit);
  visit(h->item); 
  sortR(h->r, visit);
}
void STsort(void (*visit)(Item))
{ sortR(head, visit); } 
