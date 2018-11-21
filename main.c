#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../c_akinator/lib/constants.h"
#include <ctype.h>

enum {
    UNTYPED,
    CONST,
    VAR,
    OP
    };



void printtabs(int num)
{
    assert(num >= 0);

    printf(BLUE);
    for( ; num; num--)
        printf("  | ");
    printf(RESET);
    return;
}



struct Tree_d_item
{
    int type;
    union
    {
        char code;
        float val;
    } data;
    struct Tree_d_item* left;
    struct Tree_d_item* right;
    struct Tree_d_item* parent;
};

struct Tree_d
{
    int ID;
    int size;
    struct Tree_d_item* root;
};



void Tree_d_constructor(struct Tree_d* this, int number)
{
    assert(this != NULL);

    this->ID = number;
    this->size = 0;
    this->root = NULL;
    return;
}

void Tree_d_item_delete(struct Tree_d_item* this)
{
    if(this == NULL)
        return;

    this->type = UNTYPED;

    Tree_d_item_delete(this->left);
    free(this->left);

    Tree_d_item_delete(this->right);
    free(this->right);
    return;
}

void Tree_d_destructor(struct Tree_d* this)
{
    assert(this != NULL);

    this->ID = -1;
    this->size = -1;

    Tree_d_item_delete(this->root);
    free(this->root);
    this->root = NULL;
    return;
}

void Tree_d_item_dump(struct Tree_d_item* this, int num_of_tabs)
{
    if(this == NULL)
    {
        printtabs(num_of_tabs);
        printf(BLUE"nil"RESET"\n");
        return;
    }

    printtabs(num_of_tabs);

    if(this->right == NULL)
        printf(RED);
    else
        printf(BLUE);
    if(this->type == VAR || this->type == OP)
        printf("%c", this->data.code);
    else
        printf("%g", this->data.val);
    printf(RESET"\n");

    if(this->right == NULL)
        return;

    // RIGHT

    printtabs(num_of_tabs);
    printf("(\n");

    Tree_d_item_dump(this->right, num_of_tabs + 1);

    printtabs(num_of_tabs);
    printf(")\n");

    // LEFT

    printtabs(num_of_tabs);
    printf("(\n");

    Tree_d_item_dump(this->left, num_of_tabs + 1);

    printtabs(num_of_tabs);
    printf(")\n");
    return;
}

void Tree_d_dump(struct Tree_d* this)
{
    assert(this != NULL);

    printf("Tree #%d\n", this->ID);
    printf("{ size = %d\n", this->size);
    Tree_d_item_dump(this->root, 1);
    printf("}\n");
    return;
}

struct Tree_d_item* Tree_d_item_load(FILE* infile, struct Tree_d* this)
{
    assert(infile != NULL);
    assert(this != NULL);

    char c = '\0';

    while(isspace(c = getc(infile)));

    if(c != (int) '(')
    {
        ungetc(c, infile);
        struct Tree_d_item* new_item = (struct Tree_d_item*) malloc(sizeof(struct Tree_d_item));

        if(isdigit(c))
        {
            new_item->type = CONST;
            fscanf(infile, "%g", &new_item->data.val);
        }
        else
        {
            new_item->type = VAR;
            fscanf(infile, "%c", &new_item->data.code);
            //new_item->type = (new_item->data.code == 'x') ? VAR : OP);
        }

        new_item->right = NULL;
        new_item->left = NULL;
        this->size++;

        while(isspace(c = getc(infile)));
        return new_item;
    }

    struct Tree_d_item* new_item = (struct Tree_d_item*) malloc(sizeof(struct Tree_d_item));
    new_item->type = OP;
    new_item->right = NULL;
    new_item->left = NULL;

    new_item->left = Tree_d_item_load(infile, this);
    if(new_item->left != NULL)
        new_item->left->parent = new_item;

    while(isspace(c = getc(infile)));
    ungetc(c, infile);
    fscanf(infile, "%c", &new_item->data.code);
    this->size++;

    while(isspace(c = getc(infile)));
    new_item->right = Tree_d_item_load(infile, this);
    if(new_item->right != NULL)
        new_item->right->parent = new_item;
    while(isspace(c = getc(infile)));

    return new_item;
}

void Tree_d_load(struct Tree_d* this)
{
    assert(this != NULL);

    int id = this->ID;
    Tree_d_destructor(this);
    Tree_d_constructor(this, id);

    FILE* infile = fopen("input.txt", "r");

    this->root = Tree_d_item_load(infile, this);
    if(this->root != NULL)
        this->root->parent = NULL;

    fclose(infile);
    return;
}

struct Tree_d_item* Tree_d_item_diff(struct Tree_d_item* old)
{
    assert(old != NULL);

    struct Tree_d_item* new_item = (struct Tree_d_item*) malloc(sizeof(struct Tree_d_item));

    if(old->type == CONST)
    {
        new_item->type = CONST;
        new_item->data.val = 0;
        new_item->left = NULL;
        new_item->right = NULL;
    }
    else if(old->type == VAR)
    {
        new_item->type = CONST;
        new_item->data.val = 1;
        new_item->left = NULL;
        new_item->right = NULL;
    }
    else if(old->type == OP && old->data.code == '+')
    {
        new_item->type = OP;
        new_item->data.code = '+';

        new_item->left = Tree_d_item_diff(old->left);
        new_item->right = Tree_d_item_diff(old->right);
    }
    else if(old->type == OP && old->data.code == '*')
    {
        new_item->type = OP;
        new_item->data.code = '+';

        new_item->left = (struct Tree_d_item*) malloc(sizeof(struct Tree_d_item));
        new_item->left->type = OP;
        new_item->left->data.code = '*';
        new_item->left->left = Tree_d_item_diff(old->left);
        new_item->left->right = old->right;

        new_item->right = (struct Tree_d_item*) malloc(sizeof(struct Tree_d_item));
        new_item->right->type = OP;
        new_item->right->data.code = '*';
        new_item->right->left = old->left;
        new_item->right->right = Tree_d_item_diff(old->right);
    }

    return new_item;
}

void Tree_d_diff(struct Tree_d* old, struct Tree_d* new)
{
    assert(old != NULL);
    assert(new != NULL);
    assert(old != new);

    new->root = Tree_d_item_diff(old->root);

    return;
}



int main()
{
    struct Tree_d original = {};
    Tree_d_constructor(&original, 0);
    Tree_d_dump(&original);

    Tree_d_load(&original);
    Tree_d_dump(&original);

    struct Tree_d diff = {};
    Tree_d_constructor(&diff, 1);

    Tree_d_diff(&original, &diff);
    Tree_d_dump(&diff);

    Tree_d_destructor(&diff);

    Tree_d_destructor(&original);
    return 0;
}
