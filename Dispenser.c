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
#include "SSD2119.h"
#include "Dispenser.h"
/**/
xSemaphoreHandle printing_Semaphore;
char* hours[24] = {"00", "01", "02", "03", "04", "05", "06", "07", "08", "09",
                   "10", "11", "12", "13", "14", "15", "16", "17", "18", "19",
                   "20", "21", "22", "23"};
char* mins[60] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", 
                  "12", "13", "14", "15", "16", "17", "18", "19", "20", "21", 
                  "22", "23", "24", "25", "26", "27", "28", "29", "30", "31",
                  "32", "33", "34", "35", "36", "37", "38", "39", "40", "41",
                  "42", "43", "44", "45", "46", "47", "48", "49", "50", "51",
                  "52", "53", "54", "55", "56", "57", "58", "59"};
/**/
int num_sched = 0;
/* global clock varaible */
int local_time = 0; 
/**/
volatile int scheduled_open_status = 0;
/* flag to close 3D device */
int close_status = 0;
/* flag to update output */
int lcd_update_status = 0;
/* holds pin values for all buttons used */
int buttons[NUM_BUTTONS] = { P2, P3, P4 };
/* holds pin values for all LEDs used */
int LEDS[NUM_LEDS] = { P7 };
/* holds any scheduled times */
Booking scheduled_dispenses[MAX_SCHEDULES];
int IBRD = 8, FBRD = 44;
/**/
int main (int argc, char **argv) {
  setup();
  xTaskCreate(Client_Schedule, (const char *)"Client Schedule", 1024, NULL,
              tskIDLE_PRIORITY + 3, NULL);
  xTaskCreate(LCD_Update, (const char *)"Update I/O", 1024, NULL,
              tskIDLE_PRIORITY + 2, NULL);
  xTaskCreate(Schedule_Check, (const char *)"Check Schedule", 1024, NULL,
              tskIDLE_PRIORITY + 1, NULL);
  xTaskCreate(Servo_Open, (const char *)"Servo Open", 1024, NULL,
              tskIDLE_PRIORITY + 3, NULL);
  xTaskCreate(Clock_Update, (const char *)"Clock Update", 1024, NULL,
              tskIDLE_PRIORITY + 1, NULL);
  //
  vTaskStartScheduler();
  while (1) {}
}
//------------------------------------------------------------------------------
/**/
void Clock_Update(void *p) {
  while(1) {
    int x = xTaskGetTickCount();
    local_time = x % MILLI_IN_DAY;
    //
    vTaskDelay(1);
  }
}
/**/
void Servo_Open(void *p) {
  while (1) {
    if (scheduled_open_status) {
      char *c = "just opened servo";
      TransmitData(c);
      scheduled_open_status = 0;  // clear flag
    }
    vTaskDelay(1);
  }
}
/**/
void Servo_Close(void *p) {
  
}
/**/
void Servo_Init() {
  // configuring pins and stuff  
}
/**/
// CHANGE - currently configured for use with putty
void UART_Init() {
  TGPIO_PORTD_LOCK_R = 0x4C4F434B;  // unlock reg for PD7
  TGPIO_PORTD_CR_R |= (1<<7);  // removing GPIO on PD7
  //
  TSYSCTL_RCGCGPIO_R |= (1<<3);  // enable clock to Port D
  TGPIO_PORTD_AFSEL_R = (0x3<<6);  // allow alternative function for PD6/PD7
  TGPIO_PORTD_DEN_R = (0x3<<6);  // enable digital pins
  //TGPIO_PORTD_PCTL_R &= ~(0xFF << 24);  // clear
  TGPIO_PORTD_PCTL_R  = (1<<24)|(1<<28);  // set alternative function to UART
  // Configure the UART Module
  TSYSCTL_RCGCUART_R  |= (1<<2);  // enable clock to UART Module 2
  int32_t delay = TSYSCTL_RCGCUART_R;  // delay to wait for clock setup
  TUART2_CTL_R &= ~(0x1);   // diable module before configuration
  // set the baud rate (integer & fraction parts)
  TUART2_IBRD_R = IBRD;
  TUART2_FBRD_R = FBRD;
  TUART2_LCRH_R &= ~(1<<1);  // disable parity
  TUART2_LCRH_R &= ~(1<<3);  // 1 stop bit
  TUART2_LCRH_R |= (0x3<<5);  // 8 bit word len
  TUART2_CC_R &= ~(0xF);  // use system clock
  TUART2_CTL_R = (0x1)|(1<<8)|(1<<9);  // enable UART Module
}
//------------------------------------------------------------------------------
/**/
void setup(void) {
  TSYSCTL_RCGC2_R |= TSYSCTL_RCGC2_GPIOD;  // activate PORT D
  TSYSCTL_RCGC2_R |= TSYSCTL_RCGC2_GPIOF;  // activate PORT F
  vSemaphoreCreateBinary(printing_Semaphore);
  Servo_Init();
  UART_Init();  // currently: I/O with PuTTy
  Button_Init();
  //LCD_Init();-----------------------------------------------------------------
}
/**/
void TransmitData(char* data) {
  char* temp = data;
  while(*temp != '\0') {
    while ((TUART2_FR_R & (1<<5)) != 0) {
      // wait
    }
    TUART2_DR_R = *temp;
    temp++;
  }
  //StringToChar(data);  // LCD
}
/**/
void Client_Schedule(void *p) {
  static int curr_cs_tick_time = 0;
  static int prev_cs_tick_time = 0;
  while (1) {
    curr_cs_tick_time = xTaskGetTickCount();
    if (override_button_pressed()) {
        // longer then 1/2 second?
        if (curr_cs_tick_time - prev_cs_tick_time > 1000) {
          // schedule client
          if (num_sched < MAX_SCHEDULES) {
            lcd_update_status = 1;
            prev_cs_tick_time = curr_cs_tick_time;
            pop_button();
          }
        }
    } else {
          prev_cs_tick_time = curr_cs_tick_time;
      }
    vTaskDelay(1);
  }
}
/**/
void Book(int hours, int mins) {
  int h = hours * 60 * 60 * 1000;
  int m = mins * 60 * 1000;
  long time = h + m;
  Booking b;
  b.time = time;
  b.triggered_today = (time > local_time ? 0 : 1);
  scheduled_dispenses[num_sched++] = b;
}
/**/
void LCD_Update(void *p) {
  while (1) {
    //curr_lcd_tick_time = xTaskGetTickCount();
    if (lcd_update_status) {
      // begin accepting client input for a new scheduled time
      xSemaphoreTake(printing_Semaphore, portMAX_DELAY);
      int hours = GetHours();
      int mins = GetMinutes();
      // clear flag, release lock 
      xSemaphoreGive(printing_Semaphore);
      Book(hours, mins);
      lcd_update_status = 0;
    } 
    //prev_lcd_tick_time = curr_lcd_tick_time;
    vTaskDelay(1);
  }
}
/**/
void Schedule_Check(void *p) {
  while (1) {
    for (int i = 0; i < num_sched; i++) {
      if (scheduled_dispenses[i].time < local_time) {
        if (scheduled_dispenses[i].triggered_today == 0) {
          scheduled_dispenses[i].triggered_today = 1;
          scheduled_open_status = 1;
          break;  // we will be back
        }
      } else {
        scheduled_dispenses[i].triggered_today = 0;
      }
    }
    vTaskDelay(1);
  }
}
/**/
/*

*/
void FSM(void) {
  
}
/**/ 
void vApplicationStackOverflowHook(xTaskHandle *pxTask, char *pcTaskName) {
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
  return switch_input(P2); 
}
/**/
int down_button_pressed() {
  return switch_input(P3);
}
/**/
int switch_input(int PIN) {
  if (TGPIO_PORTF_DATA_R & PIN){
    pop_button();
    return (TGPIO_PORTF_DATA_R & PIN);
  }
  return 0;
}
/**/
void pop_button(void) {
  int c = 0;
  for(int i = 0;i < 10000500;i++){
    c++;
  }
}
/**/
/* LCD */
/**/
void StringToChar(char * sentence){
  //conver string to char.
  while (*sentence) { //traverse character array (String)
    LCD_PrintChar(*(sentence++)); //print char to the LCD
  }
}
/**/
/* Initializer Methods */
/**/
void Button_Init(void) {
  for (int i = 0; i < NUM_BUTTONS; i++) {
    int PIN = buttons[i];
    TGPIO_PORTF_AMSEL_R &= ~PIN;  // disable analog
    TGPIO_PORTF_PCTL_R &= ~(mask_regular_GPIO(PIN));  // regular GPIO
    TGPIO_PORTF_DIR_R &= ~PIN;  // set as input;
    TGPIO_PORTF_AFSEL_R &= ~PIN;  // reg port function
    TGPIO_PORTF_PDR_R |= PIN; // pull down resistor
    TGPIO_PORTF_DEN_R |= PIN;  // enable digital port
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
int GetMinutes(void) {
  static int curr_lcd_tick_time = 0;
  static int prev_lcd_tick_time = 0;
 char* d = "Choose Minute:\r\n";
      TransmitData(d);
      int curMin = 0;  // midnight/noon
      TransmitData(mins[curMin]);
      int set = 1;
      pop_button();
      // loop until user hits override button
      while (set) {
        curr_lcd_tick_time = xTaskGetTickCount();
        if (up_button_pressed()) {
          int print = 1;
          prev_lcd_tick_time = curr_lcd_tick_time;
          while (curr_lcd_tick_time - prev_lcd_tick_time < 500){
            if (!up_button_pressed()) {
              print = 0;
              break;
            }
            curr_lcd_tick_time = xTaskGetTickCount();
          }
          if (print) {
            curMin = (curMin + 1) % 60;
            TransmitData(mins[curMin]);
          }
        } else if (down_button_pressed()) {
            int print = 1;
            prev_lcd_tick_time = curr_lcd_tick_time;
            while (curr_lcd_tick_time - prev_lcd_tick_time < 500) {
              if (!down_button_pressed()) {
                print = 0;
                break;
              }
              curr_lcd_tick_time = xTaskGetTickCount();
            }
            if (print) {
              curMin = (curMin + 59) % 60;  // weird subtraction
              TransmitData(mins[curMin]);
            }
        } else if (override_button_pressed()) {
           int print = 1;
           prev_lcd_tick_time = curr_lcd_tick_time;
           while (curr_lcd_tick_time - prev_lcd_tick_time < 500) {
             if (!override_button_pressed()) {
                print = 0;
                break;
             }
             curr_lcd_tick_time = xTaskGetTickCount();
           }
            if (print) {
              char *c = "done with minutes\r\n";
              TransmitData(c);
              set = 0;
            }
        }
      }  // end-while  
      return curMin;
}
/**/
int GetHours(void) {
  static int curr_lcd_tick_time = 0;
  static int prev_lcd_tick_time = 0;
 char* d = "Choose Hour:\r\n";
      TransmitData(d);
      int curHour = 0;  // midnight
      TransmitData(hours[curHour]);
      int set = 1;
      pop_button();
      // loop until user hits override button
      while (set) {
        curr_lcd_tick_time = xTaskGetTickCount();
        if (up_button_pressed()) {
          int print = 1;
          prev_lcd_tick_time = curr_lcd_tick_time;
          while (curr_lcd_tick_time - prev_lcd_tick_time < 500){
            if (!up_button_pressed()) {
              print = 0;
              break;
            }
            curr_lcd_tick_time = xTaskGetTickCount();
          }
          if (print) {
            curHour = (curHour + 1) % 24;
            TransmitData(hours[curHour]);
          }
        } else if (down_button_pressed()) {
            int print = 1;
            prev_lcd_tick_time = curr_lcd_tick_time;
            while (curr_lcd_tick_time - prev_lcd_tick_time < 500) {
              if (!down_button_pressed()) {
                print = 0;
                break;
              }
              curr_lcd_tick_time = xTaskGetTickCount();
            }
            if (print) {
              curHour = (curHour + 23) % 24;  // weird subtraction
              TransmitData(hours[curHour]);
            }
        } else if (override_button_pressed()) {
           int print = 1;
           prev_lcd_tick_time = curr_lcd_tick_time;
           while (curr_lcd_tick_time - prev_lcd_tick_time < 500) {
             if (!override_button_pressed()) {
                print = 0;
                break;
             }
             curr_lcd_tick_time = xTaskGetTickCount();
           }
            if (print) {
              char *c = "done with hours\r\n";
              TransmitData(c);
              set = 0;
            }
        }
      }  // end-while  
      return curHour;
}
/**/
int mask_regular_GPIO(int PIN) {
  int mask = 0xF;
  PIN *= 4;
  return (mask << PIN);
}
/**/
/*EOF*/