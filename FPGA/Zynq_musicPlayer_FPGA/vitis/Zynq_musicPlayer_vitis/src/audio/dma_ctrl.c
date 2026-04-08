/* ------------------------------------------------------------ */
/*				Include File Definitions						*/
/* ------------------------------------------------------------ */

#include "dma_ctrl.h"
#include "dma_bd.h"
#include "xgpiops.h"

/* Add these global variables at the top of dma_ctrl.c */
u32 g_current_position = 0;
//u32 g_total_bytes = 0;

/*
 * DMA buffers
 */
u32 DmaTxBuffer[MAX_DMA_LEN] __attribute__ ((aligned(64)));
u32 BdTxChainBuffer[BD_ALIGNMENT*4] __attribute__ ((aligned(64)));

/*
 * DMA struct
 */
XAxiDma AxiDma;

/*
 * Global state variables - SIMPLE
 */
static int dma_state = STATE_IDLE;
static Audio *g_AudioInstance = NULL;
u32 g_total_bytes = 0;
static int g_initialized = 0;
static int g_loop_mode = 0;  /* New flag for continuous playback */
static u32 *g_audio_buffer = NULL;  /* Store buffer pointer for restart */
static u32 g_buffer_length = 0;  /* Store buffer length for restart */

extern XGpioPs Gpio;

/*
 * Initialize DMA in polling mode
 */
static int XAxiDma_Initial_poll(u16 DeviceId, XAxiDma *XAxiDma)
{
    XAxiDma_Config *CfgPtr;
    int Status;

    CfgPtr = XAxiDma_LookupConfig(DeviceId);
    if (!CfgPtr) {
        return XST_FAILURE;
    }

    Status = XAxiDma_CfgInitialize(XAxiDma, CfgPtr);
    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    /* Disable all interrupts for polling mode */
    XAxiDma_IntrDisable(XAxiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DMA_TO_DEVICE);
    XAxiDma_IntrDisable(XAxiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);

    return XST_SUCCESS;
}

/*
 * START playback - now with loop mode
 */
int XAxiDma_Audio_Play_Start(Audio *AudioInstance, u32 length, u32 *Databuf)
{
    int Status;

    g_total_bytes = length * sizeof(u32);
    g_current_position = 0;

    /* If already playing, stop first */
    if (dma_state == STATE_PLAYING) {
        XAxiDma_Audio_Play_Stop();
    }

    /* Initialize DMA */
    if (!g_initialized) {
        Status = XAxiDma_Initial_poll(DMA_DEV_ID, &AxiDma);
        if (Status != XST_SUCCESS) {
            xil_printf("[ERROR] DMA Init failed\r\n");
            return XST_FAILURE;
        }
        g_initialized = 1;
    }

    /* Store parameters for loop mode */
    g_AudioInstance = AudioInstance;
    g_audio_buffer = Databuf;
    g_buffer_length = length;
    g_total_bytes = length * sizeof(u32);
    g_loop_mode = 1;  /* Enable loop mode */

    /* Create BD chain */
    CreateBdChain(BdTxChainBuffer, BD_COUNT, g_total_bytes, (u8 *)Databuf, TXPATH);

    /* Turn on LED */
    XGpioPs_WritePin(&Gpio, PLAY_LED, LED_ON);

    /* Start DMA transfer */
    Bd_Start(BdTxChainBuffer, BD_COUNT, &AxiDma, TXPATH);

    /* Enable audio */
    audio_txrx_enable(AudioInstance, TX_ENABLE_MASK);

    dma_state = STATE_PLAYING;
    xil_printf("[INFO] Play started (continuous mode)\r\n");

    return XST_SUCCESS;
}

/*
 * STOP playback
 */
int XAxiDma_Audio_Play_Stop(void)
{
    if (dma_state == STATE_PLAYING) {
        /* Turn off LED */
        XGpioPs_WritePin(&Gpio, PLAY_LED, LED_OFF);

        /* Disable audio */
        if (g_AudioInstance) {
            audio_txrx_disable(g_AudioInstance, TX_ENABLE_MASK);
        }

        /* Stop DMA transfer */
        XAxiDma_Reset(&AxiDma);

        /* Wait for reset to complete */
        int timeout = 10000;
        while (timeout--) {
            if (XAxiDma_ResetIsDone(&AxiDma))
                break;
        }

        /* Clear BD status */
        Bd_StatusClr(BdTxChainBuffer, BD_COUNT);

        g_loop_mode = 0;  /* Disable loop mode */
        dma_state = STATE_STOPPED;
        xil_printf("[INFO] Play stopped\r\n");
    }

    return XST_SUCCESS;
}

/*
 * Process DMA state - MODIFIED for continuous playback
 */
int XAxiDma_Audio_Play_Process(void)
{
    if (dma_state == STATE_PLAYING) {
        /* Check if DMA is done */
        if (!XAxiDma_Busy(&AxiDma, XAXIDMA_DMA_TO_DEVICE)) {
            if (g_loop_mode) {
                /* In loop mode, restart the transfer */

                /* Clear BD status first */
                Bd_StatusClr(BdTxChainBuffer, BD_COUNT);

                /* Small delay to ensure audio hardware is ready */
                usleep(1000);

                /* Restart with same buffer */
                CreateBdChain(BdTxChainBuffer, BD_COUNT, g_total_bytes,
                             (u8 *)g_audio_buffer, TXPATH);
                Bd_Start(BdTxChainBuffer, BD_COUNT, &AxiDma, TXPATH);

                /* Increment position counter (optional, for tracking) */
                g_current_position += g_buffer_length;
                if (g_current_position > 1000000) g_current_position = 0;  /* Wrap around */

                return XST_DEVICE_BUSY;  /* Still playing */
            } else {
                /* Non-loop mode - playback finished */
                XGpioPs_WritePin(&Gpio, PLAY_LED, LED_OFF);
                if (g_AudioInstance) {
                    audio_txrx_disable(g_AudioInstance, TX_ENABLE_MASK);
                }
                Bd_StatusClr(BdTxChainBuffer, BD_COUNT);
                xil_printf("[INFO] Play finished\r\n");
                dma_state = STATE_STOPPED;
                return XST_SUCCESS;
            }
        }
        return XST_DEVICE_BUSY;  /* Still playing */
    }

    return XST_SUCCESS;  /* Idle or stopped */
}

/*
 * Get current state
 */
int XAxiDma_Audio_GetState(void)
{
    return dma_state;
}

/*
 * Toggle loop mode (optional - if you want to control from GUI)
 */
void XAxiDma_Audio_SetLoopMode(int enable)
{
    g_loop_mode = enable;
}

/*
 * Get current position (for GUI display)
 */
u32 XAxiDma_Audio_GetPosition(void)
{
    return g_current_position;
}
