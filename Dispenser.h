/*
/
  Copyright 2019
  Authors:
   Todd Gilbert
   DJ Wadhwa 
  Contact: 
    awsunit@uw.edu

  This file contains declarations and aliases for a Tiva board program which:
    1. Is freeRTOS controlled
    2. controls a servo to open/close a 3D printed device
    3. device dispenses cereal
    4. LCD screen displays ??????????????????

/
*/ 
/**/
#ifndef DISPENSER_H_
#define DISPENSER_H_
/**/
#include "FreeRTOS.h"
#include "task.h"
/**/
/*PIN nums*/
#define P0 0x1
#define P1 0x2
#define P2 0x4
#define P3 0x8
#define P4 0x10
#define P5 0x20
#define P6 0x40
#define P7 0x80

#define MAX_SCHEDULES 20
/* up/down -> set time, override -> opens device */
#define NUM_BUTTONS 3
/* red */
#define NUM_LEDS 1
/**/
// UART enable UART modules
// offset: 0x618 | pg. 344
#define TSYSCTL_RCGCUART_R  (*((volatile uint32_t *) 0x400FE618))
#define TUART1_CTL_R (*((volatile uint32_t *)0x4000D030))
#define TUART1_LCRH_R (*((volatile uint32_t *)0x4000D02C))
#define TUART1_CC_R (*((volatile uint32_t *)0x4000DFC8))
#define TUART1_IBRD_R (*((volatile uint32_t *)0x4000D024))
#define TUART1_FBRD_R (*((volatile uint32_t *)0x4000D028))
#define TUART1_DR_R (*((volatile uint32_t *)0x4000D000))
#define TUART1_FR_R (*((volatile uint32_t *)0x4000D018))
#define TSYSCTL_RCGC2_R (*((volatile uint32_t *)0x400FE108))
#define TGPIO_PORTB_AFSEL_R (*((volatile uint32_t *)0x40005420))
#define TGPIO_PORTB_DEN_R (*((volatile uint32_t *)0x4000551C))
#define TGPIO_PORTB_PCTL_R (*((volatile uint32_t *)0x4000552C))
#define TGPIO_PORTB_DIR_R (*((volatile uint32_t *) 0x40005400))
// clock gating control | pg. 340
#define TSYSCTL_RCGCGPIO_R (*((volatile uint32_t *) 0x400FE608))
/* Port A */
#define TSYSCTL_RCGC2_GPIOA 0x01
// mode control select register
// bit clear -> pin used as GPIO and controlled via 
// GPIO registers!
// chooses peripheral function?
// offset: 0x420 | pg. 671
#define TGPIO_PORTA_AFSEL_R (*((volatile uint32_t *) 0x40004420))
// GPIO Port Control, offset 0x52C | pg. 688
// sets signal definition
#define TGPIO_PORTA_PCTL_R  (*((volatile uint32_t *)0x4000452C))
// DEN allows digit output
#define TGPIO_PORTA_DEN_R (*((volatile uint32_t *) 0x4000451C))
// controls isolation circuits
// analog circuitry requires isolations from pins
// offset: 0x528 | pg. 687
#define TGPIO_PORTA_AMSEL_R (*((volatile uint32_t *) 0x40004528))
// GPIODIR offset: 0x400
#define TGPIO_PORTA_DIR_R (*((volatile uint32_t *) 0x40004400))
//
#define TGPIO_PORTA_DATA_R (*((volatile uint32_t *) 0x400043FC))
/**/
/* flag to open for client scheduled dispensing */


/**/
/*
  Initializes registers/pins for use with:
    Servo
    LCD
    External Button
    UART - read from putty
*/
void setup(void);
/**/
/*
  Instructs servo to open 3D device

  - Opens device iff:
     Open Button being pressed
     scheduled_open_status == 1
*/
void Servo_Open(void);
/**/
/*
  Determines if a client is attempting to:
    add a new time when the 3D device should operate
    sets global flag if both buttons pressed
      - flag triggers I/O process
      - I/O requests information from client
*/
void Client_Schedule(void *p);
/**/
/*
  Determines if any Record in records are due for servicing
  modifies: scheduled_open_status
  effects: scheduled_open_status = 1 iff any record is due for servicing 
*/
void Schedule_Check(void);
/**/
/*
  Instructs servo to close 3D device

  - Closes device iff:
     close_status == 1 
*/
void Servo_Close(void);
/**/
/*
  Changes the LCD to display information

  - Tracks seconds since last display:
    updates every five seconds
    
  int c could be (command c), maybe unnecessary
*/
void LCD_Update(void *p);
/**/
/*

*/
void FSM(void);
/**/
/*
  Initializes UART0 for client I/O
*/
void UART_Init(void);
/**/
/*
  Configures Tiva for a red LED
*/
void LED_Init(void);
/**/
/*
  Configures Tiva to control Servo
*/
void Servo_Init(void);
/**/
/*
  Configures Tiva for use with external toggle buttons
*/
void Button_Init(void);
/*
  Helper to make sure button is recieving a true press
  eventual replacement with iterrupt I'm sure
*/
void pop_button(void);
/*
*/
int switch_input(int PIN);
/**/
/*
  Checks if down button is being pressed
  returns: 1 iff button pressed for 2 seconds
           0 otherwise
*/
int down_button_pressed(void);
/**/
/*
  Checks if up button is being pressed
  returns: 1 iff button pressed for 2 seconds
           0 otherwise
*/
int up_button_pressed(void);
/**/
/*
  Checks if override button is being pressed
  returns: 1 iff button pressed for 2 seconds
           0 otherwise
*/
int override_button_pressed(void);
/*
  Helper method for LCD to print a whole c-style string
*/
void StringToChar(char * sentence);
/**/
/*
  Helper function to create mask for PCTL_R
  args: PIN is a the number to left shift OxF by
  returns: 0xF << (4 * PIN)
*/
int mask_regular_GPIO(int PIN);
/**/ 
/*
  called by FreeRTOS when stack overflow occurs
*/
void vApplicationStackOverflowHook(xTaskHandle *pxTask, char *pcTaskName);
/**/
#endif  // DISPENSER_H_
/**/
/*EOF*/
