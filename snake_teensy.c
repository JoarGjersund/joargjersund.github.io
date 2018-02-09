/*
* Info for compling in bash
* avr-gcc blank.c -mmcu=atmega32u4 -Os -I../../cab
202_teensy -L../../cab202_teensy -DF_CPU=8000000UL -lcab202_teensy -lm -std=gnu99 -o blank.o
* avr-objcopy -O ihex blank.o blank.hex
* 
*/

#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include "graphics.h"
#include "sprite.h"
#include "cpu_speed.h"

int score=0, lives=5, food_x, food_y, dirTimer=0, showWalls=0;
Sprite snake [50];
Sprite food;
int size = 2;

// draws name and student number in middle of screen:
void startupScreen()
{  
	draw_string(LCD_X/2 - 15,LCD_Y/2 -5,"Joar");
	draw_string(LCD_X/2 - 22,LCD_Y/2 +5,"9793992");
	show_screen();
	_delay_ms(2000);
	clear_screen();
}

// Show current game stats:
void statusScreen()
{
	char status[20];
	sprintf(status, "%d(%d)",lives, score);	
	draw_string(2,2,status);
}

void drawSnake(int x, int y)
{
	for (int i=0;i<=size;i++)
	{
	snake[i].dx = 0;
	snake[i].dy = 0;
	snake[i].x = x+i*3;
	snake[i].y = y;
	}
}

// draws snake and returns (hit):
// 0 if snake tocuh nothing
// 1 if snake tocuh itself
// 2 if snake touch food
unsigned char moveSnake()
{
	//return value
	unsigned char hit = 0; 	
		
	// bitmap for snake head:
	unsigned char bitmap0[] = 
	{
	0b11100000, 
	0b10100000,
	0b11100000,
	};	
	
	// bitmap for snake body:
	unsigned char bitmap1[] = 
	{
	0b11100000, 
	0b11100000,
	0b11100000,
	};	
	
	
		// Draws body and detect forbidden self touching:
		for (int i=size;i>0;i--)
		{
			// set new position
			snake[i].x=snake[i-1].x;
			snake[i].y=snake[i-1].y;
				
			// check body is touching head (hit itself)
			if (snake[i].x == snake[0].x+snake[0].dx && snake[i].y == snake[0].y+snake[0].dy) hit=1;
			// draws body
			init_sprite(&snake[i], snake[i].x, snake[i].y, 3, 3, bitmap0);
			snake[i].bitmap = bitmap0;
			draw_sprite(&snake[i]);
		}
		
		
		// Draws head and detects if tocuhing food or wall
		
		// set new position
		
		snake[0].x=snake[0].x+snake[0].dx;
		snake[0].y=snake[0].y+snake[0].dy;
		// game board loop around itself (new position is other side of the screen):
		if (snake[0].x>=LCD_X) snake[0].x=0;
		else if (snake[0].x<0) snake[0].x=LCD_X; 
		if (snake[0].y>=LCD_Y) snake[0].y=0;	
		else if (snake[0].y<0) snake[0].y=LCD_Y;
		
		// check if head is touching food or wall
		if (snake[0].x ==food.x && snake[0].y==food.y) hit=2; //touching food	
		if (showWalls)
		{
		if (snake[0].y==15 && snake[0].x<=39) hit=1; // touching first wall
		if (snake[0].y<=10 && snake[0].x==51) hit=1; // touching second wall
		if (snake[0].y==30 && snake[0].x>=45) hit=1; // touching third wall	
		}
		
		//draw snake head
		float dx = snake[0].dx; // save current dx and dy 
		float dy = snake[0].dy;
		init_sprite(&snake[0], snake[0].x, snake[0].y, 3, 3, bitmap1); // this function resets dx and dy
		snake[0].dx = dx;
		snake[0].dy = dy;
		snake[0].bitmap = bitmap1;
		draw_sprite(&snake[0]);

		
	return hit;

}
void newFood ()
{
	// define grid assuming pixel size=3
	unsigned char possiblePositionsX[(int)floor(LCD_X/3)];
	unsigned char possiblePositionsY[(int)floor(LCD_Y/3)];
	for (int i=0;i<sizeof(possiblePositionsX); i++) possiblePositionsX[i] = i*3;
	for (int i=0;i<sizeof(possiblePositionsY); i++) possiblePositionsY[i] = i*3;

	// set random position (not actually random due to missing seed)
	food_x = possiblePositionsX[rand() % sizeof(possiblePositionsX)/sizeof(possiblePositionsX[0]) ];
	food_y = possiblePositionsY[rand() % sizeof(possiblePositionsY)/sizeof(possiblePositionsY[0]) ];
	
	//making sure food is not placed in invalid positions.
	if (food_y<9 && food_x<LCD_X/2) newFood(); // behind score/lives
	if (food_y==15 && food_x<=39) newFood(); // on first wall
	if (food_y<=10 && food_x==51) newFood(); // on second wall
	if (food_y==30 && food_x>=45) newFood(); // on third wall
	
}
void drawFood ()
{
	// changing food bitmap requires chaning dimentions in all other funcitons (consider making this a variable)
	unsigned char bitmap0[] = 
	{
	0b10100000, 
	0b01000000,
	0b10100000,
	};	
	init_sprite(&food, food_x, food_y, 3, 3, bitmap0);
	draw_sprite(&food);
}

void setup()
{
	DDRB |= (1<<2); // set led0 (port B1) as output
	DDRB |= (1<<3); // set led1 (port B2) as output
	DDRF&= ~(1<<5); // set SW3 (port F5) as input (right)
	DDRF&= ~(1<<6); // set SW2 (port F4) as input (left)
	
	DDRD&= ~(1<<1); // set navigation pin as input (up)	
	DDRB&= ~(1<<7); // set navigation pin as input (down)	
	DDRB&= ~(1<<0); // set navigation pin as input (centre)	
	DDRB&= ~(1<<1); // set navigation pin as input (left)	
	DDRD&= ~(1<<0); // set navigation pin as input (right)	
	
	DDRF&= ~(1<<0); // set thumbwheel port 1 as input 	
	DDRF&= ~(1<<1); // set thumbwheel port 2 as input 
	
	// Set the CPU speed to 8MHz (you must also be compiling at 8MHz)
    set_clock_speed(CPU_8MHz);
    
    //initialise the LCD screen
    lcd_init(LCD_DEFAULT_CONTRAST);
    
    //clear any characters that were written on the screen
    clear_screen();
    
	// initialize
	startupScreen();
	newFood();
	drawSnake(18, 30);
	
}
void fps()
{
	_delay_ms(10);	
    ADMUX = ( (1<<REFS0) & 0xF8)|1;
    ADCSRA = (1<<ADPS2)|(1<<ADEN)|(1<<ADPS0)|(1<<ADSC)|(1<<ADPS1);	
    while(ADCSRA & (1<<ADSC));
	for(int i =0;i<ADC;i++)_delay_ms(1);	
	
}

// updates snake dx and dy value according to user input:
void controller()
{

	if ( (PINF>>5) & 1)showWalls=1; // show walls
	if ( (PINF>>6) & 1)showWalls=0; // remove walls
	
	// SNAKE CONTROLLER
	int action = 0;
	// up
	if ( (PIND>>1) & 1)
	{ 
		action = 1;
		_delay_ms(10);
	}
	// down
	if ( (PINB>>7) & 1)
	{ 
		action = 2;
		_delay_ms(10);
	}
	// left
	if ( (PINB>>1) & 1)
	{ 
		action = 3;
		_delay_ms(10);
	}
	// right
	if ( (PIND>>0) & 1)
	{ 
		action = 4;
		_delay_ms(10);
	}

	// if direction is still undecided:		
	if (snake[0].dx ==0 && snake[0].dy == 0) dirTimer =1;
	
	if (action>0)
	{
		if (action==1)
		{
		snake[0].dx = 0;
		snake[0].dy = -3;
		}
		if (action==2)
		{
		snake[0].dx = 0;
		snake[0].dy = 3;
		}
		if (action==3)
		{
		snake[0].dx = -3;
		snake[0].dy = 0;
		}
		if (action==4)
		{
		snake[0].dx = 3;
		snake[0].dy = 0;
		}
	}
}


void drawWalls()
{
	draw_line(0, 16, 39, 16); // 39 long
	draw_line(52, 0, 52, 10); // 10 long
	draw_line(LCD_X, 31, 45, 31); //39 long
	
}

void gameOver()
{
	draw_string(18,LCD_Y/2 -5,"Game over");
	draw_string(0,LCD_Y/2 +5,"[SW3 to restart]");
	show_screen();
	_delay_ms(2000);
	while(!( (PINF>>5) & 1)) _delay_ms(1);
	clear_screen();
	score=0;
	lives=5;
	dirTimer=0;
	showWalls=0;
	size = 2;
	drawSnake(18, 30);
	newFood();
}

void process()
{
	statusScreen();
	controller();
	drawFood();
	switch (moveSnake())
	{
		case 0: break; //nothing
		case 1: // hit itself
			// if game is started yet.
			if (dirTimer==0)
			{
			lives--; 
			size=2;
			drawSnake(18,30);
			dirTimer=size;
			}
		break; 
		
		case 2: // hit apple
			score++; 
			if (showWalls)score++; // adds extra point if walls are present
			size++;
			newFood();
		break; 
		
	}
	
	if (showWalls)drawWalls();
	dirTimer=0; // set game to started.
	show_screen();
	fps();
	clear_screen();
	if (lives==0)gameOver();
	
}

int main(void) 
{
	setup();
	while(1) process();
	return 0;
}