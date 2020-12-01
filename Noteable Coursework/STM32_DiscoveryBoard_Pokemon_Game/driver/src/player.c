/*
  Peter Faulkner
  Final Project
  player.c
*/

#include "player.h"
#include <f3d_lcd_sd.h>
#include <f3d_uart.h>
#include <f3d_led.h>
#include <stm32f30x.h>
#include <stdlib.h>

int rand_battle;

void player_init(player *p){
  p->x = 64;
  p->y = 80;
  p->pokeballs = 8;
  p->caught = 0;

  f3d_lcd_drawChar(p->x,p->y,'*',BLACK,WHITE);
  f3d_led_all_on();
}

int move_player(player *p, int input){
  
// in this function: check if moved in grass, check if on next stage door
  
  f3d_lcd_drawChar(p->x,p->y," ",WHITE,WHITE);
  
  if(input == 'a') {p->x = p->x - 3;}
  else if(input == 'd') {p->x = p->x + 3;}
  else if(input == 'w') {p->y = p->y - 3;}
  else if(input == 's') {p->y = p->y + 3;}

  if(p->x > 122){p->x = 122;}
  else if (p->x < 0){p->x = 0;}
  else if(p->y > 153){p->y = 153;}
  else if (p->y < 31){p->y = 31;}

  f3d_lcd_drawChar(p->x,p->y,'*',BLACK,WHITE);

  rand_battle = rand() % 100;
  if(rand_battle > 70){
    return 1;
  }

  return 0;
  
}
