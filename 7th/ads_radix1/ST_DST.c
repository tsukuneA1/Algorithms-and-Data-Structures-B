#include <stdio.h>
#include <stdlib.h>
#include "ItemD.h"
typedef struct STnode *link;
struct STnode
{
  Item item;
  link l, r;
  int N;
};
static link head, z;
link
NEW (Item item, link l, link r, int N)
{
  link x = malloc (sizeof *x);
  x->item = item;
  x->l = l;
  x->r = r;
  x->N = N;
  return x;
}

void
STinit ()
{
  head = (z = NEW (NULLitem, 0, 0, 0));
}

int
STcount (void)
{
  return head->N;
}

Item
searchR (link h, Key v, int w)
{
  Key t = key (h->item);
  if (h == z)
    return NULLitem;
  if eq
    (v, t) return h->item;
  if (digit (v, w) == 0)
    return searchR (h->l, v, w + 1);
  else
    return searchR (h->r, v, w + 1);
}

Item
STsearch (Key v)
{
  return searchR (head, v, 0);
}

/* Exercise 9: Implement insertR here */
link
insertR (link h, Item item, int w)
{
  
}

void
STinsert (Item item)
{
  head = insertR (head, item, 0);
}

void
sortR (link h, void (*visit) (Item))
{
  if (h == z)
    return;
  sortR (h->l, visit);
  visit (h->item);
  sortR (h->r, visit);
}

void
STsort (void (*visit) (Item))
{
  sortR (head, visit);
}

void
STshow (link h, int l)
{
  int i = 0;
  if (h != NULL)
    {
      STshow (h->r, l + 1);
      for (i = 0; i < l; i++)
	printf ("\t");
      printf ("(%d,%d)\n", key (h->item), h->N);
      STshow (h->l, l + 1);
    }
}

void
STshowAll ()
{
  STshow (head, 0);
};
