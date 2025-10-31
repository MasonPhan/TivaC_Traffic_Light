#include <stdint.h>
#include "SysTick.h"
#include "PLL.h"
#include "tm4c123gh6pm.h"

void PortE_Init(void){
    SYSCTL_RCGCGPIO_R |= 0x32;      // enable clocks (B, E, F as needed)
    GPIO_PORTE_AMSEL_R = 0x00;
    GPIO_PORTE_PCTL_R  = 0x00000000;
    GPIO_PORTE_DIR_R   = 0x00;      // input
    GPIO_PORTE_AFSEL_R = 0x00;
    GPIO_PORTE_DEN_R   = 0x07;      // PE0-2 digital
}

void PortB_Init(void){
    SYSCTL_RCGCGPIO_R |= 0x32;
    GPIO_PORTB_AMSEL_R = 0x00;
    GPIO_PORTB_PCTL_R  = 0x00000000;
    GPIO_PORTB_DIR_R   = 0x3F;      // PB0-5 out
    GPIO_PORTB_AFSEL_R = 0x00;
    GPIO_PORTB_DEN_R   = 0x3F;
}

void PortF_Init(void){
    SYSCTL_RCGCGPIO_R |= 0x32;
    GPIO_PORTF_AMSEL_R = 0x00;
    GPIO_PORTF_PCTL_R  = 0x00000000;
    GPIO_PORTF_DIR_R   = 0x0E;      // PF1-3 out
    GPIO_PORTF_AFSEL_R = 0x00;
    GPIO_PORTF_DEN_R   = 0x0E;
}

const struct State {
    uint32_t Out;
    uint32_t PedOut;
    uint32_t Time;
    uint32_t Next[8];   // 3-bit input → 8 next states
};
typedef const struct State STyp;

#define goS   0
#define waitS 1
#define goW   2
#define waitW 3
#define ped   4
#define ped5  5
#define ped6  6
#define ped7  7
#define ped8  8

STyp FSM[9] = {
  // Out, PedOut, Time,             inputs 0..7
  {0x21,0x02,300,{goS,goS,waitS,waitS,waitS,waitS,waitS,waitS}},
  {0x22,0x02,100,{goW,goW,goW,goW,ped,ped,ped,ped}},
  {0x0C,0x02,300,{goW,waitW,waitW,waitW,waitW,waitW,waitW,waitW}},
  {0x14,0x02,100,{goS,goS,goS,goS,ped,ped,ped,ped}},
  // you need to trim the remaining ped states to 8 entries too
  {0x24,0x08,100,{ped,ped5,ped5,ped5,ped5,ped,ped5,ped}},
  {0x24,0x00,25, {ped6,ped6,ped6,ped6,ped,ped6,ped6,ped}},
  {0x24,0x00,25, {ped7,ped7,ped7,ped7,ped,ped7,ped7,ped7}},
  {0x24,0x00,10, {ped8,ped8,ped8,ped8,ped,ped8,ped8,ped8}},
  {0x24,0x00,10, {goS,goS,goW,goS,ped,goS,goW,goS}}
};

int main(void){
    PLL_Init(Bus80MHz);
    PortE_Init();
    PortB_Init();
    PortF_Init();
    SysTick_Init();

    uint32_t Point = goS;
    uint32_t Input;

    while(1){
        GPIO_PORTB_DATA_R = FSM[Point].Out;
        GPIO_PORTF_DATA_R = FSM[Point].PedOut;
        SysTick_Wait10ms(FSM[Point].Time);
        Input = GPIO_PORTE_DATA_R & 0x07;   // make extra sure it’s 0..7
        Point = FSM[Point].Next[Input];
    }
}
