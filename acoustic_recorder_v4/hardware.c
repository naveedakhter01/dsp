//////////////////////////////////////////////////////////////////////////////
// * File name: ezdsp5535_led.c
// *                                                                          
// * Description:  LED implementation.
// *                                                                          
// * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/ 
// * Copyright (C) 2011 Spectrum Digital, Incorporated
// *                                                                          
// *                                                                          
// *  Redistribution and use in source and binary forms, with or without      
// *  modification, are permitted provided that the following conditions      
// *  are met:                                                                
// *                                                                          
// *    Redistributions of source code must retain the above copyright        
// *    notice, this list of conditions and the following disclaimer.         
// *                                                                          
// *    Redistributions in binary form must reproduce the above copyright     
// *    notice, this list of conditions and the following disclaimer in the   
// *    documentation and/or other materials provided with the                
// *    distribution.                                                         
// *                                                                          
// *    Neither the name of Texas Instruments Incorporated nor the names of   
// *    its contributors may be used to endorse or promote products derived   
// *    from this software without specific prior written permission.         
// *                                                                          
// *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS     
// *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT       
// *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR   
// *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT    
// *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,   
// *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT        
// *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,   
// *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY   
// *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT     
// *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE   
// *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.    
// *                                                                          
//////////////////////////////////////////////////////////////////////////////

#include "ezdsp5535.h"
#include "hardware.h"
#include "ezdsp5535_gpio.h"
#include "ezdsp5535_spi.h"
#include "ezdsp5535_led.h"
#include "soc.h"
#include "cslr_cpu.h"
#include "sysclk.h"



void DisableFilters(void);


/*
 *  USBSTK5515_LED_init( )
 *
 *      Initialize LEDs
 */
void GPIO_init( )
{
	EZDSP5535_GPIO_init(); /* Initialize GPIOs */
	EZDSP5535_XF_off(); //LED
	EZDSP5535_GPIO_setDirection( 11, GPIO_OUT );  //ADC_SELECT
	ADC_Select(ADC_12BIT); //set to default 12Bit
}


/*
 *  ADC Select
 *
 *  toggle pin on multiplexer ic
 */
void ADC_Select(ADC_Type adc_select)
{
	if (adc_select == ADC_16BIT)
	{
	    EZDSP5535_GPIO_setOutput( 11, 1);
	}
	else
	{
		EZDSP5535_GPIO_setOutput( 11, 0);  
	}        
}


void DSPReadyToSend(FunctionalState NewState)     //GPIO17 used to ready ARM
{
	if(NewState != DISABLE)
     EZDSP5535_GPIO_setOutput( 17, 1);
    else
     EZDSP5535_GPIO_setOutput( 17, 0);
}


//use only when SPI1MODE is set to mode2
void SetI2S1Pins()
{
	EZDSP5535_GPIO_setDirection( 6, GPIO_OUT );  
	EZDSP5535_GPIO_setDirection( 7, GPIO_OUT );  
	EZDSP5535_GPIO_setDirection( 9, GPIO_IN );  

    EZDSP5535_GPIO_setOutput( 6, 1);
    EZDSP5535_GPIO_setOutput( 7, 1);  
}

//old using pins as opencollector 
//void DisableFilters(void)
//{
//	//PREAMP Disabled by setting pin to input and letting it pull high
//    EZDSP5535_GPIO_setDirection( 2, GPIO_IN );
//    EZDSP5535_GPIO_setOutput( 2, 0);  // 
//	
//	//LowFilter is disabled by setting pin to input and letting it pull high
//    //EZDSP5535_GPIO_setDirection( 4, GPIO_IN );
//   // EZDSP5535_GPIO_setOutput( 4, 0);  // 
//    //HighFilter is disabled by setting pin to output low  
//    EZDSP5535_GPIO_setOutput( 4, 0);  //not ready  
//    EZDSP5535_GPIO_setDirection( 4, GPIO_OUT );
//    EZDSP5535_GPIO_setOutput( 4, 0);  //not ready      
//    
//    //HighFilter is disabled by setting pin to output low  
//    EZDSP5535_GPIO_setOutput( 5, 0);  //not ready  
//    EZDSP5535_GPIO_setDirection( 5, GPIO_OUT );
//    EZDSP5535_GPIO_setOutput( 5, 0);  //not ready  
//     
//    //BatFilter is disabled by setting pin to input and letting it pull high
//    EZDSP5535_GPIO_setDirection( 10, GPIO_IN );
//    EZDSP5535_GPIO_setOutput( 10, 0);  //
//}

//using push pull to switch transistor
void InitGPIO(void)
{
	/*Init pins to ENABLE/DISABLE filters*/
	
	//PREAMP Disabled by setting pin to input and letting it pull high
    EZDSP5535_GPIO_setDirection( PREAMP_GPIO_PIN, PREAMP_GPIO_DIR );
    PREAMP_DISABLE();
	
	//LowFilter is disabled by setting pin to input and letting it pull high
    EZDSP5535_GPIO_setDirection( LOW_FILTER_GPIO_PIN, LOW_FILTER_GPIO_DIR );
    LOW_FILTER_DISABLE();
    
    //HighFilter is disabled by setting pin to output low  
    EZDSP5535_GPIO_setDirection( HIGH_FILTER_GPIO_PIN, HIGH_FILTER_GPIO_DIR );
    HIGH_FILTER_DISABLE();
     
    //BatFilter is disabled by setting pin to input and letting it pull high
    EZDSP5535_GPIO_setDirection( BAT_FILTER_GPIO_PIN, BAT_FILTER_GPIO_DIR);
    BAT_FILTER_DISABLE();  //   //will  transistor off so opamp en pin pulled high turning off

    	//set as output
	EZDSP5535_GPIO_setDirection( MIC_GPIO_PIN, MIC_GPIO_DIR); 
	MIC_DISABLE();
	
	/*Init pins that interface between ARM and DSP*/
	//inputs
	EZDSP5535_GPIO_setDirection( COMMAND_MODE_GPIO_PIN, COMMAND_MODE_GPIO_DIR); //IN
	EZDSP5535_GPIO_setDirection( COMMAND_REQUEST_GPIO_PIN, COMMAND_REQUEST_GPIO_DIR); //IN
	EZDSP5535_GPIO_setDirection( DSP_RESERVED_GPIO_PIN, DSP_RESERVED_GPIO_DIR); //IN
	//output
	EZDSP5535_GPIO_setDirection( DSP_EVENT_GPIO_PIN, DSP_EVENT_GPIO_DIR); //OUT
	DSP_EVENT_DISABLE();


}



//using push pull to switch transistor
void FilterSelect(FilterOption filterOption)
{
	if (filterOption == LOW_FILTER)	 //Lowfilter is enabled by pulling line low
	{
    	//disable these first incase they are active
    	HIGH_FILTER_DISABLE();
    	BAT_FILTER_DISABLE();
    	
    	LOW_FILTER_ENABLE();
    	PREAMP_ENABLE();  
    	MIC_ENABLE();
	}
	else if (filterOption == HIGH_FILTER)	//high filter is enabled by letting line pullup
	{
    	//disable these first incase they are active
    	LOW_FILTER_DISABLE();
    	BAT_FILTER_DISABLE();
    	
    	HIGH_FILTER_ENABLE();
    	PREAMP_ENABLE();  
    	MIC_ENABLE();
	}
	else if (filterOption == BAT_FILTER)
	{
    	//disable these first incase they are active
    	HIGH_FILTER_DISABLE();
    	LOW_FILTER_DISABLE();
    	PREAMP_DISABLE();  
    	MIC_DISABLE();
    	
    	BAT_FILTER_ENABLE();
	}
	else  //DISABLE ALL
	{
    	//disable everything
    	HIGH_FILTER_DISABLE();
    	LOW_FILTER_DISABLE();
    	PREAMP_DISABLE();  
    	BAT_FILTER_DISABLE();	
    	MIC_DISABLE();	
	}
}

void MicBiasSupply(FunctionalState state)
{
	if (state == ENABLE)
		MIC_ENABLE();
	else
		MIC_DISABLE();

}



int ReadCommandModeInput()
{
	return COMMAND_MODE_INPUT_STATE();
}

int ReadCommandRequestInput()
{
	return COMMAND_REQUEST_INPUT_STATE();
}






//use only when SPI1MODE is set to mode2
void SetUnusedPins( )
{
	ioport volatile CSL_SysRegs	*sysRegs;
	
//	EZDSP5535_GPIO_setDirection( 0, GPIO_OUT );  
//	EZDSP5535_GPIO_setDirection( 1, GPIO_OUT );  
//	EZDSP5535_GPIO_setDirection( 2, GPIO_OUT );  
//    EZDSP5535_GPIO_setDirection( 3, GPIO_OUT );  
//    EZDSP5535_GPIO_setDirection( 4, GPIO_OUT );  
//    EZDSP5535_GPIO_setDirection( 5, GPIO_OUT );  
//    EZDSP5535_GPIO_setDirection( 10, GPIO_OUT ); 
//    
//    EZDSP5535_GPIO_setOutput( 0, 0);
//    EZDSP5535_GPIO_setOutput( 1, 0);  
//    EZDSP5535_GPIO_setOutput( 2, 0);
//    EZDSP5535_GPIO_setOutput( 3, 0);  
//    EZDSP5535_GPIO_setOutput( 4, 0);
//    EZDSP5535_GPIO_setOutput( 5, 0);
//    EZDSP5535_GPIO_setOutput( 10, 1);   //amp enable always on
//    
//    
//    
//    //test as pull downs
//	EZDSP5535_GPIO_setDirection( 12, GPIO_IN );  
//	EZDSP5535_GPIO_setDirection( 13, GPIO_IN );  
//	EZDSP5535_GPIO_setDirection( 14, GPIO_IN ); 
//	//ARM handshake in 
//    EZDSP5535_GPIO_setDirection( 15, GPIO_IN );  
//    
//    EZDSP5535_GPIO_setDirection( 16, GPIO_IN );  
//  
//    //ARM handshake out 
//    EZDSP5535_GPIO_setDirection( 17, GPIO_OUT );  
//    EZDSP5535_GPIO_setOutput( 17, 0);  //not ready  
//


//    //set as pulldowns
  sysRegs = (CSL_SysRegs *)CSL_SYSCTRL_REGS;
  //sysRegs->PDINHIBR1 = 0x0020; //0x0010; , this one works for high recording
  //sysRegs->PDINHIBR1 = 0x0010; //0x0010; , this one works for low recording
   // sysRegs->PDINHIBR1 = 0x0000; //0x0010; , this one works for high recording
  
  
  //  //sysRegs->PDINHIBR2 = 0x0000;
    sysRegs->PDINHIBR3 = 0x0000; //0xBB80;      // GPIO12-16, I2S2_RX, I2S3_RX
    
    
    
//  
//  
//  
//
//    
//     //set 
//    //EZDSP5535_GPIO_setOutput( 12, 0);
//    //EZDSP5535_GPIO_setOutput( 13, 0);  
//    //EZDSP5535_GPIO_setOutput( 14, 0);
//    //EZDSP5535_GPIO_setOutput( 15, 0);  
//    //EZDSP5535_GPIO_setOutput( 16, 0); 
//    
//    
//    
//    
//    //I2S1 pins test DEBUG
//    EZDSP5535_GPIO_setDirection( 6, GPIO_OUT );  
//	EZDSP5535_GPIO_setDirection( 7, GPIO_OUT );  
//	EZDSP5535_GPIO_setDirection( 8, GPIO_OUT ); 
//    EZDSP5535_GPIO_setDirection( 9, GPIO_OUT );  
//
//    EZDSP5535_GPIO_setOutput( 6, 0);
//    EZDSP5535_GPIO_setOutput( 7, 0);  
//    EZDSP5535_GPIO_setOutput( 8, 0);
//    EZDSP5535_GPIO_setOutput( 9, 0); 
//    
//    
//    //I2S2
//    
////	EZDSP5535_GPIO_setDirection( 18, GPIO_IN );  
////	EZDSP5535_GPIO_setDirection( 19, GPIO_IN );  
////	EZDSP5535_GPIO_setDirection( 20, GPIO_IN ); 
////    EZDSP5535_GPIO_setDirection( 27, GPIO_IN ); 
//    
//    EZDSP5535_GPIO_setDirection( 18, GPIO_OUT );  
//	EZDSP5535_GPIO_setDirection( 19, GPIO_OUT );  
//	EZDSP5535_GPIO_setDirection( 20, GPIO_OUT ); 
//    EZDSP5535_GPIO_setDirection( 27, GPIO_OUT );  
//
//    EZDSP5535_GPIO_setOutput( 18, 0);
//    EZDSP5535_GPIO_setOutput( 19, 0);  
//    EZDSP5535_GPIO_setOutput( 20, 0);
//    EZDSP5535_GPIO_setOutput( 27, 0); 
    
    
    //EZDSP5535_GPIO_setDirection( 0, GPIO_IN );  
	//EZDSP5535_GPIO_setDirection( 1, GPIO_IN  );  
	//EZDSP5535_GPIO_setDirection( 2, GPIO_IN  );  
    //EZDSP5535_GPIO_setDirection( 3, GPIO_IN  );  
    //EZDSP5535_GPIO_setDirection( 4, GPIO_IN  );  
    //EZDSP5535_GPIO_setDirection( 5, GPIO_IN  );  
    //EZDSP5535_GPIO_setDirection( 6, GPIO_IN  ); 
	//EZDSP5535_GPIO_setDirection( 7, GPIO_IN  );  
	//EZDSP5535_GPIO_setDirection( 8, GPIO_IN  );  
	//EZDSP5535_GPIO_setDirection( 9, GPIO_IN  );  
    //EZDSP5535_GPIO_setDirection( 10, GPIO_IN  );  
    //EZDSP5535_GPIO_setDirection( 11, GPIO_IN  );  
    //EZDSP5535_GPIO_setDirection( 12, GPIO_IN  );  
    //EZDSP5535_GPIO_setDirection( 13, GPIO_IN  ); 
	//EZDSP5535_GPIO_setDirection( 14, GPIO_IN  );  
	//EZDSP5535_GPIO_setDirection( 15, GPIO_IN  );  
	//EZDSP5535_GPIO_setDirection( 16, GPIO_IN  );  
    //EZDSP5535_GPIO_setDirection( 17, GPIO_IN  );  
    //EZDSP5535_GPIO_setDirection( 18, GPIO_IN  );  
    //EZDSP5535_GPIO_setDirection( 19, GPIO_IN  );  
    //EZDSP5535_GPIO_setDirection( 20, GPIO_IN  );    
    //EZDSP5535_GPIO_setDirection( 21, GPIO_IN  );    
    //EZDSP5535_GPIO_setDirection( 22, GPIO_IN  );    
    //EZDSP5535_GPIO_setDirection( 23, GPIO_IN  );    
    //EZDSP5535_GPIO_setDirection( 24, GPIO_IN  );    
    //EZDSP5535_GPIO_setDirection( 25, GPIO_IN  );    
    //EZDSP5535_GPIO_setDirection( 26, GPIO_IN  );    
    //EZDSP5535_GPIO_setDirection( 27, GPIO_IN  );       
    
    
    EZDSP5535_GPIO_setDirection( 0, GPIO_OUT );  
	EZDSP5535_GPIO_setDirection( 1, GPIO_OUT );  
//	EZDSP5535_GPIO_setDirection( 2, GPIO_OUT );  
//    EZDSP5535_GPIO_setDirection( 3, GPIO_OUT );  
//    EZDSP5535_GPIO_setDirection( 4, GPIO_OUT );  
//    EZDSP5535_GPIO_setDirection( 5, GPIO_OUT );  
//    EZDSP5535_GPIO_setDirection( 6, GPIO_OUT ); 
//	EZDSP5535_GPIO_setDirection( 7, GPIO_OUT );  
//	EZDSP5535_GPIO_setDirection( 8, GPIO_OUT );  
//	EZDSP5535_GPIO_setDirection( 9, GPIO_OUT );  
//    EZDSP5535_GPIO_setDirection( 10, GPIO_OUT );  
//    EZDSP5535_GPIO_setDirection( 11, GPIO_OUT );  
//    EZDSP5535_GPIO_setDirection( 12, GPIO_OUT );  
//    EZDSP5535_GPIO_setDirection( 13, GPIO_OUT ); 
//	EZDSP5535_GPIO_setDirection( 14, GPIO_OUT );  
	EZDSP5535_GPIO_setDirection( 15, GPIO_OUT );  
//	EZDSP5535_GPIO_setDirection( 16, GPIO_OUT );  
//    EZDSP5535_GPIO_setDirection( 17, GPIO_OUT );  
    EZDSP5535_GPIO_setDirection( 18, GPIO_OUT );  
    EZDSP5535_GPIO_setDirection( 19, GPIO_OUT );  
    EZDSP5535_GPIO_setDirection( 20, GPIO_OUT );    
    EZDSP5535_GPIO_setDirection( 21, GPIO_OUT );    
//    EZDSP5535_GPIO_setDirection( 22, GPIO_OUT );    
//    EZDSP5535_GPIO_setDirection( 23, GPIO_OUT );    
//    EZDSP5535_GPIO_setDirection( 24, GPIO_OUT );    
//    EZDSP5535_GPIO_setDirection( 25, GPIO_OUT );    
//    EZDSP5535_GPIO_setDirection( 26, GPIO_OUT );    
//    EZDSP5535_GPIO_setDirection( 27, GPIO_OUT );    

EZDSP5535_GPIO_setDirection( 27, GPIO_OUT ); 
EZDSP5535_GPIO_setDirection( 27, GPIO_OUT ); 
EZDSP5535_GPIO_setDirection( 27, GPIO_OUT ); 
EZDSP5535_GPIO_setDirection( 27, GPIO_OUT ); 
//                                
//    EZDSP5535_GPIO_setOutput( 0, 0);
//    EZDSP5535_GPIO_setOutput( 1, 0);  
//    EZDSP5535_GPIO_setOutput( 2, 0);
//    EZDSP5535_GPIO_setOutput( 3, 0);  
//    EZDSP5535_GPIO_setOutput( 4, 0);
//    EZDSP5535_GPIO_setOutput( 5, 0); 
//    EZDSP5535_GPIO_setOutput( 6, 0);
//    EZDSP5535_GPIO_setOutput( 7, 0);  
//    EZDSP5535_GPIO_setOutput( 8, 0);
//    EZDSP5535_GPIO_setOutput( 9, 0);  
//    EZDSP5535_GPIO_setOutput( 10, 0);
//    EZDSP5535_GPIO_setOutput( 11, 0);
//    EZDSP5535_GPIO_setOutput( 12, 0);
//    EZDSP5535_GPIO_setOutput( 13, 0);  
//    EZDSP5535_GPIO_setOutput( 14, 0);
//    EZDSP5535_GPIO_setOutput( 15, 0);  
//    EZDSP5535_GPIO_setOutput( 16, 0);
//    EZDSP5535_GPIO_setOutput( 17, 0);
//    EZDSP5535_GPIO_setOutput( 18, 0);
//    EZDSP5535_GPIO_setOutput( 19, 0);  
//    EZDSP5535_GPIO_setOutput( 20, 0);
//    EZDSP5535_GPIO_setOutput( 21, 0);  
//    EZDSP5535_GPIO_setOutput( 22, 0);
//    EZDSP5535_GPIO_setOutput( 23, 0);
//    EZDSP5535_GPIO_setOutput( 24, 0);
//    EZDSP5535_GPIO_setOutput( 25, 0);  
//    EZDSP5535_GPIO_setOutput( 26, 0);
//    EZDSP5535_GPIO_setOutput( 27, 0);  
    
    
    
}





/**************************************************EOF***********************************************/



