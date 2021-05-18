#include <curses.h>
#include <string.h>
#include <stdio.h>
#include <iostream>

#define length(x) sizeof(x) / sizeof(x[0])
#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

using namespace std;

long one_sec = 606060;          //  Number of for loops in one second (approx.)
int row, col;                   //  Counters for row and column
int row_step = 2, col_step = 4; //  Some aux shit, distance to step to next cell, I guess...

int k_row_step, k_col_step; //  Actually player's position on the board
int p_to_win;               //  Points to win the game
int player = 1;             //  2 players, 1 and 2

int bsize; //  Board size
int gmode; //  Array of players' symbols
int table[7][7];

int comp_move_row = 0; //  Row of the cell where computer will place symbol
int comp_move_col = 0; //  Column of the cell where computer will place symbol

//  Initializing table with zeros (players' X and O are 1 and 2 respectively)
void fill_table(int (*table)[7], int value)
{
    for (int i = 0; i < 7; i++)
        for (int j = 0; j < 7; j++)
            table[i][j] = value;
}

//  Old function to create delay (skip by pressing any key)
// void hold(long double secs)
// {
//     for (long i = 0; i < secs * one_sec; i++)
//         getch();
// }

//  Actual function to create delay (skip by pressing any key)
void hold(double secs)
{
    timeout(secs * 1000);
    getch();
}

//  Imitation of string multiplication, except this function prints the string, not returns
void str_mult(char str, int times)
{
    for (int i = 0; i < times; i++)
        printw("%c", str);
}

//  Delimiter functions to introduce some clarity to the gaming interface and separate things (not used currently)
void delimiter_times(int times)
{
    str_mult('=', times);
    printw("\n");
}

//  Standard delimiter of length 30 characters (not used currently)
void delimiter() { delimiter_times(30); }

//  Multi string print to the window
void multi_string_print_center(int num, char **items)
{
    getmaxyx(stdscr, row, col);
    for (int i = 0; i < num; i++)
    {
        const char *line = items[i];
        mvwprintw(stdscr, row / 2 - num / 2 + i, (col - strlen(line)) / 2, "%s", line);
    }
}

//  Checks for x consecutive same-value (1 or 2) symbols in a row
short x_in_row(int x, int srow, int scol)
{
    for (int i = 1; i < x; i++)
        if (table[srow][scol + i - 1] != table[srow][scol + i] || table[srow][scol + i] == 0)
            return 0;
    return 1;
}

//  Checks for x consecutive same-value (1 or 2) symbols in all rows
short check_all_rows()
{
    int maxpad = bsize - p_to_win + 1;
    for (int i = 0; i < bsize; i++)
        for (int j = 0; j < maxpad; j++)
            if (x_in_row(p_to_win, i, j) == 1)
                return 1;
    return 0;
}

//  Checks for x consecutive same-value (1 or 2) symbols in a column
short x_in_col(int x, int srow, int scol)
{
    for (int i = 1; i < x; i++)
        if (table[srow + i - 1][scol] != table[srow + i][scol] || table[srow + i][scol] == 0)
            return 0;
    return 1;
}

//  Checks for x consecutive same-value (1 or 2) symbols in all columns
short check_all_cols()
{
    int maxpad = bsize - p_to_win + 1;
    for (int i = 0; i < maxpad; i++)
        for (int j = 0; j < bsize; j++)
            if (x_in_col(p_to_win, i, j) == 1)
                return 1;
    return 0;
}

//  Checks for x consecutive same-value (1 or 2) symbols in descending diagonal (left-to-right)
short x_in_desc_diag(int x, int srow, int scol)
{
    for (int i = 1; i < x; i++)
        if (table[srow + i - 1][scol + i - 1] != table[srow + i][scol + i] || table[srow + i][scol + i] == 0)
            return 0;
    return 1;
}

//  Checks for x consecutive same-value (1 or 2) symbols in all descending diagonals
short check_all_desc()
{
    int maxpad = bsize - p_to_win + 1;
    for (int i = 0; i < maxpad; i++)
        for (int j = 0; j < maxpad; j++)
            if (x_in_desc_diag(p_to_win, i, j) == 1)
                return 1;
    return 0;
}

//  Checks for x consecutive same-value (1 or 2) symbols in ascending diagonal (left-to-right)
short x_in_asc_diag(int x, int srow, int scol)
{
    for (int i = 1; i < x; i++)
        if (table[srow - i + 1][scol + i - 1] != table[srow - i][scol + i] || table[srow - i][scol + i] == 0)
            return 0;
    return 1;
}

//  Checks for x consecutive same-value (1 or 2) symbols in all ascending diagonals
short check_all_asc()
{
    int maxpad = bsize - p_to_win + 1;
    for (int i = 0; i < maxpad; i++)
        for (int j = 0; j < maxpad; j++)
            if (x_in_asc_diag(p_to_win, bsize - i - 1, j) == 1)
                return 1;
    return 0;
}

//  The below philosophy of computer vs player is next:
//  We assume that computer is always player 2
//  All functions have priority to place symbol offensively
//  This means if players both have 1 placed symbol to win
//  computer will place symbol and win rather than prevent other player's victory.

//  Checks whether computer or player are about to win in one move, in a single row
short one_to_win_horiz(int srow, int scol)
{
    int p1 = 0, p2 = 0, zeros = 0;
    for (int i = 0; i < p_to_win; i++)
    {
        if (table[srow][scol + i] == 0)
        {
            zeros++;
            comp_move_row = srow;
            comp_move_col = scol + i;
        }
        else if (table[srow][scol + i] == 1)
            p1++;
        else if (table[srow][scol + i] == 2)
            p2++;

        if (zeros > 1 || (p1 > 0 && p2 > 0))
            return 0;
    }
    if (p1 == p_to_win - 1)
        return 1;
    else if (p2 == p_to_win - 1)
        return 2;

    return 0;
}

//  Checks whether computer or player are about to win in one move, in a single column
short one_to_win_vert(int srow, int scol)
{
    int p1 = 0, p2 = 0, zeros = 0;
    for (int i = 0; i < p_to_win; i++)
    {
        if (table[srow + i][scol] == 0)
        {
            zeros++;
            comp_move_row = srow + i;
            comp_move_col = scol;
        }
        else if (table[srow + i][scol] == 1)
            p1++;
        else if (table[srow + i][scol] == 2)
            p2++;

        if (zeros > 1 || (p1 > 0 && p2 > 0))
            return 0;
    }
    if (p1 == p_to_win - 1)
        return 1;
    else if (p2 == p_to_win - 1)
        return 2;

    return 0;
}

//  Checks whether computer or player are about to win in one move, in a single descending diagonal (left-to-right)
short one_to_win_desc(int srow, int scol)
{
    int p1 = 0, p2 = 0, zeros = 0;
    for (int i = 0; i < p_to_win; i++)
    {

        if (table[srow + i][scol + i] == 0)
        {
            zeros++;
            comp_move_row = srow + i;
            comp_move_col = scol + i;
        }
        else if (table[srow + i][scol + i] == 1)
            p1++;
        else if (table[srow + i][scol + i] == 2)
            p2++;

        if (zeros > 1 || (p1 > 0 && p2 > 0))
            return 0;
    }
    if (p1 == p_to_win - 1)
        return 1;
    else if (p2 == p_to_win - 1)
        return 2;

    return 0;
}

//  Checks whether computer or player are about to win in one move, in a single ascending diagonal (left-to-right)
short one_to_win_asc(int srow, int scol)
{
    int p1 = 0, p2 = 0, zeros = 0;
    for (int i = 0; i < p_to_win; i++)
    {

        if (table[srow - i][scol + i] == 0)
        {
            zeros++;
            comp_move_row = srow - i;
            comp_move_col = scol + i;
        }
        else if (table[srow - i][scol + i] == 1)
            p1++;
        else if (table[srow - i][scol + i] == 2)
            p2++;

        if (zeros > 1 || (p1 > 0 && p2 > 0))
            return 0;
    }
    if (p1 == p_to_win - 1)
        return 1;
    else if (p2 == p_to_win - 1)
        return 2;

    return 0;
}

//  Checks whether computer or player are about to win in one move, in all rows
short all_one_to_win_horiz()
{
    int move_row, move_col;
    int flag = 0;

    for (int i = 0; i < bsize; i++)
    {
        for (int j = 0; j < bsize - p_to_win + 1; j++)
        {
            int res = one_to_win_horiz(i, j);
            if (res == 2)
                return 2;
            else if (res == 1)
            {
                move_row = comp_move_row;
                move_col = comp_move_col;
                flag = 1;
            }
        }
    }
    comp_move_row = move_row;
    comp_move_col = move_col;

    return flag;
}

//  Checks whether computer or player are about to win in one move, in all columns
short all_one_to_win_vert()
{
    int move_row, move_col;
    int flag = 0;

    for (int i = 0; i < bsize; i++)
    {
        for (int j = 0; j < bsize - p_to_win + 1; j++)
        {
            int res = one_to_win_vert(j, i);
            if (res == 2)
                return 2;
            else if (res == 1)
            {
                move_row = comp_move_row;
                move_col = comp_move_col;
                flag = 1;
            }
        }
    }
    comp_move_row = move_row;
    comp_move_col = move_col;

    return flag;
}

//  Checks whether computer or player are about to win in one move, in all descending diagonals
short all_one_to_win_desc()
{
    int move_row, move_col;
    int flag = 0;

    for (int i = 0; i < bsize - p_to_win + 1; i++)
    {
        for (int j = 0; j < bsize - p_to_win + 1; j++)
        {
            int res = one_to_win_desc(i, j);
            if (res == 2)
                return 2;
            else if (res == 1)
            {
                move_row = comp_move_row;
                move_col = comp_move_col;
                flag = 1;
            }
        }
    }
    comp_move_row = move_row;
    comp_move_col = move_col;

    return flag;
}

//  Checks whether computer or player are about to win in one move, in all ascending diagonals
short all_one_to_win_asc()
{
    int move_row, move_col;
    int flag = 0;

    for (int i = 0; i < bsize - p_to_win + 1; i++)
    {
        for (int j = 0; j < bsize - p_to_win + 1; j++)
        {
            int res = one_to_win_asc(bsize - 1 - i, j);
            if (res == 2)
                return 2;
            else if (res == 1)
            {
                move_row = comp_move_row;
                move_col = comp_move_col;
                flag = 1;
            }
        }
    }
    comp_move_row = move_row;
    comp_move_col = move_col;

    return flag;
}

//  Checks whether computer or player have 1 symbol left to win, prioritizing computers victory
short check_one_to_win()
{
    if (all_one_to_win_horiz() == 2 || all_one_to_win_vert() == 2 || all_one_to_win_desc() == 2 || all_one_to_win_asc() == 2)
        return 2;
    else if (all_one_to_win_horiz() == 1 || all_one_to_win_vert() == 1 || all_one_to_win_desc() == 1 || all_one_to_win_asc() == 1)
        return 1;
    return 0;
}

//  Selects a cell to place symbol (for computer)
void select_comp_cell()
{
    for (int i = 0; i < bsize; i++)
    {
        for (int j = 0; j < bsize; j++)
        {
            if (table[i][j] != 0)
                continue;

            for (int k = max(0, i - 1); k <= min(bsize - 1, i + 1); k++)
                for (int l = max(0, j - 1); l <= min(bsize - 1, j + 1); l++)
                {
                    if (table[k][l] == 2)
                        continue;

                    if (table[k][l] == 1)
                    {
                        comp_move_row = i;
                        comp_move_col = j;
                        return;
                    }
                }
        }
    }
}

//  Make move as a computer. If no one is about to win, make predefined move, otherwise make victorious move / prevent player's victory
void comp_make_move()
{
    if (check_one_to_win() == 0)
        select_comp_cell();
    if (table[comp_move_row][comp_move_col] == 0)
        table[comp_move_row][comp_move_col] = 2;
}

//  Draws arrows in menu functions
void draw_arrows(int row_arr, int col_start, int col_end)
{
    mvprintw(row_arr, col_start, ">");
    mvprintw(row_arr, col_end, "<");
}

//  Menu with multiple options and header
int choose_menu(char *header, int num, char **items)
{
    nodelay(stdscr, FALSE);
    int choice = 0;
    int key;
    int row_arr, col_start, col_end;
    int header_row = row / 2 - num / 2 - 2;
    int header_col = (col - strlen(header)) / 2;

    while (key != 10) // 10 is code of Enter
    {
        erase();
        mvprintw(header_row, header_col, header);
        multi_string_print_center(num, items);
        row_arr = row / 2 - num / 2 + choice;
        col_start = (col - strlen(items[choice])) / 2 - 1;
        col_end = col_start + strlen(items[choice]) + 1;
        draw_arrows(row_arr, col_start, col_end);

        switch (key = getch())
        {
        case 'w':
            choice--;
            break;
        case 's':
            choice++;
            break;
        }
        choice = choice < 0 ? num - 1 : choice;
        choice %= num;
    }
    return choice;
}

//  Places active player's symbol
void place_symbol()
{
    table[k_row_step][k_col_step] = player == 1 ? 1 : 2;
}

//  Changes player from 1 to 2 and back, after one's move
void change_player()
{
    player = player == 1 ? 2 : 1;
}

//  Draws board of size bsize
void draw_board()
{
    int bsize_2 = (bsize - 1) * (bsize - 1);
    int init_row = row / 2 - 2 * (bsize / 2) + 1;
    int init_col = col / 2 - 4 * (bsize / 2);
    for (int i = 0; i < bsize - 1; i++)
    {
        move(init_row + 2 * i, init_col - 1);
        hline(ACS_HLINE, bsize * 4 - 1);
    }

    for (int i = 0; i < bsize - 1; i++)
    {
        move(init_row - 1, init_col + 2 + 4 * i);
        vline(ACS_VLINE, bsize * 2 - 1);
    }
}

//  Draws existing symbols on the table on the screen
void draw_symbols()
{
    for (int i = 0; i < bsize; i++)
    {
        for (int j = 0; j < bsize; j++)
        {
            if (table[i][j] == 1)
            {
                mvaddch(row / 2 + (i - bsize / 2) * row_step, col / 2 + (j - bsize / 2) * col_step, 'X');
            }
            else if (table[i][j] == 2)
            {
                mvaddch(row / 2 + (i - bsize / 2) * row_step, col / 2 + (j - bsize / 2) * col_step, 'O');
            }
        }
    }
}

//  Player's icon spawns right in the center and blinks, prompting player to choose the square
void select_square(int player)
{
    int key;
    char symbol = player == 1 ? 'X' : 'O';
    k_row_step = bsize / 2, k_col_step = bsize / 2;

    while (!(key == 10 && table[k_row_step][k_col_step] == 0))
    {
        erase();
        draw_board();
        draw_symbols();
        mvaddch(row / 2 + (k_row_step - bsize / 2) * row_step, col / 2 + (k_col_step - bsize / 2) * col_step, symbol | A_BLINK);

        switch (key = getch())
        {
        case 'w':
            k_row_step--;
            break;
        case 's':
            k_row_step++;
            break;
        case 'a':
            k_col_step--;
            break;
        case 'd':
            k_col_step++;
            break;
        }

        k_row_step = k_row_step > bsize - 1 ? bsize - 1 : k_row_step;
        k_col_step = k_col_step > bsize - 1 ? bsize - 1 : k_col_step;
        k_row_step = k_row_step < 0 ? 0 : k_row_step;
        k_col_step = k_col_step < 0 ? 0 : k_col_step;
    }
}

//  Checks whether players have p_to_win symbols in a row
short check_winner()
{
    if (check_all_rows() || check_all_cols() || check_all_desc() || check_all_asc())
        return 1;
    return 0;
}

//  Checks if all fields are non-empty and returns 1 if they are filled, meaning that there are no places to put new symbol
short check_tie()
{
    for (int i = 0; i < bsize; i++)
        for (int j = 0; j < bsize; j++)
            if (table[i][j] == 0)
                return 0;
    return 1;
}
