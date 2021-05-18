#include <stdio.h>
#include <string.h>
#include <iostream>
#include <stdlib.h>
#include <curses.h>
#include "funcs.h"
using namespace std;

//  Essential functions for terminal window initialization and configuration
int init()
{
    initscr();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    return 0;
}

//  Executed in case if init() function returned 0 (successfull)
void init_ok()
{
    getmaxyx(stdscr, row, col);
    const char *mesg = "Initialization successful!";
    mvwprintw(stdscr, row / 2, (col - strlen(mesg)) / 2, "%s", mesg);
    hold(2);
    erase();
}

//  This is just greeting
void welcome()
{
    char *words[] = {"Welcome to the", "Tic-Tac-Toe game!"};
    multi_string_print_center(2, words);
    hold(3);
    erase();
}

//  Prompt player to choose game mode
int game_mode()
{
    char *words[] = {"Player vs Player", "Player vs Computer"};
    gmode = choose_menu("Choose game mode", 2, words);
    refresh();
    return gmode;
}

//  Player choses size of the board
int board_size()
{
    char *words[] = {"3x3", "5x5", "7x7"};
    bsize = choose_menu("Choose board size", 3, words);
    refresh();
    switch (bsize)
    {
    case 0:
        bsize = 3;
        break;
    case 1:
        bsize = 5;
        break;
    case 2:
        bsize = 7;
        break;
    }
    return bsize;
}

//  Here player can choose to play or exit the game
int menu()
{
    int choice;
    char *words[] = {"Enter game", "Exit"};
    choice = choose_menu("Use WASD to navigate", 2, words);
    refresh();
    return choice;
}

//  Finish the game with the final screen, with winner on it
void game_over_win()
{
    erase();
    char pl[30];
    change_player();
    snprintf(pl, 30, "Player %i is the winner!", player);
    char *words[] = {"GAME OVER!", pl, "", "Thanks for playing!"};
    multi_string_print_center(4, words);
    hold(2);
}

//  Finish the game with the final screen, tie
void game_over_tie()
{
    erase();
    char *words[] = {"GAME OVER!", "Tie!", "", "Thanks for playing!"};
    multi_string_print_center(4, words);
    hold(2);
}

//  Main loop for Player vs Player mode
void pvp()
{
    while (!check_winner() && !check_tie())
    {
        select_square(player);
        refresh();
        place_symbol();
        change_player();
    }
    if (check_winner())
        game_over_win();
    else if (check_tie())
        game_over_tie();
}

//  Main loop for Player vs Computer mode
void pvc()
{
    while (!check_winner() && !check_tie())
    {
        select_square(player);
        refresh();
        place_symbol();
        change_player();
        if (check_winner() || check_tie())
            break;

        comp_make_move();
        change_player();
    }
    if (check_winner())
        game_over_win();
    else if (check_tie())
        game_over_tie();
}

//  If player's choice is to play, ask for game mode and board size
void game()
{
    player = 1;
    game_mode();
    board_size();
    p_to_win = bsize == 3 ? 3 : 5;

    gmode == 0 ? pvp() : pvc();
}

//
//  Main function where we execute other functions in correct order to run the game properly
//
int main()
{
    int choice;
    if (init() == 0)
    {
        init_ok();
    }
    else
    {
        cout << "Initialization failed." << endl
             << "Exiting...";
        exit(1);
    }

    welcome();

    while ((choice = menu()) != 1)
    {
        fill_table(table, 0);
        game();
    }

    endwin();
    return 0;
}
