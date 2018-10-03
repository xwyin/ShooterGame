#include <RTL.h>
#include "LPC17xx.H"                    /* LPC17xx definitions               */
#include "GLCD.h"
#include "LED.h"
#include "KBD.h"
#include "ADC.h"
#include "extraThreads.h"
#include "bitmaps.h"

#define __FI        1                   /* Font index 16x24                  */
#define PROJ_CAPACITY 20
#define ASTEROID1X 160
#define ASTEROID1WIDTH 40
#define ASTEROID2X 100  
#define ASTEROID2WIDTH 40


// GLOBAL VARIABLES AND SHIT
short int position; // y position of the ship 
short int computerPosition; // y position of computer
int asteroidPosition1;
int asteroidLength1;
int asteroidPosition2;
int asteroidLength2;
unsigned char playerLives;
unsigned char computerLives;
unsigned char gameRunning;
unsigned char pauseGame;

volatile int attentionStart;
OS_MUT mut_GLCD;                        /* Mutex to controll GLCD access     */


unsigned int ADCStat = 0;
unsigned int ADCValue = 0;

typedef struct{
		unsigned char size;
    short int xyPos[PROJ_CAPACITY+1][2]
}projectiles;

projectiles rightProj;
projectiles leftProj;


void dischargeRight(unsigned char currPos)
{
	if (rightProj.size < PROJ_CAPACITY)
	{
		rightProj.xyPos[rightProj.size][0] = 25;
		rightProj.xyPos[rightProj.size][1] = currPos;
		rightProj.size++;
		os_mut_wait(mut_GLCD, 0xffff); //Rest of code updates graphics on the LCD
		GLCD_Bitmap(25,currPos,7,5, roundGreenBitmap);
		os_mut_release(mut_GLCD);
	}
}

void dischargeLeft(unsigned char currPos)
{
	if (leftProj.size < PROJ_CAPACITY)
	{
		leftProj.xyPos[leftProj.size][0] = 293;
		leftProj.xyPos[leftProj.size][1] = currPos;
		leftProj.size++;
		os_mut_wait(mut_GLCD, 0xffff); //Rest of code updates graphics on the LCD
		GLCD_Bitmap(293,currPos,7,5, roundBlueBitmap);
		os_mut_release(mut_GLCD);
	}
}

void killAll()
{
	os_tsk_delete(t_playerSpaceShip);
	os_tsk_delete(t_projectilesMovement);
	os_tsk_delete(t_buttonPress);
	os_tsk_delete(t_computerSpaceShip);
	os_tsk_delete(t_asteroid1);
	os_tsk_delete(t_asteroid2);
	
}


int detectCollision(short int xPos, short int yPos, unsigned char direction)
{
	unsigned char b;
	if (direction == 1) // shots are going right
	{		
		if (xPos == 320  || ((xPos+7 > ASTEROID1X && xPos+7 < ASTEROID1X+ASTEROID1WIDTH) && ((yPos<=asteroidPosition1+asteroidLength1 && yPos+5>=asteroidPosition1+asteroidLength1) || (yPos+5 >= asteroidPosition1 && yPos <= asteroidPosition1) || (yPos >= asteroidPosition1 && yPos+5 <= asteroidPosition1+asteroidLength1))) || ((xPos+7 > ASTEROID2X && xPos+7 < ASTEROID2X+ASTEROID2WIDTH) && ((yPos<=asteroidPosition2+asteroidLength2 && yPos+5>=asteroidPosition2+asteroidLength2) || (yPos+5 >= asteroidPosition2 && yPos <= asteroidPosition2) || (yPos >= asteroidPosition2 && yPos+5 <= asteroidPosition2+asteroidLength2))))
		{
			return 1;
		}
		else
		{
			if ((xPos+7 > 320-27) && ((yPos<=computerPosition+27 && yPos+5>=computerPosition+27) || (yPos+5 >= computerPosition && yPos <= computerPosition) || (yPos >= computerPosition && yPos+5 <= computerPosition+27))) // collision with computer
			{
				LED_Off (4 + computerLives);
				computerLives--;
				if (computerLives == 0)
				{
					os_mut_wait(mut_GLCD, 0xffff); 
					GLCD_Bitmap(320-27,computerPosition,30,30, explosionBitmap); // DISPLAY EXPLOSION
					os_dly_wait(150);
					GLCD_Clear(Green);
					GLCD_SetBackColor(Green);
					GLCD_SetTextColor(White);
					GLCD_DisplayString	(	4, 6, 1, "YOU WIN!" );	
					killAll();
					os_mut_release(mut_GLCD);
					// END GAME
				}
				else
				{
					pauseGame = 1;
					for (b = 0; b<7; b++)
					{
						os_mut_wait(mut_GLCD, 0xffff); //Rest of code updates graphics on the LCD
						GLCD_Bitmap(320-27,computerPosition,27,27, oponentBlackBitmap);
						os_mut_release(mut_GLCD);
						os_dly_wait(10);
						os_mut_wait(mut_GLCD, 0xffff); //Rest of code updates graphics on the LCD
						GLCD_Bitmap(320-27,computerPosition,27,27, oponentBitmap);
						os_mut_release(mut_GLCD);
						os_dly_wait(10);
					}
					pauseGame = 0;
					
				}
				return 1;
				// take away life ans stuff
			}
			else
			{
			return 0;
			}
		}
	}
	else // shots are going left
	{
		if (xPos == 0  || ((xPos < ASTEROID1X+ASTEROID1WIDTH && xPos > ASTEROID1X) && ((yPos<=asteroidPosition1+asteroidLength1 && yPos+5>=asteroidPosition1+asteroidLength1) || (yPos+5 >= asteroidPosition1 && yPos <= asteroidPosition1) || (yPos >= asteroidPosition1 && yPos+5 <= asteroidPosition1+asteroidLength1)))  ||  ((xPos < ASTEROID2X+ASTEROID2WIDTH && xPos > ASTEROID2X) && ((yPos<=asteroidPosition2+asteroidLength2 && yPos+5>=asteroidPosition2+asteroidLength2) || (yPos+5 >= asteroidPosition2 && yPos <= asteroidPosition2) || (yPos >= asteroidPosition2 && yPos+5 <= asteroidPosition2+asteroidLength2))))
		{
			return 1;
		}
		else
		{
			if ((xPos < 0+30) && ((yPos<=position+30 && yPos+5>=position+30) || (yPos+5 >= position && yPos <= position ) || (yPos >= position && yPos+5 <= position+30))) // collision with player
			{
				LED_Off (playerLives - 1);
				playerLives--;
				if (playerLives == 0)
				{
					os_mut_wait(mut_GLCD, 0xffff); 
					GLCD_Bitmap(0,position,30,30, explosionBitmap); // DISPLAY EXPLOSION
					os_dly_wait(150);
					GLCD_Clear(Red);
					GLCD_SetBackColor(Red);
					GLCD_SetTextColor(White);
					GLCD_DisplayString	(	4, 3, 1, "HAHA YOU SUCK!" );	
					killAll();
					os_mut_release(mut_GLCD);
					// END GAME
				}
				else
				{
					pauseGame = 1;
					for (b = 0; b<7; b++)
					{
						os_mut_wait(mut_GLCD, 0xffff); //Rest of code updates graphics on the LCD
						GLCD_Bitmap(0,position,30,30, spaceShipBlackBitmap);
						os_mut_release(mut_GLCD);
						os_dly_wait(10);
						os_mut_wait(mut_GLCD, 0xffff); //Rest of code updates graphics on the LCD
						GLCD_Bitmap(0,position,30,30, spaceShipBitmap);
						os_mut_release(mut_GLCD);
						os_dly_wait(10);
					}
					pauseGame = 0;
					
				}
				return 1;
				// take away life ans stuff
			}
			else
			{
			return 0;
			}
		}
	}
}











/*----------------------------------------------------------------------------
  ATTENTION! task
 *---------------------------------------------------------------------------*/
__task void attentionFlash(void)
{
	while(attentionStart)
	{
		os_mut_wait(mut_GLCD, 0xffff);
    GLCD_SetTextColor(Red);
		GLCD_DisplayString (	1, 4, 1, "ATTENTION!!" );
		os_mut_release(mut_GLCD);
		os_dly_wait (20);
		os_mut_wait(mut_GLCD, 0xffff);
		GLCD_DisplayString (	1, 4, 1, "           " );
		os_mut_release(mut_GLCD);
		os_dly_wait (20);
	}
	
	os_tsk_delete_self();
  
	
}


/*----------------------------------------------------------------------------
  Welcome Screen task
 *---------------------------------------------------------------------------*/
__task void welcomeScreen(void)
{
	int m;
	
	int countDown;
	int i = 0;
	char lineString[] = "THIS IS NOT A DRILL!";
	char countDownString[] = "12345";
	attentionStart = 1; // allows ATTENTION to flash... false will make ATTENTION task kill itself
	
	// JUST SET BACKGROUND COLOR TO BLACK
	os_mut_wait(mut_GLCD, 0xffff);
	GLCD_Clear(Black);
	GLCD_SetBackColor(Black);
	
	GLCD_Bitmap(75,175 ,30,30, yellowStar2Bitmap);
	GLCD_Bitmap(25,15 ,30,30, yellowStar2Bitmap);
	GLCD_Bitmap(110,70 ,25,25, blueStar2Bitmap);
	GLCD_Bitmap(5,5,20,20, blueStarBitmap);
	GLCD_Bitmap(39,75 ,20,20, blueStarBitmap);
	GLCD_Bitmap(275,136 ,25,25, blueStar2Bitmap);
	GLCD_Bitmap(282,35,15,15, yellowStarBitmap);
	GLCD_Bitmap(276,22,15,15, yellowStarBitmap);
	GLCD_Bitmap(298,11,20,20, blueStarBitmap);
	GLCD_Bitmap(276,22,15,15, yellowStarBitmap);
	GLCD_Bitmap(280,76,15,15, yellowStarBitmap);
	GLCD_Bitmap(240,82,15,15, yellowStarBitmap);
	GLCD_Bitmap(255,76,15,15, yellowStarBitmap);
	GLCD_Bitmap(250,68,15,15, yellowStarBitmap);
	GLCD_Bitmap(88,150,20,20, blueStarBitmap);
	GLCD_Bitmap(100,200,20,20, blueStarBitmap);
	GLCD_Bitmap(0,120,15,15, yellowStarBitmap);
	GLCD_Bitmap(169,163 ,30,30, yellowStar2Bitmap);
	GLCD_Bitmap(40,190 ,30,30, yellowStar2Bitmap);
	GLCD_Bitmap(200,149 ,25,25, blueStar2Bitmap);
	GLCD_Bitmap(205,210,15,15, yellowStarBitmap);
	GLCD_Bitmap(230,175,15,15, yellowStarBitmap);
	GLCD_Bitmap(180,194,15,15, yellowStarBitmap);
	GLCD_Bitmap(287,225,20,20, blueStarBitmap);
	GLCD_Bitmap(300,168,20,20, blueStarBitmap);
	GLCD_Bitmap(230,115 ,30,30, yellowStar2Bitmap);
	GLCD_Bitmap(130,120 ,30,30, yellowStar2Bitmap);

	os_mut_release(mut_GLCD);
	
	//CREATES FLASHING ATTENTION SIGN
	t_attention = os_tsk_create(attentionFlash, 0); // display ATTENTION! flashing
	
	while (lineString[i] != '\0')
	{
		os_mut_wait(mut_GLCD, 0xffff);
    GLCD_SetTextColor(White);
		GLCD_DisplayChar (	2, i, 1, lineString[i] );
		os_mut_release(mut_GLCD);
		os_dly_wait (10);
		i++;
	}
	
	os_dly_wait (100);
	
	os_mut_wait(mut_GLCD, 0xffff); 
	GLCD_SetTextColor(White);
	GLCD_DisplayString	(	4, 1, 1, "Enemy spacecrafts" );	
	GLCD_DisplayString	(	5, 1, 1, " approaching in:" );	
	os_mut_release(mut_GLCD);
	
	for (countDown = 4; countDown >= 0 ; countDown--)
	{
		os_mut_wait(mut_GLCD, 0xffff);
		if (countDown >= 3)
		{
			GLCD_SetTextColor(White);
		}
		else
		{
			GLCD_SetTextColor(Red);
		}
		GLCD_DisplayChar	(	7, 9, 1, countDownString[countDown] );	
		os_mut_release(mut_GLCD);
		os_dly_wait (100);
	}
	
	
	attentionStart = 0; // Tells the ATTENTION to stop flashing and kill itself
	os_dly_wait (20);
	os_mut_wait(mut_GLCD, 0xffff);
	GLCD_Clear (Black);
	os_mut_release(mut_GLCD);
	
	t_playerSpaceShip = os_tsk_create(playerSpaceShip, 0); // display ATTENTION! flashing
	t_projectilesMovement = os_tsk_create(projectilesMovement, 0);
	t_buttonPress = os_tsk_create(buttonPress, 0);
	t_computerSpaceShip = os_tsk_create(computerSpaceShip,0);
	t_asteroid1 = os_tsk_create(asteroid1,0);
	t_asteroid2 = os_tsk_create(asteroid2,0);
	
	for (m = 0; m<5; m++)
	{
		LED_Off (0);
		LED_Off (1);
		LED_Off (2);
		LED_Off (5);
		LED_Off (6);
		LED_Off (7);
		os_dly_wait (10);
		LED_On (0);
		LED_On (1);
		LED_On (2);
		LED_On (5);
		LED_On (6);
		LED_On (7);
		os_dly_wait (10);
		
	}
	os_tsk_delete_self();
  	
}



/*----------------------------------------------------------------------------
  Player spaceship task
 *---------------------------------------------------------------------------*/
__task void playerSpaceShip(void)
{
	int joystick;
	rightProj.size = 0;
	position = 120;
	
	os_mut_wait(mut_GLCD, 0xffff);
	GLCD_Bitmap(0,position,30,30, spaceShipBitmap);
	os_mut_release(mut_GLCD);
	
	
	while(1)
	{
		if (pauseGame)
		{
			os_dly_wait (2);
		}
		else
		{
			joystick = KBD_Get();
			if (joystick == 113 && position > 0) // LEFT
			{
				position = position - 2;
				os_mut_wait(mut_GLCD, 0xffff);
				GLCD_Bitmap(0,position,30,30, spaceShipBitmap);
				os_mut_release(mut_GLCD);
			}
			if (joystick == 89 && position < 210) // RIGHT
			{
				position = position + 2;
				os_mut_wait(mut_GLCD, 0xffff);
				GLCD_Bitmap(0,position,30,30, spaceShipBitmap);
				os_mut_release(mut_GLCD);
			}
				
			os_dly_wait (1);
		}
	
	}
	
	os_tsk_delete_self();
  
	
}


/*----------------------------------------------------------------------------
  Computer Space Ship
 *---------------------------------------------------------------------------*/
__task void computerSpaceShip(void)
{
	short int i=0; 
	computerPosition = 120;
	
	os_mut_wait(mut_GLCD, 0xffff);
	GLCD_Bitmap(293,computerPosition,27,27, oponentBitmap);
	os_mut_release(mut_GLCD);
	
	
	while(1)
	{
			while (computerPosition > 0) 
			{
				if (pauseGame)
				{
					os_dly_wait (2);
				}
				else
				{
					if (computerLives == 3)
					{
						if (i%50 == 0)
						{
							dischargeLeft(computerPosition+8);
							dischargeLeft(computerPosition+17);
						}
						computerPosition = computerPosition - 2;
						os_mut_wait(mut_GLCD, 0xffff);
						GLCD_Bitmap(293,computerPosition,27,27, oponentBitmap);
						os_mut_release(mut_GLCD);
						i++; //used to determine when to shoot
						os_dly_wait (1);
					}
					else
					{
						if(computerLives == 2)
						{
							if (i%50 == 0)
							{
								dischargeLeft(computerPosition+8);
								dischargeLeft(computerPosition+17);
							}
							computerPosition = computerPosition - 1;
							os_mut_wait(mut_GLCD, 0xffff);
							GLCD_Bitmap(293,computerPosition,27,27, oponentBitmap);
							os_mut_release(mut_GLCD);
							i++; //used to determine when to shoot
							os_dly_wait (1);
							if (i%147 == 0 || i%200 == 0)
								break;
						}
						else
						{
							if (i%20 == 0)
							{
								dischargeLeft(computerPosition+8);
								dischargeLeft(computerPosition+17);
							}
							computerPosition = computerPosition - 2;
							os_mut_wait(mut_GLCD, 0xffff);
							GLCD_Bitmap(293,computerPosition,27,27, oponentBitmap);
							os_mut_release(mut_GLCD);
							i++; //used to determine when to shoot
							os_dly_wait (1);
							if (i%147 == 0 || i%200 == 0)
								break;
							
						}
					}
				}
			}
			
			
			
			
			while (computerPosition < 240 - 27) 
			{
				if (pauseGame)
				{
					os_dly_wait (2);
				}
				else
				{
					if (computerLives == 3)
					{
						if (i%73 == 0)
						{
							dischargeLeft(computerPosition+8);
							dischargeLeft(computerPosition+17);
						}
						computerPosition = computerPosition + 2;
						os_mut_wait(mut_GLCD, 0xffff);
						GLCD_Bitmap(293,computerPosition,27,27, oponentBitmap);
						os_mut_release(mut_GLCD);
						i++; //used to determine when to shoot
						os_dly_wait (1);
					}
					else
					{
						if (computerLives == 2)
						{
							if (i%73 == 0)
							{
								dischargeLeft(computerPosition+8);
								dischargeLeft(computerPosition+17);
							}
							computerPosition = computerPosition + 1;
							os_mut_wait(mut_GLCD, 0xffff);
							GLCD_Bitmap(293,computerPosition,27,27, oponentBitmap);
							os_mut_release(mut_GLCD);
							i++; //used to determine when to shoot
							os_dly_wait (1);
							if (i%216 == 0 || i%163 == 0)
								break;
						}
						else
						{
							if (i%20 == 0)
							{
								dischargeLeft(computerPosition+8);
								dischargeLeft(computerPosition+17);
							}
							computerPosition = computerPosition + 2;
							os_mut_wait(mut_GLCD, 0xffff);
							GLCD_Bitmap(293,computerPosition,27,27, oponentBitmap);
							os_mut_release(mut_GLCD);
							i++; //used to determine when to shoot
							os_dly_wait (1);
							if (i%216 == 0 || i%163 == 0)
								break;
						}
					}
					
				}
			}
		
		
	
	}
	
	os_tsk_delete_self();
  
	
}



/*----------------------------------------------------------------------------
  ASTEROID1!
 *---------------------------------------------------------------------------*/
__task void asteroid1(void)
{
 
	
	
	while (1)
	{
		asteroidPosition1 = 240;
		os_mut_wait(mut_GLCD, 0xffff);
		GLCD_Bitmap(160,asteroidPosition1,40,20, halfAsteroidBitmap);
		os_mut_release(mut_GLCD);
		
		asteroidLength1 = 20;
		while(asteroidPosition1 > 240 - 20)
		{
			if (pauseGame)
			{
				os_dly_wait (2);
			}
			else
			{
				os_dly_wait (5);
				asteroidPosition1--;
				os_mut_wait(mut_GLCD, 0xffff);
				GLCD_Bitmap(160,asteroidPosition1,40,20, halfAsteroidBitmap);
				os_mut_release(mut_GLCD);
			}
		}
		
		asteroidLength1 = 40;
		while(asteroidPosition1 > 0)
		{
			if (pauseGame)
			{
				os_dly_wait (2);
			}
			else
			{
				os_dly_wait (5);
				asteroidPosition1--;
				os_mut_wait(mut_GLCD, 0xffff);
				GLCD_Bitmap(160,asteroidPosition1,40,40, asteroidBitmap);
				os_mut_release(mut_GLCD);
			}
		}
		
		
		while(asteroidLength1 >= 0)
		{
			if (pauseGame)
			{
				os_dly_wait (2);
			}
			else
			{
				os_dly_wait (5);
				os_mut_wait(mut_GLCD, 0xffff);
				GLCD_Bitmap(160,0,40,asteroidLength1, asteroidBitmap);
				os_mut_release(mut_GLCD);
				asteroidLength1--;
			}
		}
		
		os_dly_wait (47);
	}
	
	os_tsk_delete_self();
  
}


/*----------------------------------------------------------------------------
  ASTEROID2!
 *---------------------------------------------------------------------------*/
__task void asteroid2(void)
{

	
	while (1)
	{
		
		asteroidPosition2 = 240;
		os_mut_wait(mut_GLCD, 0xffff);
		GLCD_Bitmap(100,asteroidPosition2,40,20, halfAsteroidBitmap);
		os_mut_release(mut_GLCD);
		
		asteroidLength2 = 20;
		while(asteroidPosition2 > 240 - 20)
		{
			if (pauseGame)
			{
				os_dly_wait (2);
			}
			else
			{
				os_dly_wait (13);
				asteroidPosition2--;
				os_mut_wait(mut_GLCD, 0xffff);
				GLCD_Bitmap(100,asteroidPosition2,40,20, halfAsteroidBitmap);
				os_mut_release(mut_GLCD);
			}
		}
		
		asteroidLength2 = 40;
		while(asteroidPosition2 > 0)
		{
			if (pauseGame)
			{
				os_dly_wait (2);
			}
			else
			{
				os_dly_wait (13);
				asteroidPosition2--;
				os_mut_wait(mut_GLCD, 0xffff);
				GLCD_Bitmap(100,asteroidPosition2,40,40, asteroidBitmap);
				os_mut_release(mut_GLCD);
			}
		}
		
		
		while(asteroidLength2 >= 0)
		{
			if (pauseGame)
			{
				os_dly_wait (2);
			}
			else
			{
				os_dly_wait (13);
				os_mut_wait(mut_GLCD, 0xffff);
				GLCD_Bitmap(100,0,40,asteroidLength2, asteroidBitmap);
				os_mut_release(mut_GLCD);
				asteroidLength2--;
			}
		}
		
		os_dly_wait (25);
	}
	
	os_tsk_delete_self();
  
}




/*----------------------------------------------------------------------------
  Watch button press task
 *---------------------------------------------------------------------------*/
__task void buttonPress(void)
{

	while (1)
	{
		if (INT0_Get() == 0)
		{
			dischargeRight(position+13);
			os_dly_wait (75);
		}
		os_dly_wait (2);
	}
	os_tsk_delete_self();
  
}



/*----------------------------------------------------------------------------
  Projectiles going right task
 *---------------------------------------------------------------------------*/
__task void projectilesMovement(void)
{
	int i;
	int k;
	while (1)
	{
		
		// MOVE LASER STUFF IN RIGHT DIRECTION
		if (rightProj.size >0)
		{
			for (i = 0; i < rightProj.size ; i++)
			{
				if (detectCollision(rightProj.xyPos[i][0], rightProj.xyPos[i][1], 1)) //detect collision in traveling right
				{

					os_mut_wait(mut_GLCD, 0xffff); 
					GLCD_Bitmap(rightProj.xyPos[i][0],rightProj.xyPos[i][1],7,5, roundBlackBitmap);
					os_mut_release(mut_GLCD);
					for (k =i; k < rightProj.size  ; k++)
					{
						rightProj.xyPos[k][0] = rightProj.xyPos[k+1][0];
						rightProj.xyPos[k][1] = rightProj.xyPos[k+1][1];
					}
					rightProj.size--;
				}
				else
				{
					(rightProj.xyPos[i][0]) = (rightProj.xyPos[i][0]) + 1;
					os_mut_wait(mut_GLCD, 0xffff); 
					GLCD_Bitmap(rightProj.xyPos[i][0],rightProj.xyPos[i][1],7,5, roundGreenBitmap);
					os_mut_release(mut_GLCD);
					}
			}
		}
		
		// MOVE LASER STUFF IN LEFT DIRECTION
		if (leftProj.size >0)
		{
			for (i = 0; i < leftProj.size ; i++)
			{
				if (detectCollision(leftProj.xyPos[i][0], leftProj.xyPos[i][1], 0))
				{
					os_mut_wait(mut_GLCD, 0xffff); 
					GLCD_Bitmap(leftProj.xyPos[i][0],leftProj.xyPos[i][1],7,5, roundBlackBitmap);
					os_mut_release(mut_GLCD);
					for (k =i; k < leftProj.size  ; k++)
					{
						leftProj.xyPos[k][0] = leftProj.xyPos[k+1][0];
						leftProj.xyPos[k][1] = leftProj.xyPos[k+1][1];
					}
					leftProj.size--;
				}
				else
				{
					(leftProj.xyPos[i][0]) = (leftProj.xyPos[i][0]) - 1;
					os_mut_wait(mut_GLCD, 0xffff); 
					GLCD_Bitmap(leftProj.xyPos[i][0],leftProj.xyPos[i][1],7,5, roundBlueBitmap);
					os_mut_release(mut_GLCD);
					}
			}
		}
		
		os_dly_wait (1);
	}
	os_tsk_delete_self();
  
}





/*----------------------------------------------------------------------------
  Task 6 'init': Initialize
 *---------------------------------------------------------------------------*/
/* NOTE: Add additional initialization calls for your tasks here */
__task void init (void) {

  os_mut_init(mut_GLCD);
	t_welcome = os_tsk_create(welcomeScreen, 0); // display start up
	playerLives = 3;
	computerLives = 3;
	pauseGame = 0;

  os_tsk_delete_self ();
}

/*----------------------------------------------------------------------------
  Main: Initialize and start RTX Kernel
 *---------------------------------------------------------------------------*/
int main (void) {
	
  LED_Init ();                              /* Initialize the LEDs           */
  GLCD_Init();                              /* Initialize the GLCD           */
	KBD_Init ();                              /* initialize Push Button        */


  GLCD_Clear(White);                        /* Clear the GLCD                */

	
  os_sys_init(init);                        /* Initialize RTX and start init */
}
