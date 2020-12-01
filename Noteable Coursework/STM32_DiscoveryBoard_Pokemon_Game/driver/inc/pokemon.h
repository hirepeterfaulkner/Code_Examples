/*
  Peter Faulkner
  Final Project
  pokemon.h
*/

#define MAX_SIZE 20

typedef struct Pokemon {
  char name[MAX_SIZE];
  int tilt;
  int target_pokemon;
} pokemon;

typedef struct Player{
  int x;
  int y;
  int pokeballs;
  int caught;
  int dex[5];
  char printCaught[12];
}player;

typedef struct Grass{
  int x1,y1,x2,y2;
}grass;

typedef struct Zone{
  char printZone[8];
  grass *grassArray[5];
  int size;	
}zone;

void player_init(player*);
int move_player(zone*,player*, int);
void pokemon_battle_init(pokemon*, player*);
int catch_phase_init(pokemon*);
void drawGrass(grass *);
int checkGrass(grass *, player *);
