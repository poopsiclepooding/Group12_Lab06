#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"

volatile uint32_t dutycycle = 50;
volatile uint32_t delaycounter = 10;
volatile bool buttonpressed = false;
volatile bool button1 = false;
volatile bool button2 = false;
volatile uint32_t freq = 100;                                       // This is frequency of PWN in kHz

void PortF_INIT(void)
{
    //---------------------- SET UP GPIO PORT F DATA REGISTERS -------------------------------
    SYSCTL_RCGCGPIO_R |= 0x20;                                      // This enables clock for the GPIO Port F Register
    while ((SYSCTL_PRGPIO_R & 0x20) == 0) {};                       // This waits till clock is enabled
    GPIO_PORTF_LOCK_R = 0x4C4F434B;                                 // This unlocks Port F for working
    GPIO_PORTF_CR_R = 0xFF;                                         // This allows changing control registers of specific GPIO pins
    GPIO_PORTF_AFSEL_R = 0x02;                                      // This allows PF2 to be controlled by PWN signal
    GPIO_PORTF_PCTL_R &= 0xF0;                                      // This clears PF2 Port Control
    GPIO_PORTF_PCTL_R |= 0x50;                                      // This sets PF2 Port Control for PWN
    GPIO_PORTF_DIR_R = 0x0E;                                        // This sets input or output for each pin
    GPIO_PORTF_PUR_R = 0x11;                                        // This sets pull up resistors for each pin
    GPIO_PORTF_DEN_R = 0x1F;                                        // This enables each pin

    //---------------------- SET UP PWN REGISTERS -------------------------------
    SYSCTL_RCGCPWM_R |= 0x00000002;
    SYSCTL_RCC_R |= 0x00100000;
    SYSCTL_RCC_R &= ~0x000E0000;
    SYSCTL_RCC_R |= 0x000A0000;
    PWM1_CTL_R |= 0x04;
    PWM1_2_CTL_R = 0x00;
    PWM1_2_GENA_R = 0x0000008C;
    PWM1_2_LOAD_R = 250000 / freq - 1;
    PWM1_2_CMPA_R = (PWM1_2_LOAD_R * (100 - dutycycle)) / 100;
    PWM1_2_CTL_R |= 0x01;
    PWM1_ENABLE_R |= 0x20;

    //---------------------- SET UP GPIO PORT F INTERRUPT REGISTERS -------------------------------
    GPIO_PORTF_IM_R &= ~0x11;                                       // This sets mask so that control registers for interrupts can be changed
    GPIO_PORTF_IS_R &= ~0x11;                                       // This clears IS PF0, PF4 for enabling edge detection
    GPIO_PORTF_IEV_R |= 0x11;                                       // This sets IEV PF0, PF4 for rising edge detection
    GPIO_PORTF_ICR_R = 0x11;                                        // This sets ICR PF0, PF4 to clear interrupts for it
    GPIO_PORTF_IM_R |= 0x11;                                        // This un-masks the interrupts again

    NVIC_EN0_R = (1 << 30);                                         // This enables Port F interrupts from NVIC
}

void GPIOPortFHandler(void){
    if (GPIO_PORTF_RIS_R & 0x01) {                                  // This checks if interrupt came from PF0
        GPIO_PORTF_ICR_R = 0x01;                                    // This clears interrupt flag for PF0
        buttonpressed = true;                                       // This toggles data in Port F Pin 3
        button1 = true;                                             // This toggles data in Port F Pin 3
    }
    if (GPIO_PORTF_RIS_R & 0x10) {                                  // This checks if interrupt came from PF4
        GPIO_PORTF_ICR_R = 0x10;                                    // This clears interrupt flag for PF4
        buttonpressed = true;                                       // This toggles data in Port F Pin 3
        button2 = true;                                             // This toggles data in Port F Pin 3
    }
}

void Delay(uint32_t count) {
    while (count--) {
        volatile uint32_t i;
        for (i = 0; i < 1000; i++) {
            // Empty loop for delay
        }
    }
}
int main(void)
{
    PortF_INIT();                                                   // This calls Port F initialization
    while (1){
        if (buttonpressed){
            Delay(delaycounter);

            if (button1){
                if (dutycycle<=95){
                    dutycycle += 5;
                    PWM1_2_CMPA_R = (PWM1_2_LOAD_R * (100 - dutycycle)) / 100;
                }
                else{
                    dutycycle = 100;
                    PWM1_2_CMPA_R = (PWM1_2_LOAD_R * (100 - dutycycle)) / 100;
                }
            }

            if (button2){
                if (dutycycle>=5){
                    dutycycle -= 5;
                    PWM1_2_CMPA_R = (PWM1_2_LOAD_R * (100 - dutycycle)) / 100;
                }
                else{
                    dutycycle = 0;
                    PWM1_2_CMPA_R = (PWM1_2_LOAD_R * (100 - dutycycle)) / 100;
                }
            }
            buttonpressed = false;
            button1 = false;
            button2 = false;
        }
    }
	return 0;
}



