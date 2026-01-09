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
    int bit;
};

static link head, z;
link NEW(Item item, link l, link r, int bit)
{
    link x = malloc(sizeof *x);
    x->item = item;
    x->l = l;
    x->r = r;
    x->bit = bit;
    x->N = 0;
    return x;
}

void STinit()
{
    head = NEW(NULLitem, 0, 0, -1);
    head->l = head;
    head->r = head;
}

Item searchR(link h, Key v, int w)
{
    if (h->bit <= w)
        return h->item;
    if (digit(v, h->bit) == 0)
        return searchR(h->l, v, h->bit);
    else
        return searchR(h->r, v, h->bit);
}

Item STsearch(Key v)
{
    Item t = searchR(head->l, v, -1);
    return eq(v, key(t)) ? t : NULLitem;
}

link insertR(link h, Item item, int w, link p)
{
    link x;
    Key v = key(item);
    if ((h->bit >= w) || (h->bit <= p->bit))
    {
        x = NEW(item, 0, 0, w);
        x->l = digit(v, x->bit) ? h : x;
        x->r = digit(v, x->bit) ? x : h;
        return x;
    }
    if (digit(v, h->bit) == 0)
        h->l = insertR(h->l, item, w, h);
    else
        h->r = insertR(h->r, item, w, h);
    return h;
}

void STinsert(Item item)
{
    int i;
    Key v = key(item);
    Key t = key(searchR(head->l, v, -1));
    if (v == t)
        return;
    for (i = 0; digit(v, i) == digit(t, i); i++)
        ;
    head->l = insertR(head->l, item, i, head);
}