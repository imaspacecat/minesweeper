// -1: mine
// 0: empty
// 1: 1 near
// 2: 2 near
// etc
typedef struct
{
    int w;
    int h;
    int **arr;
} Board;

typedef struct
{
    short x;
    short y;
} Point;

typedef struct Node
{
    Point *p;
    struct Node *next;
} Node;