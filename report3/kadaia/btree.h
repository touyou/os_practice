typedef struct node {
    int v;
    struct node *left;
    struct node *right;
} tnode;

tnode *btree_create();
tnode *btree_insert(int v, tnode *t);
void btree_destroy(tnode *t);
void btree_dump(tnode *t);
