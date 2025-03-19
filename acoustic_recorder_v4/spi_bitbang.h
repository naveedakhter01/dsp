#ifndef _SPI_BITBANG_H_
#define _SPI_BITBANG_H_

#include "ezdsp5535.h"
#include "tistdtypes.h"

void SPI_Bitbang_Init(void);
void SPI_BitBang_PollClock(void);
Bool SPI_Bitbang_IsSendBufferFull(void);
void SPI_BitbangSendWord(Uint16 data);
Bool SPI_Bitbang_IsRecieveBufferFull(void);
Uint16 SPI_BitbangReceivedWord(void);

#endif

