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
#include FreeRTOS.h"
#include "task.h"
/**/
/**/
int scheduled_open_status = 0;
int close_status = 0;
int lcd_update_status = 0;
const int MAX_SCHEDULES = 20;
long scheduled_dispenses[MAX_SCHEDULES];
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
  Determines if any Record in records are due for servicing
  modifies: scheduled_open_status
  effects: scheduled_open_status = 1 iff any record is due for servicing 
*/
void Check_Schedule(void);
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
void LCD_Update(int c);
/**/
/*

*/
void FSM(void);
/**/ 
/*
  called by FreeRTOS when stack overflow occurs
*/
void vApplicationStackOverflowHook(xTaskHandle *pxTask char *pcTaskName);
/**/
#endif  // DISPENSER_H_
/**/
/*EOF*/