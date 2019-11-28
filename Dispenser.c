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
/* holds pin values for all buttons used */
int buttons[NUM_BUTTONS] = { P2, P3, P4 };
/* holds pin values for all LEDs used */
int LEDS[NUM_LEDS] = { P7 };
/* holds any scheduled times */
long scheduled_dispenses[MAX_SCHEDULES];
/**/
void setup(void) {
  Servo_Init();  //--------------------------------------------------------------
  LED_Init();  // warning light - A7
  Button_Init();  // open 3D device - A6
  UART_Init();  // I/O with PuTTy
}
/**/
void TransmitData(char* data) {
  char* temp = data;
  while(*temp != '\0') {
    // STEP 3:
    // Here you check the flag to see
    // if the UART is available
    while ((UART_FR & (1<<5)) != 0) {
      // wait
    }
    UART_DR = *temp;
    temp++;
  }
}
/**/
void LCD_Update(void *p) {
  static int curr_led_tick_time = 0;
  static int prev_led_tick_time = 0;
  while (1) {
    curr_led_tick_time = xTaskGetTickCount();
    if (lcd_update_status) {
      //------------------------------------------------------------------&&&&&&&
      // begin accepting client input for a new scheduled time
      char* data = "both pressed";   // \r\n";
      TransmitData(data);
      // clear flag 
      lcd_update_status = 0;
    } else if (curr_led_tick_time - prev_led_tick_time > 5000) {
      // regular change of I/O
      char* data = "regular change";   // \r\n";
      TransmitData(data);
    } else {
      // no change
      prev_led_tick_time = curr_led_tick_time;
    }
  }
}
//------------------------------------------------------------------------------
/**/
void Servo_Init() {
  
}
//------------------------------------------------------------------------------
/**/
/**/
/*
  Determines if a client is attempting to:
    add a new time when the 3D device should operate
    sets global flag if both buttons pressed
      - flag triggers I/O process
      - I/O requests information from client
*/
void Client_Schedule(void *p) {
  while (1) {
    // check both buttons pressed
    // PA2 & PA3
    if (up_button_pressed() & down_button_pressed()) {
      // display message
      lcd_update_status = 1;
    }
  }
}
/**/
/*
  Instructs servo to open 3D device

  - Opens device iff:
     Open Button being pressed
     scheduled_open_status == 1
*/
void Servo_Open(void) {
  
}
/**/
/*
  Determines if any Record in records are due for servicing
  modifies: scheduled_open_status
  effects: scheduled_open_status = 1 iff any record is due for servicing 
*/
void Schedule_Check(void) {
  
}
/**/
/*
  Instructs servo to close 3D device

  - Closes device iff:
     close_status == 1 
*/
void Servo_Close(void) {
  
}
/**/
/*

*/
void FSM(void) {
  
}
/**/ 
void vApplicationStackOverflowHook(xTaskHandle *pxTask char *pcTaskName) {
  // This function can not return, so loop forever.  Interrupts are disabled
  // on entry to this function, so no processor interrupts will interrupt
  // this loop.
  while (1) {}
}
/**/
int override_button_pressed() {
  return switch_input(P4);
}
/**/
int up_button_pressed(){
  return switch_input(P3); 
}
/**/
int down_button_pressed() {
  return switch_input(P2);
}
/**/
/* Initializer Methods */
/**/
void Button_Init(void) {
  // SYSCTL_RCGC2_R -> port A initialized via initialize_led----------------------
  // Loop Inv - for every PIN, p, in buttons:
  //  a PIN on PORT A has been setup for accepting button presses
  for (int i = 0; i < NUM_BUTTONS; i++) {
    int PIN = buttons[i];
    TGPIO_PORTA_AMSEL_R &= ~PIN;  // disable analog
    TGPIO_PORTA_PCTL_R &= ~(mask_regular_GPIO(PIN));  // regular GPIO
    TGPIO_PORTA_DIR_R &= ~PIN;  // set as input;
    TGPIO_PORTA_AFSEL_R &= ~PIN;  // reg port function
    TGPIO_PORTA_DEN_R |= PIN;  // enable digital port
  }
}
/**/
void LED_Init(void) {
  // Loop Inv - for every PIN, p, in buttons:
  //  a PIN on PORT A has been setup for an LED
  for (int i = 0; i < NUM_LEDS; i++) {
    int PORT = LEDS[i];
    TGPIO_PORTA_PCTL_R &= ~(mask_regular_GPIO(PORT));  // reg GPIO
    TGPIO_PORTA_AMSEL_R &= ~PORT;  // disable analog
    TGPIO_PORTA_DIR_R |= PORT;  // output
    TGPIO_PORTA_AFSEL_R &= ~PORT;  // reg port function
    TGPIO_PORTA_DEN_R |= PORT;
    TGPIO_PORTA_DATA_R &= ~PORT;
  }
}
/**/
void UART_Init() {
  // configure GPIO pins
  TSYSCTL_RCGCGPIO_R |= (1<<1);  // enable clock to PortB
  TGPIO_PORTB_AFSEL_R = (0x3);  // allow alternative function for PB 0&1
  TGPIO_PORTB_DEN_R = (0x3);  // enable digital pins
  TGPIO_PORTB_PCTL_R &= ~(0xF);  // clear
  TGPIO_PORTB_PCTL_R = (1)|(1<<4);  // set alternative function to UART
  // Configure the UART Module
  SYSCTL_RCGCUART_R  |= (1<<1);  // enable clock to UART Module 1
  int32_t delay = SYSCTL_RCGCUART_R;  // delay to wait for clock setup
  TUART1_CTL_R &= ~(0x1);   // diable module before configuration
  // set the baud rate (integer & fraction parts)
  TUART1_IBRD_R = IBRD;
  TUART1_FBRD_R = FBRD;
  TUART1_LCRH_R &= ~(1<<1);  // disable parity
  TUART1_LCRH_R &= ~(1<<3);  // 1 stop bit
  TUART1_LCRH_R |= (0x3<<5);  // 8 bit word len
  TUART1_CC_R &= ~(0xF);  // use system clock
  TUART1_CTL_R = (0x1)|(1<<8)|(1<<9);  // enable UART Module
}
/**/
/*EOF*/