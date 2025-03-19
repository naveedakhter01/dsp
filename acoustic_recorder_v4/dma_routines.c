
//////////////////////////////////////////////////////////////////////////////
// * File name: dma_routines.c
// *                                                                          
// * Description: This file includes DMA configuration functions and DMA ISR function.
// *                                                                          
// * Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/ 
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
#include "psp_dma.h"
#include "dda_dma.h"
#include "psp_i2s.h"
#include "dda_i2s.h"
#include "ezdsp5535.h"
//#include "data_types.h"
#include "register_dma.h"
#include "register_cpu.h"
#include "dma_routines.h"
#include "ezdsp5535_led.h"
#include "csl_gpio.h"
#include "csl_intc.h"
//#include "csl_dma.h"
//#include "csl_i2s.h"
#include <csl_general.h>
#include <stdio.h>
//#include "configuration.h"

//prototypes
interrupt void DMA_Isr(void);
void DDC_I2S_transEnable(DDC_I2SHandle    hI2s, Uint16 enableBit);
//variables

#define AUTO_RELOAD     1
#define CSL_TEST_FAILED         (1)
#define CSL_TEST_PASSED         (0)
#define FFT_LENGTH 1024



#define FIR_FILTER_BLOCKSIZE 768   //512  chosen as it's multiple of 2 and 3




#define WINDOW_SIZE 2048
#define SUCCESS     0


#define I2S1_I2SRXLT0               ( 0x2928 )  /* I2S1 Receive Left Data 0 Register */

#define I2S1_I2SRXRT0               ( 0x292C )  /* I2S1 Receive Right Data 0 Register */


#define I2S2_I2SRXLT0               ( 0x2A28 )  /* I2S2 Receive Left Data 0 Register */
#define I2S2_I2SRXLT1               ( 0x2A29 )  /* I2S2 Receive Left Data 1 Register */
#define I2S2_I2SRXRT0               ( 0x2A2C )  /* I2S2 Receive Right Data 0 Register */
#define I2S2_I2SRXRT1               ( 0x2A2D )  /* I2S2 Receive Right Data 1 Register */
#define PSP_I2S_RX_INST_ID  ( PSP_I2S_2 )
#define PSP_I2S_RX_INST_ID_1  ( PSP_I2S_1 )

extern Uint16 fSineWave;

Int16 doFftFlag =0;
Int16 pingFftFlag =0;
Int16 pongFftFlag =0;
//CSL_DMA_ChannelObj dmaObj;
//CSL_DMA_Handle dmaLeftTxHandle;
//CSL_DMA_Handle dmaRightTxHandle;
//CSL_DMA_Handle dmaLeftRxHandle;
//CSL_DMA_Handle dmaRightRxHandle;
//CSL_DMA_Config dmaConfig;

//CSL_I2sHandle	hhI2s;
PSP_Handle       i2sHandleRx;
DMA_ChanHandle   hDmaRxLeft;
DMA_ChanHandle   hDmaRxRight;

#pragma DATA_ALIGN(rcvAudioBuffer,8);
Int16 rcvAudioBuffer[FIR_FILTER_BLOCKSIZE*4];
#pragma DATA_ALIGN(rcvBatBuffer, 8);
Int16 rcvBatBuffer[FFT_LENGTH *4];





extern void VECSTART(void);


void startdmatransfer(void)
{
	
    DMA_StartTransfer(hDmaRxLeft);
    
    DMA_StartTransfer(hDmaRxRight);

   // I2S_transEnable(hhI2s, TRUE);
   DDC_I2S_transEnable((DDC_I2SHandle)i2sHandleRx, TRUE);  


    IRQ_enable(DMA_EVENT);    
    IRQ_globalEnable();   	       
	       
}



#define BLOCK_MASK = ((FFT_LENGTH/2) -1)
interrupt void i2s_isr(void)
{
//	static Int16 ptr = 512; //start half a block ahead, this means first fft will have zeros in fisrt half 
//	ioport  CSL_I2sRegs   *regs;
//    
//    regs = hhI2s->hwRegs;
//    RcvL1[ptr] =  regs->I2SRXLT1 ;     //get left
//    RcvR1[ptr] =  regs->I2SRXRT1 ;     //get right
//    ptr++;
//    
//    if(!(ptr & 511)) doFftFlag = 1;	
//	if(ptr >= 2048) 
//	{
//		EZDSP5535_LED_toggle(1);
//		ptr =0;
//	}
	IRQ_clear(RCV2_EVENT);

}

interrupt void dma_isr(void)
{   
    Uint16 temp;//, dma_start;
    Uint16 lastxferL;
    Uint16 lastxferR;
    
    temp = IFR0;
    IFR0 = temp;  

	temp = DMA_IFR;
    //DMA_IFR = temp;
        
    if(temp&0x0010)
    {	
    	lastxferL = DMA1_CH0_TC_MSW & 0x0002;    	
	    if(!lastxferL)
	    {
	    	pingFftFlag =1;
	    }
	    else
	    {
	    	pongFftFlag =1;
	    }	

        DMA_IFR = 0x0010;     // clear interrupt flags                
    }
    if(temp&0x0020)
    {
    	lastxferR = DMA1_CH1_TC_MSW & 0x0002;   
	    if(!lastxferR)
	    {
	    }
	    else
	    {
	    }
        DMA_IFR = 0x0020;     // clear interrupt flags        
    }
}


interrupt void dma3_isr(void)
{   
    Uint16 temp;//, dma_start;
    Uint16 lastxferL;
    //Uint16 lastxferR;
    
    temp = IFR0;
    IFR0 = temp;  

	temp = DMA_IFR;
    //DMA_IFR = temp;
    //EZDSP5535_LED_on(0);
    if(temp&0x1000)
    {	
    	lastxferL = DMA3_CH0_TC_MSW & 0x0002;    	
	    if(!lastxferL)
	    {
	    	pingFftFlag =1;
	    }
	    else
	    {
	    	pongFftFlag =1;
	    }	
        //EZDSP5535_LED_off(0);
        DMA_IFR = 0x1000;     // clear interrupt flags                
    }
}






/* SampleBySample
 * copy of CSL I2S_transEnable function to work with DDC_I2SObj type handle
 ***********************************************************************/
void DDC_I2S_transEnable(DDC_I2SHandle    hI2s, Uint16 enableBit)
{
    ioport    CSL_I2sDrvRegs      *localregs;

    localregs =     hI2s->regs;
    //localregs->SCRL = 0x2A00;

    if(enableBit == TRUE)
    {
        /*  Enable the transmit and receive bit */
        CSL_FINST(localregs->SCRL, I2S_I2SSCTRL_ENABLE, SET);
    }
    else
    {
        /*  Disable the transmit and receive bit */
        CSL_FINST(localregs->SCRL, I2S_I2SSCTRL_ENABLE, CLEAR);
    }
}



Int16 Init_I2s_Dma(ADC_Type adcType, FilterOption filterOption)
{
   //ioport volatile CSL_SysRegs      *sysRegs;
   //Int16          status = CSL_TEST_FAILED;
   //Int16          result =0;

   //I2S_Config      hwConfig;
   int i;
   //ioport    CSL_I2sDrvRegs      *localregs;
   //Uint16 temp;
    //PSP_I2SOpMode opMode;
    PSP_I2SConfig i2sRxConfig;
    PSP_DMAConfig dmaRxConfig;
    
     doFftFlag =0;
   pingFftFlag =0;
   pongFftFlag =0;
   
    for(i=0;i<FIR_FILTER_BLOCKSIZE*4;i++)
    {
         rcvAudioBuffer[i] = 0;
    }

   IRQ_globalDisable();
   IRQ_clearAll();
   IRQ_disableAll();
    IRQ_setVecs((Uint32)(&VECSTART));   
    IRQ_plug (DMA_EVENT, &dma3_isr);

        DMA_HwInit();
        DMA_DrvInit();

        /* Initialize I2S instance for Record */
        i2sRxConfig.loopBack     = PSP_I2S_LOOPBACK_DISABLE;
        i2sRxConfig.i2sMode      = PSP_I2S_MASTER;
        i2sRxConfig.signext      = PSP_I2S_SIGNEXT_DISABLE;

        i2sRxConfig.fsync_pol    = PSP_I2S_FSPOL_LOW;

        i2sRxConfig.datadelay    = PSP_I2S_DATADELAY_ONEBIT;

        i2sRxConfig.datapack     = PSP_I2S_DATAPACK_DISABLE;
        
       if(adcType == ADC_12BIT)
       {
       	i2sRxConfig.dataformat   = PSP_I2S_DATAFORMAT_DSP;
       	i2sRxConfig.datatype     = PSP_I2S_MONO_ENABLE;
       	i2sRxConfig.clk_pol      = PSP_I2S_RISING_EDGE;
       	
        i2sRxConfig.fsdiv        = PSP_I2S_FSDIV16;
        i2sRxConfig.clkdiv       = PSP_I2S_CLKDIV2;        
        i2sRxConfig.word_len     = PSP_I2S_WORDLEN_16;
        
        i2sHandleRx = I2S_INIT(PSP_I2S_RX_INST_ID_1, PSP_I2S_RECEIVE, 
            PSP_I2S_CHAN_MONO, PSP_DMA_POLLED, &i2sRxConfig, NULL);
       }
       else  //ADC_16BIT
       { 
        i2sRxConfig.datatype     = PSP_I2S_MONO_DISABLE; //PSP_I2S_MONO_ENABLE;       	
        i2sRxConfig.clk_pol      = PSP_I2S_FALLING_EDGE;
        i2sRxConfig.dataformat   = PSP_I2S_DATAFORMAT_LJUST; //PSP_I2S_DATAFORMAT_DSP;
       	
       	if (filterOption == BAT_FILTER)
       	{
	        i2sRxConfig.fsdiv        = PSP_I2S_FSDIV32;   //USE FOR HIGH CLK RATE 
	        i2sRxConfig.clkdiv       = PSP_I2S_CLKDIV4;        
	        i2sRxConfig.word_len     = PSP_I2S_WORDLEN_16;
       	}
       	else if (filterOption == HIGH_FILTER)
       	{
	        i2sRxConfig.fsdiv        = PSP_I2S_FSDIV32;   //USE FOR HIGH CLK RATE 
	        i2sRxConfig.clkdiv       = PSP_I2S_CLKDIV2;        
	        i2sRxConfig.word_len     = PSP_I2S_WORDLEN_16;
       	}
       	else
       	{
	        i2sRxConfig.fsdiv        = PSP_I2S_FSDIV32;        //USE FOR FIR_low CLK RATE
	        i2sRxConfig.clkdiv       = PSP_I2S_CLKDIV2;        
	        i2sRxConfig.word_len     = PSP_I2S_WORDLEN_16;
       	}
       	        i2sHandleRx = I2S_INIT(PSP_I2S_RX_INST_ID_1, PSP_I2S_RECEIVE, 
            PSP_I2S_CHAN_STEREO, PSP_DMA_POLLED, &i2sRxConfig, NULL);
       }


            /* Initialize DMA channels for Record */
            dmaRxConfig.pingPongMode    = TRUE;
            dmaRxConfig.autoMode        = PSP_DMA_AUTORELOAD_ENABLE;
            dmaRxConfig.burstLen        = PSP_DMA_TXBURST_1WORD;
            dmaRxConfig.chanDir         = PSP_DMA_READ;
            dmaRxConfig.trigger         = PSP_DMA_EVENT_TRIGGER;
            dmaRxConfig.trfType         = PSP_DMA_TRANSFER_IO_MEMORY;
            
            if (filterOption == BAT_FILTER)
            {
	            dmaRxConfig.dataLen         =  (FFT_LENGTH*8); //fft length * 2(num of buffers) * 2(extra word sent from dma) * 2(2bytes in a word)
	            dmaRxConfig.destAddr        = (Uint32)rcvBatBuffer;
            }
            else //LOW AND HIGH AUDIO
            {
	            dmaRxConfig.dataLen         = FIR_FILTER_BLOCKSIZE * 8; //block size * 2(num of buffers(ping&pong)) * 2(extra word sent from dma) * 2(2bytes in a word)
	            dmaRxConfig.destAddr        = (Uint32)rcvAudioBuffer;
            }
            
            dmaRxConfig.callback        = I2S_DmaRxLChCallBack;
            dmaRxConfig.dmaEvt          = PSP_DMA_EVT_I2S1_RX;
            dmaRxConfig.srcAddr         = (Uint32)I2S1_I2SRXLT0;

            /* Request and configure a DMA channel for left channel data */
            hDmaRxLeft = I2S_DMA_INIT(i2sHandleRx, &dmaRxConfig);
            if (hDmaRxLeft == NULL)
            {
                return 0;
            }
            DDC_I2S_transEnable((DDC_I2SHandle)i2sHandleRx, FALSE);
            DMA_StartTransfer(hDmaRxLeft);
            
           // I2S_transEnable(hhI2s, TRUE);
          DDC_I2S_transEnable((DDC_I2SHandle)i2sHandleRx, TRUE);  

    IRQ_enable(DMA_EVENT);    
    IRQ_globalEnable();             
                      
            
    return 0;
            
}


void disable_dma(void)
{
      DMA_StopTransfer(hDmaRxLeft);
      IRQ_disable(DMA_EVENT);
}






