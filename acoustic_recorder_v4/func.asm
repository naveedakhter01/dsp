               .mmregs
               
                .def _ApplyWindow12bit
                .def _ApplyWindow16bit
                .def _fftdivide
                .def _fftcompress
                .def _fftmagnitude
                .def _fftsubtract
                .def _fftbuckets
                .def _fftCheckAudio
                .def _fftCheckBat
                .def _fftstorebuckets
                .def _fftaverage
                .def _fftroc
                .def _comparemax
                .def _maxbatband            
                .def _maxaudioband
                .def _GetUltrasonicThreshold
                .def _Decimate16bit1024To256
                .def _Decimate16bit512To256 
                .def _Unsigned32by16Divide 
                .def _BlockFirFilterHigh
                .def _BlockFirFilterLow
     ;           .include "lpva200.inc"
       ;         .include "macros_hwa_remap.inc"
       
       
       

       

                .C54CM_off
                .CPL_on
                .ARMS_off
 

;******************************************************************************
;   Define constants
;******************************************************************************
				;block fir 
N_SAMP	.set	768		;
N_TAP_HIGH	.set	148		;		
N_TAP_LOW	.set	193		;



;OUT_SEL_DATA    .set    0       ; indicates HWA output located in input data vector
;OUT_SEL_SCRATCH .set    1       ; indicates HWA output located in scratch data vector

; Define HWAFFT instructions
;HWAFFT_INIT     .set    0x00
;HWAFFT_SINGLE   .set    0x01
;HWAFFT_DOUBLE   .set    0x02





                .text
          
SPI_WORD_OUT .macro
		;CSL_SPI_REGS->SPICMD1 = 0x0000 | 1 -1;   //Frame Length = N-1
			*port(#03004h) = #0
		;CSL_SPI_REGS->SPIDR2 = *src
			AC2 = *AR4+
			*port(#03009h) = AC2
		;CSL_SPI_REGS->SPIDR1 = 0x0000;
			;*port(#03008h) = #0   // don't need to writye the bottom half of shift register as this is not been sent out
		;CSL_FINS(CSL_SPI_REGS->SPICMD2, SPI_SPICMD2_CMD, SPI_WRITE_CMD);
			AC2 = *port(#03005h)
			AC2 = AC2 & #65532
			AC2 = AC2 | #02  ; write cmd
			*port(#03005h) = AC2
			.endm            
          
                
_ApplyWindow12bit:
                ; Inputs:
                ; XAR0 : inbuf
                ; XAR1 : windowcoeff
                ; XAR2 : outbuf
                ; AC0 : blocknum         

                pshboth(XAR0)
                pshboth(XAR1) 
                pshboth(XAR2)
                pshboth(XAR3) 
                pshboth(XAR4)
                pshboth(AC0) 
                pshboth(AC1)
                pshboth(AC2) 
                pshboth(AC3)     
                
                 
                
                AR3 = AR0  ;save pointer
                T0 = AC0   ;save blocknum
                AC0 = AC0 << #16
                AC1 = AC0 * #1024 ; AC1 = blocknum* 512
                AR0 = AC1 + AR0   ;  AR0 = pointer + offset
                AC3 = #1000h   ;XOR Mask value
                AR2 = AR2 + #-2   ; subtract 2 off dst pntr becuase we add 2 before we use it in loop
                AR4 = AR2 + #1               ;
                BRC0 = 511    ;512
                localrepeat {
                   AC1 = AC3 ^ *AR0 || AC2 = uns(*AR1+) << #16  ;  shift AC1 left 4,
                   AC1 = AC1 << #19 || AR0 = AR0 + #2
                   AC1 = AC1 << #0  || AR2 = AR2 + #2 ; (signed shift) sets the top sgoign bits 
                  AC1 = AC1 * AC2   || AR4 = AR4 + #2
                  *AR2 = HI(AC1)    || *AR4 = #0  ; save real componet(AC1),  save img (0)
                 }
                AR0 = AR3 ;restore pointer
                AC0 = T0 + 1  ; add one to block count
                AC0 = AC0 & #03h   ;mask 
                AC0 = AC0 << #16
                AC1 = AC0 * #1024;
                AR0 = AC1 + AR0 
                
                 BRC0 = 511      ;512
                localrepeat {
                   AC1 = AC3 ^ *AR0 || AC2 = uns(*AR1+) << #16  ;  shift AC1 left 4,
                   AC1 = AC1 << #19 || AR0 = AR0 + #2
                   AC1 = AC1 << #0  || AR2 = AR2 + #2 ; (signed shift) sets the top sign bits 
                  AC1 = AC1 * AC2   || AR4 = AR4 + #2
                  *AR2 = HI(AC1)    || *AR4 = #0  ; save real componet(AC1),  save img (0)
                 }  
				AC3 = popboth()
				AC2 = popboth()
				AC1 = popboth()
				AC0 = popboth()
				XAR4 = popboth()
				XAR3 = popboth()
				XAR2 = popboth()
				XAR1 = popboth()
				XAR0 = popboth()								            
                
                return



_ApplyWindow16bit:
                ; Inputs:
                ; XAR0 : inbuf
                ; XAR1 : windowcoeff
                ; XAR2 : outbuf
                ; AC0 : blocknum         
                pshboth(XAR0)
                pshboth(XAR1) 
                pshboth(XAR2)
                pshboth(XAR3) 
                pshboth(XAR4)
                pshboth(AC0) 
                pshboth(AC1)
                pshboth(AC2) 
                pshboth(AC3)  
                AR3 = AR0  ;save pointer
                T0 = AC0   ;save blocknum
                AC0 = AC0 << #16
                AC1 = AC0 * #1024 ; AC1 = blocknum* 512
                AR0 = AC1 + AR0   ;  AR0 = pointer + offset
                AC3 = #8000h << #16  ;XOR Mask value
                AR2 = AR2 + #-2   ; subtract 2 off dst pntr because we add 2 before we use it in loop
                AR4 = AR2 + #1               ;
                BRC0 = 511    ;512
                localrepeat {
                   AC1 = dbl(*AR0+)
                   AC1 = AC3 ^ AC1 || AC2 = uns(*AR1+) << #16  ;  shift AC1 left 4,
                  ; AC1 = AC1 << #4 
                   AC1 = AC1 << #0  || AR2 = AR2 + #2 ; (signed shift) sets the top sign bits 
                  AC1 = AC1 * AC2   || AR4 = AR4 + #2
                  *AR2 = HI(AC1)    || *AR4 = #0  ; save real componet(AC1),  save img (0)
                 }
                AR0 = AR3 ;restore pointer
                AC0 = T0 + 1  ; add one to block count
                AC0 = AC0 & #03h   ;mask 
                AC0 = AC0 << #16
                AC1 = AC0 * #1024;
                AR0 = AC1 + AR0 
                
                 BRC0 = 511      ;512
                localrepeat {
                   AC1 = dbl(*AR0+)
                   AC1 = AC3 ^ AC1 || AC2 = uns(*AR1+) << #16  ;  shift AC1 left 4,
                   AC1 = AC1 << #0  || AR2 = AR2 + #2; (signed shift) sets the top sign bits 
                  AC1 = AC1 * AC2   || AR4 = AR4 + #2
                  *AR2 = HI(AC1)    || *AR4 = #0  ; save real componet(AC1),  save img (0)
                 }   
                 
				AC3 = popboth()
				AC2 = popboth()
				AC1 = popboth()
				AC0 = popboth()
				XAR4 = popboth()
				XAR3 = popboth()
				XAR2 = popboth()
				XAR1 = popboth()
				XAR0 = popboth()                               
                 return



				;OLD LOOP CODE
                 ;  AC1 = dbl(*AR0+)
                 ;  AC1 = AC1 ^ (#0800h <<< #16)  ; invert what will be the sign
                 ;  AC1 = AC1 << #4
                 ;  AC1 = AC1 << #0  ; sets the top sign bits   
                 ;  AC2 = uns(*AR1+) << #16 
                 ; AC1 = AC1 * AC2
                 ; AC2 = AC1 & (#0FFFFh <<< #16)
                ;  dbl(*AR2+) = AC2
;////////////////////////

_fftdivide:
                ; Inputs:
                ; XAR0 : inbuf
                ; XAR1 : outbuf

                AC2 = #0
                AC3 = #0
                BRC0 = #127    ;1024
                BRC1 = #7
                blockrepeat{
	             AC2 = #0 || AC3 = #0  ;reset
	                localrepeat {
	                T0 = *AR0+
	                AC2 = max(T0,AC2)   ;get max value
	                T0 = *AR0+
	                AC3 = max(T0,AC3)   ;get max value
	                  ;AC2 = AC2 + *AR0+  ;real
					  ;AC3 = AC3 + *AR0+  ;img
	                 }
                 ;AC2 = AC2 <<< #-3  ;divide by 8
                 ;AC3 = AC3 <<< #-3  ;divide by 8
                 *AR1+ = AC2  || *AR1+ = AC3 
				}
                return
      
_fftcompress:
                ; Inputs:
                ; XAR0 : inbuf
                ; XAR1 : outbuf
                pshboth(XAR0)
                pshboth(XAR1)     
                pshboth(XAR2)
                AC2 = #0
                AC3 = #0
                AR2 = AR0 + #1
                BRC0 = #63    ; only need to process half the bins
                BRC1 = #7
                blockrepeat{
	             AC2 = #0 || AC3 = #0  ;reset
	                localrepeat {
	                AC0 = *AR0+ || AC3 = *AR2+ 
	                AC0 = |AC0| || AR0 = AR0 + #1 
	                AC3 = |AC3| || AR2 = AR2 + #1
	                AC1 = AC0 
                    AC0 = max(AC3,AC0)  ;get max value
	                AC1 = min(AC3,AC1)   ;get min value
	                AC0 = AC0 + (AC1 << #-1)  ;was AC0 = AC0 + (AC1 << #2)
	                AC2 = max(AC0,AC2)
	                 }
                 *AR1+ = AC2
				}
				XAR2 = popboth() 
				XAR1 = popboth()
				XAR0 = popboth() 
                return 
 

_fftmagnitude: 
                ; Inputs:
                ; XAR0 : fftdata current
                ; XAR1 : fftdata previous
                pshboth(XAR0)
                pshboth(XAR1)     
                pshboth(XAR4)
                AR4 = AR0 + #1
                BRC0 = #(512 - 1)   ; magnitude for 512 bins               
                blockrepeat{
                 AC0 = *AR0  || AC3 = *AR4
                 T0 = |AC0| || AC3 = |AC3|
                 AC1 = T0  || T0 = min(AC3,T0)                ; ||        
                 AC1 = max(AC3,AC1) || *AR0 = T0
                 AC1 = AC1 + (uns(*AR0) << #-1) || AR4 = AR4 + #2     ;was AC0 = AC0 + (AC1 << #2)
                 dbl(*AR1+) = AC1 || AR0 = AR0 + #2 
   				}   
				XAR4 = popboth() 
				XAR1 = popboth()
				XAR0 = popboth() 
               return  
   
                ; Inputs:
                ; XAR0 : fftmag current
                ; XAR1 : fftmag previous
                ; xar2  : output
_fftsubtract:   
           
                BRC0 = #(512 - 1)   ; subtract 512 bins                
                blockrepeat{
                 AC0 = dbl(*AR0+) 
                 AC0 = AC0 - dbl(*AR1+)
                 dbl(*AR2+) = AC0
   				 }   
               return  
   
                ; divides 512bins into 32 groups of 16, then averages the 16 in each group
                ;output is now 16bit 
                ; XAR0 : subtracted data
                ; XAR1 : avg of groups of 16bins
_fftbuckets:   
                T0 = #10 || AR0 = AR0 + #1
                BRC0 = #(32 - 1)    ;32 groups
                || BRC1 = #(16 - 1)     ;16 bins summed in each group
                blockrepeat{
	             AC2 = #0  ;reset
	                localrepeat {
	                 AC0 = *AR0 << #16
	                 AC0 = |AC0| || AR0 = AR0 + #2
	                 AC2 = AC2 + (AC0 * T0)
	                }
	             AC2 = AC2 << #-4   
                 *AR1+ = AC2 
				}
				
				return 
				
;                ; aver
;                ; XAR0 : start of circular buffer
;                ; XAR1 : avg of groups
;_fftaverage:  		
 ;               BRC0 = #(32 - 1)   ;32 groups
;                BRC1 = #(8 - 1)     ;8 buckets average
;                blockrepeat{
;	              ;reset
;	              AC2 = #0 || AR4 = AR0
;	                localrepeat {
;	                 AC2 = AC2 + *AR4
;	                 AR4 = AR4 + #32
;	                }
;	             AC2 = AC2 << #-3 || AR0 = AR0 + #1   
 ;                *AR1+ = AC2
;				}              
 ;              return  
               
                ; averages by adding new value added and removing last value rather than adding entire buffer 
                ; XAR0 : start of new bucket values
                ; XAR1 : start of old bucket values
                ; XAR2 : sum of bucket arrays
                ; XAR3 : avg of groups
_fftaverage:  		
                pshboth(XAR0)
                pshboth(XAR1)     
                pshboth(XAR2)
                pshboth(XAR3)
                BRC0 = #(32 - 1) ;32 groups
                blockrepeat{
	              ;reset
	              AC2 = dbl(*AR2)
	              AC2 = AC2 + *AR0+  ;add new bucket value
	              AC2 = AC2 - *AR1+  ;remove last bucket value 
                  dbl(*AR2+) = AC2

	             AC2 = AC2 << #-9    ; 2^9 = 512   
                 *AR3+ = AC2
				}
				XAR3 = popboth()
				XAR2 = popboth() 
				XAR1 = popboth()
				XAR0 = popboth() 
               return            

                ; determines if level gone beyond trigger point, compares current buckets against average buckets
                ; compares audio frequencies 
                ; XAR0 : start of current bucket values 
                ; XAR1 : start of average bucket values
_fftCheckAudio:  		
                ;check audio frequencies
                pshboth(XAR0)
                pshboth(XAR1)
                pshboth(AC1) 
                pshboth(AC2)
                pshboth(AC3);
                               
                AR0 = AR0 + #1 || T0 = #0  ; use t0 as audio flag
                BRC0 = #(5 - 1) || AR1 = AR1 + #1  ;6 groups       first 5 buckets 2.75- 16.5khz ignoring bucket 0
                blockrepeat{
	              AC3 = *AR0+ ;set current bucket value to top 16 bits
	              || AC1 = *AR1 <<#16 ;set avg bucket value to top 16 bits
	              AC2 = AC1 * #01E0h ;     fractional number xx.yy   eg 1400h = 1.400h
	              AC2 = AC2 << #-8 
	              AC3 = max(AC2,AC3)
	              if(!CARRY) goto NO_AUDIO_FLAG  ;if current higher than average 
	              T0 = #1
NO_AUDIO_FLAG:	              
                  AR1 = AR1 + #1     
				} 
				     
				AC3 = popboth()
				AC2 = popboth() 
				AC1 = popboth()    
				XAR1 = popboth()
				XAR0 = popboth()				            
               return 

                ; determines if level gone beyond trigger point, compares current buckets against average buckets
                ; compares bat frequencies 
                ; XAR0 : start of current bucket values 
                ; XAR1 : start of average bucket values
                ; T0 = threshold
                ;check bat frequencies
_fftCheckBat:            
                pshboth(XAR0)
                pshboth(XAR1) 
                pshboth(AC1) 
                pshboth(AC2)
                pshboth(AC3);
                                    
                T1 = T0 ; threshold
                AR0 = AR0 + #8 || T0 = 0 ; use t0 as flag   ; start at 22khz  (8 * 2.75khz)
                BRC0 = #(12 - 1) || AR1 = AR1 + #8  ;12 groups       first 12 buckets 22- 55khz
                blockrepeat{
	              AC3 = *AR0+ ;set current bucket value to top 16 bits
	              || AC1 = *AR1 <<#16 ;set avg bucket value to top 16 bits
	              AC2 = uns(T1 * *AR1);AC2 = AC1 * uns(T1) ;  fractional number xx.yy   eg 0140h = 01.40h (1.25d)
	              AC2 = AC2 << #-8 
	              AC3 = max(AC2,AC3)
	              if(!CARRY) goto NO_BAT_FLAG   
	              T0 = #1        ;if current higher than average
NO_BAT_FLAG:	              
                  AR1 = AR1 + 1     
				}  
				AC3 = popboth()
				AC2 = popboth() 
				AC1 = popboth()    				
				XAR1 = popboth()
				XAR0 = popboth()   
               return   
               



                ; gets average the long way by summing up all elements in buffer and dividing 
                ; done at start of sampling to 
                ; XAR0 : circ bucket array
                ; XAR1 : sum of bucket array
                ; XAR2 : avg of groups
_getlongaverage:  		
                BRC0 = #(512 - 1) ;512 previous samples
                || BRC1 = #(32 - 1)     ;32 buckets summed in each group
                blockrepeat{
	              ;reset
                  AR4 = AR1;    ;reset sum pointer
	                localrepeat {
	                 AC0 = *AR0+
	                 AC1 = AC0 + dbl(*AR4)
	                 dbl(*AR4+) =  AC1
	                }
				}
				AR4 = AR1
				BRC0 = #(32 - 1) ;32 buckets averaged
				blockrepeat{
					AC2 = dbl(*AR4+)
					AC2 = AC2 << #-9    ; 2^9 = 512 
					*AR3+ = AC2
				}

               return  

                ; divides 512bins into 32 groups of 16, then averages the 16 in each group
                ;output is now 16bit 
                ; XAR0 : magnitude data
                ; XAR1 : mean bin value buckets
                ;xar2 : max bin value bucket
_fftstorebuckets:   
                pshboth(XAR0)
                pshboth(XAR1)    
                pshboth(XAR2)    
                T0 = #10 || AR0 = AR0 + #1
                BRC0 = #(32 - 1)    ;32 groups
                || BRC1 = #(16 - 1)     ;16 bins summed in each group
                blockrepeat{
	             AC2 = #0  || AC3 = #0;reset
	                localrepeat {
	                 AC0 = *AR0 << #16
	                 AC0 = |AC0| || AR0 = AR0 + #2
	                 AC3 = max(AC0,AC3)           ;max
	                 || AC2 = AC2 + (AC0 * T0)      ;-average
	                }
	             AC2 = AC2 << #-4   ; average
	             || AC3 = AC3 * T0      ;max
                 *AR1+ = AC2   ;avg
                 || *AR2+ = AC3  ;max
				}
				XAR2 = popboth()
				XAR1 = popboth()
				XAR0 = popboth() 
               return
               
                ; fills RateOfChange buffer based on current bucket values over averages of past buffer of bucket values
                ; XAR0 : current bucket
                ; XAR1 : avg of groups
_fftroc:  		
				bit(ST1,#ST1_SXMD) = #0 ; Clear SXMD (sign extension off)
                BRC0 = #(32 - 1)    ;32 groups
                blockrepeat{
                 AC0 = *AR0 << #16
                 AR0 = AR0 + #1 ;subtract average AC0 = AC0 - (*AR1 << #16)
	             AC0 = AC0 * #10  || *AR1 = *AR1 + #0x0001 ; add 1 to divisor in case it's zero
				 repeat( #(16 - 1) ) ; Execute subc 16 times
				 subc( *AR1, AC0, AC0 ) ; AR1 points to Divisor
				 *AR2 = AC0  || AR1 = AR1 + #1 ; Store Quotient
				 
	             AR2 = AR2 + #1
				}
                bit(ST1,#ST1_SXMD) = #1 ; Set SXMD (sign extension on) 
               return                 
            
            
      
                ; subtracts a MaxRocBat value from the past with the highest MaxRocAudio value found in the entire circular buffer
                ; XAR0 : maxrocbuffer
                ; AC0 : index of MaxRocBat value in buffer used to compare against MaxRocAudio value(usually set 4 samples in past)
_comparemax:    
                AC3 = XAR0  ; points to maxrocbuffer
                AR1 = AR0 + #1  || T2 = #10 ;points to MaxRocAudio offset in buffer
                BRC0 = #(8 - 1) || AC2 =0   ;32 groups
                blockrepeat{                      ; find the maximum MaxRocAudio value from entire range of buffer 
                 AC1 = *AR1
                 AC2 = max(AC1,AC2) || AR1 = AR1 + #2
				}
				AC3 = AC3 + (AC0 << #1) || T2 = max(AC2,T2) ;AC3 points to MaxRocBat value in past, T2 = MacRocAudio value will be always => 10 (T2 initialised to 10)    
				XAR0 = AC3 || AC2 = T2  ;incase t2 is greater than 0x7fff
				AC3 = *AR0 - AC2 ; subtract MaxRocBat value with MaxRocAudio value
				T0 = AC3   ; return value
				
                return       
                   
                   
                ; gets max value in band 22khz - 56khz
                ; XAR0 : pointer
                ;T0  : output of max value
            
_maxbatband:            
                T0 = #0 || AR0 = AR0 + #8
                BRC0 = #(12 - 1)   ;                
                blockrepeat{
                 AC0 = *AR0+
                 T0 = max(AC0,T0) 
   				 }   
               return               
            
                ; gets max value in band 3khz - 17khz
                ; XAR0 : pointer
                ;T0  : output of max value
_maxaudioband:            
                AC1 = #0 || AR0 = AR0 + #1
                BRC0 = #(5 - 1)   ;                
                blockrepeat{
                 AC0 = *AR0+
                 AC1 = max(AC0,AC1) 
   				 }  
   				 AC1 = AC1 << #16 
   				 AC0 = AC1 * AC1 ;square to increase influence of audio roc
   				 T0 = AC0
   				 
               return   
               
               

; looks through average audio levels and sets a trigger threshold
; eg quiet backgroud audio noise results in lower trigger level
;    loud background noise results in high trigger level
;

                ;XAR0 = Avg buckets
                ;XAR1 = Multiply table
                ;T0 = InitMultiplier
                ;ret T0 = threshold
_GetUltrasonicThreshold:
                pshboth(XAR0)
                pshboth(XAR1)
                AR0 = AR0 + #1 || AC0 = #0333h << #16  ; skip first bucket, fractional multiply 0.333hex = .2000 dec
                BRC0 = #(5 - 1) || AC2 = #0  ;                
                blockrepeat{         ; get mean by multipling by 1/(the number of values) 
                 ;AC0 = *AR0+ << #16 
                 ;AC1 = max(AC0,AC1)
                 AC1 = *AR0+ * AC0     ; multiply  in this case 1/5
                 AC2 = AC2 + (AC1 << #4) 
   				 }  
   				 ; AC2 now is mean of audio bands
   				               ;assume highest mean will be around 8K -set multi table to cover this range - anything over will be set to fixed multiplier
   				 AC2 = AC2 <<#(-(3+16)) ; divide by 8 (rightshift3)to create index for 512 element table  (66536 -> 512),also shift right by 16 to fix the fractional calculation
	             || AC3 = #511 ;set to highest element in table 
	              AC3 = max(AC2,AC3) ; if value over highest then set to highest
	              if(CARRY) goto NO_FIX   
	              AC2 = #511        ;fix index to highest in table
NO_FIX:    
   				 AR1 = AR1 + AC2 ; pointer to multiply value
   				   
   				 AC1 = *AR1+ * T0 ;multiply the initial multiplier with the noise level multiplier
   				 AC1 = AC1 << #-8 ; fix fractional product
   				 
   				 
   				 T0 = LO(AC1)
   				 XAR1 = popboth()
   				 XAR0 = popboth()
   				  
                 return
                          
           
           
;Takes 1024 samples and converts and transfers every 10th one.            
_Decimate16bit1024To256: 
                ; Inputs:
                ; XAR0 : inbuf
                ; XAR1 : windowcoeff
                ; XAR2 : outbuf
                ; AC0 : blocknum         
                pshboth(XAR0)
                pshboth(XAR1) 
                pshboth(AC0) 
                pshboth(AC1)
                pshboth(AC3)  
                AC0 = AC0 << #16
                AC1 = AC0 * #1024 ; AC1 = blocknum* 512
                AR0 = AC1 + AR0   ;  AR0 = pointer + offset
                AC3 = #8000h << #16  ;XOR Mask value             
                BRC0 = 255    ;
                localrepeat {
                   AC1 = dbl(*AR0) 
                   AC1 = AC3 ^ AC1 || AR0 = AR0 + #8; // take every fourth word which in this case will be every
                  *AR1+ = HI(AC1)  ; save real componet(AC1),  save img (0)
             	}
				AC3 = popboth()
				AC1 = popboth()
				AC0 = popboth()
				XAR1 = popboth()
				XAR0 = popboth()                               
                 return           
           
;Takes 512 samples and converts and.            
_Decimate16bit512To256: 
                ; Inputs:
                ; XAR0 : inbuf
                ; XAR1 : windowcoeff
                ; XAR2 : outbuf
                ; AC0 : blocknum         
                pshboth(XAR0)
                pshboth(XAR1) 
                pshboth(AC0) 
                pshboth(AC1)
                pshboth(AC3)  
                AC0 = AC0 << #16
                AC1 = AC0 * #1024 ; AC1 = blocknum* 512
                AR0 = AC1 + AR0   ;  AR0 = pointer + offset
                AC3 = #8000h << #16  ;XOR Mask value             
                BRC0 = 255    ;
                localrepeat {
                   AC1 = dbl(*AR0) 
                   AC1 = AC3 ^ AC1 || AR0 = AR0 + #4 ; // take every fourth word which in this case will be every second sample
                  *AR1+ = HI(AC1)  ;
             	}
				AC3 = popboth()
				AC1 = popboth()
				AC0 = popboth()
				XAR1 = popboth()
				XAR0 = popboth()                               
                 return             
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;
;
_BlockFirFilterHigh:
    ; Inputs:
    ; XAR0 :  input buffer
    ; XAR1 :  coefficients
    ; XAR2 :  output buffer
    ; AC0 :   blocknum 
    
    
    ;setup circular buffer on AR0 and AR1 

    push(@BSA01_L) || mmap()
	push(@BSA45_L) || mmap()    
    pshboth(XAR0)

    BSA01 = *SP(0)  ; set circular buffer start address to that of incoming adc data
    pshboth(XAR1)
    pshboth(XAR2)
    BSA45 = *SP(0)  ; set circular buffer start address to that of filtered output data
    pshboth(XAR3)
    pshboth(XAR4)
    pshboth(XAR5)
    pshboth(AC0)
    pshboth(AC1)
    pshboth(AC2)
    pshboth(AC3)
    
    push(T0)
	push(T1)
    push(@BK03_L) || mmap()
	push(@BK47_L) || mmap()
	push(@BRC0_L) || mmap()
    push(@ST1_L) || mmap()
    push(@ST2_L) || mmap()	
	pshboth(XCDP)

    
    
    ;BSA01 = #1060h
    BK03 = #(N_SAMP*2*2)  ; 768 buffer * 2 (2 buffers) * 2( extra unused 16bit)
    BK47 = #(N_SAMP/2)   ; 768/2 = 386 bytes     (because we oversample by 2) 

    
    
    AC0 = AC0 << #16
    AC0 = AC0 * #(N_SAMP * 2) ; 768*2       
    XAR0 = AC0
    
	;Pointer setup
	XAR3 = XAR1
	XAR1 = #0 ;clear pointer
	XCDP = XAR3 ; #a0 ;pointer to coefficient array
	
;	XAR0 = #(x + N_TAP – 1) ;pointer to input vector
;	XAR1 = #(x + N_TAP) ;2nd pointer to input vector
;	XAR2 = #y ;pointer to output array
    ;Configure ST1: set SATD, SXMD, FRCT
    @ST1_L = @ST1_L | #0000001101000000b || mmap()  
    @ST2_L = @ST2_L | #0000000000010011b || mmap() ;set circular bits AR09 & AR1	& AR4
	
	
	pshboth(XAR0)  ;save AR0
	AC3 = #8000h  ;XOR Mask value  
	AR1 = AR0 + #2 || T0 = #4
    ;BRC0 = #((N_SAMP/2)-1)   ;768
    BRC0 = #((N_SAMP/2)-1)   ;768
    localrepeat {   
	    AC1 = *AR0 || AC2 = *AR1
	    AC1 = AC3 ^ AC1 
	    AC2 = AC3 ^ AC2    	    
	    *(AR0+T0) = AC1 || *(AR1+T0) = AC2
	}
	XAR0 = popboth()  ;restore Ar0
	
	AR1 = AR0 + #(2 * 2)  ; start the second mac chain 4 samples ahead, we only want to output 1 for every 2 input samples  
	;AR1 = AR0 + #2   ; address for the second  	
	
	;Other setup
	BRC0 = #(((N_SAMP / 2) /2) - 1);init local repeat counter  768 samples in buffer / 2(two chains) / 2( skip every 2th sample)
	;BRC1 = #((((N_TAP_LOW - 4)/2)/2)-1) ; first div by two because we perform duplicate instructions inside loop, another div by two because we have two seperate loops
	
	XAR4 =  #383   ;set the spi data pointer to last byte in circular buffer because we need to be one step (word) behind the generation of filtered data bytes
	
	T0 = #2 ; skip 2 becuase dma places 16 bit data every 32 bits 
	T1 = #((N_TAP_HIGH * 2) + ((4*2)-2) ) ;ARx rewind increment, bit of a mess because of the extra unsed byte and we skip 4(really 8) ahead
      
	||localrepeat { ;start the outer loop
		;first convert sign on incoming data
	
		;First tap is multiply only (no accumulate)
		AC0 = *(AR0-T0) * coef(*CDP+),
		AC1 = *(AR1-T0) * coef(*CDP+)
		
		nop
		||repeat( #(((N_TAP_HIGH-4)/4)-1) ) ;single repeat for inner loop
			AC0 = AC0 + ( *(AR0-T0) * coef(*CDP+) ),
			AC1 = AC1 + ( *(AR1-T0) * coef(*CDP+) )	
		
		AC0 = AC0 + ( *(AR0-T0) * coef(*CDP+) ),
		AC1 = AC1 + ( *(AR1-T0) * coef(*CDP+) )	
		nop
		;Taps 2 through (N_TAPS – 1)
		||repeat( #(((N_TAP_HIGH-4)/4)-1) ) ;single repeat for inner loop
			AC0 = AC0 + ( *(AR0-T0) * coef(*CDP+) ),
			AC1 = AC1 + ( *(AR1-T0) * coef(*CDP+) )
		; take a break from macs here to send data out spi
		SPI_WORD_OUT ; sends data from address AR4 out spi 
		nop  ;to allow dma access
		||repeat( #(((N_TAP_HIGH-4)/4)-1) ) ;single repeat for inner loop
			AC0 = AC0 + ( *(AR0-T0) * coef(*CDP+) ),
			AC1 = AC1 + ( *(AR1-T0) * coef(*CDP+) )		
			
		AC0 = AC0 + ( *(AR0-T0) * coef(*CDP+) ),
		AC1 = AC1 + ( *(AR1-T0) * coef(*CDP+) )
		nop
		||repeat( #(((N_TAP_HIGH-4)/4)-1) ) ;single repeat for inner loop
			AC0 = AC0 + ( *(AR0-T0) * coef(*CDP+) ),
			AC1 = AC1 + ( *(AR1-T0) * coef(*CDP+) )	
					
		;SPI_WORD_OUT  ; sends data from address AR4 out spi 
		;Last tap has different pointer increments
		AC0 = AC0 + ( *(AR0+T1) * coef(*CDP) ),
		AC1 = AC1 + ( *(AR1+T1) * coef(*CDP) )
		
		*AR2+ = pair(HI(AC0))  || CDP = AR3  ;write both results, restore CDP pointer  ;
		
		SPI_WORD_OUT ; sends data from address AR4 out spi 
	}
	
    ;Configure ST1: set SXMD, FRCT
    ;@ST1_L = @ST1_L & #1111111010111111b || mmap()	
	;@ST2_L = @ST2_L & #1111111111101100b || mmap() ;set linear mode bits AR09 & AR1 & AR4
	
	
	XCDP = popboth()
	@ST2_L = pop() || mmap()
	@ST1_L = pop() || mmap()	   	
	@BRC0_L = pop() || mmap()
	@BK47_L = pop() || mmap()
	@BK03_L = pop() || mmap()
	T1 = pop()
	T0 = pop()
    AC3 = popboth()
    AC2 = popboth()     
	AC1 = popboth()
	AC0 = popboth()
	XAR5 = popboth()
	XAR4 = popboth()
	XAR3 = popboth()
	XAR2 = popboth()         
	XAR1 = popboth()
	XAR0 = popboth() 
	@BSA45_L = pop() || mmap()
	@BSA01_L = pop() || mmap()    

  	return         
  	
  	
  	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;
;
_BlockFirFilterLow:
    ; Inputs:
    ; XAR0 :  input buffer
    ; XAR1 :  coefficients
    ; XAR2 :  output buffer
    ; AC0 :   blocknum 
    ;setup circular buffer on AR0 and AR1 
     
    push(@BSA01_L) || mmap()
	push(@BSA45_L) || mmap() 
	     
    pshboth(XAR0)

    BSA01 = *SP(0)  ; set circular buffer start address to that of incoming adc data
    pshboth(XAR1)
    pshboth(XAR2)
    BSA45 = *SP(0)  ; set circular buffer start address to that of filtered output data
    pshboth(XAR3)
    pshboth(XAR4)
    pshboth(AC0)
    pshboth(AC1)
    pshboth(AC2)
    pshboth(AC3)
    
    push(T0)
	push(T1)
    push(@BK03_L) || mmap()
	push(@BK47_L) || mmap()
	push(@BRC0_L) || mmap()
    push(@ST1_L) || mmap()
    push(@ST2_L) || mmap()		
	pshboth(XCDP)
    
    BK03 = #(N_SAMP*2*2)  ; 768 buffer * 2 (2 buffers) * 2( extra unused 16bit)
    BK47 = #(N_SAMP/3)   ; 768/3 = 256 bytes     (because we oversample by 3)  
      
      
    AC0 = AC0 << #16
    AC0 = AC0 * #(N_SAMP * 2) ; 768*2 ;
    XAR0 = AC0
    
	;Pointer setup
	XAR3 = XAR1
	XAR1 = #0 ;clear pointer
	XCDP = XAR3 ; #a0 ;pointer to coefficient array
	
;	XAR0 = #(x + N_TAP – 1) ;pointer to input vector
;	XAR1 = #(x + N_TAP) ;2nd pointer to input vector
;	XAR2 = #y ;pointer to output array
    ;Configure ST1: set SATD, SXMD, FRCT
    @ST1_L = @ST1_L | #0000001101000000b || mmap()  
    @ST2_L = @ST2_L | #0000000000010011b || mmap() ;set circular bits AR0 & AR1, and AR4	
	
	pshboth(XAR0)  ;save AR0
	AC3 = #8000h  ;XOR Mask value   
	AR1 = AR0 + #2 || T0 = #4
    ;BRC0 = #((N_SAMP/2)-1)   ;768
    BRC0 = #((N_SAMP/2)-1)   ;768
    localrepeat {   
	    AC1 = *AR0 || AC2 = *AR1
	    AC1 = AC3 ^ AC1 
	    AC2 = AC3 ^ AC2    	    
	    *(AR0+T0) = AC1 || *(AR1+T0) = AC2
	}
	XAR0 = popboth()  ;restore Ar0

	AR1 = AR0 + #(3 * 2)  ; start the second mac chain 3 samples ahead, we only want to output 1 for every 3 input samples  	
	
	;Other setup
	BRC0 = #(((N_SAMP / 2) /3) - 1);init local repeat counter  768 samples in buffer / 2(two chains) / 3( skip every 3rd sample)
	;BRC1 = #(((N_TAP_LOW-5)/4)-1)


    XAR4 = #255 ; set the spi data pointer to last byte in circular buffer because we need to be one step (word) behind the generation of filtered data bytes 

	T0 = #2 ; skip 2 becuase dma places 16 bit data every 32 bits 
	T1 = #((N_TAP_LOW * 2) + ((4*3)-2) ) ;ARx rewind increment, bit of a mess because of the extra unsed byte and we skip 6(really 12) ahead
   
	||localrepeat { ;start the outer loop
		;first convert sign on incoming data
	
		;First tap is multiply only (no accumulate)
		AC0 = *(AR0-T0) * coef(*CDP+),
		AC1 = *(AR1-T0) * coef(*CDP+)
		
		
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;1
		nop                 
		||repeat( #(((N_TAP_LOW-3)/5)-1) ) ;single repeat for inner loop
			AC0 = AC0 + ( *(AR0-T0) * coef(*CDP+) ),
			AC1 = AC1 + ( *(AR1-T0) * coef(*CDP+) )	
		
		nop

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;2
		nop                 
		||repeat( #(((N_TAP_LOW-3)/5)-1) ) ;single repeat for inner loop
			AC0 = AC0 + ( *(AR0-T0) * coef(*CDP+) ),
			AC1 = AC1 + ( *(AR1-T0) * coef(*CDP+) )	
		nop

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;3
		nop                 
		||repeat( #(((N_TAP_LOW-3)/5)-1) ) ;single repeat for inner loop
			AC0 = AC0 + ( *(AR0-T0) * coef(*CDP+) ),
			AC1 = AC1 + ( *(AR1-T0) * coef(*CDP+) )	
		nop
		
		AC0 = AC0 + ( *(AR0-T0) * coef(*CDP+) ),
		AC1 = AC1 + ( *(AR1-T0) * coef(*CDP+) )				
		; take a break from macs here to send data out spi
		;*AR4 = #06699h
		SPI_WORD_OUT ; sends data from address AR4 out spi 
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;4
		nop                 
		||repeat( #(((N_TAP_LOW-3)/5)-1) ) ;single repeat for inner loop
			AC0 = AC0 + ( *(AR0-T0) * coef(*CDP+) ),
			AC1 = AC1 + ( *(AR1-T0) * coef(*CDP+) )	
		nop
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;5
		nop                 
		||repeat( #(((N_TAP_LOW-3)/5)-1) ) ;single repeat for inner loop
			AC0 = AC0 + ( *(AR0-T0) * coef(*CDP+) ),
			AC1 = AC1 + ( *(AR1-T0) * coef(*CDP+) )	
		nop
					
		
		;Last tap has different pointer increments
		AC0 = AC0 + ( *(AR0+T1) * coef(*CDP) ),
		AC1 = AC1 + ( *(AR1+T1) * coef(*CDP) )
		
		*AR2+ = pair(HI(AC0))  || CDP = AR3  ;write both results, restore CDP pointer  ;
		;*AR4 = #0AA55h
		SPI_WORD_OUT ; sends data from address AR4 out spi 

	}
	
    ;Configure ST1: set SXMD, FRCT
    ;@ST1_L = @ST1_L & #1111111010111111b || mmap()	
	;@ST2_L = @ST2_L & #1111111111101100b || mmap() ;set linear mode bits AR09 & AR1 & AR4
	
	
	XCDP = popboth()
	@ST2_L = pop() || mmap()
	@ST1_L = pop() || mmap()	
	@BRC0_L = pop() || mmap()
	@BK47_L = pop() || mmap()
	@BK03_L = pop() || mmap()
	T1 = pop()
	T0 = pop()
    AC3 = popboth()
    AC2 = popboth()     
	AC1 = popboth()
	AC0 = popboth()
	XAR4 = popboth()
	XAR3 = popboth()
	XAR2 = popboth()         
	XAR1 = popboth()
	XAR0 = popboth() 
	@BSA45_L = pop() || mmap()
	@BSA01_L = pop() || mmap()   
	
  	return   	
  	
       
;***************************************************************************
; Pointer assignments: ___________
; AR0 -> Dividend Divisor ) Dividend
; AR1 -> Divisor
; AR2 -> Quotient
; AR3 -> Remainder
;
; Algorithm notes:
; - Unsigned division, 16-bit dividend, 16-bit divisor
; - Sign extension turned off. Dividend & divisor are positive numbers.
; - After division, quotient in AC0(15-0), remainder in AC0(31-16)
;***************************************************************************
_Unsigned16by16Divide: 
	bit(ST1,#ST1_SXMD) = #0 ; Clear SXMD (sign extension off)
	AC0 = *AR0 ; Put Dividend into AC0
	repeat( #(16 - 1) ) ; Execute subc 16 times
	subc( *AR1, AC0, AC0 ) ; AR1 points to Divisor
	*AR2 = AC0 ; Store Quotient
	;*AR3 = HI(AC0) ; Store Remainder        
	bit(ST1,#ST1_SXMD) = #1 ; Set SXMD (sign extension on)  
    return           
               
               
;***************************************************************************
; Pointer assignments: ___________
; AR0 - Dividend high half Divisor ) Dividend
; Dividend low half
; ...
; AR1 - Divisor
; ...
; AR2 - Quotient high half
; Quotient low half
; ...
; AR3 - Remainder
;
; Algorithm notes:
; -Unsigned division, 32-it dividend, 16-it divisor
; -Sign extension turned off. Dividend & divisor are positive numbers.
; -Before 1st division: Put high half of dividend in AC0
; -After 1st division: High half of quotient in AC0(15-)
; -Before 2nd division: Put low part of dividend in AC0
; -After 2nd division: Low half of quotient in AC0(15-) and
; Remainder in AC0(31-6)
;***************************************************************************
_Unsigned32by16Divide:   
				bit(ST1,#ST1_SXMD) = #0 ; Clear SXMD (sign extension off)
				AC0 = *AR0+ ; Put high half of Dividend in AC0
				|| repeat( #(15 - 1) ) ; Execute subc 15 times
				subc( *AR1, AC0, AC0) ; AR1 points to Divisor
				subc( *AR1, AC0, AC0) ; Execute subc final time
				|| AR4 = #8 ; Load AR4 with AC0_L memory address
				*AR2+ = AC0 ; Store high half of Quotient
				*AR4 = *AR0+ ; Put low half of Dividend in AC0_L
				repeat( #(16 - 1) ) ; Execute subc 16 times
				subc( *AR1, AC0, AC0)
				*AR2+ = AC0 ; Store low half of Quotient
				;*AR3 = HI(AC0) ; Store Remainder
				bit(ST1,#ST1_SXMD) = #1 ; Set SXMD (sign extension on)  
                return
   
   
   
 
                
     .end                
                