#ifndef _COMMAND_H_
#define _COMMAND_H_


#include "ezdsp5535.h"
#include "hardware.h"




void InitCommandI2c(void);
void DisableCommandI2c(void);



void CommandMode(void);
Bool IsInCommandMode(DetectType detect);
Bool IsCommandRequest();
FilterOption GetFilterOption(void);
#endif

/*********************************EOF*******************************/

