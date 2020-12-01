/*
  Peter Faulkner
  Final Project
  main.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <f3d_uart.h>
#include <f3d_led.h>
#include <f3d_lcd_sd.h>
#include <f3d_accel.h>
#include <f3d_i2c.h>
#include <pokemon.h>
#include <f3d_user_btn.h>
#include <f3d_delay.h>



void grass_init(grass* g, int x1, int y1, int x2, int y2){
  g->x1 = x1;
  g->x2 = x2;
  g->y1 = y1;
  g->y2 = y2;
}

void zone_init(zone* z, int current_zone){
  grass *grass1 = malloc (sizeof (struct Grass));
  grass *grass2 = malloc (sizeof (struct Grass));
  grass *grass3 = malloc (sizeof (struct Grass));
  grass *grass4 = malloc (sizeof (struct Grass));
  grass *grass5 = malloc (sizeof (struct Grass));

  if(current_zone == 1){
    z->size = 1;
    grass_init(grass1,16,16,112,64);
    z->grassArray[0] = grass1;
  }
  else if(current_zone == 2){
    z->size = 2;
    grass_init(grass1,0,16,128,64);
    grass_init(grass2,0,96,128,160);
    z->grassArray[0] = grass1;
    z->grassArray[1] = grass2; 
  }
  else if(current_zone == 3){
    z->size = 3;
    grass_init(grass1,0,16,32,160);
    grass_init(grass2,64,80,128,160);
    grass_init(grass3,32,16,96,48);
    z->grassArray[0] = grass1;
    z->grassArray[1] = grass2;
    z->grassArray[2] = grass3;
  }
  else if(current_zone == 4){
    z->size = 4;
    grass_init(grass1,0,16,128,48);
    grass_init(grass2,96,48,128,128);
    grass_init(grass3,0,128,128,160);
    grass_init(grass4,0,64,64,112);
    z->grassArray[0] = grass1;
    z->grassArray[1] = grass2;
    z->grassArray[2] = grass3;
    z->grassArray[3] = grass4;
  }
  else if(current_zone == 5){
    z->size = 5;
    grass_init(grass1,0,16,48,64);
    grass_init(grass2,80,16,128,64);
    grass_init(grass3,48,64,80,112);
    grass_init(grass4,0,112,48,160);
    grass_init(grass5,80,112,128,160);
    z->grassArray[0] = grass1;
    z->grassArray[1] = grass2;
    z->grassArray[2] = grass3;
    z->grassArray[3] = grass4;
    z->grassArray[4] = grass5;
  }
}

void drawZone(zone* z){
  int i;
  for(i = 0; i < z->size; i ++){
    drawGrass(z->grassArray[i]);
  }
}

void board_init(player* p, zone *z,int current_zone){
  f3d_lcd_fillScreen(WHITE);
  f3d_lcd_draw_textbox();
  drawZone(z);
  drawPlayer(p);
  sprintf(z->printZone, "Zone: %d", current_zone);
  sprintf(p->printCaught, "Caught: %d", p->caught);
  f3d_lcd_drawString(80,0,z->printZone,BLACK,WHITE);
  f3d_lcd_drawString(0,0,p->printCaught,BLACK,WHITE);
}

void pokedex_init(player* p) {
  char print_me[1];
  f3d_lcd_fillScreen(WHITE);
  drawPokemon(0,5,5);
  sprintf(print_me, "%d", p->dex[0]);
  f3d_lcd_drawString(35,5,print_me,BLACK,WHITE);
  drawPokemon(1,75,5);
  sprintf(print_me, "%d", p->dex[1]);
  f3d_lcd_drawString(105,5,print_me,BLACK,WHITE);
  drawPokemon(2,5,50);
  sprintf(print_me, "%d", p->dex[2]);
  f3d_lcd_drawString(35,50,print_me,BLACK,WHITE);
  drawPokemon(3,75,50);
  sprintf(print_me, "%d", p->dex[3]);
  f3d_lcd_drawString(105,50,print_me,BLACK,WHITE);
  drawPokemon(4,5,100);
  sprintf(print_me, "%d", p->dex[4]);
  f3d_lcd_drawString(35,100,print_me,BLACK,WHITE);
  while(1){if (user_btn_read()) {break;}}
}

int main (void){

  // init tools
  f3d_uart_init();
  f3d_user_btn_init();
  f3d_led_init();
  f3d_lcd_init();
  f3d_i2c1_init();
  f3d_accel_init();
  setvbuf(stdin, NULL, _IONBF, 0);
  setvbuf(stdout, NULL, _IONBF, 0);
  setvbuf(stderr, NULL, _IONBF, 0);

  // init variables and player struct
  player * test = malloc (sizeof (struct Player));
  pokemon * test_pokemon = malloc (sizeof (struct Pokemon));
  zone * zone_test = malloc (sizeof (struct Zone));
  int input, state;
  int current_zone = 1;
  enum {ZONE, BATTLE, POKEDEX, END};
  
  // init board and player
  zone_init(zone_test, current_zone);
  player_init(test);
  board_init(test, zone_test, current_zone);
  state = ZONE;
  
  while(1){
    switch(state){
    case ZONE:
      input = getchar_pls();
      if(input == 'w' || input == 'a' ||  input == 's' || input == 'd'){
	if(move_player(zone_test,test, input)){
	  state = BATTLE;
	}
      }
      else if (input == 'p') {
	state = POKEDEX;
      }  
      break;
    case BATTLE:
      battle_screen_init();
      pokemon_battle_init(test_pokemon, test);
      if(test->pokeballs <= 7){
	current_zone = current_zone +1;
	if(current_zone > 5){
	   state = END;
	   break;
	}else{
	   zone_init(zone_test,current_zone);
	   test->pokeballs = 8;
           f3d_led_all_on();
	 }
      }
      board_init(test,zone_test,current_zone);
      state = ZONE;
      break;
    case POKEDEX:
      pokedex_init(test);
      board_init(test, zone_test,current_zone);
      state = ZONE;
      break;
    case END:
      endscreen(test);
    }
  }
}

void endscreen(player* p){
  f3d_lcd_fillScreen(WHITE);
  int i;
  int score = 0;
  char toBePrinted[32];
  int scores[5] = {3,2,6,4,7};
  for (i=0; i<5; i++) {
    score = score + (p->dex[i] * scores[i]);
  }
  sprintf(toBePrinted, "Score is: %d",score );
  f3d_lcd_drawString(10,50,toBePrinted, BLACK,WHITE);
  while(1);
}

void assert_failed(uint8_t* file, uint32_t line) {
  while(1);
}
