//////////////////////////////////////////////////////////////////////////////
// * File name: ezdsp5535_spiflash.c
// *                                                                          
// * Description:  SPI FLASH interface.
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
#include "ezdsp5535_spi.h"
#include "csl_spi.h"

CSL_Status SPI_write16 (CSL_SpiHandle hSpi,
						Uint16	*writeBuffer,
						Uint16	bufLen);

CSL_Status SPI_read16	(CSL_SpiHandle hSpi,
					Uint16	*readBuffer,
					Uint16	bufLen);

//static Uint16 spiflashbuf[8];
CSL_SpiHandle   hSpi;

/*
 *  EZDSP5535_SPI_init( )
 *
 *      Enables and configures SPI for the SPI
 *      ( CS0, EBSR Mode 1, 8-bit, 100KHz clock )
 */
Int16 EZDSP5535_SPI_init( Uint16 clk)
{
	volatile Uint16 delay;
	ioport volatile CSL_SysRegs	*sysRegs;
    SPI_Config      hwConfig;
    Int16           result = 0;

	if (hSpi != NULL) SPI_close(hSpi);	

    hSpi->configured = FALSE;   // Set as unconfigured
    hSpi = NULL;                // Remove previous settings
    //result += SPI_init();       // Enable SPI
	sysRegs = (CSL_SysRegs *)CSL_SYSCTRL_REGS;
	CSL_FINS(sysRegs->PCGCR1, SYS_PCGCR1_SPICG, CSL_SYS_PCGCR1_SPICG_ACTIVE);
	/* Value of 'Reset Counter' */
	CSL_FINS(sysRegs->PSRCR, SYS_PSRCR_COUNT, 0x20);
	CSL_FINS(sysRegs->PRCR, SYS_PRCR_PG4_RST, CSL_SYS_PRCR_PG4_RST_RST);
	for(delay = 0; delay < 100; delay++);
	//CSL_FINS(sysRegs->EBSR, SYS_EBSR_PPMODE, CSL_SYS_EBSR_PPMODE_MODE5);
    result+= CSL_SOK;
    
    hSpi = SPI_open(SPI_CS_NUM_2, SPI_POLLING_MODE); // Enable CS2
    
    //CSL_SYSCTRL_REGS->EBSR = (CSL_SYSCTRL_REGS->EBSR 
    //                             & 0x0fff) | 0x1000; // EBSR Mode 1 (7 SPI pins)

    /* Configuration for SPIFLASH */
    hwConfig.wLen       = SPI_WORD_LENGTH_16;    // 8-bit
    if (clk ==0)
    	hwConfig.spiClkDiv  = 0x004;               // For FIR_LOW set to 9 (+1 = div 10), 
    else if (clk == 1)
    	hwConfig.spiClkDiv  = 0x004;               // For FIR_HIGH set to 9 (+1 = div 10), 
    else
    	hwConfig.spiClkDiv  = 0x013;               //
    hwConfig.csNum      = SPI_CS_NUM_2;         // Select CS2
    hwConfig.csPol 		= SPI_CSP_ACTIVE_LOW;
    hwConfig.frLen      = 1;
    hwConfig.dataDelay  = SPI_DATA_DLY_0;
    hwConfig.clkPol     = SPI_CLKP_HIGH_AT_IDLE;
    hwConfig.clkPh      = SPI_CLK_PH_FALL_EDGE;
    
    /* Configure SPIFLASH */
    result += SPI_config(hSpi, &hwConfig);
    
    return result;  
}





/*
 *  EZDSP5535_SPI_read()
 */
Int16 EZDSP5535_SPI_read(Uint16 *src, Uint32 len )
{
    Int16 result = 0;

    /* Execute SPI read cycle */
    CSL_SPI_REGS->SPICMD1 = 0x0000 | len -1;
    //result += SPI_dataTransaction(hSpi ,spiflashbuf, 4, SPI_WRITE);
    //result += SPI_dataTransaction(hSpi ,spiflashbuf, 1, SPI_READ);
    result += SPI_read16(hSpi ,src, len);
    return result;
}

/*
 *  EZDSP5535_SPI_write()
 */
Int16 EZDSP5535_SPI_write(Uint16 *src, Uint32 len ) 
{
    Int16 result = 0;

        /* Issue WPEN */
        CSL_SPI_REGS->SPICMD1 = 0x0000 | len -1;   //Frame Length = N-1
        //result += SPI_dataTransaction(hSpi ,src, len, SPI_WRITE);
        result += SPI_write16(hSpi ,src, len);
    return result;
}


CSL_Status SPI_write16 (CSL_SpiHandle hSpi,
						Uint16	*writeBuffer,
						Uint16	bufLen)
{
	//Uint16 	getWLen;
	volatile Uint16 	bufIndex;
	Uint16 	spiStatusReg;
	volatile Uint16 	spiBusyStatus;
	volatile Uint16 	spiWcStaus;
	volatile Uint16     delay;

	bufIndex = 0;

	/* Read the word length form the register */
	//getWLen = 16;

	/* Write Word length set by the user */
	while(bufIndex < bufLen)
	{
		/* Write to registers more then 16 bit word length */
			CSL_SPI_REGS->SPIDR2 = writeBuffer[bufIndex];
			CSL_SPI_REGS->SPIDR1 = 0x0000;
			bufIndex ++;
		/* Set command for Writting to registers */
		CSL_FINS(CSL_SPI_REGS->SPICMD2, SPI_SPICMD2_CMD, SPI_WRITE_CMD);

		for(delay = 0; delay < 10; delay++);
		do
		{
			spiStatusReg = CSL_SPI_REGS->SPISTAT1;
			spiBusyStatus = (spiStatusReg & CSL_SPI_SPISTAT1_BSY_MASK);
			spiWcStaus = (spiStatusReg & CSL_SPI_SPISTAT1_CC_MASK);
		}while((spiBusyStatus == CSL_SPI_SPISTAT1_BSY_BUSY) &&
				(spiWcStaus != CSL_SPI_SPISTAT1_CC_MASK));
	}
	return (CSL_SOK);
}






// modified read spi function 
// word len set to 16

CSL_Status SPI_read16	(CSL_SpiHandle hSpi,
					Uint16	*readBuffer,
					Uint16	bufLen)
{
	volatile Uint16 	bufIndex;
	Int16 	spiStatusReg;
	volatile Int16 	spiBusyStatus;
	volatile Int16 	spiWcStaus;
	volatile Uint16 delay;

	bufIndex = 0;
	if((NULL == readBuffer) || (0 == bufLen))
	{
		return (CSL_ESYS_INVPARAMS);
	}

	/* Read the word length form the register */


	/* Read Word length set by the user */
	while(bufIndex < bufLen)
	{
		/* Set command for reading buffer */
		CSL_FINS(CSL_SPI_REGS->SPICMD2, SPI_SPICMD2_CMD, CSL_SPI_SPICMD2_CMD_READ);

		for(delay = 0; delay < 10; delay++);

		do
		{
			spiStatusReg = CSL_SPI_REGS->SPISTAT1;
			spiBusyStatus = (spiStatusReg & CSL_SPI_SPISTAT1_BSY_MASK);
			spiWcStaus = (spiStatusReg & CSL_SPI_SPISTAT1_CC_MASK);
		}while((spiBusyStatus == CSL_SPI_SPISTAT1_BSY_BUSY) &&
				(spiWcStaus != CSL_SPI_SPISTAT1_CC_MASK));

			readBuffer[bufIndex] = CSL_SPI_REGS->SPIDR1;
			bufIndex++;
	}
	return (CSL_SOK);
}




/*******************************
 * writes one 16bit value to spi port
 * 
 * - doesn't check for busy afterwards
 * 
 */
void SPI_write16_once (Uint16 *src)
{
	CSL_SPI_REGS->SPICMD1 = 0x0000 | 1 -1;   //Frame Length = N-1

	/* Write to registers more then 16 bit word length */
	CSL_SPI_REGS->SPIDR2 = *src;
	CSL_SPI_REGS->SPIDR1 = 0x0000;
	
	/* Set command for Writting to registers */
	CSL_FINS(CSL_SPI_REGS->SPICMD2, SPI_SPICMD2_CMD, SPI_WRITE_CMD);
}






















