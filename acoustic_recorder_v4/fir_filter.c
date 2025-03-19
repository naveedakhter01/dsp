

#include "fir_filter.h"

#include "ezdsp5535.h"
#include "ezdsp5535_spi.h"
#include "hwafft.h"
#include "func.h"
#include "hardware.h"
#include "fir_coefficients.h"
#include "command.h"



#pragma DATA_SECTION(filteredData, "data_buf");
#pragma DATA_ALIGN (filteredData, 2);
Uint32 filteredData[384];


extern Int16 rcvAudioBuffer[];

extern Int16 pingFftFlag;
extern Int16 pongFftFlag;








/*********************************
* Test fir filter
* 
* 
**********************************/
Uint16 FirFilterHigh()
{
    // i;
    Int32 blocknum;
    
    blocknum =0;
      
     
    //write testdata to block to check if data on arm side is correct
 
   EZDSP5535_XF_on(); 
   while(!IsInCommandMode(LEVEL)) //while out of command mode
    {
      if((pingFftFlag) || (pongFftFlag))   
       {

          if(pingFftFlag)  {
             blocknum = 0;
          }
          else if(pongFftFlag){
             blocknum = 1;
          }
          else break;
          	pingFftFlag =0;
	        pongFftFlag =0;
			//APPLY FIRST WINDOW AND PERFORM FIRST FFT
			// ApplyWindow16bit(RcvL1,blackman_table, data, blocknum);   //apply window to first sets of data
			
			//EZDSP5535_XF_on( );
			BlockFirFilterHigh(rcvAudioBuffer, highpass_coefficients, filteredData, blocknum); //RcvL1
			//Decimate16bit512To256(RcvL1, (Uint16*)filteredData, blocknum);

	 EZDSP5535_XF_off( );
		
    }
   }
   
   //DSPReadyToSend(DISABLE); //tell arm we  finished

   return 1; 
}  

/*********************************
* Test fir filter
* 
* 
**********************************/
Uint16 FirFilterLow()
{
    Int32 blocknum;   

    blocknum =0;  
      
 
    //write testdata to block to check if data on arm side is correct
 
   EZDSP5535_XF_on(); 
   while(!IsInCommandMode(LEVEL)) //while out of command mode 
    {
      if((pingFftFlag) || (pongFftFlag))   
       {
          if(pingFftFlag)  {
             blocknum = 0;
          }
          else if(pongFftFlag){
             blocknum = 1;
          }
          else break;
          	pingFftFlag =0;
	        pongFftFlag =0;
			//APPLY FIRST WINDOW AND PERFORM FIRST FFT
			// ApplyWindow16bit(RcvL1,blackman_table, data, blocknum);   //apply window to first sets of data
			
			//EZDSP5535_XF_on( );
			BlockFirFilterLow(rcvAudioBuffer, lowpass_coefficients, filteredData, blocknum); //RcvL1

	//	outdata = (Uint16*)filteredData;	
		//outdata = (Uint16*)testData;	   
		//EZDSP5535_SPI_write(outdata, (256) ); //first fft half data (2bytes in word)

	 EZDSP5535_XF_off( );
		
    }
   }
   
   //DSPReadyToSend(DISABLE); //tell arm we  finished

   return 1; 
}  



/************************************EOF************************************/



