/* ------------------------------------------------------------ */
/*				Include File Definitions						*/
/* ------------------------------------------------------------ */

#include "math.h"
#include "xscugic.h"
#include "xaxidma.h"
#include "sleep.h"
#include "audio.h"

/*
 *DMA redefines
 */
#define MAX_DMA_LEN		   0x800000      /* DMA max length in byte */
#define DMA_DEV_ID		   XPAR_AXIDMA_0_DEVICE_ID
#define S2MM_INTR_ID       XPAR_FABRIC_AXI_DMA_0_S2MM_INTROUT_INTR
#define MM2S_INTR_ID       XPAR_FABRIC_AXI_DMA_0_MM2S_INTROUT_INTR

/*
 * GPIO defines
 */
#define KEY            59
#define RECORD_LED     7
#define PLAY_LED       8
/* LED States */
#define LED_ON		   0x1
#define LED_OFF		   0x0
/*
 *DMA BD defines
 */
#define BD_COUNT         16

/*
 *Function defines
 */
int XAxiDma_Audio_Play(XScuGic *InstancePtr, Audio *AudioInstance, u32 length, u32 *Databuf) ;

/*
 * Function defines - changed to polling version
 */
int XAxiDma_Audio_Play_Poll(Audio *AudioInstance, u32 length, u32 *Databuf, int *is_playing);

/*
 * Simple state definitions
 */
#define STATE_IDLE      0
#define STATE_PLAYING   1
#define STATE_STOPPED   2

/* Global variables for position tracking */
extern u32 g_current_position;
extern u32 g_total_bytes;

/*
 * Function defines
 */
int XAxiDma_Audio_Play_Start(Audio *AudioInstance, u32 length, u32 *Databuf);
int XAxiDma_Audio_Play_Stop(void);
int XAxiDma_Audio_Play_Process(void);
int XAxiDma_Audio_GetState(void);
int XAxiDma_Audio_GetVolume(void);
int XAxiDma_Audio_GetState(void);
