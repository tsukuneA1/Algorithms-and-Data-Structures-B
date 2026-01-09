// C言語の標準ライブラリ、malloc関数を呼び出す
#include <stdlib.h>
#include "Item.h"
typedef struct STnode* link;
// Nはノード以下の項目数
struct STnode { Item item; link l, r; int N; };
// headは木の根、zは葉を表す。番兵的な役割を果たす
static link head, z;
link NEW(Item item, link l, link r, int N)
{
    link x = malloc(sizeof *x);
    x->item = item;
    x->l = l;
    x->r = r;
    x->N = N;
    return x;
}

void STinit()
{
    head = (z = NEW(NULLitem, 0, 0, 0));
}

int STcount(void)
{
    return head->N;
}

Item searchR(link h, Key v)
{
    Key t = key(h->item);
    if (h == z) return NULLitem;
    if eq(v, t) return h->item;
    if less(v, t) return searchR(h->l, v);
    else return searchR(h->r, v);
}

Item STsearch(Key v)
{
    return searchR(head, v);
}

link insertR(link h, Item item)
{
    Key v = key(item);
    Key t = key(h->item);
    if (h == z) return NEW(item, z, z, 1);
    if less(v, t)
        h->l = insertR(h->l, item);
    else h->r = insertR(h->r, item);
    (h->N)++; return h;
}

void STinsert(Item item)
{
    head = insertR(head, item);
}

void sortR(link h, void (*visit)(Item))
{
    if (h == z) return;
    sortR(h->l, visit);
    visit(h->item);
    sortR(h->r, visit);
}

void STsort(void (*visit)(Item))
{
    sortR(head, visit);
}

Item selectR(link h, int k)
{
    int t = h->l->N;
    if (h==z) return NULLitem;
    if (t > k) return selectR(h->l, k);
    if (t < k) return selectR(h->r, k-t-1);
    return h->item;
}

Item STselect(int k)
{
    return selectR(head, j);
}

link partR(link h, int k)
{
    int t = h->l->N;
    if (t > k)
        { h->l = partR(h->l, k); h = rotR(h); }
    if (t < k)
        { h->r = partR(h->r, k-t-1); h = rotL(h); }
    return h;
}

link joinLR(link a, link b)
{
    if (b == z) return a;
    b = partR(b, 0); b->l = a;
    return b;
}

link deleteR(link h, Key v)
{
    link x; Key t = key(h->item);
    if (h == z) return z;
    if (less(v, t)) h->l = deleteR(h->l, v);
    if (less(t, v)) h->r = deleteR(h->r, v);
    if (eq(v, t))
        { x = h; h = joinLR(h->l, h->r); free(x); }
    return h;
}

void STdelete(Key v)
    { head = deleteR(head, v); }
