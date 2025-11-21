#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

typedef int Key;
typedef struct Item {
  Key key;
} Item;

static inline int less(Key a, Key b) { return a < b; }
static inline Key key(Item it) { return it.key; }

typedef struct STnode *link;
struct STnode {
    Item item;
    link l, r;
    int N;
    int red;
};

static link z = NULL;
static link head = NULL;

static link NEW(Item item, link l, link r, int N, int red) {
    link x = (link)malloc(sizeof *x);
    x->item = item;
    x->l = l;
    x->r = r;
    x->N = N;
    x->red = red;
    return x;
}

static void fixN(link h) {
    if (h == z) return;
    int ln = (h->l == z) ? 0 : h->l->N;
    int rn = (h->r == z) ? 0 : h->r->N;
    h->N = ln + rn + 1;
}

static link rotL(link h) {
    link x = h->r;
    h->r = x->l;
    x->l = h;
    x->red = h->red;
    h->red = 1;
    fixN(h);
    fixN(x);
    return x;
}

static link rotR(link h) {
    link x = h->l;
    h->l = x->r;
    x->r = h;
    x->red = h->red;
    h->red = 1;
    fixN(h);
    fixN(x);
    return x;
}

static link RBinsert(link h, Item item, int sw) {
    Key v = key(item);

    if (h == z)
        return NEW(item, z, z, 1, 1);

    if ((h->l->red) && (h->r->red)) {
        h->red = 1;
        h->l->red = 0;
        h->r->red = 0;
    }

    if (less(v, key(h->item))) {
        h->l = RBinsert(h->l, item, 0);
        if (h->red && h->l->red && sw)
            h = rotR(h);

        if (h->l->red && h->l->l->red) {
            h = rotR(h);
            h->red = 0;
            h->r->red = 1;
        }
    } else {
        h->r = RBinsert(h->r, item, 1);
        if (h->red && h->r->red && !sw)
            h = rotL(h);

        if (h->r->red && h->r->r->red) {
            h = rotL(h);
            h->red = 0;
            h->l->red = 1;
        }
    }

    fixN(h);
    return h;
}

static void STinit(void) {
    if (z == NULL) {
        z = (link)malloc(sizeof *z);
        z->item.key = 0;
        z->l = z->r = z;
        z->N = 0;
        z->red = 0;
    }
    head = z;
}

static void STinsert(Item item) {
    head = RBinsert(head, item, 0);
    head->red = 0;
}

static void printTreeRec(link h, int depth, int bh) {
    if (h == z) {
        for (int i = 0; i < depth; ++i) putchar(' ');
        printf("• [leaf] bh=%d\n", bh);
        return;
    }
    printTreeRec(h->r, depth + 4, bh + (h->red ? 0 : 1));
    for (int i = 0; i < depth; ++i) putchar(' ');
    printf("%d(%c)\n", key(h->item), (h->red ? 'R' : 'B'));
    printTreeRec(h->l, depth + 4, bh + (h->red ? 0 : 1));
}

static void printTree(const char* title) {
    printf("==== %s ====\n", title);
    if (head == z) {
        printf("(empty)\n");
        return;
    }
    printTreeRec(head, 0, 0);
    printf("\n");
}

static void shuffle_ints(int* a, int n) {
    for (int i = n - 1; i > 0; --i) {
        int j = rand() % (i + 1);
        int tmp = a[i]; a[i] = a[j]; a[j] = tmp;
    }
}

int main(void) {
    srand((unsigned)time(NULL));

    STinit();
    int A[20];
    for (int i = 0; i < 20; ++i) A[i] = i + 1;
    shuffle_ints(A, 20);
    for (int i = 0; i < 20; ++i) {
        Item it = { .key = A[i] };
        STinsert(it);
    }
    printTree("Case 1: ランダム順で20要素を挿入");

    STinit();
    for (int i = 0; i < 20; ++i) {
        Item it = { .key = i + 1 };
        STinsert(it);
    }
    printTree("Case 2: 昇順で20要素を挿入");

    return 0;
}
