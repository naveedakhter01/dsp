
#include "sysclk.h"



#define CSL_SYS_CGCR1_MH_MASK            (0x0FFFu)


#define CSL_PLL_DIV_000    (0)
#define CSL_PLL_DIV_001    (1u)
#define CSL_PLL_DIV_002    (2u)
#define CSL_PLL_DIV_003    (3u)
#define CSL_PLL_DIV_004    (4u)
#define CSL_PLL_DIV_005    (5u)
#define CSL_PLL_DIV_006    (6u)
#define CSL_PLL_DIV_007    (7u)

#define CSL_PLL_CLOCKIN    REF_CLOCK //(32768u)



/***********************************************
* SWITCH CLOCK SPEED
* 
* - powerdown pll and set to bypassmode
* - write new config values 
* 
*/

CSL_Status SwitchSysClk(SysClk_Typedef clkspd)
{
         PLL_Obj      pllObj;
         PLL_Config      configInfo;
         PLL_Handle      hPll;
         Uint32         pllInstId;
         PLL_Config     *pconfigInfo;
         Uint16       timeout = TIMEOUT;
         CSL_Status      status = CSL_SOK;
         Uint16         temp;
         
         pllInstId = 0;
         status = PLL_init(&pllObj,pllInstId);
         hPll = &pllObj;
       if(clkspd == BAT_IDLE_CLK)
       {  
         configInfo.PLLCNTL1 = PLLCNTL1_BAT_IDLE;     //32768 * (PLLCNTL1 & 0x0fff) =  clock rate   //CGCR1
         configInfo.PLLOUTCNTL = PLLOUTCNTL_BAT_IDLE;          //CGCR4 output divider   1
       } 
       else if (clkspd == BAT_RUN_CLK) //FAST
       {  
         configInfo.PLLCNTL1 = PLLCNTL1_BAT_RUN;     //32768 * (PLLCNTL1 & 0x0fff) =  clock rate   //CGCR1
         configInfo.PLLOUTCNTL = PLLOUTCNTL_BAT_RUN;          //CGCR4 output divider   3+1 = 4 ; 90112000/3 = 22528000
       }        
       else if (clkspd == FIR_HIGH_CLK)
       {
         configInfo.PLLCNTL1 = PLLCNTL1_FIR_HIGH;     //   32768 * ((PLLCNTL1 & 0x0fff)+4) =  clock rate, so 32768 * 2500 (2496+4) = 81920000   //CGCR1
         configInfo.PLLOUTCNTL = PLLOUTCNTL_FIR_HIGH;          //CGCR4 output divider  div by 5(4+1) to = 16384000 ; 81920000/5 = 16384000
       }
       else if (clkspd == FIR_LOW_CLK)
       {
         configInfo.PLLCNTL1 = PLLCNTL1_FIR_LOW;     //   32768 * ((PLLCNTL1 & 0x0fff)+4) =  clock rate, so 32768 * 2625 (2621+4) = 86016000   //CGCR1
         configInfo.PLLOUTCNTL = PLLOUTCNTL_FIR_LOW;          //CGCR4 output divider  div by 14(13+1) to = 6144000 ; 86016000/14 = 6144000
       }
       else if (clkspd == COMMAND_CLK)
       {
         configInfo.PLLCNTL1 = PLLCNTL1_COMMAND;     //   32768 * ((PLLCNTL1 & 0x0fff)+4) =  clock rate, so 32768 * 2625 (2621+4) = 86016000   //CGCR1
         configInfo.PLLOUTCNTL = PLLOUTCNTL_COMMAND;          //CGCR4 output divider  div by 14(13+1) to = 6144000 ; 86016000/14 = 6144000
       }
       
       configInfo.PLLINCNTL = PLLINCNTL_REGVALUE;           //CGCR2
       configInfo.PLLCNTL2 = PLLCNTL2_REGVALUE;            //CGCR3
         //status = PLL_config(hPll, &configInfo);

         pconfigInfo = &configInfo;

   if(NULL == hPll)
    {
      status = CSL_ESYS_BADHANDLE;
      return status;
   }

   if(NULL == pconfigInfo)
    {
      status = CSL_ESYS_INVPARAMS;
      return status;
   }

   hPll->pllConfig = pconfigInfo;

    /* Force to BYPASS mode */
    CSL_FINST(hPll->sysAddr->CCR2, SYS_CCR2_SYSCLKSEL, BYPASS);
    CSL_FINST(hPll->sysAddr->CGCR1, SYS_CGCR1_PLL_PWRDN, POWERDWN);
    
    
    /* Set CLR_CTRL = 0 in CGCR1 */
    CSL_FINST(hPll->sysAddr->CGCR1, SYS_CGCR1_CLR_CNTL, CLEAR);

    hPll->sysAddr->CGICR = pconfigInfo->PLLINCNTL;
    hPll->sysAddr->CGOCR = pconfigInfo->PLLOUTCNTL;
   hPll->sysAddr->CGCR2 = pconfigInfo->PLLCNTL2;

    /*
     * Set PLL_PWRDN = 0, PLL_STANDBY = 0, CLR_CNTL = 1 and program MH in CGCR1
     * according to your required settings
     */
    CSL_FINST(hPll->sysAddr->CGCR1, SYS_CGCR1_PLL_PWRDN, POWERED);
    CSL_FINST(hPll->sysAddr->CGCR1, SYS_CGCR1_PLL_STANDBY, ACTIVE);
    CSL_FINST(hPll->sysAddr->CGCR1, SYS_CGCR1_CLR_CNTL, SET);

    temp =  hPll->sysAddr->CGCR1;

   hPll->sysAddr->CGCR1 = (CSL_SYS_CGCR1_MH_MASK & pconfigInfo->PLLCNTL1) | (temp & ~CSL_SYS_CGCR1_MH_MASK);

    while (!((hPll->sysAddr->CGCR2 & (0x0008u)) >> (0x0003u)) && timeout--) ;

   /* Select pll */
    CSL_FINST(hPll->sysAddr->CCR2, SYS_CCR2_SYSCLKSEL, LOCK);

    //select 1.05v core voltage
    *(volatile ioport Uint16 *)(0x7004) = 0x0002; 

   return (status);
}



/***********************************************
*ENABLES PLL
*/
void Enable_PLL(void)
{
	asm(" bit(ST1,#13) = #0 "); // Turn OFF XF

   // Enable clock to all peripherals
   asm("   *port(#0x1C02) = #0x0 ");
   asm("   *port(#0x1C03) = #0x0 ");

   // Bypass the PLL as the system clock source
   asm("   *port(#0x1C1F) = #0x0 "); //Clock Configuration MSW reg

    //  program PLL to 50MHz 
   asm("   *port(#0x1c20) = #0x8BE8 "); //PLL Control 1 reg
   asm("   *port(#0x1c21) = #0x8000 "); //PLL Control 2 reg   
   asm("   *port(#0x1c22) = #0x8006 "); //PLL Control 3 reg
    asm("   *port(#0x1C23) = #0x0000 "); //PLL Output Control4 reg   
   
   // wait 1 milli sec for PLL to lock
   asm("   repeat(0xC350)   "); 
   asm("      nop      ");   // wait 1ms @ 50Mhz: 0x001s * 50 Mhz = 50000
   
   asm("   *port(#0x1c1F) = #0x0001 "); // Clock configuration MSW reg

   // Program DSP_LDO to 1.05V
   asm("   *port(#0x7004) = #0x0002 ");

   // wait 1 milli sec
//   asm("   repeat(0xC350)   "); 
//   asm("      nop      ");   // wait 1ms @ 50Mhz: 0x001s * 50 Mhz = 50000
}




/**
 *  \brief  Function to calculate the clock at which system is running
 *
 *  \param    none
 *
 *  \return   System clock value in KHz
 */
#if (defined(CHIP_C5505_C5515) || defined(CHIP_C5504_C5514))

Uint32 getSysClk(void)
{
	Bool      pllRDBypass;
	Bool      pllOutDiv;
	Uint32    sysClk;
	Uint16    pllVP;
	Uint16    pllVS;
	Uint16    pllRD;
	Uint16    pllVO;

	pllVP = CSL_FEXT(CSL_SYSCTRL_REGS->CGCR1, SYS_CGCR1_VP);
	pllVS = CSL_FEXT(CSL_SYSCTRL_REGS->CGCR1, SYS_CGCR1_VS);

	pllRD = CSL_FEXT(CSL_SYSCTRL_REGS->CGICR, SYS_CGICR_RDRATIO);
	pllVO = CSL_FEXT(CSL_SYSCTRL_REGS->CGOCR, SYS_CGOCR_OD);

	pllRDBypass = CSL_FEXT(CSL_SYSCTRL_REGS->CGICR, SYS_CGICR_RDBYPASS);
	pllOutDiv   = CSL_FEXT(CSL_SYSCTRL_REGS->CGOCR, SYS_CGOCR_OUTDIVEN);

	sysClk = CSL_PLL_CLOCKIN;

	if (0 == pllRDBypass)
	{
		sysClk = sysClk/(pllRD + 4);
	}

	sysClk = (sysClk * ((pllVP << 2) + pllVS + 4));

	if (1 == pllOutDiv)
	{
		sysClk = sysClk/(pllVO + 1);
	}

	/* Return the value of system clock in KHz */
	return(sysClk/1000);
}

#else

Uint32 getSysClk(void)
{
	Bool      pllRDBypass;
	Bool      pllOutDiv;
	Bool      pllOutDiv2;
	Uint32    sysClk;
	Uint16    pllVP;
	Uint16    pllVS;
	Uint16    pllRD;
	Uint16    pllVO;
	Uint16    pllDivider;
	Uint32    pllMultiplier;

	pllVP = CSL_FEXT(CSL_SYSCTRL_REGS->CGCR1, SYS_CGCR1_MH);
	pllVS = CSL_FEXT(CSL_SYSCTRL_REGS->CGICR, SYS_CGICR_ML);

	pllRD = CSL_FEXT(CSL_SYSCTRL_REGS->CGICR, SYS_CGICR_RDRATIO);
	pllVO = CSL_FEXT(CSL_SYSCTRL_REGS->CGOCR, SYS_CGOCR_ODRATIO);

	pllRDBypass = CSL_FEXT(CSL_SYSCTRL_REGS->CGICR, SYS_CGICR_RDBYPASS);
	pllOutDiv   = CSL_FEXT(CSL_SYSCTRL_REGS->CGOCR, SYS_CGOCR_OUTDIVEN);
	pllOutDiv2  = CSL_FEXT(CSL_SYSCTRL_REGS->CGOCR, SYS_CGOCR_OUTDIV2BYPASS);

	pllDivider = ((pllOutDiv2) | (pllOutDiv << 1) | (pllRDBypass << 2));

	pllMultiplier = ((Uint32)CSL_PLL_CLOCKIN * ((pllVP << 2) + pllVS + 4));

	switch(pllDivider)
	{
		case CSL_PLL_DIV_000:
		case CSL_PLL_DIV_001:
			sysClk = pllMultiplier / (pllRD + 4);
		break;

		case CSL_PLL_DIV_002:
			sysClk = pllMultiplier / ((pllRD + 4) * (pllVO + 4) * 2);
		break;

		case CSL_PLL_DIV_003:
			sysClk = pllMultiplier / ((pllRD + 4) * 2);
		break;

		case CSL_PLL_DIV_004:
		case CSL_PLL_DIV_005:
			sysClk = pllMultiplier;
		break;

		case CSL_PLL_DIV_006:
			sysClk = pllMultiplier / ((pllVO + 4) * 2);
		break;

		case CSL_PLL_DIV_007:
			sysClk = pllMultiplier / 2;
		break;
	}

	/* Return the value of system clock in KHz */
	return(sysClk/1000);
}

#endif






























/***********************************EOF***************************************************/
