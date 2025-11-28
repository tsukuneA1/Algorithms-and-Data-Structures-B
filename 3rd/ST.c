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
    Key v = key(item), t = key(h->item);
    if (h==z) return NEW(item, z, z, 1);
    if (rand() < RAND_MAX/(h->N+1))
        return insertT(h, item);
    if less(v, t) h->l = insertR(h->l, item);
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
