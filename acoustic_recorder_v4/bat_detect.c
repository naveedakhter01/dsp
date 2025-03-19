


#include "bat_detect.h"
#include "ezdsp5535_spi.h"
#include "window_tables.h"
#include "multiply_tables.h"
#include "hwafft.h"
#include "func.h"
#include "hardware.h"
#include "command.h"



#define BAT_RECHECK // in the BatDataOut function this determines whether to continue 
					// checking for bat sounds and cut off early if none




#define FFT_LENGTH 1024
#define BLOCK_LENGTH (FFT_LENGTH/2)
#define FFT_FLAG ( 0 ) /* HWAFFT to perform FFT */
#define IFFT_FLAG ( 1 ) /* HWAFFT to perform IFFT */
#define SCALE_FLAG ( 0 ) /* HWAFFT to scale butterfly output */
#define NOSCALE_FLAG ( 1 ) /* HWAFFT not to scale butterfly output */
#define OUT_SEL_DATA ( 0 ) /* Indicates HWAFFT output located in input data vector */
#define OUT_SEL_SCRATCH ( 1 ) /* Indicates HWAFFT output located in scratch vector */


#define NUMOFFFTS 96*2 // ~172 *2(overlap)) ffts per second(1/5.8ms), buffer 1.5 seconds worth
//#define NUMOFFFTS 4*2 // ~172 *2(overlap)) ffts per second(1/5.8ms), buffer 1.5 seconds worth

#define NUM_OF_BUCKETS 32
//#define NUM_OF_BUFFERED 8
#define NUM_OF_BUFFERED 512

#define WAIT_FOR_TRIG 0
#define SEND_TO_ARM 1



//timeout is decremented every 5.8ms based on fft calc
#define WAIT_FOR_TRIG_TIMEOUT 172 //time for sencond trigger to happen     144 * 5.8ms = 1.5sec
#define MIN_TIME_SEC 172*3    // time 3 seconds

#define SEND_TO_ARM_TIMEOUT 2580  //2580   // > 12 sec of recording                15sec * 172 = 2580




extern Int16 rcvBatBuffer[];


//Proto's
void ResetTrigBuffers(void);





#pragma DATA_SECTION(scratch_buf, "scratch");
Int32 scratch_buf[FFT_LENGTH*2];  //two blocks of data so that previous fft can be saved

#pragma DATA_SECTION(data_buf, "data_buf");
//Static Allocation to Section: "data_buf : > DARAM" in Linker CMD File
Int32 data_buf[2*FFT_LENGTH];

#define ALIGNMENT 2*FFT_LENGTH // ALIGNS data_br_buf to an address with log2(4*N) zeros in the
// least significant bits of the byte address
#pragma DATA_SECTION(data_br_buf, "data_br_buf");
// Allocation to Section: "data_br _buf : > DARAM" in Linker CMD File
#pragma DATA_ALIGN (data_br_buf, ALIGNMENT);
Int32 data_br_buf[FFT_LENGTH*2]; //two blocks of data so that previous fft can be saved



#pragma DATA_SECTION(circBuckets, "CircBuckets");
#pragma DATA_ALIGN (circBuckets, 2);
Uint16 circBuckets[NUM_OF_BUCKETS*NUM_OF_BUFFERED];      //512bins divided up into 32 groups of avg 16bins, then 8 previous samples 
Uint16 maxBinBuckets[NUM_OF_BUCKETS];      //used to save the buckets with highest bin values


#pragma DATA_ALIGN (magnitude, 2);
Uint32 magnitude[FFT_LENGTH*2];  // two seperate fft's for ping and pong


Uint16 newCircBufIndex;
Uint16 oldCircBufIndex;
Uint16 tempCircBufIndex;   // used as temp buf index

Int32 sumBuckets[32]; //used to store sum of buckets for averaging
Uint16 avgBuckets[32];

Int16 compr_data_buf[NUMOFFFTS][128]; //circ buf of 64 bin multidimensional arrays (fftsize = 1024 / 8(compress to 8) / 2 (use bottom half of fft only))
Int32 read_compr_buf_index =0;  //index to read from
Int32 write_compr_buf_index =0; //index to write to

#define MIN_MULTIPLIER 0x2C0 //the base multiplier
#define MAX_MULTIPLIER 0xf00

static Int16 stoptrig;   //used to prevent trigger before all buffer values have been set 
static Int16 delaytrig;   //used to prevent trigger before all buffer values have been set 

static Uint16 audioflag = 0;
static Uint16 batflag = 0; // set when bat detected 
   
static Uint16 initMultiplier = MIN_MULTIPLIER; //initial multiplier used to determine the base threshold when audio noise levels ~ 0 
static Uint16 bufMultiplier;  // used to buffer the newly determined actual multiplier, is discarded if lower than the last actual multiplier value 
static Uint16 actualMultiplier = MAX_MULTIPLIER;  //Actual multiplier deterimined from initial multiplier and the background audio noise levels


static Bool buffersReset = FALSE; 


extern Int16 pingFftFlag;
extern Int16 pongFftFlag;








/*
 * Reset state of all elements in buffers
 *
 */ 
void ResetTrigBuffers(void)
{
	Int16 i;
	
    for(i=0;i<(FFT_LENGTH*2);i++) { scratch_buf[i] =0; }
    for(i=0;i<32;i++) { sumBuckets[i] =0; }
    for(i=0;i<(NUM_OF_BUCKETS*NUM_OF_BUFFERED);i++){  circBuckets[i] = 0;  }  
    newCircBufIndex =0;  // index in array to store new buckets
    stoptrig = 512; //move this later
    
    actualMultiplier = initMultiplier; //
    
    buffersReset = TRUE;
    
}






/*********************************
* Bat Detect
*
* analyses sampled data from adc and exits when "bat" detected
* window adc data 
* bit reverse
* fft
* test for bat signature
* 
* - buffers are shared between this and the BatDataOut routine, so that routine can send out some of the sampled data from this one
* -
* 
**********************************/      
Uint16 BatDetect(void)
{
   Int32 *data;
    Int32 *data_br, *scratch;
    Int32 *result;
    Uint16 fft_flag, scale_flag, out_sel;
    
    Int16 i;
    Int32 blocknum;

    Uint16 trigflag = 0; //
    
    data = data_buf;
    data_br = data_br_buf;
    scratch = scratch_buf;

    fft_flag = FFT_FLAG;
    scale_flag = SCALE_FLAG;   
   
    blocknum =0;

    trigflag = 0;  //reset trigger     
    delaytrig = -1;
    
    //NewCircBufIndex =0;  // index in array to store new buckets IS SET IN RESET BUFFERS LOOP
 	if (!buffersReset) ResetTrigBuffers();
 
    for (i=0;i<(FFT_LENGTH*2);i++) { scratch[i] =0; }
    
    
    if(stoptrig < 50) { stoptrig = 50; }
	actualMultiplier += initMultiplier;
	if (actualMultiplier >= MAX_MULTIPLIER) actualMultiplier = MAX_MULTIPLIER;
    
    while(!trigflag && !IsInCommandMode(LEVEL) )// exit if bat detected or arm signal
    {
      asm(" idle");
      
      
      if((pingFftFlag) || (pongFftFlag))   
       {
          //EZDSP5535_XF_on(); 
          if(pingFftFlag) 
          {
             blocknum = 0;
             
            //APPLY FIRST WINDOW AND PERFORM FIRST FFT
             ApplyWindow12bit(rcvBatBuffer,blackman_table, data, blocknum);   //apply window to first sets of data
            
            hwafft_br(data, data_br, FFT_LENGTH);  // bit-reverse input data
            out_sel = hwafft_1024pts(data_br, scratch, fft_flag, scale_flag); // perform FFT 
            if (out_sel == OUT_SEL_DATA)  result = data_br;
            else  result = scratch;
            
            fftmagnitude(result, &magnitude[0]);   //fill array with magnitude values
            fftstorebuckets(&magnitude[0],&circBuckets[newCircBufIndex*NUM_OF_BUCKETS],maxBinBuckets); //sum into 16bin groups and store in avg array
          }
          else if(pongFftFlag) 
          {
             blocknum = 2;
             
            //APPLY FIRST WINDOW AND PERFORM FIRST FFT
             ApplyWindow12bit(rcvBatBuffer,blackman_table, data, blocknum);   //apply window to first sets of data
            hwafft_br(data, &data_br[FFT_LENGTH], FFT_LENGTH);  // bit-reverse input data
            out_sel = hwafft_1024pts(&data_br[FFT_LENGTH], &scratch[FFT_LENGTH], fft_flag, scale_flag); // perform FFT 
            if (out_sel == OUT_SEL_DATA)  result = &data_br[FFT_LENGTH];
            else  result = &scratch[FFT_LENGTH];
            
            fftmagnitude(result, &magnitude[FFT_LENGTH]); //fill array with magnitude values
            fftstorebuckets(&magnitude[FFT_LENGTH],&circBuckets[newCircBufIndex*NUM_OF_BUCKETS],maxBinBuckets); //sum into 16bin groups and store in avg array
          }
          else break;
         
         
         pingFftFlag =0;
         pongFftFlag =0;
         
            //test if audio level jumped up, if true then disable triggering for a number of cycles
            audioflag = fftCheckAudio(&circBuckets[newCircBufIndex*NUM_OF_BUCKETS],avgBuckets);
            if(audioflag) { stoptrig = 9;}
            
            //test if bat detected, if true set trigger to go off after set delay period
            if(delaytrig == -1) // if delaytrigger hasn't been set
            {
	            batflag = fftCheckBat(&circBuckets[newCircBufIndex*NUM_OF_BUCKETS],avgBuckets,actualMultiplier); //do a 
	            batflag |= fftCheckBat(maxBinBuckets,avgBuckets,(actualMultiplier*3));
	            if(batflag) {
	            	delaytrig = 5;   // delay this many cycles before trigger, allows cancellation if audio noise detected
	            } 
            }
            //calc average bucket value from all previous samples, does it quickly by adding new values to sum and subtracting old values then averaging
            oldCircBufIndex  = (newCircBufIndex + 1) & (NUM_OF_BUFFERED-1);
            fftaverage(&circBuckets[newCircBufIndex*NUM_OF_BUCKETS],&circBuckets[oldCircBufIndex*NUM_OF_BUCKETS]
                                     ,sumBuckets, avgBuckets);  //average previous buckets         
            //determine the bat noise threshold level based on the average audio noise level                                            
          
           
            //the threshold always decrements,but will be set to a level based on the audio level. If
            //   the audio level rises quickly then drops the threshold will imediately be set to a higher value 
            //   then slowly drop over time.     
           {   actualMultiplier --;   }  //always decremt every cycle
            bufMultiplier  = GetUltrasonicThreshold(avgBuckets,multi_table,initMultiplier);  // get the threshold level based on audio level
           if(bufMultiplier > actualMultiplier) //if multiplier has decremented lower than the recommended one based audio level
           {                                    //then set to recommended multiplier 
           	 actualMultiplier = bufMultiplier;   
           }     
         
           newCircBufIndex  = ++newCircBufIndex & (NUM_OF_BUFFERED-1);              
                                                      
            EZDSP5535_XF_off();                          
            if(stoptrig > 0 )
            {   
            	stoptrig--;  //decrement every cycle
            	delaytrig = -1;  //stop the trigger from happening
            }
  
            if(delaytrig > 0)
            {
              delaytrig--;
              if(delaytrig == 0) //
              {
              	delaytrig = -1;  //disable delaytrigger
              	trigflag =1;
              	EZDSP5535_XF_on();
              }
            }
         
           //EZDSP5535_XF_off(0); 

       }
       
       
       
    }
    
    
   EZDSP5535_XF_off(); 
       return 0;
}  



/*********************************
 *Bat Data Out
* 
* window adc data
* bit reverse
* fft
* check for trig
* compress
* buffers fft data
**********************************/
Uint16 BatDataOut(void)
{
   Int32 *data;
    Int32 *data_br, *scratch;
    Int32 *result;
    Uint16 *outdata;
    Uint16 fft_flag, scale_flag, out_sel;
    // i;
    Int32 blocknum,blocknum2;
    
    int timeout = WAIT_FOR_TRIG_TIMEOUT; //about a second to trigger
    int modeState = WAIT_FOR_TRIG; 
    int minRecTime = MIN_TIME_SEC;

    data = data_buf;
    data_br = data_br_buf;
    scratch = scratch_buf;

    fft_flag = FFT_FLAG;
    scale_flag = SCALE_FLAG; 
   
    blocknum =0;

    audioflag = 0;
    batflag = 0; // set when bat detected 
    
    read_compr_buf_index =0;  //reset index to read from
    write_compr_buf_index =0; //reset index to write to     
         
    stoptrig = 9;
      
 
   EZDSP5535_XF_on(); 
   while(timeout > 0  && !IsInCommandMode(LEVEL))
   {
      if((pingFftFlag) || (pongFftFlag))   
       {
       	timeout--;
          if(pingFftFlag)  {
             blocknum = 3;
             blocknum2 = 0;
          }
          else if(pongFftFlag){
             blocknum = 1;
             blocknum2 =2;
          }
          else break;
          
         //APPLY FIRST WINDOW AND PERFORM FIRST FFT
         ApplyWindow16bit(rcvBatBuffer,blackman_table, data, blocknum);   //apply window to first sets of data
         hwafft_br(data, data_br, FFT_LENGTH);  // bit-reverse input data
         out_sel = hwafft_1024pts(data_br, scratch, fft_flag, scale_flag); // perform FFT 
         if (out_sel == OUT_SEL_DATA)  result = data;
         else  result = scratch;
         //compression
         
	     fftcompress(result,&compr_data_buf[write_compr_buf_index][0]);    //compress first block
           
         //APPLY SECOND WINDOW AND PERFORM SECOND FFT
         ApplyWindow16bit(rcvBatBuffer,blackman_table, data, blocknum2);   //apply window to first sets of data
         hwafft_br(data, data_br, FFT_LENGTH);  // bit-reverse input data
         out_sel = hwafft_1024pts(data_br, scratch, fft_flag, scale_flag); // perform FFT 
         if (out_sel == OUT_SEL_DATA)  result = data;
         else  result = scratch;
         //compression
		 fftcompress(result,&compr_data_buf[write_compr_buf_index][64] ); //compress second block
        
         if(++write_compr_buf_index >= NUMOFFFTS) write_compr_buf_index =0;  //inc write index


         if(write_compr_buf_index == read_compr_buf_index) break; //BUFFER OVERRUN - EXIT

#ifdef BAT_RECHECK
       
       
	       if(pingFftFlag)
	       {
	            fftmagnitude(result, &magnitude[0]);   //fill array with magnitude values
	            fftstorebuckets(&magnitude[0],&circBuckets[newCircBufIndex*NUM_OF_BUCKETS],maxBinBuckets); //sum into 16bin groups and store in avg array
	       } 
	       else //pong
	       {
	            fftmagnitude(result, &magnitude[FFT_LENGTH]); //fill array with magnitude values
	            fftstorebuckets(&magnitude[FFT_LENGTH],&circBuckets[newCircBufIndex*NUM_OF_BUCKETS],maxBinBuckets); //sum into 16bin groups and store in avg array
	       }
	    
#endif	    
	    
	        pingFftFlag =0;
	        pongFftFlag =0;


#ifdef BAT_RECHECK
	
	       
	        //test if audio level jumped up, if true then disable triggering for a number of cycles
            audioflag = fftCheckAudio(&circBuckets[newCircBufIndex*NUM_OF_BUCKETS],avgBuckets);
            if(audioflag) { stoptrig = 9;}
            
            //check bat audio level has passed threshold and audio level hasn't triggered  
            batflag = fftCheckBat(&circBuckets[newCircBufIndex*NUM_OF_BUCKETS],avgBuckets,actualMultiplier); //do a 
            batflag |= fftCheckBat(maxBinBuckets,avgBuckets,(actualMultiplier*3));  // multiplier needs to be much higher than the mean multiplier
            
            
            //calc average bucket value from all previous samples, does it quickly by adding new values to sum and subtracting old values then averaging
            oldCircBufIndex  = (newCircBufIndex + 1) & (NUM_OF_BUFFERED-1);
            fftaverage(&circBuckets[newCircBufIndex*NUM_OF_BUCKETS],&circBuckets[oldCircBufIndex*NUM_OF_BUCKETS]
                                     ,sumBuckets, avgBuckets);  //average previous buckets             
	   
            {actualMultiplier --;}
            bufMultiplier  = GetUltrasonicThreshold(avgBuckets,multi_table,initMultiplier);  // get the threshold level based on audio level
            if(bufMultiplier > actualMultiplier){ //if current threshold level lower than the determined audio level
            	 actualMultiplier = bufMultiplier; 
            } 	    

	         newCircBufIndex  = ++newCircBufIndex & (NUM_OF_BUFFERED-1);
	         
	         
#endif	         
		   if(modeState == WAIT_FOR_TRIG) //check for second bat trigger event
		   {
		         if(stoptrig > 0) { //don't trigger until a number of fft's have been performed
		         	stoptrig--;
		         }
		         else if(batflag){ // if FTV greater than threshold then we have trigger (this is the second trigger)
	               	modeState = SEND_TO_ARM; //set next state to wait for arm to wake up
	               	timeout = SEND_TO_ARM_TIMEOUT;  //set next timeout
	                if (!IsInCommandMode(LEVEL))  // if not exit by arm signal then send back event signal
						DSP_EVENT_ENABLE(); // tell arm to get ready as data maybe comming
	                batflag=0;
		         }   
		   }
	       else if(modeState == SEND_TO_ARM)  //arm has responded so send data out through spi
	       {
				outdata = (Uint16*)&compr_data_buf[read_compr_buf_index][0];	
				
				//outdata = testData;	   
				EZDSP5535_SPI_write(outdata, (128)); //first fft half data (2bytes in word)
				if(++read_compr_buf_index >= NUMOFFFTS) read_compr_buf_index = 0;  //increment read index
		
		         if(stoptrig > 0){  //don't trigger until a number of fft's have been performed
		         	stoptrig--;
		         }
		         else if(batflag){ // if FTV greater than threshold then we have trigger (this is the second trigger)
		            minRecTime = MIN_TIME_SEC; //if trigger , reset the terminate early delay
		         }   
	
		         //Terminate Recording early if no triggers
		         if(minRecTime < 0) {          //only terminate after a minimum period
		                   timeout = 0;
		         }
		         else{
		         	minRecTime--;
		         }
	       }		   
		   
   


	 //EZDSP5535_XF_off( );
		
    }
   }
   
   //DSPReadyToSend(DISABLE); //tell arm we  finished
   EZDSP5535_XF_off( );
   return 1; 
}  





/************************************EOF************************************/

















