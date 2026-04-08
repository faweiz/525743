/*
 * audio.c
 *
 *  Created on: 2019Äê7ÔÂ18ÈÕ
 *      Author: myj
 */
#include "xiicps.h"
#include "audio.h"

#define WM8960_DEVADDR  0x1A

/* Global variable to store current volume */
static int current_volume = WM8960_VOLUME_DEFAULT;

struct reginfo wm8731_init_data[] =
{
		{15, 0x0000},          //reset
		{25, 0x00FC},          //(Left Line In) = 0x97: left line in mute
		{26, 0x01F9},          //(Right Line In) = 0x97: right line in mute
		{47, 0x000C},          //(Left Headphone out) = 0x7f :left headphone +6dB
		{34, 0x0100},          //(right Headphone out) = 0x7f :right headphone +6dB
		{37, 0x0100},          //(analogue audio path control) = 0x15 : MIC select to DAC
		{ 5, 0x0000},          //(digital Audio path control) = 0x07  high pass filter
		{ 2, 0x0179},          //(Power down control) = 0x00
		{ 3, 0x0179},          //(Digital Audio interface format) = 0x02 : i2s slave mode;
		{ 2, 0x0100},          //(Power down control) = 0x00
		{ 3, 0x0100},          //(Digital Audio interface format) = 0x02 : i2s slave mode;
		{51, 0x011B},          //(Sampling control) = 0x00  48KHz
		{49, 0x00F7},          //(Active control) = 0x01
		{40, 0x0179},
		{41, 0x0179},
		{43, 0x0050},
		{44, 0x000A},
		{ 7, 0x000A},
		{ 4, 0x0005},
		{52, 0x0028},
		{SEQUENCE_END, 0x00}
};

int audio_i2c_read(XIicPs *IicInstance, char iic_addr, u8 reg_addr,u8 *read_buf)
{
	*read_buf = i2c_reg8_read(IicInstance,iic_addr, reg_addr);
	return XST_SUCCESS;
}

int audio_i2c_write(XIicPs *IicInstance, char iic_addr, u8 reg_addr, u16 data)
{
    // WM8960 expects register + data combined into 16-bit value
    // Format: [6-bit addr][1-bit reserved][9-bit data]
    u16 reg_data = ((reg_addr & 0x7F) << 9) | (data & 0x1FF);
    return i2c_reg16_write(IicInstance, iic_addr, reg_data, 0);
}

/* write a array of registers  */
void audio_reg_init(XIicPs *IicInstance, Audio *AudioPtr, struct reginfo *regarray)
{
	int i = 0;
	while (regarray[i].reg != SEQUENCE_END) {
		audio_i2c_write(IicInstance, WM8960_DEVADDR, regarray[i].reg, regarray[i].val);
		i++;
	}
}

/*
 * Audio reset
 * data : TX_FIFO_RESET_MASK; RX_FIFO_RESET_MASK
 * flag : 1 if reset
 * 		  0 if reset over
 */
void audio_txrx_reset(Audio *AudioPtr, u32 data, u8 flag)
{
	unsigned int value = 0;
	/* Read register value  */
	value = Audio_ReadReg(AudioPtr->BaseAddr, AUDIO_REG_I2S_RESET) ;

	if (flag)
		value |= data ;
	else
		value &= ~data ;
	/* write register  */
	Audio_WriteReg(AudioPtr->BaseAddr, AUDIO_REG_I2S_RESET, data) ;
}

/*
 * Audio tx and rx enable
 * channel : TX_ENABLE_MASK or RX_ENABLE_MASK
 */
void audio_txrx_enable(Audio *AudioPtr, u32 channel)
{
	unsigned int value = 0;
	/* Read register value  */
	value = Audio_ReadReg(AudioPtr->BaseAddr, AUDIO_REG_I2S_CTRL) ;
	value |= channel ;
	/* write register  */
	Audio_WriteReg(AudioPtr->BaseAddr, AUDIO_REG_I2S_CTRL, value) ;
}

/*
 * Audio tx and rx disable
 * channel : TX_ENABLE_MASK or RX_ENABLE_MASK
 */
void audio_txrx_disable(Audio *AudioPtr, u32 channel)
{
	unsigned int value = 0;
	/* Read register value  */
	value = Audio_ReadReg(AudioPtr->BaseAddr, AUDIO_REG_I2S_CTRL) ;
	value &= ~channel ;
	/* write register  */
	Audio_WriteReg(AudioPtr->BaseAddr, AUDIO_REG_I2S_CTRL, value) ;
}

/*
 * Audio rx stream length setting
 */
void Audio_RxStreamLengthSetting(Audio *AudioPtr, u32 length)
{
	/* write register  */
	Audio_WriteReg(AudioPtr->BaseAddr, AUDIO_REG_I2S_PERIOD, length) ;
}

/*
 * axi memory write; only available in pl330 dma mode
 */
void audio_mem_write(Audio *AudioPtr, u32 data)
{
	Audio_WriteReg(AudioPtr->BaseAddr, AUDIO_REG_I2S_TX_FIFO, data) ;
}

/*
 * axi memory read; only available in pl330 dma mode
 */
int audio_mem_read(Audio *AudioPtr)
{
	return Audio_ReadReg(AudioPtr->BaseAddr, AUDIO_REG_I2S_RX_FIFO) ;
}

/*
 * axi memory write, left and right sound track
 * only available in pl330 dma mode
 */
void audio_mem_write_lr(Audio *AudioPtr, u32 *writebuf)
{
	audio_mem_write(AudioPtr, writebuf[0]) ;
	audio_mem_write(AudioPtr, writebuf[1]) ;
}

/*
 * axi memory read, left and right sound track
 * only available in pl330 dma mode
 */
void audio_mem_read_lr(Audio *AudioPtr, u32 *readbuf)
{
	readbuf[0] = audio_mem_read(AudioPtr) ;
	readbuf[1] = audio_mem_read(AudioPtr) ;
}

/* ================ VOLUME CONTROL FUNCTIONS ================ */

/*
 * Convert linear volume (0-255) to WM8960 register value
 * WM8960 headphone volume: 0x0 = -73.5dB (mute), 0x7F = 0dB, 0xFF = +6dB
 */
static u16 volume_to_regval(int volume)
{
    if (volume <= 48) return 0x30;        /* Mute */
    if (volume >= 127) return 0x7F;      /* Max +6dB */

    /* Scale: 0-255 -> 0x30-0x7F */
    return (u16)((volume * 127) / 127);
}

/*
 * Set headphone volume (both left and right channels)
 */
int audio_volume_set(Audio *AudioPtr, XIicPs *IicInstance, int volume)
{
    u16 regval;

    /* Clamp volume to valid range */
    if (volume < WM8960_VOLUME_MIN) volume = WM8960_VOLUME_MIN;
    if (volume > WM8960_VOLUME_MAX) volume = WM8960_VOLUME_MAX;

    /* Convert to register value */
    regval = volume_to_regval(volume);
    //xil_printf("[AUDIO] regval= 0x%x\r\n", regval);

    /*
     * For Speaker Volume Control (R40 and R41):
     * WM8960 Datasheet: page 41
     * Bits 6-0: Volume control (1111111 = +6dB, ... 0110000 = -73dB)
     * Bit 8: Update bit (1 = Update left/right channel gains)
     */

    /* Set left speaker volume (Register 40 - 0x28) */
    /* regval only contains bits 6-0 and we need to set bit bit 8 to 1 (update) */
    regval = (regval & 0x7F) | (0 << 7) | (1 << 8);  // Set bit8=1 (update)
    audio_i2c_write(IicInstance, WM8960_DEVADDR, WM8960_LEFT_SPEAKER_VOLUME, regval);

    /* Set right speaker volume (Register 41 - 0x29) */
    audio_i2c_write(IicInstance, WM8960_DEVADDR, WM8960_RIGHT_SPEAKER_VOLUME, regval);

    /* Store current volume */
    current_volume = volume;


    xil_printf("[AUDIO] Speaker volume set to %d (0x%03X)\r\n", volume, regval);

    return XST_SUCCESS;
}

/*
 * Get current volume
 */
int audio_volume_get(Audio *AudioPtr, XIicPs *IicInstance)
{
    /* Convert register value (48-127) to percentage (0-100) */
    int get_current_volume = ((double)(current_volume - WM8960_VOLUME_MIN) /
                              (double)(WM8960_VOLUME_MAX - WM8960_VOLUME_MIN)) * 100.0;
    return get_current_volume;
}

/*
 * Increase volume by step
 */
void audio_volume_up(Audio *AudioPtr, XIicPs *IicInstance)
{
    int new_volume = current_volume + 1;
    if (new_volume > WM8960_VOLUME_MAX) {
        new_volume = WM8960_VOLUME_MAX;
    }
    audio_volume_set(AudioPtr, IicInstance, new_volume);
}

/*
 * Decrease volume by step
 */
void audio_volume_down(Audio *AudioPtr, XIicPs *IicInstance)
{
    int new_volume = current_volume - 1;
    if (new_volume < WM8960_VOLUME_MIN) {
        new_volume = WM8960_VOLUME_MIN;
    }
    audio_volume_set(AudioPtr, IicInstance, new_volume);
}

/*
 * Initialize audio register and set I2S to 48KHz
 */
int audio_init(Audio *AudioPtr, XIicPs *IicInstance)
{
	AudioPtr->BaseAddr = XPAR_AXI_I2S_ADI_0_BASEADDR ;
	AudioPtr->IicDevAddr = WM8960_DEVADDR ;

	/* Initial audio registers */
	audio_reg_init(IicInstance, AudioPtr, wm8731_init_data);

	/* MCLK:12.288MHz£¬256fs£¬ setting to 48KHz */
	Audio_WriteReg(AudioPtr->BaseAddr, AUDIO_REG_I2S_CLK_CTRL, (64/2 - 1)<<16 | (4/2 - 1)) ;/* LRCLK = BCLK / 64 and BCLK = MCLK / 4. */

	/* Set default volume */
	audio_volume_set(AudioPtr, IicInstance, WM8960_VOLUME_DEFAULT);

	return XST_SUCCESS ;
}
