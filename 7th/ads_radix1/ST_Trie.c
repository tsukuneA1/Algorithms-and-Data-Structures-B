#include <stdio.h>
#include <stdlib.h>
#include "ItemD.h"

#define leaf(A) ((h->l == z) && (h->r == z))

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

void
countR (link h)
{
  if (h == z)
    return;
  countR (h->l);
  if (key (h->item) != 0)
    head->N++;
  countR (h->r);
}

int
STcount (void)
{
  head->N = 0;
  countR (head);
  return head->N;
}

Item
searchR (link h, Key v, int w)
{
  Key t = key (h->item);
  if (h == z)
    return NULLitem;
  if (leaf (h))
    return eq (v, t) ? h->item : NULLitem;
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


link
split (link p, link q, int w)
{
  link t = NEW (NULLitem, z, z, 2);
  switch (digit (key (p->item), w) * 2 + digit (key (q->item), w))
    {
    case 0:
      t->l = split (p, q, w + 1);
      break;
    case 1:
      t->l = p;
      t->r = q;
      break;
    case 2:
      t->r = p;
      t->l = q;
      break;
    case 3:
      t->r = split (p, q, w + 1);
      break;
    }
  return t;
}


link
insertR (link h, Item item, int w)
{
  Key v = key (item);
  if (h == z)
    return NEW (item, z, z, 1);
  if (leaf (h))
    {
      return split (NEW (item, z, z, 1), h, w);
    }
  if (digit (v, w) == 0)
    h->l = insertR (h->l, item, w + 1);
  else
    h->r = insertR (h->r, item, w + 1);
  return h;
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
      printf ("(%d)\n", key (h->item));
      STshow (h->l, l + 1);
    }
}

void
STshowAll ()
{
  STshow (head, 0);
};

