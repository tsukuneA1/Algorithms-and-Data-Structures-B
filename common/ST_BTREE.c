#define BTREE_ITEM
#include <stdlib.h>
#include <stdio.h>
#include "Item.h"

#ifndef M
#define M 5
#endif
#define ws (M+1)

typedef struct STnode* link;
typedef struct
{
    Key key;
    union {
        link next;
        Item item;
    } ref;
} entry;

struct STnode {
    entry b[ws];
    int m;
};

static link head;
static int H, N;
static long total_nodes = 0;
static long total_items = 0;

link NEW()
{
    link x = malloc(sizeof *x);
    if (x == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(1);
    }
    x->m = 0;
    total_nodes++;
    return x;
}

void STinit(int maxN)
{
    total_nodes = 0;
    total_items = 0;
    head = NEW();
    H = 0;
    N = 0;
}

link split(link h)
{
    link t = NEW();
    for (int i = 0; i < ws / 2; i++)
        t->b[i] = h->b[ws / 2 + i];
    h->m = ws / 2;
    t->m = ws / 2;
    return t;
}

Item searchR(link h, Key v, int H)
{
    int j;
    if (H == 0)
        for (j = 0; j < h->m; j++)
            if (eq(v, h->b[j].key))
                return h->b[j].ref.item;
    if (H != 0)
        for (j = 0; j < h->m; j++)
            if ((j + 1 == h->m) || less(v, h->b[j + 1].key))
                return searchR(h->b[j].ref.next, v, H - 1);
    return NULLitem;
}

Item STsearch(Key v)
{
    return searchR(head, v, H);
}

link insertR(link h, Item item, int H)
{
    int i, j;
    Key v = key(item);
    entry x;
    link t, u;

    x.key = v;
    x.ref.item = item;

    if (H == 0) {
        for (j = 0; j < h->m; j++)
            if (less(v, h->b[j].key))
                break;
    } else {
        for (j = 0; j < h->m; j++) {
            if ((j + 1 == h->m) || less(v, h->b[j + 1].key)) {
                t = h->b[j].ref.next;
                u = insertR(t, item, H - 1);

                if (u == NULL) return NULL;

                x.key = u->b[0].key;
                x.ref.next = u;
                j++;
                break;
            }
        }
    }

    for (i = (h->m)++; i > j; i--)
        h->b[i] = h->b[i - 1];
    h->b[j] = x;

    if (H == 0) {
        total_items++;
        N++;
    }

    if (h->m < ws)
        return NULL;
    else
        return split(h);
}

void STinsert(Item item)
{
    link t, u = insertR(head, item, H);
    if (u == NULL)
        return;

    t = NEW();
    t->m = 2;
    t->b[0].key = head->b[0].key;
    t->b[0].ref.next = head;
    t->b[1].key = u->b[0].key;
    t->b[1].ref.next = u;
    head = t;
    H++;
}

int STcount(void)
{
    return N;
}

// B木構造の表示用関数（assignment_3で使用）
void printR(link h, int level, int H)
{
    int i;

    for (i = 0; i < level; i++) {
        printf("|   ");
    }

    printf("Level %d: [ ", level);
    for (i = 0; i < h->m; i++) {
        printf("%d", h->b[i].key);
        if (i < h->m - 1) {
            printf(", ");
        }
    }
    printf(" ] (%d keys)\n", h->m);

    if (level < H) {
        for (i = 0; i < h->m; i++) {
            link child = h->b[i].ref.next;

            if (child != NULL) {
                printR(child, level + 1, H);
            }
        }
    }
}

void STprint()
{
    printf("--- B木構造表示 (M=%d, 高さ=%d, 項目数=%ld) ---\n", M, H + 1, total_items);
    printR(head, 0, H);
    printf("--------------------------------------------------\n");
}

// メモリ効率レポート関数（assignment_2で使用）
void STreport()
{
    long max_items_per_node = M - 1;
    long max_capacity;
    double memory_efficiency;

    max_capacity = total_nodes * max_items_per_node;

    if (max_capacity == 0) {
        memory_efficiency = 0.0;
    } else {
        memory_efficiency = ((double)total_items / max_capacity) * 100.0;
    }

    printf("  総ページ数: %ld\n", total_nodes);
    printf("  理論最大容量: %ld\n", max_capacity);
    printf("  メモリ効率: %.2f%%\n", memory_efficiency);
    printf("  木の高さ: %d\n", H);
    printf("----------------------------------\n");
}

// 統計情報取得関数
long STgetNodes() { return total_nodes; }
long STgetItems() { return total_items; }
int STgetHeight() { return H; }
