#include <stdio.h>
#include <stdlib.h>

typedef int Key;
typedef struct {
  Key key;
} Item;

typedef struct STnode* link;
struct STnode {
  Item item;
  link l, r;
  int N;
};

link head, z;

Key key(Item item) { return item.key; }
int less(Key a, Key b) { return a < b; }
int equal(Key a, Key b) { return a == b; }

link NEW(Item item, link l, link r, int N) {
  link x = (link)malloc(sizeof(*x));
  x->item = item;
  x->l = l;
  x->r = r;
  x->N = N;
  return x;
}

link rotR(link h) {
  link x = h->l;
  h->l = x->r;
  x->r = h;
  x->N = h->N;
  h->N = (h->l->N) + (h->r->N) + 1;
  return x;
}

link rotL(link h) {
  link x = h->r;
  h->r = x->l;
  x->l = h;
  x->N = h->N;
  h->N = (h->l->N) + (h->r->N) + 1;
  return x;
}

link splay(link h, Item item) {
  Key v = key(item);

  if (h == z)
    return NEW(item, z, z, 1);

  if (equal(v, key(h->item))) {
    return h; 
  }

  if (less(v, key(h->item))) {
    if (h->l == z)
      return NEW(item, z, h, h->N + 1);

    if (equal(v, key(h->l->item))) {
      return rotR(h); 
    }

    if (less(v, key(h->l->item))) {
      h->l->l = splay(h->l->l, item);
      h = rotR(h);
    } else {
      h->l->r = splay(h->l->r, item);
      h->l = rotL(h->l);
    }
    return rotR(h);
  } else {
    if (h->r == z)
      return NEW(item, h, z, h->N + 1);

    if (equal(v, key(h->r->item))) {
      return rotL(h); 
    }

    if (less(key(h->r->item), v)) {
      h->r->r = splay(h->r->r, item);
      h = rotL(h);
    } else {
      h->r->l = splay(h->r->l, item);
      h->r = rotR(h->r);
    }
    return rotL(h);
  }
}

void STinsert(Item item) {
  head = splay(head, item);
}

void printTree(link h, int depth) {
  if (h == z) return;
  printTree(h->r, depth + 1);
  for (int i = 0; i < depth; i++) printf("    ");
  printf("%d\n", h->item.key);
  printTree(h->l, depth + 1);
}

void STinit() {
  z = (link)malloc(sizeof(*z));
  z->l = z->r = z;
  z->N = 0;
  head = z;
}

int main() {
  STinit();

  int keys[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 5};

  for (int i = 0; i < 10; i++) {
    Item item = {keys[i]};
    STinsert(item);
    printf("挿入: %d\n", keys[i]);
    printTree(head, 0);
    printf("\n");
  }

  printf("最終的な木の形状:\n");
  printTree(head, 0);

  return 0;
}
