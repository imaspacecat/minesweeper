#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "raylib.h"
#include "mine.h"
#include <string.h>

int smalloc_id = 0;
int buffer = 7777;
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

Node *create_board_points(Board *board)
{
    Node *head = NULL;
    Node *current = NULL;
    Node *new_node = NULL;

    for (int i = 0; i < board->w; i++)
    {
        for (int j = 0; j < board->h; j++)
        {
            new_node = create_node(i, j, NULL);
            if (head == NULL)
            {
                head = new_node;
                current = head;
            }
            else
            {
                current->next = new_node;
                current = new_node;
            }
        }
    }

    return head;
}

const int directions[8][2] = {
    {0, 1},
    {1, 1},
    {1, 0},
    {1, -1},
    {0, -1},
    {-1, -1},
    {-1, 0},
    {-1, 1},
};

void init_board(Board *board, int mine_count)
{
    for (int i = 0; i < board->w; i++)
        for (int j = 0; j < board->h; j++)
            board->arr[i][j] = 0;

    Node *head = create_board_points(board);
    Point points[mine_count];
    int x, y;
    int rand_i;
    for (int i = 0; i < mine_count; i++)
    {
        rand_i = rand() % (board->w * board->h - i);
        Node *rand_point = get_node(head, rand_i);
        x = rand_point->p->x;
        y = rand_point->p->y;
        points[i].x = x;
        points[i].y = y;

        remove_node(head, rand_point);

        for (int i = 0; i < 8; i++)
        {
            int newX = x + directions[i][0];
            int newY = y + directions[i][1];
            if (newX >= 0 && newX < board->w && newY >= 0 && newY < board->h)
            {
                board->arr[newX][newY] += 1;
            }
        }
    }

    free_nodes(head);
    for (int i = 0; i < mine_count; i++)
    {
        board->arr[points[i].x][points[i].y] = -1;
    }
}

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

void reveal_adjacent_empty(Board *revealed, Board *board, int x, int y)
{
    revealed->arr[x][y] = board->arr[x][y];

    for (int i = 0; i < 8; i++)
    {
        int newX = x + directions[i][0];
        int newY = y + directions[i][1];
        if (newX >= 0 && newX < board->w && newY >= 0 && newY < board->h)
        {
            if ((board->arr[newX][newY] == 0 && revealed->arr[newX][newY] != 0 && revealed->arr[x][y] == 0) || (!(board->arr[newX][newY] <= 0) && revealed->arr[x][y] == 0))
            {
                reveal_adjacent_empty(revealed, board, newX, newY);
            }
        }
    }
}

const int board_width = 20;
const int board_height = 40;

const int screen_width = 1000;
const int screen_height = (board_width / (float)board_height) * screen_width;
// const int screen_height = (board_width * screen_width) / board_height;

void draw_text_centered(const char *s, int font_size, Color color, int x_offset, int y_offset)
{
    Vector2 text_size = MeasureTextEx(GetFontDefault(), s, font_size, 0);
    DrawText(s, screen_width / 2 - text_size.x / 2 + x_offset, screen_height / 2 - text_size.y / 2 + y_offset, font_size, color);
}

const int square_size = screen_height / board_width;
/*Beginner is usually on an 8x8 or 9x9 board containing 10 mines,
Intermediate is usually on a 16x16 board with 40 mines and expert is usually on a 30x16 board with 99 mines*/
const int mine_count = 99;

int main(void)
{
    time_t t;
    srand((unsigned)time(&t));

    Board *board = create_board(board_width, board_height);
    init_board(board, mine_count);
    print_board(board);

    InitWindow(screen_width, screen_height, "sweep the mines");
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI);
    SetTargetFPS(60);

    Image image = LoadImage("/home/spacecat/code/minesweeper/resources/flag.png");
    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);

    Board *revealed = create_board(board_width, board_height);
    for (int i = 0; i < board_width; i++)
        for (int j = 0; j < board_height; j++)
            revealed->arr[i][j] = -2;

    bool reveal_bombs = false;
    bool all_bombs_revealed = false;
    while (!WindowShouldClose())
    {
        BeginDrawing();

        if (all_bombs_revealed)
        {
            draw_text_centered("YOU LOST", 60, RED, 0, 0);
            draw_text_centered("Press ENTER to play again", 40, GREEN, 0, 60);
            draw_text_centered("Press Q to quit", 40, WHITE, 0, 120);

            if (IsKeyPressed(KEY_Q))
            {
                break;
            }

            if (IsKeyPressed(KEY_ENTER))
            {
                init_board(board, mine_count);
                for (int i = 0; i < board_width; i++)
                    for (int j = 0; j < board_height; j++)
                        revealed->arr[i][j] = -2;
                reveal_bombs = false;
                all_bombs_revealed = false;
            }

            ClearBackground(BLACK);
        }
        else
        {
            for (int i = 0; i < board->w; i++)
            {
                for (int j = 0; j < board->h; j++)
                {
                    Rectangle rect = {(float)(j * square_size), (float)(i * square_size), (float)(square_size),
                                      (float)(square_size)};

                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                    {
                        Vector2 mouse = {GetMouseX(), GetMouseY()};
                        if (CheckCollisionPointRec(mouse, rect))
                        {
                            revealed->arr[i][j] = board->arr[i][j];
                            if (revealed->arr[i][j] == 0)
                            {
                                // reveal all adjacent empty squares and bordering number squares
                                reveal_adjacent_empty(revealed, board, i, j);
                            }
                        }
                    }

                    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
                    {
                        Vector2 mouse = {GetMouseX(), GetMouseY()};
                        if (CheckCollisionPointRec(mouse, rect))
                        {
                            revealed->arr[i][j] = 10;
                        }
                    }

                    if (reveal_bombs && board->arr[i][j] == -1)
                    {
                        DrawRectangleRec(rect, RED);
                        // game over screen
                    }
                    else if (revealed->arr[i][j] == -2)
                    { // empty
                        DrawRectangleRec(rect, GRAY);
                    }
                    else if (revealed->arr[i][j] == 10)
                    {
                        Vector2 pos = {(rect.x), (rect.y)};
                        DrawRectangleRec(rect, GRAY);
                        DrawTextureEx(texture, pos, 0, 0.3, WHITE);
                    }
                    else if (revealed->arr[i][j] == 0)
                    {
                        DrawRectangleRec(rect, DARKGRAY);
                    }
                    else if (revealed->arr[i][j] != -1)
                    { // numbers
                        // DrawRectangleRec(rect, RED);
                        const char *s = TextFormat("%d", revealed->arr[i][j]);
                        int font_size = 30;
                        Vector2 text_size = MeasureTextEx(GetFontDefault(), s, font_size, 0);
                        DrawText(s, rect.x + (rect.width - text_size.x) / 2, rect.y + (rect.height - text_size.y) / 2 + 2, font_size, YELLOW);
                    }
                    else
                    { // bomb
                        DrawRectangleRec(rect, RED);
                        reveal_bombs = true;
                    }

                    DrawRectangleLinesEx(rect, 1, WHITE);
                }
                if (reveal_bombs)
                {
                    break;
                }
            }
            if (reveal_bombs)
            {
                all_bombs_revealed = true;
            }

            ClearBackground(BLACK);
        }
        EndDrawing();
    }

    CloseWindow();

    free_board(board);
    free_board(revealed);

    return 0;
}
