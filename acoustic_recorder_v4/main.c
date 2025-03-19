//////////////////////////////////////////////////////////////////////////////
// * File name: main.c
// *                                                                          
// * Description:  Main function.   
// *                                                                          
// *                                                                          
//////////////////////////////////////////////////////////////////////////////

#include "stdio.h"
#include "ezdsp5535.h"
#include "window_tables.h"
#include "ezdsp5535_spi.h"
#include "csl_pll.h"
#include "mod_i2s.h"
#include "ezdsp5535_led.h"
#include "ezdsp5535_i2c.h"
#include "ezdsp5535_gpio.h"
#include "dma_routines.h"
#include "hardware.h"
#include "bootloader.h"
#include "spi_bitbang.h"
#include "sysclk.h"
#include "timer.h"
#include "command.h"
#include "fir_filter.h"
#include "bat_detect.h"

//#define POWER_TEST
#define BAT_TEST

typedef enum
{
  WAIT_FOR_TRIGGER = 0,	 
  WAIT_2ND_TRIGGER,
  BAT_RECORDING
}BatRecState;


typedef enum
{
  COMMAND = 0,	 
  DATA
} Mode_TypeDef;




void SystemCleanup(void);
void SystemInit(void);
void FlashLED(int numOfFlashes);
void PowerDown();

void RunFirFilterHigh(void);
void RunFirFilterLow(void);
void RunBatTrigger(void);


//void FlashLED( );

void Enable_PLL(void);
extern Int16 led_test( );
//extern Int32* computeFFT( );
//extern Int16 spi_test( );
extern Uint16 FFT_Window();
extern Uint16 test_FFT_timing();
extern Uint16 FFTTrigLoop();
extern Uint16 FFT2ndTrigLoop();
extern Uint16 FFTRecordLoop();
extern Uint16 DecimationLoop();
extern Uint16 FirFilterHighLoop();
extern Uint16 FirFilterLowLoop();
extern void ResetTrigBuffers(void);
extern Int16 aic3204_loop_linein( );


Uint8 version_id_0 = 1;   // MAJOR VERSION
                  // .  
Uint8 version_id_1 = 0;   // MINOR VERSION

Uint8 version_id_2 = 'P';  // not used
Uint8 version_id_3 = 'V';  //not used




extern void TestI2SRead(void);

int  TestFail    = (int)-1;   

void StopTest()
{
    //SW_BREAKPOINT;
    return;
}

/*
 *
 *  main( )
 *
 */
void main( void )
{
	FilterOption filterOption;
	
	SystemCleanup();	
	SwitchSysClk(COMMAND_CLK);
	SystemInit();

	SetUnusedPins( );  //To reduce power consumption make sure no floating pins

	InitGPIO();
	FilterSelect(NO_FILTER); //disable filters

	//CSL_gptIntrTest();
	
   while(1)
   {
   		//run command mode until taken out of it by arm, command mode is used to setup sampling mode as well as testing and other things
   		CommandMode();

   	   //exited command mode now check which filter to run
        filterOption = GetFilterOption();
          	
       	if (filterOption == LOW_FILTER)
       	{
       		RunFirFilterLow();
       	}
       	else if (filterOption == HIGH_FILTER)
       	{
       		RunFirFilterHigh();  
       	}
       	else if (filterOption == BAT_FILTER)
       	{
       		RunBatTrigger();
       	}
       	else
       	{
       	 //wait for command mode to go high again	
       		while (!IsInCommandMode(LEVEL)) {}
       	}
       	
       	SwitchSysClk(COMMAND_CLK); //switch clk back to default clk
       	
   }
}


/*****************************************************
 * FIR FILTER INIT
 * 
 * Runs a loop that sends filtered adc data out spi 
 * 
 * - sampling frequency of ADC will be 32kHz 
 * 
 * 
 */
 
void RunFirFilterHigh(void)
{
	 FilterSelect(HIGH_FILTER); //enable the hardware for the filter
	 SwitchSysClk(FIR_HIGH_CLK);  //switch the sysclk  to be a multiple of the sampling frequency
	 EZDSP5535_wait(100);
	 ADC_Select(ADC_16BIT);       // use the 16bit adc for audio recording
	 Init_I2s_Dma(ADC_16BIT,HIGH_FILTER);  // setup the dma to automatically sample and buffer adc
	 EZDSP5535_SPI_init(1);    //setup spi to send data to arm
	 
	 FirFilterHigh();      // run loop to continuously filter and send data to arm
	 
	 disable_dma();//
	 FilterSelect(NO_FILTER); //disable filters
}

void RunFirFilterLow(void)
{
	FilterSelect(LOW_FILTER); //enable the hardware for the filter
	 SwitchSysClk(FIR_LOW_CLK);  //switch the sysclk  to be a multiple of the sampling frequency
	 EZDSP5535_wait(100);
	 ADC_Select(ADC_16BIT);       // use the 16bit adc for audio recording
	 Init_I2s_Dma(ADC_16BIT,LOW_FILTER);  // setup the dma to automatically sample and buffer adc
	 EZDSP5535_SPI_init(0);    //setup spi to send data to arm
	 
	 FirFilterLow();      // run loop to continuously filter and send data to arm
	 
	 disable_dma();//
	 FilterSelect(NO_FILTER); //disable filters
}



void RunBatTrigger(void)
{
	BatRecState recState;
	
	
	recState = WAIT_FOR_TRIGGER;
	FilterSelect(BAT_FILTER); //enable the hardware for the filter
	ResetTrigBuffers(); //need to reset buffers just the once
	
	while(!IsInCommandMode(LEVEL)) // exit if signaled by arm	
	{
		if (recState == WAIT_FOR_TRIGGER)
		{
			SwitchSysClk(BAT_IDLE_CLK);
			ADC_Select(ADC_12BIT); // 
			Init_I2s_Dma(ADC_12BIT,BAT_FILTER);
			
			BatDetect();   //
			
			disable_dma();//
			
			//This is were we tell ARM to wake up and record but it's now done in the bat data out function - after detect second trigger
			//if (!IsInCommandMode(LEVEL))  // if not exit by arm signal then send back event signal
			//	DSP_EVENT_ENABLE(); // tell arm to get ready as data maybe comming
			
			recState = BAT_RECORDING;
		}
		else //BAT_RECORDING
		{
			SwitchSysClk(BAT_RUN_CLK);
			ADC_Select(ADC_16BIT); // 
			Init_I2s_Dma(ADC_16BIT,BAT_FILTER);
			EZDSP5535_SPI_init(2);   // init spi clk for high sampling
			
			BatDataOut();   //
			
			disable_dma();//
			DSP_EVENT_DISABLE(); //tell arm we've finished
			recState = WAIT_FOR_TRIGGER;
		}
	}
	
	FilterSelect(NO_FILTER); //disable filters
}







/***********************************************
*FLASHES THE LED A SET NUMBER OF TIMES
*/
void FlashLED(int numOfFlashes)
{
	int i;
	numOfFlashes = numOfFlashes*2;
    for ( i=0 ; i<numOfFlashes ; i++ )
    {
         EZDSP5535_wait(10000);  //
         EZDSP5535_XF_toggle();
    }  	
    EZDSP5535_XF_off();
}


/***********************************************
*Part of the system initialise
*/
void SystemCleanup(void)
{
	
	Uint16 i;
	
    /* Disable interrupts */
    *(int*)0x0003 = *(int*)0x0003 | 0x0800; // Set INTM
    *(int*)0x0000 = 0;                      // Clear IER0
    *(int*)0x0045 = 0;                      // Clear IER1
    *(int*)0x0004 = *(int*)0x0004 | 0x2000; // Set CACLR (Clear Cache)

     
    //asm(" bit(ST3,2) = #1"); // set clkout disabled


    /* Disable each DMA channels */
    asm("   *port(#0xC01) = #0x0 ");
    asm("   *port(#0xC21) = #0x0 ");
    asm("   *port(#0xC41) = #0x0 ");
    asm("   *port(#0xC61) = #0x0 ");

//perihp reset
	asm("   *port(#0x1c04) = #0x0 ");
	asm("   *port(#0x1c05) = #0x0 ");





    //select 1.05v core voltage
   
    for(i=0;i<0xff;i++);
     *(volatile Uint16 *)(0x0049) = 0x0002;  //*(short *)IVPD@data = 0x027F; // Load interrupt vector pointer


	  //IDLE_ICR = 0x000E;    
	  asm("   *port(#0x0001) = #0x0E "); // take all clock domains out of idle (especially MPORT)
      asm(" nop_16");
      asm(" nop_16");
      asm(" nop_16");
      asm(" nop_16");
      asm(" nop_16");
      asm(" nop_16");
      asm(" idle");

}

/***********************************************
*Part of the system initialise
*/
void SystemInit(void)
{
	
	ioport volatile CSL_SysRegs	*sysRegs;

    /* Initialize BSL */
    EZDSP5535_init( );

    //set I2C
    //EZDSP5535_I2C_init( );
    EZDSP5535_wait(100);  //
    //set GPIO for LED
    //setup I2S and interrupt
    //set SPI
   // EZDSP5535_SPI_init( );
    EZDSP5535_XF_off();
    //EZDSP5535_LED_init( );	
    GPIO_init( );
    
    //set analog inputs as grounded outputs
    
    //SARGPOCTRL 701A
    asm("   *port(#0x701A) = #0x00F0 ");
    
    //EZDSP5535_wait(100);  //
    
   sysRegs = (CSL_SysRegs *)CSL_SYSCTRL_REGS;
    
   //gate periph clocks
   CSL_FINS(sysRegs->PCGCR1, SYS_PCGCR1_I2S2CG, CSL_SYS_PCGCR1_I2S2CG_DISABLED);
   CSL_FINS(sysRegs->PCGCR1, SYS_PCGCR1_I2S3CG, CSL_SYS_PCGCR1_I2S3CG_DISABLED);
   CSL_FINS(sysRegs->PCGCR1, SYS_PCGCR1_SPICG, CSL_SYS_PCGCR1_SPICG_ACTIVE);    
   
   //set mode
   CSL_FINST(CSL_SYSCTRL_REGS->EBSR, SYS_EBSR_PPMODE, MODE6); //Mode 6 (SPI, I2S2, I2S3, and GPIO).
   CSL_FINST(CSL_SYSCTRL_REGS->EBSR, SYS_EBSR_SP0MODE, MODE2);// MODE 2 ALL 6 GPIO  
   CSL_FINST(CSL_SYSCTRL_REGS->EBSR, SYS_EBSR_SP1MODE, MODE1); //MODE 1 4 signals of I2S1 and 2 GPIO


    //Disable
   CSL_FINS(sysRegs->PCGCR1, SYS_PCGCR1_TMR0CG, CSL_SYS_PCGCR1_TMR0CG_DISABLED);
   CSL_FINS(sysRegs->PCGCR1, SYS_PCGCR1_TMR1CG, CSL_SYS_PCGCR1_TMR1CG_DISABLED);
   CSL_FINS(sysRegs->PCGCR1, SYS_PCGCR1_TMR2CG, CSL_SYS_PCGCR1_TMR2CG_DISABLED);
   CSL_FINS(sysRegs->PCGCR1, SYS_PCGCR1_MMCSD1CG, CSL_SYS_PCGCR1_MMCSD1CG_DISABLED);  
   CSL_FINS(sysRegs->PCGCR1, SYS_PCGCR1_I2CCG, CSL_SYS_PCGCR1_I2CCG_DISABLED);   
   CSL_FINS(sysRegs->PCGCR1, SYS_PCGCR1_MMCSD0CG, CSL_SYS_PCGCR1_MMCSD0CG_DISABLED);   
   //CSL_FINS(sysRegs->PCGCR1, SYS_PCGCR1_DMA0CG, CSL_SYS_PCGCR1_DMA0CG_DISABLED);       
   CSL_FINS(sysRegs->PCGCR1, SYS_PCGCR1_UARTCG, CSL_SYS_PCGCR1_UARTCG_DISABLED);   
   CSL_FINS(sysRegs->PCGCR2, SYS_PCGCR2_LCDCG, CSL_SYS_PCGCR2_LCDCG_DISABLED);       
   CSL_FINS(sysRegs->PCGCR2, SYS_PCGCR2_SARCG, CSL_SYS_PCGCR2_SARCG_DISABLED);     
   CSL_FINS(sysRegs->PCGCR2, SYS_PCGCR2_USBCG, CSL_SYS_PCGCR2_USBCG_DISABLED);       
   CSL_FINS(sysRegs->PCGCR2, SYS_PCGCR2_DMA1CG, CSL_SYS_PCGCR2_DMA1CG_DISABLED);   	
   CSL_FINS(sysRegs->PCGCR2, SYS_PCGCR2_DMA2CG, CSL_SYS_PCGCR2_DMA2CG_DISABLED);       
   CSL_FINS(sysRegs->PCGCR2, SYS_PCGCR2_DMA3CG, CSL_SYS_PCGCR2_DMA3CG_DISABLED);  
   CSL_FINS(sysRegs->PCGCR2, SYS_PCGCR2_ANAREGCG , CSL_SYS_PCGCR2_ANAREGCG_DISABLED);   


   // CSL_FINS(sysRegs->EBSR, SYS_EBSR_SP1MODE,
    //              CSL_SYS_EBSR_SP1MODE_MODE1);    //MODE 1 4 signals of I2S1 and 2 GPIO
   // CSL_FINS(sysRegs->EBSR, SYS_EBSR_SP0MODE,
   //               CSL_SYS_EBSR_SP0MODE_MODE2);  // MODE 2 ALL 6 GPIO               
    //enable periph clock gate    
    CSL_FINS(sysRegs->PCGCR1, SYS_PCGCR1_I2S1CG, CSL_SYS_PCGCR1_I2S1CG_ACTIVE);
       // CSL_FINS(sysRegs->PCGCR1, SYS_PCGCR1_I2S1CG, CSL_SYS_PCGCR1_I2S1CG_DISABLED); //DEBUG 
    
       //      CSL_FINS(sysRegs->PCGCR1, SYS_PCGCR1_I2S2CG,CSL_SYS_PCGCR1_I2S2CG_ACTIVE);
            //CSL_FINS(sysRegs->PCGCR1, SYS_PCGCR1_SPICG, CSL_SYS_PCGCR1_SPICG_ACTIVE);                  
    //reset periph
    //CSL_FINS(sysRegs->PRCR, SYS_PRCR_PG3_RST, CSL_SYS_PRCR_PG3_RST_RST);
   //CSL_FINS(sysRegs->PRCR, SYS_PRCR_PG4_RST, CSL_SYS_PRCR_PG4_RST_RST); 
   
}





//SHUTSDOWN EVERYTHING BEFORE GOING INTO IDLE MODE
void PowerDown()
{
	ioport volatile CSL_SysRegs	*sysRegs;
	
	
	sysRegs = (CSL_SysRegs *)CSL_SYSCTRL_REGS;
	//disable usb clock domain-  no need as no power to usb domain


	//idle all the desired periphs
	   //gate periph clocks
   CSL_FINS(sysRegs->PCGCR1, SYS_PCGCR1_TMR0CG, CSL_SYS_PCGCR1_TMR0CG_DISABLED);
   CSL_FINS(sysRegs->PCGCR1, SYS_PCGCR1_TMR1CG, CSL_SYS_PCGCR1_TMR1CG_DISABLED);
   CSL_FINS(sysRegs->PCGCR1, SYS_PCGCR1_TMR2CG, CSL_SYS_PCGCR1_TMR2CG_DISABLED);
   CSL_FINS(sysRegs->PCGCR1, SYS_PCGCR1_MMCSD1CG, CSL_SYS_PCGCR1_MMCSD1CG_DISABLED);  
   CSL_FINS(sysRegs->PCGCR1, SYS_PCGCR1_I2CCG, CSL_SYS_PCGCR1_I2CCG_DISABLED);   
   CSL_FINS(sysRegs->PCGCR1, SYS_PCGCR1_MMCSD0CG, CSL_SYS_PCGCR1_MMCSD0CG_DISABLED);   
   CSL_FINS(sysRegs->PCGCR1, SYS_PCGCR1_DMA0CG, CSL_SYS_PCGCR1_DMA0CG_DISABLED);       
   CSL_FINS(sysRegs->PCGCR1, SYS_PCGCR1_UARTCG, CSL_SYS_PCGCR1_UARTCG_DISABLED);   
   
   
   CSL_FINS(sysRegs->PCGCR2, SYS_PCGCR2_LCDCG, CSL_SYS_PCGCR2_LCDCG_DISABLED);       
   CSL_FINS(sysRegs->PCGCR2, SYS_PCGCR2_SARCG, CSL_SYS_PCGCR2_SARCG_DISABLED);     
   CSL_FINS(sysRegs->PCGCR2, SYS_PCGCR2_USBCG, CSL_SYS_PCGCR2_USBCG_DISABLED);       
   CSL_FINS(sysRegs->PCGCR2, SYS_PCGCR2_DMA1CG, CSL_SYS_PCGCR2_DMA1CG_DISABLED);   	
   CSL_FINS(sysRegs->PCGCR2, SYS_PCGCR2_DMA2CG, CSL_SYS_PCGCR2_DMA2CG_DISABLED);       
   CSL_FINS(sysRegs->PCGCR2, SYS_PCGCR2_DMA3CG, CSL_SYS_PCGCR2_DMA3CG_DISABLED);  
   CSL_FINS(sysRegs->PCGCR2, SYS_PCGCR2_ANAREGCG , CSL_SYS_PCGCR2_ANAREGCG_DISABLED);       

   
   //disable clock domain
    asm("   *port(#0x1C1F) = #0x0 ");
   
   
	//clear int's by writing ones to IFR0 and IFR1
    *(int*)0x0001 = 0;                      // Clear IFR0
    *(int*)0x0046 = 0;                      // Clear IFR1
	
	//enable wakeup pin - we won't do this
	
	//Disable the CPU domain by setting to 1 the CPUI, MPORTI, XPORTI, DPORTI, IPORTI, and CPI bits
    //of the idle configuration register (ICR). 
	  asm("   *port(#0x0001) = #0x003F "); //                take all clock domains out of idle (especially MPORT)
      asm(" nop_16");
      asm(" nop_16");
      asm(" nop_16");
      asm(" nop_16");
      asm(" nop_16");
      asm(" nop_16");
      asm(" idle");

	
}













