#include <RTL.h>
#include "LPC17xx.H"                    /* LPC17xx definitions               */
#include "GLCD.h"
#include "LED.h"
#include "KBD.h"
#include "ADC.h"



// TASK IDs
OS_TID t_welcome; // welcome task id
OS_TID t_attention; //ATTENTION task
OS_TID t_playerSpaceShip;
OS_TID t_projectilesMovement;
OS_TID t_buttonPress;
OS_TID t_computerSpaceShip;
OS_TID t_asteroid1;
OS_TID t_asteroid2;


OS_TID t_led;                           /* assigned task id of task: led */
OS_TID t_adc;                           /* assigned task id of task: adc */
OS_TID t_kbd;                           /* assigned task id of task: keyread */
OS_TID t_jst   ;                        /* assigned task id of task: joystick */
OS_TID t_clock;                         /* assigned task id of task: clock   */
OS_TID t_lcd;                           /* assigned task id of task: lcd     */



// THREADS 
__task void attentionFlash(void);
__task void welcomeScreen(void);
__task void playerSpaceShip(void);
__task void projectilesMovement(void);
__task void buttonPress(void);
__task void computerSpaceShip(void);
__task void asteroid1(void);
__task void asteroid2(void);
