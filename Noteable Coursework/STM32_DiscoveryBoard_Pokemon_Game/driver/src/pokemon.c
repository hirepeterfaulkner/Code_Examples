/*
  Peter Faulkner
  Final Project
  pokemon.c
*/

#include <f3d_user_btn.h>
#include <pokemon.h>
#include <stdio.h>
#include <stdlib.h>
#include <f3d_lcd_sd.h>
#include <f3d_uart.h>
#include <f3d_led.h>
#include <f3d_i2c.h>
#include <f3d_accel.h>
#include <math.h>
#include <draw.h>

// pokemon attributed
char *pokemon_names[] = {"bulbasaur", "togepi", "jigglypuff", "pikachu",
			 "snorlax"};
int tilt_options[] = {0,1,2,3};
char toBePrinted[30];

// variables for user inputs during battle
int choice,input;

// player's int to check if battle occurs
int rand_battle;

void player_init(player *p){
  p->x = 64;
  p->y = 80;
  p->pokeballs = 8;
  p->caught = 0;
  p->dex[0] = 0;
  p->dex[1] = 0;
  p->dex[2] = 0;
  p->dex[3] = 0;
  p->dex[4] = 0;
  f3d_led_all_on();
}

void pokemon_battle_init(pokemon *poke,player *p){
  f3d_user_btn_init();
  
  // create pokemon
  int target_pokemon = rand() % 5;
  poke->target_pokemon = target_pokemon;
  sprintf(poke->name, "%s", pokemon_names[target_pokemon]);
  poke->tilt = tilt_options[rand() % 4];
  sprintf(toBePrinted, "%s appeared!", poke->name);
  f3d_lcd_drawString(5,50,toBePrinted, BLACK,WHITE);
  drawPokemonOutline(target_pokemon);

  // draw catch / run options
  f3d_lcd_drawString(20,80, "catch", WHITE, BLACK);
  f3d_lcd_drawString(80,80, "run", BLACK, WHITE);
  
  // wait for user to choose catch or run
  choice = 0;
  while(1){

    input = getchar_pls();
    if(input == 'a' || input == 'd'){
      choice += 1;
      if(choice%2 == 1){
	f3d_lcd_drawString(20,80, "catch", BLACK, WHITE);
	f3d_lcd_drawString(80,80, "run", WHITE, BLACK);
      }else{
	f3d_lcd_drawString(20,80, "catch", WHITE, BLACK);
	f3d_lcd_drawString(80,80, "run", BLACK, WHITE); 
      }
    }
    // if player hits enter
    else if(input == 13){
    // if run is selected, break and return to zone
      if(choice%2 == 1){
	break;
      }
    // else catch was selected
      else{
	int got_away = rand() % 100;
	// check if player has balls
	if(p->pokeballs <= 0){
	  f3d_lcd_drawString(10,90,"No Pokeballs Left!",BLACK,WHITE);
	}
        else if(catch_phase_init(poke)){
	  f3d_lcd_fillScreen(WHITE);
	  drawPokeball();
	  sprintf(toBePrinted,"%s was caught!", poke->name);
	  f3d_lcd_drawString(0,50,toBePrinted,BLACK,WHITE);
	  delay(2000);
	  p->pokeballs = p->pokeballs - 1;
	  f3d_led_off(p->pokeballs);
	  p->caught = p->caught + 1;
	  p->dex[target_pokemon] = p->dex[target_pokemon] + 1;
	  break;
	}
	else if(got_away > 50){
	  p->pokeballs = p->pokeballs - 1;
          f3d_led_off(p->pokeballs);
	  sprintf(toBePrinted,"%s escaped!", poke->name);
	  f3d_lcd_drawString(0,80,toBePrinted,BLACK,WHITE);
	  delay(2000);
	  break;
	}else{
	  p->pokeballs = p->pokeballs - 1;
	  f3d_led_off(p->pokeballs);
	  sprintf(toBePrinted, "%s", "catch failed");
          f3d_lcd_drawString(0,80,toBePrinted,BLACK,WHITE);
	  delay(2000);

          // redraw catch / run options
          f3d_lcd_fillScreen(WHITE);
  	  sprintf(toBePrinted, "%s appeared!", poke->name);
  	  f3d_lcd_drawString(5,50,toBePrinted, BLACK,WHITE);
  	  drawPokemonOutline(target_pokemon);
	  f3d_lcd_drawString(20,80, "catch", WHITE, BLACK);
          f3d_lcd_drawString(80,80, "run", BLACK, WHITE);
	}
      }
    }
  }
}

int catch_phase_init(pokemon *poke){
  f3d_lcd_fillScreen(WHITE);
  drawPokemonOutline(poke->target_pokemon);
  f3d_accel_init();

  f3d_lcd_drawString(10,50,"User button=catch",BLACK,WHITE);
  f3d_lcd_drawString(10,60,"Tilt board=+chance",BLACK,WHITE);
  
  float accel_data[3];
  float pitchX;
  float rollY;

  int flag = 0;
  int catch_attempt;

  while(1){
    f3d_accel_read(accel_data);
    pitchX = atan(accel_data[0]/ sqrt( pow(accel_data[1], 2)
				    + pow(accel_data[2], 2) )) * 180 / M_PI;

    rollY = atan(accel_data[1]/ sqrt( pow(accel_data[0], 2)
				    + pow(accel_data[2], 2) )) * 180 / M_PI;

    switch(poke->tilt){
    case 0:
      if(pitchX >= 30){flag = 1;}
      break;
    case 1:
      if(pitchX <= -30){flag = 1;}
      break;
    case 2:
      if(rollY >= 30){flag = 1;}
      break;
    case 3:
      if(rollY <= -30){flag = 1;}
      break;
    }
    if(flag){
      drawPokemon(poke->target_pokemon,50,5);
      if(user_btn_read()){
	catch_attempt = rand() % 100;
	if(catch_attempt > 10){
	  return 1;
	}
	else{return 0;}
      }
    }
    else{
      if(user_btn_read()){
	catch_attempt = rand() % 100;
	if(catch_attempt > 60){
	  return 1;
	}
	else{return 0;}
      }
    }
  }
}

int move_player(zone * z, player *p, int input){
  int grassFlag = 0;

  int i;
  for(i = 0; i < z->size; i++){
    if(checkGrass(z->grassArray[i],p))
      grassFlag = 1;
  }
  if(grassFlag){
    undrawPlayerGrass(p);
  }else{
    undrawPlayer(p);
  }
  
  if(input == 'a') {p->x = p->x - 16;}
  else if(input == 'd') {p->x = p->x + 16;}
  else if(input == 'w') {p->y = p->y - 16;}
  else if(input == 's') {p->y = p->y + 16;}

  if(p->x > 112){p->x = 112;}
  else if (p->x < 0){p->x = 0;}
  else if(p->y > 144){p->y = 144;}
  else if (p->y < 16){p->y = 16;}

  if(grassFlag){
    rand_battle = rand() % 100;
    if(rand_battle > 70){
      return 1;
    }
  }
  drawPlayer(p);
  return 0;
}

void drawGrass(grass * g){
  int i,j;
  for(i = g->y1; i < g->y2; i++){
    for(j = g->x1; j < g->x2; j++){
      f3d_lcd_drawPixel(j,i,GREEN);
    }
  }
}

int checkGrass(grass *g, player * p){
  if(p->x < g->x2 && p->x >= g->x1 && p->y >= g->y1 && p->y < g->y2){
    return 1;
  }
  return 0;
}

