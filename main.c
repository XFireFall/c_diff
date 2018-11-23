#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#include "../c_akinator/lib/constants.h"


#define L  Tree_d_item_copy(old->left)
#define dL Tree_d_item_diff(L)

#define R  Tree_d_item_copy(old->right)
#define dR Tree_d_item_diff(R)

#define CONST( var )  (Tree_d_item_create(CONST, var,   NULL, NULL))
#define PLUS( a, b )  (Tree_d_item_create(OP,    '+',   a,    b   ))
#define MINUS( a, b ) (Tree_d_item_create(OP,    '-',   a,    b   ))
#define MUL( a, b )   (Tree_d_item_create(OP,    '*',   a,    b   ))
#define DIV( a, b )   (Tree_d_item_create(OP,    '/',   a,    b   ))
#define POW( a, b )   (Tree_d_item_create(OP,    '^',   a,    b   ))
#define LN( var )     (Tree_d_item_create(OP,    F_LN,  var,  NULL))
#define SIN( var )    (Tree_d_item_create(OP,    F_SIN, var,  NULL))
#define COS( var )    (Tree_d_item_create(OP,    F_COS, var,  NULL))

#define IS_MUL( var )   (var->type == OP && var->data.code == '*')
#define IS_CONST( var ) (var->type == CONST)
#define IS_PLUS( var )  (var->type == OP && var->data.code == '+')
#define IS_DIV( var )   (var->type == OP && var->data.code == '/')
#define IS_MINUS( var ) (var->type == OP && var->data.code == '-')
#define IS_POW( var )   (var->type == OP && var->data.code == '^')
#define IS_SIN( var )   (var->type == OP && var->data.code == F_SIN)
#define IS_COS( var )   (var->type == OP && var->data.code == F_COS)

enum {
    UNTYPED,
    CONST,
    VAR,
    OP
    };

enum {
    F_SIN = 's',
    F_COS = 'c',
    F_LOG = 'l',
    F_LN  = 'n'
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

    if(this->left == NULL)
        return;

    // RIGHT

    //printtabs(num_of_tabs);
    //printf("(\n");

    Tree_d_item_dump(this->left, num_of_tabs + 1);

    //printtabs(num_of_tabs);
    //printf(")\n");

    // LEFT

    //printtabs(num_of_tabs);
    //printf("(\n");

    Tree_d_item_dump(this->right, num_of_tabs + 1);

    //printtabs(num_of_tabs);
    //printf(")\n");
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

char get_func(char* s)
{
    if(!strcmp(s, "sin"))
        return F_SIN;
    else if(!strcmp(s, "cos"))
        return F_COS;
    else if(!strcmp(s, "ln"))
        return F_LN;
    else if(!strcmp(s, "log"))
        return F_LOG;

    return -1;
}

char get_argc(char f)
{
    switch(f)
    {
    case F_SIN:
    case F_COS:
    case F_LN:
        return 1;
        break;

    case F_LOG:
        return 2;
        break;
    }

    return -1;
}

struct Tree_d_item* Tree_d_item_load(FILE* infile, struct Tree_d* this)
{
    assert(infile != NULL);
    assert(this != NULL);

    char c = '\0';

    while(isspace(c = getc(infile)));

    if(c != (int) '(' && c != (int) '[' && c != (int) '{')
    {
        ungetc(c, infile);
        struct Tree_d_item* new_item = (struct Tree_d_item*) malloc(sizeof(struct Tree_d_item));

        new_item->right = NULL;
        new_item->left = NULL;

        if(isdigit(c))
        {
            new_item->type = CONST;
            fscanf(infile, "%g", &new_item->data.val);
        }
        else
        {
            char s[MAX_STR_LEN] = "";
            fscanf(infile, "%[A-Za-z]s", s);

            if(!strcmp(s, "x"))
            {
                new_item->type = VAR;
                new_item->data.code = 'x';
            }
            else
            {
                new_item->data.code = get_func(s);
int argc = get_argc(new_item->data.code);
                new_item->type = OP;

                while(isspace(c = getc(infile)));
                new_item->left = Tree_d_item_load(infile, this);
                if(new_item->left != NULL)
                    new_item->left->parent = new_item;
                if(argc == 2)
                {
                    //while(isspace(c = getc(infile)));
                    //while(isspace(c = getc(infile)));
                    new_item->right = Tree_d_item_load(infile, this);
                    if(new_item->right != NULL)
                        new_item->right->parent = new_item;
                }
            }
        }

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

struct Tree_d_item* Tree_d_item_create(int type, float data, struct Tree_d_item* left, struct Tree_d_item* right)
{
    struct Tree_d_item* new_item = (struct Tree_d_item*) malloc(sizeof(struct Tree_d_item));
    new_item->type = type;
    if(new_item->type == CONST)
        new_item->data.val = data;
    else
        new_item->data.code = (char) (int) data;
    new_item->left = left;
    new_item->right = right;
    return new_item;
}

struct Tree_d_item* Tree_d_item_copy(struct Tree_d_item* this)
{
    if(this == NULL)
        return NULL;

    return Tree_d_item_create(this->type, (this->type == CONST) ? this->data.val : this->data.code, Tree_d_item_copy(this->left), Tree_d_item_copy(this->right));
}

struct Tree_d_item* Tree_d_item_diff(struct Tree_d_item* old)
{
    assert(old != NULL);

    switch(old->type)
    {
    case CONST:
        return CONST(0);
        break;

    case VAR:
        return CONST(1);
        break;

    case OP:
        switch(old->data.code)
        {
        case '+':
            return PLUS(dL, dR);
            break;

        case '-':
            return MINUS(dL, dR);
            break;

        case '*':
            return PLUS(MUL(dL, R), MUL(L, dR));
            break;

        case '/':
            return DIV(MINUS(MUL(dL, R), MUL(L, dR)), MUL(R, R));
            break;

        case '^':
            return PLUS(MUL(POW(L, R                 ), MUL(dR, LN(L))),
                        MUL(POW(L, MINUS(R, CONST(1))), MUL(dL, R)));
            break;

        case F_SIN:
            return MUL(COS(L), dL);
            break;

        case F_COS:
            return MUL(CONST(-1), MUL(SIN(L), dL));
            break;

        case F_LOG:
            return MINUS(DIV(dR, MUL(R, LN(L))),
                         DIV(MUL(LN(R), dL), MUL(LN(L), R)));
            break;

        case F_LN:
            return DIV(dL, L);

        default:
            return NULL;
        }
        break;

    default:
        return NULL;
    }

    return NULL;
}

void Tree_d_diff(struct Tree_d* old, struct Tree_d* new)
{
    assert(old != NULL);
    assert(new != NULL);
    assert(old != new);

    int id = new->ID;
    Tree_d_destructor(new);
    Tree_d_constructor(new, id);

    new->root = Tree_d_item_diff(old->root);

    return;
}


struct Tree_d_item* Tree_d_item_ease(struct Tree_d_item* this)
{
    if(this == NULL)
        return NULL;

    this->left = Tree_d_item_ease(this->left);
    this->right = Tree_d_item_ease(this->right);

    if(IS_MUL(this) && IS_CONST(this->left) && this->left->data.val == 0)   // 0*
    {
        Tree_d_item_delete(this);
        return Tree_d_item_create(CONST, 0, NULL, NULL);
    }

    if(IS_MUL(this) && IS_CONST(this->right) && this->right->data.val == 0) // *0
    {
        Tree_d_item_delete(this);
        return Tree_d_item_create(CONST, 0, NULL, NULL);
    }

    if(IS_MUL(this) && IS_CONST(this->left) && this->left->data.val == 1)   // 1*
    {
        free(this->left);
        return this->right;
    }

    if(IS_MUL(this) && IS_CONST(this->right) && this->right->data.val == 1) // *1
    {
        free(this->right);
        return this->left;
    }



    if(IS_DIV(this) && IS_CONST(this->left) && this->left->data.val == 0)   // 0/
    {
        Tree_d_item_delete(this);
        return Tree_d_item_create(CONST, 0, NULL, NULL);
    }

    if(IS_DIV(this) && IS_CONST(this->right) && this->right->data.val == 1) // /1
    {
        free(this->right);
        return this->left;
    }



    if(IS_PLUS(this) && IS_CONST(this->left) && this->left->data.val == 0)  // 0+
    {
        free(this->left);
        return this->right;
    }

    if(IS_PLUS(this) && IS_CONST(this->right) && this->right->data.val == 0)// +0
    {
        free(this->right);
        return this->left;
    }



    if(IS_MINUS(this) && IS_CONST(this->right) && this->right->data.val == 0)// -0
    {
        free(this->right);
        return this->left;
    }

    if(IS_POW(this) && IS_CONST(this->right) && this->right->data.val == 1) // ^1
    {
        free(this->right);
        return this->left;
    }

    if(IS_POW(this) && IS_CONST(this->right) && this->right->data.val == 0) // ^0
    {
        Tree_d_item_delete(this);
        return Tree_d_item_create(CONST, 1, NULL, NULL);
    }

    if(IS_SIN(this) && IS_CONST(this->left) && this->left->data.val == 0)   // sin0
    {
        Tree_d_item_delete(this);
        return Tree_d_item_create(CONST, 0, NULL, NULL);
    }

    if(IS_COS(this) && IS_CONST(this->left) && this->left->data.val == 0)   // cos0
    {
        Tree_d_item_delete(this);
        return Tree_d_item_create(CONST, 0, NULL, NULL);
    }

    if(this->type == OP && IS_CONST(this->left) && this->right == NULL)
    {
        return this;

        float a = this->left->data.val;

        Tree_d_item_delete(this);

        switch(this->data.code)
        {
        case F_SIN:
            //return CONST(sin(a));
            break;

        case F_COS:
            //return CONST(cos(a));
            break;

        case F_LN:
            //return CONST(ln(a));
            //return CONST(log(a));
            break;
        }
    }

    if(this->type == OP && IS_CONST(this->left) && IS_CONST(this->right))
    {
        float a = this->left->data.val;
        float b = this->right->data.val;

        //Tree_d_item_delete(this);

        switch(this->data.code)
        {
        case '+':
            return CONST(a + b);
            break;

        case '-':
            return CONST(a - b);
            break;

        case '*':
            return CONST(a * b);
            break;

        case '/':
            //return CONST(a / b);
            return this;
            break;

        case '^':
            return CONST(pow(a, b));
            break;

        case F_LOG:
            //return CONST(log(b)/log(a));
            return this;
            break;
        }
    }

    return this;
}

void Tree_d_ease(struct Tree_d* this)
{
    assert(this != NULL);

    this->root = Tree_d_item_ease(this->root);
    return;
}



int main()
{
    //char s[MAX_STR_LEN] = "espeak";
    //system(s);

    struct Tree_d original = {};
    struct Tree_d diff = {};

    Tree_d_load(&original);
    Tree_d_diff(&original, &diff);

    Tree_d_dump(&original);
    Tree_d_dump(&diff);

    Tree_d_ease(&diff);
    Tree_d_dump(&diff);

    Tree_d_destructor(&diff);
    Tree_d_destructor(&original);
    return 0;
}

