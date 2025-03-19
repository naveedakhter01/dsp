aa;*******************************************************************************
;*
;*       File name    : hwafft.asm
;*       Description  : Contains FFT/IFFT routines for HWA FFT.
;*       Author       : Frank Livingston
;*       Date         : 2/1/2008
;*
;*******************************************************************************
;*       Changelog:
;*			2012-08-14: Replaced 16-bit AR register copies with 23-bit XAR register copies to allow allocation of data & scratch buffers anywhere in memory
;*******************************************************************************

                .mmregs
                
                .def _hwafft_br
                ;.def _hwafft_8pts
                ;.def _hwafft_16pts
                ;.def _hwafft_32pts
                ;.def _hwafft_64pts
                ;.def _hwafft_128pts
                ;.def _hwafft_256pts
                ;.def _hwafft_512pts
                .def _hwafft_1024pts

                .include "lpva200.inc"
                .include "macros_hwa_remap.inc"

                .C54CM_off
                .CPL_on
                .ARMS_off
 

;******************************************************************************
;   Define constants
;******************************************************************************
OUT_SEL_DATA    .set    0       ; indicates HWA output located in input data vector
OUT_SEL_SCRATCH .set    1       ; indicates HWA output located in scratch data vector

; Define HWAFFT instructions
HWAFFT_INIT     .set    0x00
HWAFFT_SINGLE   .set    0x01
HWAFFT_DOUBLE   .set    0x02
HWAFFT_FRC_SC   .set    0x05
HWAFFT_UPD_SC   .set    0x06
HWAFFT_DIS_SC   .set    0x07
HWAFFT_START    .set    0x09
HWAFFT_COMPUTE  .set    0x10

; Define HWAFFT data vector lengths
DATA_LEN_8      .set    8
DATA_LEN_16     .set    16
DATA_LEN_32     .set    32
DATA_LEN_64     .set    64
DATA_LEN_128    .set    128
DATA_LEN_256    .set    256
DATA_LEN_512    .set    512
DATA_LEN_1024   .set    1024


                .text

;******************************************************************************
; Bit-reverses data vector
;******************************************************************************
_hwafft_br:
                ; XAR0 : input vector address
                ; XAR1 : output vector address
                ; T0 : data_len -- size of input/output vectors
				pshboth(XAR0)
				pshboth(XAR1)
                bit(ST2, #15) = #0           ; clear ARMS

                T1 = T0 - #1
                BRC0 = T1
                localrepeat {
                    AC3 = dbl(*AR0+)                   
                    dbl(*(AR1 + T0B)) = AC3
                }

                bit(ST2, #15) = #1           ; set ARMS
                XAR1 = popboth()  
                XAR0 = popboth()
                return
 
;******************************************************************************
; Computes 1024-point FFT/IFFT
;******************************************************************************
_hwafft_1024pts:
                ; Inputs:
                ; XAR0 : bit-reversed input vector address
                ; XAR1 : scratch vector address
                ; T0 : FFT/IFFT flag
                ; T1 : SCALE/NOSCALE flag               
                ; Outputs:
                ; T0 : OUT_SEL flag

			;	pshboth(XAR0)
			;	pshboth(XAR1)
			;	pshboth(XAR2)
			;	pshboth(XAR3)
			;	pshboth(XAR4)
                pshboth(XAR5)
			;	pshboth(AC0)
			;	pshboth(AC1)
			;	pshboth(AC2)
            ;    pshboth(AC3)



                _Hwa_remap_hwa0                     ; enable HWA #0 (FFT coproc.)

                ; Initialize HWA FFT
                AC1 = T0
                AC1 = AC1 <<< #1
                AC1 |= T1
                AC1 = AC1 <<< #16
                AC1 += #(DATA_LEN_1024-1)           ; N-1       ; set FFT N=1024
                AC1 = copr(#HWAFFT_INIT, AC0, AC1)              ; init 1024-pts FFT

                T0 = #(((DATA_LEN_1024*3/4)-1)*2)   ;=1534      ; (N*3/4)-1 * 2 bytes => 767 * 2
                T1 = #((DATA_LEN_1024/4)*2)         ;=512       ; N/4 * 2 bytes => 256 * 2

                ; Save pointers to data buffers
                AC2 = XAR0
                AC2 += #((DATA_LEN_1024-1)*2)
                XAR4 = AC2
                AC2 = XAR1
                AC2 += #((DATA_LEN_1024-1)*2)
                XAR5 = AC2

                ; Start 1st double stage
                XAR0 = XAR4
                XAR1 = XAR5

                AC1 = copr(#HWAFFT_START, AC0, dbl(*AR0-))                    
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0-))                    

                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0-))
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0-))
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0-))
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0-))
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0-))
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0-))
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0-))
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0-))
             
                BRC0 = #((DATA_LEN_1024-16)/4)      ; =252
                localrepeat {
                    AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0-)), dbl(*(AR1-T1))=AC1  ; store 1st output, 1st double stage
                    AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0-)), dbl(*(AR1-T1))=AC1
                    AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0-)), dbl(*(AR1-T1))=AC1
                    AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0-)), dbl(*(AR1+T0))=AC1
                }

                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0-)), dbl(*(AR1-T1))=AC1
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0)), dbl(*(AR1-T1))=AC1

                ; Start second double stage
                XAR2 = XAR5
                XAR3 = XAR4
 
                AC1 = copr(#HWAFFT_START, AC0, dbl(*AR2-)), dbl(*(AR1-T1))=AC1
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR2-)), dbl(*(AR1+T0))=AC1

                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR2-)), dbl(*(AR1-T1))=AC1
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR2-)), dbl(*(AR1-T1))=AC1
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR2-)), dbl(*(AR1-T1))=AC1
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR2-)), dbl(*(AR1+T0))=AC1
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR2-)), dbl(*(AR1-T1))=AC1
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR2-)), dbl(*(AR1-T1))=AC1
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR2-)), dbl(*(AR1-T1))=AC1
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR2-)), dbl(*(AR1+T0))=AC1      ; store last output, 1st double stage

                BRC0 = #((DATA_LEN_1024-16)/4)      ; =252
                localrepeat {
                    AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR2-)), dbl(*(AR3-T1))=AC1  ; store 1st output, 2nd double stage
                    AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR2-)), dbl(*(AR3-T1))=AC1
                    AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR2-)), dbl(*(AR3-T1))=AC1
                    AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR2-)), dbl(*(AR3+T0))=AC1
                }

                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR2-)), dbl(*(AR3-T1))=AC1
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR2)), dbl(*(AR3-T1))=AC1

                ; Start third double stage
                XAR0 = XAR4
                XAR1 = XAR5
                
                AC1 = copr(#HWAFFT_START, AC0, dbl(*AR0-)), dbl(*(AR3-T1))=AC1
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0-)), dbl(*(AR3+T0))=AC1

                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0-)), dbl(*(AR3-T1))=AC1
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0-)), dbl(*(AR3-T1))=AC1
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0-)), dbl(*(AR3-T1))=AC1
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0-)), dbl(*(AR3+T0))=AC1
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0-)), dbl(*(AR3-T1))=AC1
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0-)), dbl(*(AR3-T1))=AC1
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0-)), dbl(*(AR3-T1))=AC1
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0-)), dbl(*(AR3+T0))=AC1      ; store last output, 2nd double stage     

                BRC0 = #((DATA_LEN_1024-16)/4)      ; =252
                localrepeat {
                    AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0-)), dbl(*(AR1-T1))=AC1  ; store 1st output, 3rd double stage
                    AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0-)), dbl(*(AR1-T1))=AC1
                    AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0-)), dbl(*(AR1-T1))=AC1
                    AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0-)), dbl(*(AR1+T0))=AC1
                }

                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0-)), dbl(*(AR1-T1))=AC1
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0)), dbl(*(AR1-T1))=AC1

                ; Start fourth double stage
                XAR2 = XAR5
                XAR3 = XAR4

                AC1 = copr(#HWAFFT_START, AC0, dbl(*AR2-)), dbl(*(AR1-T1))=AC1
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR2-)), dbl(*(AR1+T0))=AC1

                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR2-)), dbl(*(AR1-T1))=AC1
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR2-)), dbl(*(AR1-T1))=AC1
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR2-)), dbl(*(AR1-T1))=AC1
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR2-)), dbl(*(AR1+T0))=AC1
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR2-)), dbl(*(AR1-T1))=AC1
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR2-)), dbl(*(AR1-T1))=AC1
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR2-)), dbl(*(AR1-T1))=AC1
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR2-)), dbl(*(AR1+T0))=AC1      ; store last output, 3rd double stage
             
                BRC0 = #((DATA_LEN_1024-16)/4)      ; =252
                localrepeat {
                    AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR2-)), dbl(*(AR3-T1))=AC1  ; store 1st output, 4th double stage
                    AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR2-)), dbl(*(AR3-T1))=AC1
                    AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR2-)), dbl(*(AR3-T1))=AC1
                    AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR2-)), dbl(*(AR3+T0))=AC1
                }

                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR2-)), dbl(*(AR3-T1))=AC1
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR2)), dbl(*(AR3-T1))=AC1

                ; Start fifth double (last) stage
                XAR0 = XAR4
                XAR1 = XAR5

                AC1 = copr(#HWAFFT_START, AC0, dbl(*AR0-)), dbl(*(AR3-T1))=AC1
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0-)), dbl(*(AR3+T0))=AC1

                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0-)), dbl(*(AR3-T1))=AC1
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0-)), dbl(*(AR3-T1))=AC1
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0-)), dbl(*(AR3-T1))=AC1
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0-)), dbl(*(AR3+T0))=AC1
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0-)), dbl(*(AR3-T1))=AC1
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0-)), dbl(*(AR3-T1))=AC1
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0-)), dbl(*(AR3-T1))=AC1
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0-)), dbl(*(AR3+T0))=AC1      ; store last output, 4th double stage

                BRC0 = #((DATA_LEN_1024-16)/4)      ; =252
                localrepeat {
                    AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0-)), dbl(*(AR1-T1))=AC1  ; store 1st output, 5th double stage
                    AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0-)), dbl(*(AR1-T1))=AC1
                    AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0-)), dbl(*(AR1-T1))=AC1
                    AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0-)), dbl(*(AR1+T0))=AC1
                }

                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0-)) ,dbl(*(AR1-T1))=AC1
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0)), dbl(*(AR1-T1))=AC1

                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0)), dbl(*(AR1-T1))=AC1
                AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0)), dbl(*(AR1+T0))=AC1

                BRC0 = #1
                localrepeat {
                    AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0)), dbl(*(AR1-T1))=AC1
                    AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0)), dbl(*(AR1-T1))=AC1
                    AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0)), dbl(*(AR1-T1))=AC1
                    AC1 = copr(#HWAFFT_COMPUTE, AC0, dbl(*AR0)), dbl(*(AR1+T0))=AC1
                }
                
                T0 = #(OUT_SEL_SCRATCH)
                
            ;    AC3 = popboth()
            ;    AC2 = popboth()
            ;    AC1 = popboth()
            ;    AC0 = popboth()
                XAR5 = popboth()
            ;    XAR4 = popboth()
              ;  XAR3 = popboth()
              ;  XAR2 = popboth()
              ;  XAR1 = popboth()  
              ;  XAR0 = popboth()   
                return


                .end
