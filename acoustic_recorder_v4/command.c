
#include "command.h"
#include "ezdsp5535_spi.h"
#include "csl_i2c.h"
#include "csl_intc.h"
#include "sysclk.h"




#define ICMDR_BC_MASK 0x0007
#define ICMDR_FDF_MASK 0x0008
#define ICMDR_STB_MASK 0x0010
#define ICMDR_IRS_MASK 0x0020
#define ICMDR_DLB_MASK 0x0040
#define ICMDR_RM_MASK 0x0080
#define ICMDR_XA_MASK 0x0100
#define ICMDR_TRX_MASK 0x0200
#define ICMDR_MST_MASK 0x0400
#define ICMDR_STP_MASK 0x0800
#define ICMDR_RESERVED_MASK 0x1000
#define ICMDR_STT_MASK 0x2000
#define ICMDR_FREE_MASK 0x4000
#define ICMDR_NACKMOD_MASK 0x8000



#define ICSTR_AL_MASK 0x0001
#define ICSTR_NACK_MASK 0x0002
#define ICSTR_ARDY_MASK 0x0004
#define ICSTR_ICRRDY_MASK 0x0008
#define ICSTR_ICXRDY_MASK 0x0010
#define ICSTR_SCD_MASK 0x0020
#define ICSTR_AD0_MASK 0x0100
#define ICSTR_AAS_MASK 0x0200
#define ICSTR_XSMT_MASK 0x0400
#define ICSTR_RSFULL_MASK 0x0800
#define ICSTR_BB_MASK 0x1000
#define ICSTR_NACKSNT_MASK 0x2000
#define ICSTR_SDIR_MASK 0x4000


#define ICPSC_IPSC_MASK 0x00FF

#define ICIMR_AL_MASK 0x0001
#define ICIMR_NACK_MASK 0x0002
#define ICIMR_ARDY_MASK 0x0004
#define ICIMR_ICRRDY_MASK 0x0008
#define ICIMR_ICXRDY_MASK 0x0010
#define ICIMR_SCD_MASK 0x0020
#define ICIMR_AAS_MASK 0x0040


#define ICOAR 	*((ioport volatile unsigned *)0x1A00)  //I2C Own Address Register Section 9.3.1
#define ICIMR 	*((ioport volatile unsigned *)0x1A04)  //I2C //Interrupt Mask Register Section 9.3.2
#define ICSTR 	*((ioport volatile unsigned *)0x1A08)  //I2C //Interrupt Status Register Section 9.3.3
#define ICCLKL 	*((ioport volatile unsigned *)0x1A0C)  //I2C //Clock Low-Time Divider Register Section 9.3.4
#define ICCLKH 	*((ioport volatile unsigned *)0x1A10)  //I2C //Clock High-Time Divider Register Section 9.3.4
#define ICCNT 	*((ioport volatile unsigned *)0x1A14)  //I2C //Data Count Register Section 9.3.5
#define ICDRR 	*((ioport volatile unsigned *)0x1A18)  //I2C //Data Receive Register Section 9.3.6
#define ICSAR 	*((ioport volatile unsigned *)0x1A1C)  //I2C //Slave Address Register Section 9.3.7
#define ICDXR 	*((ioport volatile unsigned *)0x1A20)  //I2C //Data Transmit Register Section 9.3.8
#define ICMDR 	*((ioport volatile unsigned *)0x1A24)  //I2C //Mode Register Section 9.3.9
#define ICIVR 	*((ioport volatile unsigned *)0x1A28)  //I2C //Interrupt Vector Register Section 9.3.10
#define ICEMDR 	*((ioport volatile unsigned *)0x1A2C)  //I2C //Extended Mode Register Section 9.3.11
#define ICPSC 	*((ioport volatile unsigned *)0x1A30)  //I2C Prescaler Register Section 9.3.12
#define ICPID1 	*((ioport volatile unsigned *)0x1A34)  //I2C Peripheral Identification Register 1 Section 9.3.13
#define ICPID2 	*((ioport volatile unsigned *)0x1A38)


#define IC2_CONTROLLER_FREQ 7680000	
#define IC2_SERIAL_CLOCK_FREQ 384000 // not quite 400khz





//commands 

typedef enum {
	VERSION_ID_0, 	
	VERSION_ID_1, 	
	VERSION_ID_2, 		 
	VERSION_ID_3, 	
	SAMPLING_MODE, 	
	TEST_MODE,	
	TOGGLE_READ
} CommandID;





//test mode option bits
typedef enum {
	PREAMP_SWITCH = 0,
	LOW_SWITCH,
	HIGH_SWITCH,
	BAT_SWITCH,
	MIC_SWITCH,
	LED_SWITCH
} TestSwicthes;


//const Uint8 version_id_0 = 'A';
//const Uint8 version_id_1 = 'S';
//const Uint8 version_id_2 = 'P';
//const Uint8 version_id_3 = 'V';

extern Uint8 version_id_0;   // MAJOR VERSION
                  // .  
extern Uint8 version_id_1;   // MINOR VERSION


extern Uint8 version_id_2;  // not used
extern Uint8 version_id_3;  //not used

FilterOption filterOption = LOW_FILTER; //default mode
Uint8 testMode;


//
//void CommandMode(void)
//{
//   Uint16 rcvWord, sendWord;
//	Uint8 data;
//	CommandID command;
//	Bool writeMode;
//	int toggleMask =0;
//
//   DSP_EVENT_ENABLE(); // keep the dsp event line high during command mode
//   while (IsInCommandMode(EDGE) )
//   {
//   	
//   	 if ( IsCommandRequest())
//   	 {
//   	 	EZDSP5535_XF_on( );
//	   	EZDSP5535_SPI_read(&rcvWord, 1); //read one word from arm 
//   	    data = (Uint8)(rcvWord & 0xff);
//   	    command = (CommandID)(rcvWord>>8);
//   	
//   		writeMode = FALSE;
//   	    if ((command & 0x80 ) != 0) writeMode = TRUE;
//        command &= 0x7F;
//		
//		switch (command)
//		{
//			case VERSION_ID_0:
//			    data = version_id_0;
//				break;
//			case VERSION_ID_1:
//				data = version_id_1;
//				break;
//			case VERSION_ID_2:
//				data = version_id_2;
//				break;
//			case VERSION_ID_3:
//				data = version_id_3;
//				break;										
//			case SAMPLING_MODE:
//				if (writeMode) 
//					filterOption = (FilterOption)data;
//				else
//					data = (Uint8)filterOption;	
//				break;
//			case TEST_MODE:
//				if (writeMode) 
//				{
//					testMode = data;
//					//now modify switches
//					((testMode & (1<<PREAMP_SWITCH)) != 0) ?  PREAMP_ENABLE() : PREAMP_DISABLE();
//					(testMode & (1<<LOW_SWITCH)) != 0 ?  LOW_FILTER_ENABLE() : LOW_FILTER_DISABLE();
//					(testMode & (1<<HIGH_SWITCH)) != 0 ?  HIGH_FILTER_ENABLE() : HIGH_FILTER_DISABLE();
//					(testMode & (1<<BAT_SWITCH)) != 0 ?  BAT_FILTER_ENABLE() : BAT_FILTER_DISABLE();
//					(testMode & (1<<MIC_SWITCH)) != 0 ?  MIC_ENABLE() : MIC_DISABLE();
//					(testMode & (1<<LED_SWITCH)) != 0 ?  LED_ON() : LED_OFF();
//				}	
//				else
//					data = testMode;	
//				break;
//				
//				// this sends back alternating 0x55 and 0xAA for checking if dsp is in dsp mode
//			case TOGGLE_READ: 
//			    if (!writeMode)
//			    {
//			    	if (toggleMask != 0)
//			    		data= 0x55;
//			    	else
//			    		data = 0xAA;	
//			    	toggleMask ^=0x01;	
//			    }
//			    break;   	
//			default:
//				writeMode = FALSE;
//			
//				break;
//		}
//
//            EZDSP5535_XF_off( );
//			EZDSP5535_wait(1000);
//
//			sendWord = 0;
//			sendWord = ((Uint16)data &0xFF);
//			sendWord |= (Uint16)command << 8;
//			EZDSP5535_XF_on( );
//			EZDSP5535_SPI_write(&sendWord, 1); 
//            EZDSP5535_XF_off( );
//   	 }
//   }
//   
//   DSP_EVENT_DISABLE(); // keep the dsp event line high during command mode
//
//}


//read input pin from arm to determine if command mode running
// detect type deterimes if to use level or edge of signal to trigger
// - must detect a high to low transistion to deterime when to come out of command mode
// this allows the arm pin to start up as output 0 and still run command mode
Bool IsInCommandMode(DetectType detect)
{
	Bool ret = TRUE;  // assume we are in command mode
	static int lastState = 0;
	int nowState;
	
	if (detect == LEVEL)
	{
		if (ReadCommandModeInput() == 0) return FALSE; // turns out we aren't
	}
	else //EDGE DETECTION
	{
		nowState = ReadCommandModeInput();
		if ((nowState == 0 ) && (lastState != 0)) ret = FALSE; // turns out we aren't
		lastState = nowState;	
	}

	return ret;
}






Bool IsCommandRequest()
{
	Bool ret = FALSE;
	static int lastState = 0;
	int nowState;
	
	nowState = ReadCommandRequestInput();
	if ((nowState == 0 ) && (lastState != 0)) ret = TRUE;
	
	lastState = nowState;	
		
	return ret;
}


FilterOption GetFilterOption(void)
{
	return filterOption;	
}





void InitCommandI2c(void)
{


  // IRQ_globalDisable();
  // IRQ_clearAll();
  // IRQ_disableAll();
  //  IRQ_setVecs((Uint32)(&VECSTART));   
  //  IRQ_plug (I2C_EVENT, &i2c_isr);


	
	/* Enable I2C module in Idle PCGCR */
	CSL_FINST(CSL_SYSCTRL_REGS->PCGCR1,
	          SYS_PCGCR1_I2CCG, ACTIVE);
	
	
	//2. RESET I2C Module
	ICMDR &= ~ICMDR_IRS_MASK; //clear bit

	//3.Set own address 
	ICMDR &= ~ICMDR_XA_MASK; //clear bit
	ICOAR = 0x22; //own address 


	//4.interrupt mask
	ICIMR &= ~ICIMR_NACK_MASK; 
	ICIMR &= ~ICIMR_AL_MASK; 
	ICIMR |= ICIMR_AAS_MASK;  //Expect an interrupt when Master's Address matches yours
	ICIMR |= ICIMR_ICRRDY_MASK; //Expect a receive interrupt when a byte worth data sent 
						// from the master is ready to be read
	ICIMR |= ICIMR_ICXRDY_MASK; //Expect to receive interrupt when the transmit register is 
						//ready to be written with a new data that is to be sent to the master.
	ICIMR |= ICIMR_SCD_MASK;  //Expect to receive interrupt when Stop Condition is detected.


	//5.I2C operating freq (between 6.7 and 13.3 Mhz)
	ICPSC = ((SYSCLK_CLOCK_COMMAND / IC2_CONTROLLER_FREQ) -1)  & ICPSC_IPSC_MASK;

	//6. 
	ICCLKH = ((IC2_CONTROLLER_FREQ / IC2_SERIAL_CLOCK_FREQ) / 2) - 7; //7 if ipsc = 0
	ICCLKL = ((IC2_CONTROLLER_FREQ / IC2_SERIAL_CLOCK_FREQ) / 2) - 7;
	
	//7. mode register
	
	ICMDR &= ~ICMDR_MST_MASK; //Configure the I2C Controller to operate as SLAVE.
	ICMDR &= ~ICMDR_FDF_MASK; //Free Data Format is disabled.
	ICMDR &= ~ICMDR_BC_MASK; //Set data width to 8 bytes.
	ICMDR &= ~ICMDR_DLB_MASK; //Disable Loopback Mode.
	ICMDR &= ~ICMDR_STB_MASK; //I2C Controller can detect Start condition via H/W.
	ICMDR |= ICMDR_RM_MASK; //, STP=0, STT=1. See Table 16. (No Activity case).
	ICMDR &= ~ICMDR_STP_MASK;
	ICMDR |= ICMDR_STT_MASK;
	ICMDR &= ~ICMDR_FREE_MASK; // remaining bits to 0
	ICMDR &= ~ICMDR_NACKMOD_MASK;
	ICMDR &= ~ICMDR_TRX_MASK; // transmitter mode = 1, receiver mode = 0
	
	//8.Release I2C from Reset.
    ICMDR |= ICMDR_IRS_MASK; // release i2c from reset
    
	//9.Make sure Interrupt Status Register is cleared.
	ICSTR = ICSTR;  //clear int status
	while (ICIVR != 0) {}//wait for icivr to be clear
	 
	//10.Instruct I2C Controller to detect START Condition and Its Own Address.
	ICMDR |= ICMDR_STT_MASK;
     
     
   // IRQ_enable(I2C_EVENT);    
   // IRQ_globalEnable();           
     
     
}


void DisableCommandI2c(void)
{
	//2. RESET I2C Module
	ICMDR &= ~ICMDR_IRS_MASK; //clear bit		
	
	/* Enable I2C module in Idle PCGCR */
	CSL_FINST(CSL_SYSCTRL_REGS->PCGCR1,
	          SYS_PCGCR1_I2CCG, DISABLED);
	
}



int ReadCommand(int command);
void WriteCommand(int command, int data);

void CommandMode(void)
{
	static Bool commandValid = FALSE;	
	Bool dataIncomming = FALSE;	
	static int command;
		
   //enable the i2c 	
   InitCommandI2c();		
		
   DSP_EVENT_ENABLE(); // keep the dsp event line high during command mode
   while (IsInCommandMode(EDGE) )
   {
   	
		if ((ICSTR & ICSTR_AAS_MASK) != 0)
		{
			ICSTR = ICSTR_AAS_MASK;
			dataIncomming = TRUE;	
		}		
		
		if (dataIncomming)
		{	
			commandValid = FALSE;
			while (dataIncomming && IsInCommandMode(LEVEL))
			{
				if ((ICSTR & ICSTR_ICRRDY_MASK) != 0) 
				{   
					ICSTR = ICSTR_ICRRDY_MASK;
					// first received byte is the command
					if (!commandValid)
					{
					  command = ICDRR;
					  commandValid =TRUE;	
					  
					  ICDXR = ReadCommand(command); //save command value into read buffer if next request is 12c read
					}
					else  // if second or more data write then store value and inc command 
					{
					  	WriteCommand(command, ICDRR);
						command++;
					}
				    
				}	
				// data is ready to be written to read buffer 
				if ((ICSTR & ICSTR_ICXRDY_MASK) != 0) 
				{
					command++; 
					ICDXR = ReadCommand(command);
				}
				
				if ((ICSTR & ICSTR_SCD_MASK) != 0)
				{
					ICSTR = ICSTR_SCD_MASK; 
					dataIncomming = FALSE;
				}
			}	
	
			
		}


   }
   
   DSP_EVENT_DISABLE(); // keep the dsp event line high during command mode
   DisableCommandI2c(); // disable i2c
}



int ReadCommand(int command)
{
	static int toggleMask = 0;
	int data;
	
	switch (command)
	{
		case VERSION_ID_0:
		    data = version_id_0;
			break;
		case VERSION_ID_1:
			data = version_id_1;
			break;
		case VERSION_ID_2:
			data = version_id_2;
			break;
		case VERSION_ID_3:
			data = version_id_3;
			break;										
		case SAMPLING_MODE:
			data = (Uint8)filterOption;	
			break;
		case TEST_MODE:
			data = testMode;	
			break;
			
			// this sends back alternating 0x55 and 0xAA for checking if dsp is in dsp mode
		case TOGGLE_READ: 
	    	if (toggleMask != 0)
	    		data= 0x55;
	    	else
	    		data = 0xAA;	
	    	toggleMask ^=0x01;	 
		    break;   	
		default:
			break;
	}
		
	return data;
}


void WriteCommand(int command, int data)
{
		switch (command)
		{
			case VERSION_ID_0:
				break; // don't do anything
			case VERSION_ID_1:
				break; // don't do anything
			case VERSION_ID_2:
				break;  // don't do anything
			case VERSION_ID_3:
				break;		 // don't do anything								
			case SAMPLING_MODE:
				filterOption = (FilterOption)data;   // don't do anything
				break;
			case TEST_MODE:
				testMode = data;
				//now modify switches
				((testMode & (1<<PREAMP_SWITCH)) != 0) ?  PREAMP_ENABLE() : PREAMP_DISABLE();
				(testMode & (1<<LOW_SWITCH)) != 0 ?  LOW_FILTER_ENABLE() : LOW_FILTER_DISABLE();
				(testMode & (1<<HIGH_SWITCH)) != 0 ?  HIGH_FILTER_ENABLE() : HIGH_FILTER_DISABLE();
				(testMode & (1<<BAT_SWITCH)) != 0 ?  BAT_FILTER_ENABLE() : BAT_FILTER_DISABLE();
				(testMode & (1<<MIC_SWITCH)) != 0 ?  MIC_ENABLE() : MIC_DISABLE();
				(testMode & (1<<LED_SWITCH)) != 0 ?  LED_ON() : LED_OFF();
				break;
			case TOGGLE_READ: 
			    break;   	 // don't do anything
			default:
				break;  // don't do anything
		}
	
}


















