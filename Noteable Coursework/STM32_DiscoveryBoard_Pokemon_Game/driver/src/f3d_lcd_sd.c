/* f3d_lcd_sd.c --- 
 * 
 * Filename: f3d_lcd_sd.c
 * Description: 
 * Author: Bryce Himebaugh
 * Maintainer: 
 * Created: Thu Oct 24 05:18:36 2013
 * Last-Updated: 
 *           By: 
 *     Update #: 0
 * Keywords: 
 * Compatibility: 
 * 
 */

/* Commentary: 
 * 
 * 
 * 
 */

/* Change log:
 * 
 * 
 */

/* Copyright (c) 2004-2007 The Trustees of Indiana University and 
 * Indiana University Research and Technology Corporation.  
 * 
 * All rights reserved. 
 * 
 * Additional copyrights may follow 
 */

/* Code: */
#include <f3d_lcd_sd.h>
#include <f3d_delay.h>
#include <glcdfont.h>

static uint8_t madctlcurrent = MADVAL(MADCTLGRAPHICS);

void f3d_lcd_sd_interface_init(void) {
 /* vvvvvvvvvvv pin initialization for the LCD goes here vvvvvvvvvv*/ 
  
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_StructInit(&GPIO_InitStructure);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

  // pins 9 10 11 and 12
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11| GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOB,&GPIO_InitStructure);

  GPIO_PinAFConfig(GPIOB,9,GPIO_AF_15);
  GPIO_PinAFConfig(GPIOB,10,GPIO_AF_15);
  GPIO_PinAFConfig(GPIOB,11,GPIO_AF_15);
  GPIO_PinAFConfig(GPIOB,12,GPIO_AF_15);

  // pins 13 14 and 15
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOB,&GPIO_InitStructure);

  GPIO_PinAFConfig(GPIOB,13,GPIO_AF_5);
  GPIO_PinAFConfig(GPIOB,14,GPIO_AF_5);
  GPIO_PinAFConfig(GPIOB,15,GPIO_AF_5);
  
  /* ^^^^^^^^^^^ pin initialization for the LCD goes here ^^^^^^^^^^ */
 
  // Section 4.1 SPI2 configuration
  // Note: you will need to add some code in the last three functions
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2 , ENABLE);
  SPI_InitTypeDef SPI_InitStructure;
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_Init(SPI2, &SPI_InitStructure);
  SPI_RxFIFOThresholdConfig(SPI2, SPI_RxFIFOThreshold_QF);
  SPI_Cmd(SPI2, ENABLE);
  
} 


struct lcd_cmdBuf {
  uint8_t command;
  uint8_t delay;
  uint8_t len;
  uint8_t data [16];
};

static const struct lcd_cmdBuf initializers[] = {
  // SWRESET Software reset
  { 0x01, 150, 0, 0},
  // SLPOUT Leave sleep mode
  { 0x11, 150, 0, 0},
  // FRMCTR1, FRMCTR2 Frame Rate configuration -- Normal mode, idle
  // frame rate = fosc / (1 x 2 + 40) * (LINE + 2C + 2D)
  { 0xB1, 0, 3, { 0x01, 0x2C, 0x2D }},
  { 0xB2, 0, 3, { 0x01, 0x2C, 0x2D }},
  // FRMCTR3 Frame Rate configureation -- partial mode
  { 0xB3, 0, 6, { 0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D }},
  // INVCTR Display inversion (no inversion)
  { 0xB4, 0, 1, { 0x07 }},
  /* ... */
  /*! llcmdstop2 !*/
  /*! llcmdstart3 !*/
  // PWCTR1 Power control -4.6V, Auto mode
  { 0xC0, 0, 3, { 0xA2, 0x02, 0x84}},
  // PWCTR2 Power control VGH25 2.4C, VGSEL -10, VGH = 3 * AVDD
  { 0xC1, 0, 1, { 0xC5}},
  // PWCTR3 Power control, opamp current smal, boost frequency
  { 0xC2, 0, 2, { 0x0A, 0x00 }},
  // PWCTR4 Power control, BLK/2, opamp current small and medium low
  { 0xC3, 0, 2, { 0x8A, 0x2A}},
  // PWRCTR5, VMCTR1 Power control
  { 0xC4, 0, 2, { 0x8A, 0xEE}},
  { 0xC5, 0, 1, { 0x0E }},
  // INVOFF Don't invert display
  { 0x20, 0, 0, 0},
  // Memory access directions. row address/col address, bottom to top refesh (10.1.27)
  { ST7735_MADCTL, 0, 1, {MADVAL(MADCTLGRAPHICS)}},
  // Color mode 16 bit (10.1.30
  { ST7735_COLMOD, 0, 1, {0x05}},
  // Column address set 0..127
  { ST7735_CASET, 0, 4, {0x00, 0x00, 0x00, 0x7F }},
  // Row address set 0..159
  { ST7735_RASET, 0, 4, {0x00, 0x00, 0x00, 0x9F }},
  // GMCTRP1 Gamma correction
  { 0xE0, 0, 16, {0x02, 0x1C, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2D,
0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10 }},
  // GMCTRP2 Gamma Polarity corrction
  { 0xE1, 0, 16, {0x03, 0x1d, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D,
0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10 }},
  // DISPON Display on
  { 0x29, 100, 0, 0},
  // NORON Normal on
  { 0x13, 10, 0, 0},
  // End
  { 0, 0, 0, 0}
};

void f3d_lcd_init(void) {
  const struct lcd_cmdBuf *cmd;

  f3d_lcd_sd_interface_init();    // Setup SPI2 Link and configure GPIO pins
  LCD_BKL_ON();                   // Enable Backlight

  // Make sure that the chip select and reset lines are deasserted
  LCD_CS_DEASSERT();              // Deassert Chip Select

  LCD_RESET_DEASSERT();           
  delay(100);
  LCD_RESET_ASSERT();
  delay(100);
  LCD_RESET_DEASSERT();
  delay(100);

  // Write initialization sequence to the lcd
  for (cmd=initializers; cmd->command; cmd++) {
    LcdWrite(LCD_C,&(cmd->command),1);
    if (cmd->len)
      LcdWrite(LCD_D,cmd->data,cmd->len);
    if (cmd->delay) {
      delay(cmd->delay);
    }
  }
}

static void LcdWrite(char dc,const char *data,int nbytes) {
  GPIO_WriteBit(LCD_PORT,GPIO_PIN_DC,dc); // dc 1 = data , 0 = control
  GPIO_ResetBits(LCD_PORT,GPIO_PIN_SCE);
  spiReadWrite(SPILCD,0,data,nbytes,LCDSPEED);
  GPIO_SetBits(LCD_PORT,GPIO_PIN_SCE);
}

static void LcdWrite16(char dc,const uint16_t *data,int cnt) {
  GPIO_WriteBit(LCD_PORT,GPIO_PIN_DC,dc); 
  GPIO_ResetBits(LCD_PORT,GPIO_PIN_SCE);
  spiReadWrite16(SPILCD,0,data,cnt,LCDSPEED);
  GPIO_SetBits(LCD_PORT,GPIO_PIN_SCE);
}

int spiReadWrite(SPI_TypeDef *SPIx,uint8_t *rbuf, const uint8_t *tbuf, int cnt, uint16_t speed) {
  int i;
  int timeout;

  SPIx->CR1 = (SPIx->CR1&~SPI_BaudRatePrescaler_256)|speed;
  for (i = 0; i < cnt; i++){
    if (tbuf) {
      SPI_SendData8(SPIx,*tbuf++);
    } 
    else {
      SPI_SendData8(SPIx,0xff);
    }
    timeout = 100;
    while (SPI_I2S_GetFlagStatus(SPIx,SPI_I2S_FLAG_RXNE) == RESET);
    if (rbuf) {
      *rbuf++ = SPI_ReceiveData8(SPIx);
    } 
    else {
      SPI_ReceiveData8(SPIx);
    }
  }
  return i;
}

int spiReadWrite16(SPI_TypeDef *SPIx,uint8_t *rbuf, const uint16_t *tbuf, int cnt, uint16_t speed) {
  int i;
  
  SPIx->CR1 = (SPIx->CR1&~SPI_BaudRatePrescaler_256)|speed;
  SPI_DataSizeConfig(SPIx, SPI_DataSize_16b);

  for (i = 0; i < cnt; i++){
    if (tbuf) {
      //      printf("data=0x%4x\n\r",*tbuf);
      SPI_I2S_SendData16(SPIx,*tbuf++);
    } 
    else {
      SPI_I2S_SendData16(SPIx,0xffff);
    }
    while (SPI_I2S_GetFlagStatus(SPIx,SPI_I2S_FLAG_RXNE) == RESET);
    if (rbuf) {
      *rbuf++ = SPI_I2S_ReceiveData16(SPIx);
    } 
    else {
      SPI_I2S_ReceiveData16(SPIx);
    }
  }
  SPI_DataSizeConfig(SPIx, SPI_DataSize_8b);

  return i;
}

void f3d_lcd_setAddrWindow ( uint16_t x0 , uint16_t y0 , uint16_t x1 , uint16_t y1 , uint8_t madctl) {
  madctl = MADVAL ( madctl );
  if ( madctl != madctlcurrent ){
    f3d_lcd_writeCmd(ST7735_MADCTL);
    LcdWrite(LCD_D, &madctl, 1);
    madctlcurrent = madctl ;
  }
  f3d_lcd_writeCmd(ST7735_CASET);
  LcdWrite16(LCD_D,&x0,1);
  LcdWrite16(LCD_D,&x1,1);
  f3d_lcd_writeCmd(ST7735_RASET);
  LcdWrite16(LCD_D,&y0,1);
  LcdWrite16(LCD_D,&y1,1);
  f3d_lcd_writeCmd(ST7735_RAMWR);
}

void f3d_lcd_pushColor(uint16_t *color,int cnt) {
  LcdWrite16(LCD_D,color,cnt);
}

static void f3d_lcd_writeCmd(uint8_t c) {
  LcdWrite(LCD_C,&c,1);
}

void f3d_lcd_fillScreen(uint16_t color) {
  uint8_t y;
  uint16_t x[ST7735_width];
  for (y = 0; y < ST7735_width; y++) x[y] = color;
  f3d_lcd_setAddrWindow (0,0,ST7735_width-1,ST7735_height-1,MADCTLGRAPHICS);
  for (y=0;y<ST7735_height; y++) {
    f3d_lcd_pushColor(x,ST7735_width);
  }
}

void f3d_lcd_drawPixel(uint8_t x, uint8_t y, uint16_t color) {
  if ((x >= ST7735_width) || (y >= ST7735_height)) return;
  f3d_lcd_setAddrWindow(x,y,x+1,y+1,MADCTLGRAPHICS);
  f3d_lcd_pushColor(&color,1);
}

void f3d_lcd_drawChar(uint8_t x, uint8_t y, unsigned char c, uint16_t color, uint16_t background_color) {
  int i, j;
  for (i = 0; i < 5; i++) {
    for (j = 0; j < 8; j++){ 
      f3d_lcd_drawPixel(x+i,y+j, background_color);
    }
  }
  for (i = 0; i < 5; i++) {
    uint8_t byte = ASCII[c*5 + i];
    for (j = 0; j < 8; j++){
      if (byte & (1)) {
	f3d_lcd_drawPixel(x+i,y+j, color);
      }
      byte >>= 1;
    }
  }
}

void f3d_lcd_drawString(uint8_t x, uint8_t y, char *c, uint16_t color, uint16_t background_color) {
  while (c && *c) {
    f3d_lcd_drawChar(x, y, *c++, color, background_color);
    x += 6;
    if (x + 5 >= ST7735_width) {
      y += 10;
      x = 0;
    }
  }
  f3d_lcd_drawChar(x, y,"  ", background_color, background_color);
}

void f3d_lcd_positive_drawBar(uint8_t y, uint16_t color){
  int i, j;
  for(i = 1; i <= y; i++)
    for(j = 54; j <= 74; j++){
      f3d_lcd_drawPixel(j, 80-i, color);
    } 
}

void f3d_lcd_negative_drawBar(uint8_t y, uint16_t color){
  int i, j;
  for(i = 1; i <= y; i++)
    for(j = 54; j <= 74; j++){
      f3d_lcd_drawPixel(j, 80+i, color);
    } 
}

void f3d_lcd_drawBar(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2,uint8_t length ,uint8_t direction,uint16_t color){
  int x,y;
  if(direction == 0){
    for(y = y1; y > y1-length; y--){
      for(x = x1; x < x2; x++){
	f3d_lcd_drawPixel(x,y,color);
      }
    }
  }

  else if(direction == 2){
    for(y = y1; y < y1+length; y++){
      for(x = x1; x < x2; x++){
	f3d_lcd_drawPixel(x,y,color);
      }
    }
  }
  else if(direction == 1){
    for(x = x1; x < x2+length; x++){
      for(y = y1; y < y2; y++){
	f3d_lcd_drawPixel(x,y,color);
      }
    }
  }
  else if(direction == 3){
    for(x = x1; x > x2-length; x--){
      for(y = y1; y < y2; y++){
	f3d_lcd_drawPixel(x,y,color);
      }
    }
  }

}

void f3d_lcd_drawPitch(int8_t angle){
  
  if(angle < 5 && angle >=0){
    f3d_lcd_drawBar(59,0,69,0,74,2,WHITE);
    f3d_lcd_drawBar(59,159,69,159,74,0,WHITE);
  }
  else if(angle >= 5 && angle <= 15){
    f3d_lcd_drawBar(59,0,69,0,64,2,WHITE);
    f3d_lcd_drawBar(59,74,69,74,10,0,BLUE);
  }
  else if(angle >= 16 && angle <= 30){
    f3d_lcd_drawBar(59,0,69,0,54,2,WHITE);
    f3d_lcd_drawBar(59,74,69,74,20,0,BLUE);
  }
  else if(angle >= 31 && angle <= 45){
    f3d_lcd_drawBar(59,0,69,0,44,2,WHITE);
    f3d_lcd_drawBar(59,74,69,74,30,0,BLUE);
  }
  else if(angle >= 46 && angle <= 60){
    f3d_lcd_drawBar(59,0,69,0,34,2,WHITE);
    f3d_lcd_drawBar(59,74,69,74,40,0,BLUE);
  }
  else if(angle >= 61 && angle <= 75){
    f3d_lcd_drawBar(59,0,69,0,24,2,WHITE);
    f3d_lcd_drawBar(59,74,69,74,50,0,BLUE);
  }
  else if(angle >= 76 && angle <= 90){
    f3d_lcd_drawBar(59,0,69,0,14,2,WHITE);
    f3d_lcd_drawBar(59,74,69,74,60,0,BLUE);
  }
  else if(angle <= -5 && angle >= -15 ){
    f3d_lcd_drawBar(59,159,69,159,64,0,WHITE);
    f3d_lcd_drawBar(59,85,69,85,10,2,RED);
  }
  else if(angle <= -16 && angle >= -30 ){
    f3d_lcd_drawBar(59,159,69,159,54,0,WHITE);
    f3d_lcd_drawBar(59,86,69,86,20,2,RED);
  }
  else if(angle <= -31 && angle >= -45 ){
    f3d_lcd_drawBar(59,159,69,159,34,0,WHITE);
    f3d_lcd_drawBar(59,86,69,86,30,2,RED);
  }
  else if(angle <= -46 && angle >= -60 ){
    f3d_lcd_drawBar(59,159,69,159,24,0,WHITE);
    f3d_lcd_drawBar(59,86,69,86,40,2,RED);
  }
  else if(angle <= -61 && angle >= -75 ){
    f3d_lcd_drawBar(59,159,69,159,14,0,WHITE);
    f3d_lcd_drawBar(59,86,69,86,50,2,RED);
  }
  else if(angle <= -76 && angle >= -90 ){
    f3d_lcd_drawBar(59,159,69,159,4,0,WHITE);
    f3d_lcd_drawBar(59,86,69,86,60,2,RED);
  }
}

void f3d_lcd_drawRoll(int8_t angle){
  if(angle < 5 && angle >=0){
    f3d_lcd_drawBar(127,75,127,85,59,3,WHITE);
    f3d_lcd_drawBar(0,75,0,85,59,1,WHITE);
  }
  else if(angle >= 5 && angle <= 15){
    f3d_lcd_drawBar(127,75,127,85,51,3,WHITE);
    f3d_lcd_drawBar(69,75,69,85,8,1,BLUE);
  }
  else if(angle >= 16 && angle <= 30){
    f3d_lcd_drawBar(127,75,127,85,43,3,WHITE);
    f3d_lcd_drawBar(69,75,69,85,16,1,BLUE);
  }
  else if(angle >= 31 && angle <= 45){
    f3d_lcd_drawBar(127,75,127,85,35,3,WHITE);
    f3d_lcd_drawBar(69,75,69,85,24,1,BLUE);
  }
  else if(angle >= 46 && angle <= 60){
    f3d_lcd_drawBar(127,75,127,85,27,3,WHITE);
    f3d_lcd_drawBar(69,75,69,85,32,1,BLUE);
  }
  
  else if(angle >= 61 && angle <= 75){
    f3d_lcd_drawBar(127,75,127,85,19,3,WHITE);
    f3d_lcd_drawBar(69,75,69,85,40,1,BLUE);
  }
  else if(angle >= 76 && angle <= 90){
    f3d_lcd_drawBar(127,75,127,85,11,3,WHITE);
    f3d_lcd_drawBar(69,85,69,85,48,1,BLUE);
  }
  else if(angle <= -5 && angle >= -15 ){
    f3d_lcd_drawBar(0,75,0,85,50,1,WHITE);
    f3d_lcd_drawBar(59,75,59,85,8,3,RED);
  }
  else if(angle <= -16 && angle >= -30 ){
    f3d_lcd_drawBar(0,75,0,85,42,1,WHITE);
    f3d_lcd_drawBar(59,75,59,85,16,3,RED);
  }
  else if(angle <= -31 && angle >= -45 ){
    f3d_lcd_drawBar(0,75,0,85,34,1,WHITE);
    f3d_lcd_drawBar(59,75,59,85,24,3,RED);
  }
  else if(angle <= -46 && angle >= -60 ){
    f3d_lcd_drawBar(0,75,0,85,26,1,WHITE);
    f3d_lcd_drawBar(59,75,59,85,32,3,RED);
  }
  else if(angle <= -61 && angle >= -75 ){
    f3d_lcd_drawBar(0,75,0,85,18,1,WHITE);
    f3d_lcd_drawBar(59,75,59,85,40,3,RED);
  }
  else if(angle <= -76 && angle >= -90 ){
    f3d_lcd_drawBar(0,75,0,85,10,1,WHITE);
    f3d_lcd_drawBar(59,75,59,85,48,3,RED);
  }
}

void f3d_lcd_draw_textbox(){
  int i;
  for(i = 0; i <= 127; i++)
    f3d_lcd_drawPixel(i,15,BLACK);
}

void battle_screen_init(){
  int i = 0;
  while( i < 5){
    f3d_lcd_fillScreen(RED);
    delay(100);
    f3d_lcd_fillScreen(WHITE);
    delay(100);
    i++;
  }
}

void f3d_lcd_drawNorth(int x, int y){
  f3d_lcd_drawChar(x, y, 'N', BLACK, WHITE);
}
/* f3d_lcd_sd.c ends here */
