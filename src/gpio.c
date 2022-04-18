/*
  gpio.c

   Created on: Dec 12, 2018
       Author: Dan Walkes
   Updated by Dave Sluiter Dec 31, 2020. Minor edits with #defines.

   March 17
   Dave Sluiter: Use this file to define functions that set up or control GPIOs.

 */


// *****************************************************************************
// Students:
// We will be creating additional functions that configure and manipulate GPIOs.
// For any new GPIO function you create, place that function in this file.
// *****************************************************************************

#include <stdbool.h>
#include "em_gpio.h"
#include <string.h>


// Student Edit: Define these, 0's are placeholder values.
// See the radio board user guide at https://www.silabs.com/documents/login/user-guides/ug279-brd4104a-user-guide.pdf
// and GPIO documentation at https://siliconlabs.github.io/Gecko_SDK_Doc/efm32g/html/group__GPIO.html
// to determine the correct values for these.

#define LED0_port  gpioPortF //since LED0 is at portF, wkt port F is numbered 5
#define LED0_pin   4         //since LED0 is at PF4
#define LED1_port  gpioPortF //since LED1 is at portF, wkt port F is numbered 5
#define LED1_pin   5         //since LED1 is at PF5

#define SNSR_EN_port gpioPortD // Sensor Enable port set to PortD
#define SNSR_EN_pin 15         // Sensor Enable pin set to 15

#define EXT_COM_IN_port gpioPortD // EXT COM INPUT Enable port set to PortD
#define EXT_COM_IN_pin 13         // EXT COM INPUT Enabke pin set to 13



#include "gpio.h"




// Set GPIO drive strengths and modes of operation
void gpioInit()
{

  // Student Edit:

  GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthStrongAlternateStrong);
  //GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthWeakAlternateWeak);
  GPIO_PinModeSet(LED0_port, LED0_pin, gpioModePushPull, false);

  //GPIO_DriveStrengthSet(LED1_port, gpioDriveStrengthStrongAlternateStrong);
  //GPIO_DriveStrengthSet(LED1_port, gpioDriveStrengthWeakAlternateWeak);
  //GPIO_PinModeSet(LED1_port, LED1_pin, gpioModePushPull, false);

  GPIO_DriveStrengthSet(SNSR_EN_port, gpioDriveStrengthStrongAlternateStrong);
  //GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthWeakAlternateWeak);
  GPIO_PinModeSet(SNSR_EN_port, SNSR_EN_pin, gpioModePushPull, false);

  GPIO_DriveStrengthSet(EXT_COM_IN_port, gpioDriveStrengthStrongAlternateStrong);
  //GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthWeakAlternateWeak);
  GPIO_PinModeSet(EXT_COM_IN_port, EXT_COM_IN_pin, gpioModePushPull, false);
} // gpioInit()


void gpioLed0SetOn()
{
  GPIO_PinOutSet(LED0_port,LED0_pin);
}


void gpioLed0SetOff()
{
  GPIO_PinOutClear(LED0_port,LED0_pin);
}


void gpioLed1SetOn()
{
  GPIO_PinOutSet(LED1_port,LED1_pin);
}


void gpioLed1SetOff()
{
  GPIO_PinOutClear(LED1_port,LED1_pin);
}

void gpioSensorEnSetOn(){
  GPIO_PinOutSet(SNSR_EN_port, SNSR_EN_pin);
}

/* GPIO function ON for External Communication Input ON */
void gpioextcominOn(){
  GPIO_PinOutSet(EXT_COM_IN_port, EXT_COM_IN_pin);

}

/* GPIO function OFF for External Communication Input ON */
void gpioextcominOff(){
  GPIO_PinOutClear(EXT_COM_IN_port, EXT_COM_IN_pin);

}

/*Connection for GPIO Set Display for External Communication Input*/
void gpioSetDisplayExtcomin(bool value){

  if(value == 1){
      gpioextcominOn();
  }
  else if(value == 0){
      gpioextcominOff();
  }
  else{
      //No Action
  }
}




