#define _ITEM_C_
#include <stdio.h>
#include <stdlib.h>
#include "Item.h"

#ifdef BTREE_ITEM
Item NULLitem = NULL;

Key ITEMrand(void)
{ return rand()%100000; }
int ITEMscan(Key *x)
{ return scanf("%d", x); }
void ITEMshow(Item x)
{
    if (x == NULL) {
        printf("NULL ");
    } else {
        printf("%3d ", key(x));
    }
}
Item ITEMmake(Key k) {
    Key *p = (Key *)malloc(sizeof(Key));
    if (p == NULL) {
        fprintf(stderr, "Memory allocation failed for item.\n");
        exit(1);
    }
    *p = k;
    return (Item)p;
}
void ITEMfree(Item item) {
    free(item);
}
#else
Item NULLitem = {-1, "NULL"};

Key ITEMrand(void)
{ return rand()%100000; }
int ITEMscan(Key *x)
{ return scanf("%d", x); }
void ITEMshow(Item x)
{ printf("%3d ", key(x)); }
#endif  
