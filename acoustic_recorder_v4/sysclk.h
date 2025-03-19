

#ifndef _SYSCLK_H_
#define _SYSCLK_H_

#include "ezdsp5535.h"
#include "csl_pll.h"



#define REF_CLOCK 32768


#define PLLCNTL1_MASK 0x8000
#define PLLOUTCNTL_MASK 0x0200
#define PLLINCNTL_REGVALUE 0x8000	//CGCR2   set value
#define PLLCNTL2_REGVALUE 0x0806	//CGCR3   set value     

//sysclk after pll post divider 
#define SYSCLK_CLOCK_BAT_IDLE 	5632000
#define SYSCLK_CLOCK_BAT_RUN	22528000
#define SYSCLK_CLOCK_FIR_HIGH   4096000
#define SYSCLK_CLOCK_FIR_LOW	1536000 //6144000
#define SYSCLK_CLOCK_COMMAND	7680000 //

//output clock from pll (pll clock must be between 60Mhz and 120Mhz)
#define PLL_CLOCK_BAT_IDLE 	90112000
#define PLL_CLOCK_BAT_RUN 	90112000
#define PLL_CLOCK_FIR_HIGH 	81920000
#define PLL_CLOCK_FIR_LOW   61440000
#define PLL_CLOCK_COMMAND   61440000

// for PLLCNTRL1 register
#define PLLCNTL1_BAT_IDLE  (((PLL_CLOCK_BAT_IDLE / REF_CLOCK) - 4) |  PLLCNTL1_MASK)
#define PLLCNTL1_BAT_RUN   (((PLL_CLOCK_BAT_RUN / REF_CLOCK) - 4) |  PLLCNTL1_MASK)
#define PLLCNTL1_FIR_HIGH  (((PLL_CLOCK_FIR_HIGH / REF_CLOCK) - 4) |  PLLCNTL1_MASK)
#define PLLCNTL1_FIR_LOW   (((PLL_CLOCK_FIR_LOW / REF_CLOCK) - 4) |  PLLCNTL1_MASK)
#define PLLCNTL1_COMMAND   (((PLL_CLOCK_COMMAND / REF_CLOCK) - 4) |  PLLCNTL1_MASK)

// for PLLOUTCNTL register
#define PLLOUTCNTL_BAT_IDLE (((PLL_CLOCK_BAT_IDLE / SYSCLK_CLOCK_BAT_IDLE) - 1) | PLLOUTCNTL_MASK)
#define PLLOUTCNTL_BAT_RUN  (((PLL_CLOCK_BAT_RUN / SYSCLK_CLOCK_BAT_RUN) - 1)  | PLLOUTCNTL_MASK)
#define PLLOUTCNTL_FIR_HIGH (((PLL_CLOCK_FIR_HIGH / SYSCLK_CLOCK_FIR_HIGH) - 1)  | PLLOUTCNTL_MASK)
#define PLLOUTCNTL_FIR_LOW  (((PLL_CLOCK_FIR_LOW / SYSCLK_CLOCK_FIR_LOW) - 1) | PLLOUTCNTL_MASK)
#define PLLOUTCNTL_COMMAND  (((PLL_CLOCK_COMMAND / SYSCLK_CLOCK_COMMAND) - 1) | PLLOUTCNTL_MASK)



#if (PLL_CLOCK_BAT_IDLE % 32768 !=0 ) || ( PLL_CLOCK_BAT_IDLE % SYSCLK_CLOCK_BAT_IDLE != 0)
	#error "BAT IDLE pll clock not divisible by sysclk or refclock"
#elif (PLL_CLOCK_BAT_RUN % 32768 !=0 ) || ( PLL_CLOCK_BAT_RUN % SYSCLK_CLOCK_BAT_RUN != 0)
	#error "BAT RUN pll clock not divisible by sysclk or refclock"
#elif (PLL_CLOCK_FIR_HIGH % 32768 !=0 ) || ( PLL_CLOCK_FIR_HIGH % SYSCLK_CLOCK_FIR_HIGH != 0)
	#error "FIR_HIGH pll clock not divisible by sysclk or refclock"
#elif (PLL_CLOCK_FIR_LOW % 32768 !=0 ) || ( PLL_CLOCK_FIR_LOW % SYSCLK_CLOCK_FIR_LOW != 0)
	#error "FIR LOW pll clock not divisible by sysclk or refclock"
#elif (PLL_CLOCK_COMMAND % 32768 !=0 ) || ( PLL_CLOCK_COMMAND % SYSCLK_CLOCK_COMMAND != 0)
	#error "Command pll clock not divisible by sysclk or refclock"
#endif




typedef enum
{
	BAT_IDLE_CLK,
	BAT_RUN_CLK,
	FIR_HIGH_CLK,
	FIR_LOW_CLK,
	COMMAND_CLK
} SysClk_Typedef;

	
	


CSL_Status SwitchSysClk(SysClk_Typedef clkspd);
void Enable_PLL(void);
Uint32 getSysClk(void);






#endif











