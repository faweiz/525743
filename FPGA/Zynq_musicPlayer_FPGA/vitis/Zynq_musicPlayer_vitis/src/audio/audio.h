/*
 * audio.h
 *
 *  Created on: 2019Äê7ÔÂ18ÈÕ
 *      Author: myj
 */
#include "xil_types.h"
#include "PS_i2c.h"

struct reginfo
{
    u8 reg;
    u16 val;
};

#define SEQUENCE_INIT        0x00
#define SEQUENCE_NORMAL      0x01

#define SEQUENCE_PROPERTY    0xFFFD
#define SEQUENCE_WAIT_MS     0xFFFE
#define SEQUENCE_END	     0xFF
#define IIC_DEVICE_ID	XPAR_XIICPS_0_DEVICE_ID


/* Audio register map definitions */
#define AUDIO_REG_I2S_RESET 		 0x00   //Write only
#define AUDIO_REG_I2S_CTRL			 0x04
#define AUDIO_REG_I2S_CLK_CTRL 		 0x08
#define AUDIO_REG_I2S_PERIOD 		 0x18
#define AUDIO_REG_I2S_RX_FIFO 		 0x28   //Read only
#define AUDIO_REG_I2S_TX_FIFO 		 0x2C	//Write only

/* I2S reset mask definitions  */
#define TX_FIFO_RESET_MASK 		 	0x00000002
#define RX_FIFO_RESET_MASK 		 	0x00000004

/* I2S Control mask definitions  */
#define TX_ENABLE_MASK 		 	0x00000001
#define RX_ENABLE_MASK 		 	0x00000002

/* I2S Clock Control mask definitions  */
#define BCLK_DIV_MASK 		 	0x000000FF
#define LRCLK_DIV_MASK 		 	0x00FF0000

typedef struct {
	u32 BaseAddr ;
	u8 IicDevAddr ;
	u32 *TxBufferPtr ;
	u32 *RxBufferPtr ;
} Audio ;

/* WM8960 Register Addresses for Volume Control */
#define WM8960_LEFT_OUTPUT_VOLUME       0x02    /* Left Headphone Output */
#define WM8960_RIGHT_OUTPUT_VOLUME      0x03    /* Right Headphone Output */
#define WM8960_LEFT_SPEAKER_VOLUME      0x28    /* R40 - Left Speaker Volume */
#define WM8960_RIGHT_SPEAKER_VOLUME     0x29    /* R41 - Right Speaker Volume */

/* Volume range for WM8960 */
#define WM8960_VOLUME_MIN      48       //  -73.5dB 0x30 (mute)
#define WM8960_VOLUME_MAX      127      //   +6.0dB 0x7F
#define WM8960_VOLUME_DEFAULT  60      // ~ +5.0dB 0x79

/* Function prototypes */
int audio_init(Audio *AudioPtr, XIicPs *IicInstance);
void audio_reg_init(XIicPs *IicInstance, Audio *AudioPtr, struct reginfo *regarray);
int audio_i2c_write(XIicPs *IicInstance, char iic_addr, u8 reg_addr, u16 data);
int audio_i2c_read(XIicPs *IicInstance, char iic_addr, u8 reg_addr, u8 *read_buf);

/* TX/RX control */
void audio_txrx_enable(Audio *AudioPtr, u32 channel);
void audio_txrx_disable(Audio *AudioPtr, u32 channel);
void audio_txrx_reset(Audio *AudioPtr, u32 data, u8 flag);

/* Volume control - NEW FUNCTIONS */
int audio_volume_set(Audio *AudioPtr, XIicPs *IicInstance, int volume);
int audio_volume_get(Audio *AudioPtr, XIicPs *IicInstance);
void audio_volume_up(Audio *AudioPtr, XIicPs *IicInstance);
void audio_volume_down(Audio *AudioPtr, XIicPs *IicInstance);

/* DMA functions */
void Audio_RxStreamLengthSetting(Audio *AudioPtr, u32 length);
void audio_mem_write(Audio *AudioPtr, u32 data);
int audio_mem_read(Audio *AudioPtr);
void audio_mem_write_lr(Audio *AudioPtr, u32 *writebuf);
void audio_mem_read_lr(Audio *AudioPtr, u32 *readbuf);

/* Register access macros */
#define Audio_ReadReg(BaseAddr, RegOffset) \
    Xil_In32((BaseAddr) + (RegOffset))
#define Audio_WriteReg(BaseAddr, RegOffset, Data) \
    Xil_Out32((BaseAddr) + (RegOffset), Data)


//#endif /* AUDIO_H_ */
