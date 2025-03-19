

#ifndef _SPI_H_
#define _SPI_H_

/* ------------------------------------------------------------------------ *
 *  Prototype                                                               *
 * ------------------------------------------------------------------------ */
Int16  EZDSP5535_SPI_init( Uint16 clk);
Int16  EZDSP5535_SPI_read(Uint16 *src, Uint32 len );
Int16  EZDSP5535_SPI_write(Uint16 *src, Uint32 len );

void SPI_write16_once (Uint16 *src);
#endif
