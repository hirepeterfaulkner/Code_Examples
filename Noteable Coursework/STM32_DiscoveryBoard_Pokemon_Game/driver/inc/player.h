/*
  Peter Faulkner
  Final Project
  player.h
*/

typedef struct Player{
  int x;
  int y;
  int pokeballs;
  int caught;
}player;

void player_init(player*);
int move_player(player*, int);
