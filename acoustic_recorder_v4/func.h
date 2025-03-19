

#ifndef FUNC_H
#define FUNC_H








void fftdivide(Int32 *inbuf, Int32 *outbuf);
void fftcompress(Int32 *inbuf, Int16 *outbuf);
void ApplyWindow12bit(Int16 *inbuf, Uint16 *windowcoeff,Int32 *outbuf, Int32 blocknum);
void ApplyWindow16bit(Int16 *inbuf, Uint16 *windowcoeff,Int32 *outbuf, Int32 blocknum);
int fftmagnitude(Int32 *fftdata, Uint32 *fftmagnitudedata);
int fftsubtract(Uint32 *fftdata, Uint32 *prevfftdata, Uint32 *output);
int fftbuckets(Uint32 *fftdiff, Uint16 *output);
int fftstorebuckets(Uint32 *magnitude, Uint16 *meanbuckets, Uint16 *maxbuckets);
Uint16 fftCheckAudio(Uint16 *newbuckets, Uint16 *averagebuckets);
Uint16 fftCheckBat(Uint16 *newbuckets, Uint16 *averagebuckets, Uint16 Threshold);
//int fftaverage(Uint16 *circbuckets, Uint16 *average);
int fftaverage(Uint16 *newbuckets,Uint16 *oldbuckets,Int32 *sumbuckets, Uint16 *averagebuckets);
int fftroc(Uint16 *CircBuckets, Uint16 *AvgBuckets, Uint16 *RocBuckets);
Uint16 maxbatband(Uint16 *RocBuckets);
Uint16 maxaudioband(Uint16 *RocBuckets);
Int16 comparemax(Uint16 *MaxRocBuffer, Int32 PastTrigIndex);
Uint16 GetUltrasonicThreshold(Uint16* AvgBuckets, Uint16* MultiTable ,Uint16 InitMultiplier);
void Decimate16bit1024To256(Int16 *inBuf, Int16 *outBuf, Int32 blocknum);
void Decimate16bit512To256(Int16 *inBuf, Int16 *outBuf, Int32 blocknum);
void BlockFirFilterHigh(Int16 *inbuf, Uint16 *coeff, Uint32 *outbuf, Int32 blocknum);
void BlockFirFilterLow(Int16 *inbuf, Uint16 *coeff, Uint32 *outbuf, Int32 blocknum);


#endif

