
/* implementation of the NXP_I2C API on Linux */

#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

#include <NXP_I2C.h>
#include <lxScribo.h>
#include <Scribo.h>
#include "tfa9890_cust.h"
//#include "Tfa98xx.h"	// for device driver interface special file


int gI2cBufSz=NXP_I2C_MAX_SIZE;
/*
 * accounting globals
 */
int gNXP_i2c_writes=0, gNXP_i2c_reads=0;

#ifndef __BOOL_DEFINED
  typedef unsigned char bool;
  #define true ((bool)(1==1))
  #define false ((bool)(1==0))
#endif

static NXP_I2C_Error_t translate_error(uint32_t error);

typedef struct
{
  char msg[32];
} NXP_I2C_ErrorStr_t;

NXP_I2C_Error_t i2cExecuteRS(int NrOfWriteBytes,
		const unsigned char * WriteData, int NrOfReadBytes, unsigned char * ReadData);

/* set to true to show the actually I2C message in stdout
 * set to false otherwise
 */
static int bDebug = false;
/* for debug logging, keep track of number of I2C messages and when sent */
static long cnt = 0;
static long start;
static char debug_buffer[5000];

static int bInit = false;

static int i2cTargetFd=-1;						 /* file descriptor for target device */

#if 1
int NXP_I2C_verbose;
#else
extern int NXP_I2C_verbose;
#endif

//#define VERBOSE(format, args...) if (NXP_I2C_verbose) printf(format, ##args)
#define VERBOSE if (NXP_I2C_verbose)

static NXP_I2C_Error_t init_I2C(void)
 {
#ifdef TFA_CHECK_REV
 unsigned char wbuf[2], rbuf[3];
#endif 
//	VERBOSE printf("%s is disabled, to be removed\n", __FUNCTION__);
	VERBOSE printf("%s\n", __FUNCTION__);
#if 1 // disabled, the registry will have done init
	/*	TODO properly define open device configs */
#ifdef TFA_I2CDEVICE
	if (i2cTargetFd < 0 ) {
		i2cTargetFd = lxScriboRegister(TFA_I2CDEVICE);
	}
#else
	if (i2cTargetFd < 0 ) {
	//	i2cTargetFd = lxScriboRegister("/dev/ttyScribo");
		i2cTargetFd = lxScriboGetFd();
	}
#endif
	if (i2cTargetFd < 0 ){
		fprintf( stderr, "!i2c device was not opened\n");
		return NXP_I2C_NoInterfaceFound;
	}

#ifdef TFA_CHECK_REV
        // check the contents of the ID regsiter
        wbuf[0] = TFA_I2CSLAVE<<1;
        wbuf[1] = 0x03; // ID register
        rbuf[0] = (TFA_I2CSLAVE<<1)+1; // read
        i2cExecuteRS( sizeof(wbuf), wbuf, sizeof(rbuf), rbuf);
        if ( rbuf[1]!=(TFA_CHECK_REV>>8) || rbuf[2] != (TFA_CHECK_REV&0xff) ) {
                fprintf( stderr, "!wrong ID expected:0x%04x received:0x%04x, register 3 of slave 0x%02x\n", 
                             TFA_CHECK_REV ,  rbuf[1]<<8 | rbuf[2], TFA_I2CSLAVE);
              return NXP_I2C_NoInterfaceFound;
              }
#endif
#endif // disable
	bInit = true;

	return NXP_I2C_Ok;
}

static const NXP_I2C_ErrorStr_t errorStr[NXP_I2C_ErrorMaxValue] =
{
  { "UnassignedErrorCode" },
  { "Ok" },
  { "NoAck" },
  { "SclStuckAtOne" },
  { "SdaStuckAtOne" },
  { "SclStuckAtZero" },
  { "SdaStuckAtZero" },
  { "TimeOut" },
  { "ArbLost" },
  { "NoInit" },
  { "Disabled" },
  { "UnsupportedValue" },
  { "UnsupportedType" },
  { "NoInterfaceFound" },
  { "NoPortnumber" }
};

#define DUMMIES
#ifdef DUMMIES
/*
 * dummies TODO fix dummies
 */
int GetTickCount(void)
{
	return 0;
}

void  i2cDebugStr(char * GetString)
{
	GetString = "TBD" ;
}
void  i2cDebug(int err)
{
	printf("%s %d\n", __FUNCTION__, err);
}
static void hexdump(int num_write_bytes, const unsigned char * data)
{
	int i;

	for(i=0;i<num_write_bytes;i++)
	{
		printf("0x%02x ", data[i]);
	}

}
NXP_I2C_Error_t i2cExecute(int num_write_bytes, unsigned char * data)
{
	uint32_t error;

	VERBOSE printf("%s   W %3d: ",__FUNCTION__, num_write_bytes);
	VERBOSE hexdump(num_write_bytes, data);
	VERBOSE printf("\n");

	if ( 0 == (*lxWrite)(i2cTargetFd, num_write_bytes, data, &error) ) {
//		VERBOSE printf(" NoAck\n");
		return translate_error(error);
	}
	return translate_error(error);
}

NXP_I2C_Error_t i2cExecuteRS(int NrOfWriteBytes,
		const unsigned char * WriteData, int NrOfReadBytes, unsigned char * ReadData)
{
	uint32_t error;

	VERBOSE printf("%s W %d bytes:\t",__FUNCTION__, NrOfWriteBytes);
	VERBOSE hexdump(NrOfWriteBytes, WriteData);
	VERBOSE printf("\n");

	NrOfReadBytes = (*lxWriteRead)(i2cTargetFd,  NrOfWriteBytes, WriteData ,
			NrOfReadBytes, ReadData, &error);
	if (NrOfReadBytes ) {
		VERBOSE printf("%s R %3d: ",__FUNCTION__, NrOfReadBytes);
		VERBOSE hexdump(NrOfReadBytes, ReadData);
		VERBOSE printf("\n");

	}
	else {
		return translate_error(error);
	}
	gNXP_i2c_writes += NrOfWriteBytes;
	gNXP_i2c_reads  += NrOfReadBytes;
	return translate_error(error);
}
#endif

#if 0
i2cErrorTypes i2cError(void)
{
	return i2cNoError;
}
#endif
static NXP_I2C_Error_t translate_error(uint32_t error)
{
  NXP_I2C_Error_t retval;

//  error=eNone;//TODO handle errors properly at lower i2c levels

  switch (error)
  {
    case eNone: retval=NXP_I2C_Ok;
      break;
    case eI2C_SLA_NACK: retval=NXP_I2C_NoAck;
      break;
#if 0
    case i2cSclStuckAtOne: retval=NXP_I2C_SclStuckAtOne;
      break;
    case i2cSdaStuckAtOne: retval=NXP_I2C_SdaStuckAtOne;
      break;
    case i2cSclStuckAtZero: retval=NXP_I2C_SclStuckAtZero;
      break;
    case i2cSdaStuckAtZero: retval=NXP_I2C_SdaStuckAtZero;
      break;
    case i2cTimeOut: retval=NXP_I2C_TimeOut;
      break;
    case i2cArbLost: retval=NXP_I2C_ArbLost;
      break;
    case i2cNoInit: retval=NXP_I2C_NoInit;
      break;
    case i2cDisabled: retval=NXP_I2C_Disabled;
      break;
    case i2cUnsupportedValue: retval=NXP_I2C_UnsupportedValue;
      break;
    case i2cUnsupportedType: retval=NXP_I2C_UnsupportedType;
      break;
    case i2cNoInterfaceFound: retval=NXP_I2C_NoInterfaceFound;
      break;
    case i2cNoPortnumber: retval=NXP_I2C_NoPortnumber;
      break;
#endif
    default:
      /* unexpected error */
	  printf("Got unexpected error 0x%08x\n", error);
      assert(0);
		  retval=NXP_I2C_UnassignedErrorCode;
  }

  if (retval != NXP_I2C_Ok)
	{
    printf("I2C error %d (%s)\n", error, (char*)&errorStr[retval].msg);
	}
  return retval;
}

static NXP_I2C_Error_t init_if_firsttime(void)
{
  NXP_I2C_Error_t retval = NXP_I2C_Ok;

  if (!bInit)
  {
    retval = init_I2C();
  }
  return retval;
}

static void debugInfo(void)
{
	cnt++;
	if (bDebug)
	{
		long t = GetTickCount();
		debug_buffer[0] = '\0';
		i2cDebugStr(debug_buffer);
		printf("%d (%d) %s\n", (int)cnt, (int)(t-start), (char*)debug_buffer);
		fflush(stdout);
	}
}

NXP_I2C_Error_t NXP_I2C_Write(  unsigned char sla,
								int num_write_bytes,
                                const unsigned char data[] )
{
  NXP_I2C_Error_t retval;

	if (num_write_bytes > gI2cBufSz)
	{
		fprintf(stderr, "%s: too many bytes: %d\n", __FUNCTION__, num_write_bytes);
		return NXP_I2C_UnsupportedValue;
	}

	retval = init_if_firsttime();
	if (NXP_I2C_Ok == retval) {
		unsigned char buffer[NXP_I2C_MAX_SIZE+1];	  
		buffer[0] = sla;
	    memcpy((void*)&buffer[1], (void*)data, num_write_bytes); // prepend slave address
		
#ifdef NXP_I2C_MAX_BURST
 TODO fix buffer
		unsigned char slave, chunkbuf[NXP_I2C_MAX_BURST];
		int count, tail, chunks, maxdata = NXP_I2C_MAX_BURST - 1;
		int chunk0_payload, chunk_payload, tail_payload, payload, round;
		if (num_write_bytes > NXP_I2C_MAX_BURST) {
			chunkbuf[0] = data[0]; // remember the slave address
			chunkbuf[1] = data[1]; // 0x71 /*TFA98XX_CF_MAD*/ or 0x72; /*TFA98XX_CF_MEM*/
			payload = num_write_bytes - 4;
			chunk_payload = NXP_I2C_MAX_BURST - 1 - NXP_I2C_MAX_BURST % 3;// middle chunks only have slave+reg
			if (data[1] == 0x71 /*TFA98XX_CF_MAD*/) {
				// inputdata: slave,0x71,memadd[15:8],memadd[7:0], xmem data[3],...
				chunk0_payload = NXP_I2C_MAX_BURST - 4
						- (NXP_I2C_MAX_BURST - 4) % 3; //max data bytes for the 1st chunk
				round = (chunk0_payload / 3);// used for 3bytes word alignment
				// chunk0_payload -= chunk0_payload - chunk0_payload%3; //remove the last word of too long
				chunks = (payload - chunk0_payload) / chunk_payload; //nr of chunks, rest is in tail
				tail_payload =
						chunks > 1 ?
								payload - chunk0_payload
										- chunks * chunk_payload :
								0;
				printf("payload71=%d of %d \n", payload, num_write_bytes);
				printf(
						" chunks=%d, chunk0_payload=%d, chunk_payload=%d, tail_payload=%d\n",
						chunks, chunk0_payload, chunk_payload, tail_payload);
				memcpy((void*) &chunkbuf[2], (void*) &data[2],
						NXP_I2C_MAX_BURST - 2);
				i2cExecute(NXP_I2C_MAX_BURST, chunkbuf);
				chunkbuf[1] = 0x72;
				for (count = 1; count < chunks; count++) {
					memcpy((void*) &chunkbuf[2],
							(void*) &data[1 + count * maxdata], maxdata);
					i2cExecute(NXP_I2C_MAX_BURST, chunkbuf);
				}
				if (tail_payload) {
					memcpy((void*) &chunkbuf[2],
							(void*) &data[1 + chunks * maxdata], tail_payload);
					i2cExecute(tail_payload + 1, chunkbuf);
				}
			} else if (data[1] == 0x72 /*TFA98XX_CF_MEM*/) {
				// inputdata: slave, 0x72, xmem data[3],...
				chunks = payload / chunk_payload; //nr of chunks, rest is in tail
				tail_payload = chunks ? payload - chunks * chunk_payload : 0;
				printf(" chunks=%d, chunk_payload=%d, tail_payload=%d\n",
						chunks, chunk_payload, tail_payload);
				printf("payload72=%d of %d\n", num_write_bytes - 2,
						num_write_bytes);
				chunkbuf[1] = 0x72;
				for (count = 0; count < chunks; count++) {
					memcpy((void*) &chunkbuf[2],
							(void*) &data[1 + count * maxdata], maxdata);
					i2cExecute(NXP_I2C_MAX_BURST, chunkbuf);
				}
				if (tail_payload) {
					memcpy((void*) &chunkbuf[2],
							(void*) &data[1 + chunks * maxdata], tail_payload);
					i2cExecute(tail_payload + 1, chunkbuf);
				}

			}
		} else
#endif // NXP_I2C_MAX_BURST
		retval = i2cExecute(num_write_bytes+1, buffer); //+sla

		debugInfo();
	}
	return retval;
}

NXP_I2C_Error_t NXP_I2C_WriteRead(  unsigned char sla,
									int num_write_bytes,
                                    const unsigned char write_data[],
                                    int num_read_bytes,
                                    unsigned char read_data[] )
{
  NXP_I2C_Error_t retval;

	if (num_write_bytes > gI2cBufSz)
	{
		fprintf(stderr, "%s: too many bytes to write: %d\n", __FUNCTION__, num_write_bytes);
		return NXP_I2C_UnsupportedValue;
	}
	if (num_read_bytes > gI2cBufSz)
	{
		fprintf(stderr, "%s: too many bytes to read: %d\n", __FUNCTION__, num_read_bytes);
		return NXP_I2C_UnsupportedValue;
	}

  retval = init_if_firsttime();
  if (NXP_I2C_Ok==retval)
  {
	  unsigned char wbuffer[NXP_I2C_MAX_SIZE+1], rbuffer[NXP_I2C_MAX_SIZE+1];

	  wbuffer[0] = sla;
	  memcpy((void*)&wbuffer[1], (void*)write_data, num_write_bytes); // prepend slave address

	  rbuffer[0] = sla|1; //read slave
	  retval = i2cExecuteRS(num_write_bytes+1, wbuffer, num_read_bytes+1, rbuffer);//+sla
	  memcpy((void*)read_data, (void*)&rbuffer[1], num_read_bytes); // remove slave address
      debugInfo();
  }
  return retval;
}

NXP_I2C_Error_t NXP_I2C_EnableLogging(int bEnable)
{
  NXP_I2C_Error_t retval;

  retval = init_if_firsttime();
  if (NXP_I2C_Ok==retval)
  {
	  bDebug = bEnable;
	  i2cDebug(bDebug);
  }
	return retval;
}
int NXP_I2C_BufferSize()
{
	NXP_I2C_Error_t error;
	error = init_if_firsttime();
	if (error == NXP_I2C_Ok) {
		return gI2cBufSz > 0 ? gI2cBufSz : NXP_I2C_MAX_SIZE;
	}
	return 254; //254 is minimum
}

