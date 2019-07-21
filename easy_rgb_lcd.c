/*
 * easy_rgb_lcd.c
 *
 *  Created on: Jul 10, 2019
 *  Author: @ArduinoEasy
 *	Github:	https://github.com/mahmood-ul-hassan/Grove_LCD_RGB_Backlight
 *	Source: Based on SeeedStudio Arduino Library 
 *			(https://github.com/Seeed-Studio/Grove_LCD_RGB_Backlight)
 *
 */

#include "easy_rgb_lcd.h"

uint8_t _displayfunction;
uint8_t _displaycontrol;
uint8_t _displaymode;

uint8_t _initialized;

uint8_t _numlines,_currline;

/*************************************************************************/
/*
 *		For STM32 MCU ONLY:
 *			Modify the I2C_HandleTypeDef according to your code
 *			Replace -> hi2c1 
 *					with your own I2C handler
 *		For Anyother MCU:
 *			Remove/Replace -> extern I2C_HandleTypeDef hi2c1;
 *					with MCU specific I2C handler structure if required
 *
*/
/*************************************************************************/
extern I2C_HandleTypeDef hi2c1;

/*************************************************************************/
/*
 *		For STM32 MCU ONLY:
 *			Modify the I2C_HandleTypeDef according to your code
 *			Replace -> hi2c1 
 *					with your own I2C handler
 *		For Anyother MCU:
 *			Replace -> HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)I2C_RGB_ADDR, (uint8_t*)data, 2, 1000);
 *					with MCU specific I2C transmission function using I2C_RGB_ADDR
 *
 *	@-> I2C_RGB_ADDR is 1-bit left shifted address of RGB backlight
 *	->	because STM32 HAL functions doesnt automatically shift 
 *	->	7-bit I2C slave address to left by 1-bit
 *	->	For other MCU I2C_RGB_ADDR might require 1-bit right shift
 *	->	in 7-bit I2C slave addressing mode
 *	->	i.e:	
 *	->			#define I2C_RGB_ADDR  (0xC4 >> 1)
 *	->	For any other MCU Modify this address in easy_rgb_lcd.h file
 *	->	as shown above (if required)
 *
*/
/*************************************************************************/


#define I2C_RGB_ADDR  0xC4
#define I2C_LCD_ADDR  0x7C

static void LCD_setReg(unsigned char reg, unsigned char val){
	uint8_t data[2] = {reg, val};

	HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)I2C_RGB_ADDR, (uint8_t*)data, 2, 1000);
}

/*************************************************************************/
/*
 *		For STM32 MCU ONLY:
 *			Modify the I2C_HandleTypeDef according to your code
 *			Replace -> hi2c1 
 *					with your own I2C handler
 *		For Anyother MCU:
 *			Replace -> HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)I2C_LCD_ADDR, (uint8_t*)data, len, 1000);
 *					with MCU specific I2C transmission function using I2C_LCD_ADDR
 *
 *	@-> I2C_LCD_ADDR is 1-bit left shifted address of Grove 16x2 LCD
 *	->	because STM32 HAL functions doesnt automatically shift 
 *	->	7-bit I2C slave address to left by 1-bit
 *	->	For other MCU I2C_LCD_ADDR might require 1-bit right shift
 *	->	in 7-bit I2C slave addressing mode
 *	->	i.e:	
 *	->			#define I2C_LCD_ADDR  (0x7C >> 1)
 *	->	For any other MCU Modify this address in easy_rgb_lcd.h file
 *	->	as shown above (if required)
 *
*/
/*************************************************************************/
static void i2c_send_byteS(uint8_t *data, uint32_t len)
{
	HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)I2C_LCD_ADDR, (uint8_t*)data, len, 1000);
}


/*************************************************************************/
/*
 *		Code given below is consist of LCD related functions 
 *		and it will work with any MCU.
 *		It only require Hardware/Software based I2C transmssion functions
 *
 *		For Delay when using anyother MCU:
 *			Replace all the -> HAL_Delay(uint32_t msec);
 *					function with MCU related delay function.
 *
*/
/*************************************************************************/

void LCD_begin(uint8_t cols, uint8_t lines, uint8_t dotsize){
	if (lines > 1) {
        _displayfunction |= LCD_2LINE;
    }
    _numlines = lines;
    _currline = 0;

    // for some 1 line displays you can select a 10 pixel high font
    if ((dotsize != 0) && (lines == 1)) {
        _displayfunction |= LCD_5x10DOTS;
    }

    // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
    // according to datasheet, we need at least 40ms after power rises above 2.7V
    // before sending commands. Arduino can turn on way before 4.5V so we'll wait 50 msec

    HAL_Delay(50);	// 50-msec delay

    // this is according to the hitachi HD44780 datasheet
    // page 45 figure 23

    // Send function set command sequence
    LCD_command(LCD_FUNCTIONSET | _displayfunction);
    HAL_Delay(5);  // wait more than 4.1ms

    // second try
    LCD_command(LCD_FUNCTIONSET | _displayfunction);
    HAL_Delay(1);

    // third go
    LCD_command(LCD_FUNCTIONSET | _displayfunction);


    // finally, set # lines, font size, etc.
    LCD_command(LCD_FUNCTIONSET | _displayfunction);

    // turn the display on with no cursor or blinking default
    _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
    LCD_display();

    // clear it off
    LCD_clear();

    // Initialize to default text direction (for romance languages)
    _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
    // set the entry mode
    LCD_command(LCD_ENTRYMODESET | _displaymode);

    // backlight init
    LCD_setReg(0, 0);
    LCD_setReg(1, 0);
    LCD_setReg(0x08, 0xAA);     // all led control by pwm

    LCD_setColorWhite();
}

void LCD_clear(void){
	LCD_command(LCD_CLEARDISPLAY);        // clear display, set cursor position to zero
	HAL_Delay(2); // this command takes a long time!
}
void LCD_home(void){
	LCD_command(LCD_RETURNHOME);        // clear display, set cursor position to zero
	HAL_Delay(2); // this command takes a long time!
}

void LCD_noDisplay(void){
    _displaycontrol &= ~LCD_DISPLAYON;
    LCD_command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LCD_display(void){
    _displaycontrol |= LCD_DISPLAYON;
    LCD_command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LCD_noBlink(void){
    _displaycontrol &= ~LCD_BLINKON;
    LCD_command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LCD_blink(void){
    _displaycontrol |= LCD_BLINKON;
    LCD_command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LCD_noCursor(void){
    _displaycontrol &= ~LCD_CURSORON;
    LCD_command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LCD_cursor(void){
	_displaycontrol |= LCD_CURSORON;
	LCD_command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LCD_scrollDisplayLeft(void){
	LCD_command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void LCD_scrollDisplayRight(void){
	LCD_command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}
void LCD_leftToRight(void){
    _displaymode |= LCD_ENTRYLEFT;
    LCD_command(LCD_ENTRYMODESET | _displaymode);
}
void LCD_rightToLeft(void){
    _displaymode &= ~LCD_ENTRYLEFT;
    LCD_command(LCD_ENTRYMODESET | _displaymode);
}
void LCD_autoscroll(void){
    _displaymode |= LCD_ENTRYSHIFTINCREMENT;
    LCD_command(LCD_ENTRYMODESET | _displaymode);
}
void LCD_noAutoscroll(void){
    _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
    LCD_command(LCD_ENTRYMODESET | _displaymode);
}
void LCD_createChar(uint8_t location, uint8_t charmap[]){
    location &= 0x7; // we only have 8 locations 0-7
    LCD_command(LCD_SETCGRAMADDR | (location << 3));


    unsigned char dta[9];
    dta[0] = 0x40;
    for(int i=0; i<8; i++)
    {
        dta[i+1] = charmap[i];
    }
    i2c_send_byteS(dta, 9);
}
void LCD_setCursor(uint8_t col, uint8_t row){
    col = (row == 0 ? col|0x80 : col|0xc0);
    unsigned char dta[2] = {0x80, col};
    i2c_send_byteS(dta, 2);
}

void LCD_write(uint8_t value){
    unsigned char dta[2] = {0x40, value};
    i2c_send_byteS(dta, 2);
}

void LCD_print(char* data, uint8_t size){
	while(size>0){
		LCD_write(data[0]);
		data++;
		size--;
	}
}

void LCD_command(uint8_t value){
    unsigned char dta[2] = {0x80, value};
    i2c_send_byteS(dta, 2);
}

// color control
void LCD_setRGB(unsigned char r, unsigned char g, unsigned char b){
	LCD_setReg(REG_RED, r);
    LCD_setReg(REG_GREEN, g);
    LCD_setReg(REG_BLUE, b);
}
void LCD_setPWM(unsigned char color, unsigned char pwm){
	LCD_setReg(color, pwm);
}

const unsigned char color_define[4][3] =
{
    {255, 255, 255},            // white
    {255, 0, 0},                // red
    {0, 255, 0},                // green
    {0, 0, 255},                // blue
};

void LCD_setColor(unsigned char color){
    if(color > 3)return ;
    LCD_setRGB(color_define[color][0], color_define[color][1], color_define[color][2]);
}

void LCD_setColorAll(void){
	LCD_setRGB(0, 0, 0);
}

void LCD_setColorWhite(void){
	LCD_setRGB(255, 255, 255);
}
