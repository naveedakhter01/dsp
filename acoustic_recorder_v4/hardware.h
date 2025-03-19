

#ifndef _HARDWARE_H_
#define _HARDWARE_H_

//////////////////////////////////////////////////////////////////////////////
// * File name: hardware.h
//////////////////////////////////////////////////////////////////////////////
#include "ezdsp5535.h"
#include "ezdsp5535_gpio.h"
#include "ezdsp5535_led.h"



typedef enum {DISABLE = 0, ENABLE = !DISABLE} FunctionalState;


#define PREAMP_GPIO_PIN 			2
#define PREAMP_GPIO_DIR 			GPIO_OUT

#define LOW_FILTER_GPIO_PIN 		4
#define LOW_FILTER_GPIO_DIR 		GPIO_OUT

#define HIGH_FILTER_GPIO_PIN 		5
#define HIGH_FILTER_GPIO_DIR 		GPIO_OUT

#define BAT_FILTER_GPIO_PIN 		10
#define BAT_FILTER_GPIO_DIR 		GPIO_OUT

#define MIC_GPIO_PIN 				3
#define MIC_GPIO_DIR 				GPIO_OUT


//arm to dsp connections
#define COMMAND_MODE_GPIO_PIN 		14 // is used by arm to indicate that the DSP should be in command mode
#define COMMAND_MODE_GPIO_DIR 		GPIO_IN

#define COMMAND_REQUEST_GPIO_PIN 	16  //is used by arm to indicate to dsp that a command is available via spi
#define COMMAND_REQUEST_GPIO_DIR 	GPIO_IN

#define DSP_EVENT_GPIO_PIN 			13 // is uaed by dsp to tell arm that an event has happened eg. bat data ready
#define DSP_EVENT_GPIO_DIR 			GPIO_OUT

#define DSP_RESERVED_GPIO_PIN 		17
#define DSP_RESERVED_GPIO_DIR 		GPIO_IN





#define DSP_EVENT_ENABLE()  EZDSP5535_GPIO_setOutput( DSP_EVENT_GPIO_PIN,1)
#define DSP_EVENT_DISABLE()  EZDSP5535_GPIO_setOutput( DSP_EVENT_GPIO_PIN, 0)

#define COMMAND_MODE_INPUT_STATE() EZDSP5535_GPIO_getInput(COMMAND_MODE_GPIO_PIN)
#define COMMAND_REQUEST_INPUT_STATE() EZDSP5535_GPIO_getInput(COMMAND_REQUEST_GPIO_PIN)
#define COMMAND_RESERVED_INPUT_STATE() EZDSP5535_GPIO_getInput(COMMAND_RESERVED_GPIO_PIN)



#define PREAMP_ENABLE()  EZDSP5535_GPIO_setOutput( PREAMP_GPIO_PIN, 0)
#define PREAMP_DISABLE()  EZDSP5535_GPIO_setOutput( PREAMP_GPIO_PIN, 1)

#define LOW_FILTER_ENABLE()  EZDSP5535_GPIO_setOutput( LOW_FILTER_GPIO_PIN, 0)
#define LOW_FILTER_DISABLE()  EZDSP5535_GPIO_setOutput( LOW_FILTER_GPIO_PIN, 1)

#define HIGH_FILTER_ENABLE()  EZDSP5535_GPIO_setOutput( HIGH_FILTER_GPIO_PIN, 0)
#define HIGH_FILTER_DISABLE()  EZDSP5535_GPIO_setOutput( HIGH_FILTER_GPIO_PIN, 1)

#define BAT_FILTER_ENABLE()  EZDSP5535_GPIO_setOutput( BAT_FILTER_GPIO_PIN, 1)
#define BAT_FILTER_DISABLE()  EZDSP5535_GPIO_setOutput( BAT_FILTER_GPIO_PIN, 0)
 
#define MIC_ENABLE()  EZDSP5535_GPIO_setOutput( MIC_GPIO_PIN, 1)
#define MIC_DISABLE()  EZDSP5535_GPIO_setOutput( MIC_GPIO_PIN, 0)


#define LED_ON() EZDSP5535_XF_on( )
#define LED_OFF() EZDSP5535_XF_off( )

//filter mode options
typedef enum{
	RESERVED = 0,
	LOW_FILTER,
	HIGH_FILTER,
	BAT_FILTER,
	NO_FILTER		
} FilterOption;

typedef enum
{
  ADC_12BIT = 0,	 
  ADC_16BIT
}ADC_Type;


typedef enum
{
	LEVEL,
	EDGE
} DetectType;


/* ------------------------------------------------------------------------ *
 *  Prototypes                                                              *
 * ------------------------------------------------------------------------ */
void GPIO_init( );
void ADC_Select(ADC_Type adc_select);
void SetI2S1Pins();
void SetUnusedPins( );

Int16 WaitForArm( );
void InitGPIO(void);
void FilterSelect(FilterOption filterOption);
void MicBiasSupply(FunctionalState state);

int ReadCommandModeInput();
int ReadCommandRequestInput();


#endif









