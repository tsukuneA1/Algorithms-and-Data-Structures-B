#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define M 5
#define ws (M + 1)

typedef int Key;
typedef void *Item;

int eq(Key k1, Key k2) {return k1 == k2;}
int less(Key k1, Key k2) {return k1 < k2;}
Key key(Item item) {return *(Key *)item;}

Item make_item(Key k, void *data) {
    Key *p = (Key *)malloc(sizeof(Key));
    *p = k;
    return (Item)p;
}

typedef struct STnode *link;
typedef struct
{
    Key key;
    union
    {
        link next;
        Item item;
    } ref;
} entry;
struct STnode
{
    entry b[ws];
    int m;
};

static link head;
static int H;
static int N;
static long total_nodes = 0;
static long total_items = 0;

link NEW()
{
    link x = malloc(sizeof *x);
    if (x == NULL) { fprintf(stderr, "Memory alllocation failed.\n"); exit(1); }
    x->m = 0;
    total_nodes++;
    return x;
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

link insertR(link h, Item item, int H)
{
    int i, j;
    Key v = key(item);
    entry x;
    link t, u;

    x.key = v;
    x.ref.item = item;

    if (H == 0) 
    {
        for (j = 0; j < h->m; j++)
            if (less(v, h->b[j].key)) break;
    }
    else 
    {
        for (j = 0; j < h->m; j++)
            if ((j + 1 == h->m) || less(v, h->b[j + 1].key))
            {
                t = h->b[j].ref.next;
                u = insertR(t, item, H - 1);

                if (u == NULL) return NULL; 

                x.key = u->b[0].key;
                x.ref.next = u;
                j++; 

                break;
            }
    }

    for (i = (h->m)++; i > j; i--)
        h->b[i] = h->b[i - 1];
    h->b[j] = x;

    if (H == 0) { total_items++; N++; }

    if (h->m < ws) return NULL; 
    else return split(h); 
}

void STinit(int maxN)
{
    total_nodes = 0;
    total_items = 0;
    head = NEW();
    H = 0;
    N = 0;
}

void STinsert(Item item)
{
    link t, u = insertR(head, item, H);
    if (u == NULL) return;

    t = NEW();
    t->m = 2;
    t->b[0].key = head->b[0].key;
    t->b[0].ref.next = head;
    t->b[1].key = u->b[0].key;
    t->b[1].ref.next = u;
    head = t;
    H++;
}

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

void STprint() {
    printf("--- B木構造表示 (M=%d, 高さ=%d, 項目数=%ld) ---\n", M, H + 1, total_items);
    printR(head, 0, H);
    printf("--------------------------------------------------\n");
}

int main()
{
    int i;
    int maxN = 20;

    printf("==================================================\n");
    printf("  ケース 1: ランダム挿入 (N=20)\n");
    printf("==================================================\n");
    STinit(maxN);
    srand(time(NULL)); 
    printf("挿入されたキー: ");
    for (i = 0; i < maxN; i++)
    {
        Key k = rand() % 1000; 
        Item item = make_item(k, NULL);
        STinsert(item);
        printf("%d ", k);
    }
    printf("\n");
    STprint();

    printf("\n==================================================\n");
    printf("  ケース 2: 昇順挿入 (N=20)\n");
    printf("==================================================\n");
    STinit(maxN);
    printf("挿入されたキー: ");
    for (i = 1; i <= maxN; i++) 
    {
        Key k = i; 
        Item item = make_item(k, NULL);
        STinsert(item);
        printf("%d ", k);
    }
    printf("\n");
    STprint();
    
    return 0;
}
