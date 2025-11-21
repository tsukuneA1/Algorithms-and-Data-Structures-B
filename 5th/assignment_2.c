#define BTREE_ITEM
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../common/Item.h"
#include "../common/Item.c"

#define M 5
#include "../common/ST_BTREE.c"

int main()
{
    int i;
    int maxN = 1000;

    printf("=== B木テスト条件 ===\n");
    printf("次数 M: %d\n", M);
    printf("挿入項目数: %d\n", maxN);
    printf("=====================\n");

    printf("\n--- 1. ランダムな順序で挿入 ---\n");
    STinit(maxN);
    srand(time(NULL));
    for (i = 0; i < maxN; i++)
    {
        Key k = rand() % 100000;
        Item item = ITEMmake(k);
        STinsert(item);
    }
    STreport();

    printf("\n--- 2. 昇順で挿入 ---\n");
    STinit(maxN);
    for (i = 0; i < maxN; i++)
    {
        Key k = i;
        Item item = ITEMmake(k);
        STinsert(item);
    }
    STreport();

    printf("\n--- 3. 降順で挿入 ---\n");
    STinit(maxN);
    for (i = maxN - 1; i >= 0; i--)
    {
        Key k = i;
        Item item = ITEMmake(k);
        STinsert(item);
    }
    STreport();

    return 0;
}
