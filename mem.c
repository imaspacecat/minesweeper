#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int smalloc_id = 0;
int buffer = 7777;

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

void *pmalloc(size_t data)
{
    // 4 bytes for an id, 4 bytes of 7777 to detect overflow
    void *p = malloc(4 + data + 4);
    if (!p)
    {
        perror("malloc failed");
        return NULL;
    }
    memcpy((char *)p, &smalloc_id, 4);
    memcpy(p + 4 + data, &buffer, 4);
    printf("malloc id: %d\n", smalloc_id);
    smalloc_id++;
    return (p + 4);
}

void pfree(void *p)
{
    printf("free id: %d\n", *((int *)(p - 4)));
    free(p - 4);
}

Node *create_node(int x, int y, Node *next)
{
    Node *node = pmalloc(sizeof(Node));
    node->p = malloc(sizeof(Point));
    node->next = next;
    node->p->x = x;
    node->p->y = y;
    return node;
}

void free_node(Node *node)
{
    free(node->p);
    pfree(node);
}

void free_nodes(Node *head)
{
    Node *current = head;
    while (current != NULL)
    {
        Node *next = current->next;
        free_node(current);
        current = next;
    }
}

Node *get_node(Node *head, int n)
{
    Node *current = head;
    for (int i = 0; i < n; i++)
    {
        Node *next = current->next;
        current = next;
    }

    return current;
}

void print_nodes(Node *head)
{
    Node *current = head;
    while (current != NULL)
    {
        Node *next = current->next;
        printf("x: %d, y: %d\n", current->p->x, current->p->y);
        current = next;
    }
}

void remove_node(Node *head, Node *toremove)
{
    Node *current = head;
    while (current->next != toremove)
    {
        Node *next = current->next;
        current = next;
    }
    current->next = toremove->next;
    free_node(toremove);
}

typedef struct
{
    int w;
    int h;
    int **arr;
} Board;

Board *create_board(int w, int h)
{
    Board *board = pmalloc(sizeof(Board));
    int **arr = pmalloc(w * sizeof(int *));
    for (int i = 0; i < w; i++)
        arr[i] = pmalloc(h * sizeof(int));
    board->h = h;
    board->w = w;
    board->arr = arr;
    return board;
}

void free_board(Board *board)
{
    for (int i = 0; i < board->w; i++)
        pfree(board->arr[i]);
    pfree(board->arr);
    pfree(board);
}

void print_board(Board *board)
{
    printf("w: %d, h: %d\n", board->w, board->h);
    for (int i = 0; i < board->w; i++)
    {
        for (int j = 0; j < board->h; j++)
        {
            printf("%-4d", board->arr[i][j]);
        }
        printf("\n");
    }
}

int main()
{

    int n = 100000;
    // int** ps = pmalloc(n* sizeof(int *));
    // for (int i = 0; i < n; i++)
    // {
    //     ps[i] = pmalloc(sizeof(int*));
    // }

    // for (int i = 0; i < n; i++)
    // {
    //     pfree(ps[i]);
    // }

    // Node *head = create_node(0, 0, NULL);
    // Node *current = head;
    // for (int i = 0; i < n; i++)
    // {
    //     current->next = create_node(i, i, NULL);
    //     current = current->next;
    // }

    // Node *get = get_node(head, 100);
    // printf("x: %d, y: %d\n", get->p->x, get->p->y);
    // remove_node(head, get);
    // get = get_node(head, 100);
    // printf("x: %d, y: %d\n", get->p->x, get->p->y);
    // remove_node(head, get);
    // free_nodes(head);

    Board * board = create_board(100, 100);
    // print_board(board);
    free_board(board);

    return 0;
}
