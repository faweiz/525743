/*
 *  main.c
 *  Remote Music Player via lwIP Application
 *  Controls: PS_KEY1 (Play/Stop), PL_KEY0 (Volume Up), PL_KEY1 (Volume Down)
 */
#include <stdio.h>
#include "xparameters.h"
#include "xil_cache.h"
#include "sleep.h"
#include "ff.h"
#include "xscugic.h"
#include "xgpiops.h"
/* Project includes */
#include "i2c/PS_i2c.h"
#include "dma_ctrl.h"
#include "SD/SD.h"
/* LWIP includes */
#include "netif/xadapter.h"
#include "lwip_example_platform.h"
#include "lwip_example_platform_config.h"
#include "lwipopts.h"
#include "lwip/priv/tcp_priv.h"
#include "lwip/init.h"
#include "lwip/inet.h"
// udp
#include "udp_control.h"
// tcp
#include "tcp_control.h"
// FTP
#include "ftpd.h"
extern int ftpd_init(void);

#if LWIP_DHCP==1
#include "lwip/dhcp.h"
extern volatile int dhcp_timoutcntr;
#endif

extern volatile int TcpFastTmrFlag;
extern volatile int TcpSlowTmrFlag;

/* ========================================================================= */
/*                           CONSTANT DEFINITIONS                            */
/* ========================================================================= */
/* Network configuration */
#define DEFAULT_IP_ADDRESS      "192.168.68.44"
#define DEFAULT_IP_MASK         "255.255.255.0"
#define DEFAULT_GW_ADDRESS      "192.168.68.1"
/* GPIO Device ID */
#define GPIO_DEVICE_ID          XPAR_XGPIOPS_0_DEVICE_ID
/* LED Pin Definitions */
#define PL_LED0                 60  /* EMIO6 - J18 */
#define PL_LED1                 61  /* EMIO7 - H18 */
#define PS_LED0                 7   /* MIO0 */
#define PS_LED1                 8   /* MIO7 */

u8 PS_LED_0_flag  = 0;
u8 PS_LED_1_flag  = 0;
u8 PL_LED_0_flag  = 0;
u8 PL_LED_1_flag  = 0;
u8 PS_LED0_status = 0;
u8 PS_LED1_status = 0;
u8 PL_LED0_status = 0;
u8 PL_LED1_status = 0;

/* KEY Pin Definitions */
#define PL_KEY0                 58  /* EMIO4 - L20 */
#define PL_KEY1                 59  /* EMIO5 - J20 */
#define PS_KEY0                 12  /* MIO12 */
#define PS_KEY1                 11  /* MIO11 */

u8 PS_KEY_0_flag = 0;
u8 PS_KEY_1_flag = 0;
u8 PL_KEY_0_flag = 0;
u8 PL_KEY_1_flag = 0;
u8 prev_PS_KEY_0 = 0xFF;
u8 prev_PS_KEY_1 = 0xFF;
u8 prev_PL_KEY_0 = 0xFF;
u8 prev_PL_KEY_1 = 0xFF;
int btn_changed = 0;
int btn_tcp_send = 0;

/* Audio buffer size */
#define PLAY_LEN                0x4000000
#define RECORD_LEN              960000

/* ========================================================================= */
/*                           TYPE DEFINITIONS                                */
/* ========================================================================= */

/* Button states structure */
typedef struct {
    /* Current pin values */
	int ps_key0;
    int ps_key1;
    int pl_key0;
    int pl_key1;

    /* Previous values for edge detection */
    int last_ps_key0;
    int last_ps_key1;
    int last_pl_key0;
    int last_pl_key1;

    /* Pressed flags */
    int ps_key0_pressed;
    int ps_key1_pressed;
    int pl_key0_pressed;
    int pl_key1_pressed;

    /* LED states */
    int pl_led0_state;
    int pl_led1_state;
    int ps_led0_state;
} ButtonStates_t;

/* ========================================================================= */
/*                           GLOBAL VARIABLES                                */
/* ========================================================================= */

/* Network */
struct netif server_netif;
struct netif *netif;

/* GPIO */
XGpioPs Gpio;

/* Interrupt Controller */
XScuGic GicInstance;

/* Audio */
Audio AudioInst;
XIicPs ps_i2c0;

/* File System */
static FIL fil;
static Wave_t _File;

/* Audio buffers */
u32 WriteBuf[PLAY_LEN] __attribute__ ((aligned(64)));
u32 ReadBuf[RECORD_LEN] __attribute__ ((aligned(64)));
u8 waveframe[PLAY_LEN] __attribute__ ((aligned(64)));

/* WAV file info */
unsigned int WavLength;

/* Button states */
static ButtonStates_t btn = {
	.last_ps_key0 = 1,
    .last_ps_key1 = 1,
    .last_pl_key0 = 1,
    .last_pl_key1 = 1,
    .pl_led0_state = LED_OFF,
    .pl_led1_state = LED_OFF,
	.ps_led0_state = LED_OFF,
};

// Music from CVI
char g_current_music_file[64] = "test audio.wav";  /* Default file */
int g_new_file_available = 0;  /* Flag to indicate new file uploaded */

/* ========================================================================= */
/*                         FUNCTION PROTOTYPES                               */
/* ========================================================================= */

/* GPIO and Interrupt */
static int GpioSetup(XGpioPs *Gpioinstance, u16 Deviceid);

/* WAV Processing */
static unsigned int wav_read(char *wave, u8 *frame);
static u32 SD_LoadFile(char* File);

/* Network */
static int lwip_music_init(void);
static void print_ip(char *msg, ip_addr_t *ip);
static void print_ip_settings(ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw);
static void assign_default_ip(ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw);
void start_application(void);
void print_app_header(void);

/* Main loop processing */
static void process_network(void);
static void read_all_buttons(void);
static void detect_button_presses(void);
static void update_leds(void);
static void handle_button_actions(void);
static void print_system_help(void);

/* ========================================================================= */
/*                         GPIO INITIALIZATION                               */
/* ========================================================================= */

static int GpioSetup(XGpioPs *Gpioinstance, u16 Deviceid)
{
    int Status;
    XGpioPs_Config *ConfigPtr;

    /* Initialize the GPIO driver */
    ConfigPtr = XGpioPs_LookupConfig(Deviceid);
    Status = XGpioPs_CfgInitialize(Gpioinstance, ConfigPtr,
            ConfigPtr->BaseAddr);
    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    /* Configure LEDs as outputs */
    XGpioPs_SetDirectionPin(Gpioinstance, PL_LED0, 1);
    XGpioPs_SetDirectionPin(Gpioinstance, PL_LED1, 1);
    XGpioPs_SetDirectionPin(Gpioinstance, PS_LED0, 1);
    XGpioPs_SetDirectionPin(Gpioinstance, PS_LED1, 1);

    /* Enable LED outputs */
    XGpioPs_SetOutputEnablePin(Gpioinstance, PL_LED0, 1);
    XGpioPs_SetOutputEnablePin(Gpioinstance, PL_LED1, 1);
    XGpioPs_SetOutputEnablePin(Gpioinstance, PS_LED0, 1);
    XGpioPs_SetOutputEnablePin(Gpioinstance, PS_LED1, 1);

    /* Configure keys as inputs */
    XGpioPs_SetDirectionPin(Gpioinstance, PL_KEY0, 0);
    XGpioPs_SetDirectionPin(Gpioinstance, PL_KEY1, 0);
    XGpioPs_SetDirectionPin(Gpioinstance, PS_KEY0, 0);
    XGpioPs_SetDirectionPin(Gpioinstance, PS_KEY1, 0);

    return XST_SUCCESS;
}

/* ========================================================================= */
/*                           WAV PROCESSING                                  */
/* ========================================================================= */

static unsigned int wav_read(char *wave, u8 *frame)
{
    unsigned int x;
    FRESULT res;
    unsigned char TMPBUF[100];
    unsigned int br;
    unsigned int FrameLength;

    res = f_open(&fil, wave, FA_OPEN_EXISTING | FA_READ);
    if (res != FR_OK) {
        return 0;
    }

    res = f_read(&fil, TMPBUF, 88, &br);
    if (res != FR_OK) {
        f_close(&fil);
        return 0;
    }

    /* Check WAV header */
    if (TMPBUF[0] == 'R' && TMPBUF[1] == 'I' && TMPBUF[2] == 'F' && TMPBUF[3] == 'F' &&
        TMPBUF[8] == 'W' && TMPBUF[9] == 'A' && TMPBUF[10] == 'V' && TMPBUF[11] == 'E') {
        FrameLength = ((unsigned int)TMPBUF[7] << 24) +
                      ((unsigned int)TMPBUF[6] << 16) +
                      ((unsigned int)TMPBUF[5] << 8) +
                      TMPBUF[4];
        xil_printf("wave length is %x\r\n", FrameLength);
    } else {
        f_close(&fil);
        return 0;
    }

    res = f_read(&fil, frame, FrameLength - 80, &br);
    if (res != FR_OK) {
        f_close(&fil);
        return 0;
    }

    /* Convert 16-bit PCM to 32-bit format for DMA */
    for (x = 0; x < (FrameLength - 80) / 2; x++) {
        WriteBuf[x] = (u32)frame[x * 2 + 1] << 24 | (u32)frame[x * 2 + 0] << 16;
    }

    f_close(&fil);
    return (FrameLength - 80) / 2;
}

static u32 SD_LoadFile(char* File)
{
    if (SD_LoadFileFromCard(File, &_File)) {
        return XST_FAILURE;
    }

    xil_printf("  File size: %lu bytes\n\r", _File.RIFF.Header.ChunkSize + 8);
    xil_printf("  File format: %lu\n\r", _File.Format.AudioFormat);
    xil_printf("  Channels: %lu\n\r", _File.Format.NumChannels);
    xil_printf("  Sample rate: %lu Hz\n\r", _File.Format.SampleRate);
    xil_printf("  Bits per sample: %lu bits\n\r", _File.Format.BitsPerSample);
    xil_printf("  Block align: %lu bytes\n\r", _File.Format.BlockAlign);
    xil_printf("  Data bytes per channel: %lu bytes\n\r",
               _File.RIFF.Header.ChunkSize / _File.Format.NumChannels);
    xil_printf("  Samples: %lu\n\r",
               8 * _File.RIFF.Header.ChunkSize / _File.Format.NumChannels / _File.Format.BitsPerSample);

    WavLength = wav_read(File, waveframe);
    xil_printf("  WavLength: %d\n\r", WavLength);

    /* Check for valid file format */
    if ((_File.Format.BitsPerSample != 16) || (_File.Format.NumChannels > 2)) {
        xil_printf("[ERROR] Invalid file format!\n\r");
        return XST_FAILURE;
    }

    return XST_SUCCESS;
}

/* ========================================================================= */
/*                           NETWORK FUNCTIONS                               */
/* ========================================================================= */

static void print_ip(char *msg, ip_addr_t *ip)
{
    print(msg);
    xil_printf("%d.%d.%d.%d\r\n", ip4_addr1(ip), ip4_addr2(ip),
            ip4_addr3(ip), ip4_addr4(ip));
}

static void print_ip_settings(ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw)
{
    print_ip("Board IP:       ", ip);
    print_ip("Netmask :       ", mask);
    print_ip("Gateway :       ", gw);
}

static void assign_default_ip(ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw)
{
    int err;

    xil_printf("Configuring default IP %s \r\n", DEFAULT_IP_ADDRESS);

    err = inet_aton(DEFAULT_IP_ADDRESS, ip);
    if (!err)
        xil_printf("Invalid default IP address: %d\r\n", err);

    err = inet_aton(DEFAULT_IP_MASK, mask);
    if (!err)
        xil_printf("Invalid default IP MASK: %d\r\n", err);

    err = inet_aton(DEFAULT_GW_ADDRESS, gw);
    if (!err)
        xil_printf("Invalid default gateway address: %d\r\n", err);
}

static int lwip_music_init(void)
{
    /* MAC address - should be unique per board */
    unsigned char mac_ethernet_address[] = {
        0x00, 0x0a, 0x35, 0x00, 0x01, 0x02
    };

    netif = &server_netif;
    init_platform();

    /* Initialize lwIP */
    lwip_init();

    /* Add network interface */
    if (!xemac_add(netif, NULL, NULL, NULL, mac_ethernet_address,
                PLATFORM_EMAC_BASEADDR)) {
        return XST_FAILURE;
    }
    netif_set_default(netif);

    /* Enable interrupts */
    platform_enable_interrupts();
    netif_set_up(netif);

#if (LWIP_DHCP == 1)
    /* DHCP client */
    dhcp_start(netif);
    dhcp_timoutcntr = 24;
    while (((netif->ip_addr.addr) == 0) && (dhcp_timoutcntr > 0))
        xemacif_input(netif);

    if (dhcp_timoutcntr <= 0) {
        if ((netif->ip_addr.addr) == 0) {
            xil_printf("[ERROR] DHCP request timed out\r\n");
            assign_default_ip(&(netif->ip_addr),
                    &(netif->netmask), &(netif->gw));
        }
    }
#else
    assign_default_ip(&(netif->ip_addr), &(netif->netmask), &(netif->gw));
#endif

    print_ip_settings(&(netif->ip_addr), &(netif->netmask), &(netif->gw));
    xil_printf("\r\n");
    print_app_header();
    start_application();
    xil_printf("\r\n");

    return XST_SUCCESS;
}

/* ========================================================================= */
/*                           MAIN LOOP PROCESSING                            */
/* ========================================================================= */

static void process_network(void)
{
    if (TcpFastTmrFlag) {
        tcp_fasttmr();
        TcpFastTmrFlag = 0;
    }
    if (TcpSlowTmrFlag) {
        tcp_slowtmr();
        TcpSlowTmrFlag = 0;
    }
    xemacif_input(netif);
}

static void read_all_buttons(void)
{
	btn.ps_key0 = XGpioPs_ReadPin(&Gpio, PS_KEY0);
    btn.ps_key1 = XGpioPs_ReadPin(&Gpio, PS_KEY1);
    btn.pl_key0 = XGpioPs_ReadPin(&Gpio, PL_KEY0);
    btn.pl_key1 = XGpioPs_ReadPin(&Gpio, PL_KEY1);
}

static void detect_button_presses(void)
{
	char buffer[64];
    /* PS_KEY0: detect falling edge */
    if (btn.last_ps_key0 == 1 && btn.ps_key0 == 0) {
        xil_printf("[BUTTON] PS_KEY0 pressed\r\n");
        btn.ps_key0_pressed = 1;

        /* Toggle PS_LED0 when button pressed (optional) */
        u8_t current = XGpioPs_ReadPin(&Gpio, PS_LED0);
        XGpioPs_WritePin(&Gpio, PS_LED0, current == LED_ON ? LED_OFF : LED_ON);

        /* Send button status to host */
        tcp_control_send_button_status(BUTTON_PS_KEY0, 1);

        /* Also send updated LED status */
        u8_t new_state = XGpioPs_ReadPin(&Gpio, PS_LED0);// == LED_ON ? 1 : 0;
        sprintf(buffer, "LED:PS_LED0:%d\r\n", new_state);
        tcp_control_send_response(buffer);
    }
    btn.last_ps_key0 = btn.ps_key0;

    /* PS_KEY1: Play/Stop toggle - detect falling edge */
    if (btn.last_ps_key1 == 1 && btn.ps_key1 == 0) {
        xil_printf("[BUTTON] PS_KEY1 pressed\r\n");
        btn.ps_key1_pressed = 1;

        /* Send button status to host */
        tcp_control_send_button_status(BUTTON_PS_KEY1, 1);

        /* PS_LED1 will be updated in handle_button_actions when play/stop toggles */
    }
    btn.last_ps_key1 = btn.ps_key1;

    /* PL_KEY0: Volume Up - detect falling edge */
    if (btn.last_pl_key0 == 1 && btn.pl_key0 == 0) {
        xil_printf("[BUTTON] PL_KEY0 pressed - Volume Up\r\n");
        btn.pl_key0_pressed = 1;

        /* Send button status to host */
        tcp_control_send_button_status(BUTTON_PL_KEY0, 1);

        /* PL_LED0 lights up while button is pressed (handled in update_leds) */
    }
    btn.last_pl_key0 = btn.pl_key0;

    /* PL_KEY1: Volume Down - detect falling edge */
    if (btn.last_pl_key1 == 1 && btn.pl_key1 == 0) {
        xil_printf("[BUTTON] PL_KEY1 pressed - Volume Down\r\n");
        btn.pl_key1_pressed = 1;

        /* Send button status to host */
        tcp_control_send_button_status(BUTTON_PL_KEY1, 1);

        /* PL_LED1 lights up while button is pressed (handled in update_leds) */
    }
    btn.last_pl_key1 = btn.pl_key1;
}

static void update_leds(void)
{
	char buffer[64];
    /* PL_LED0: ON when PL_KEY0 is pressed */
    if (btn.pl_key0 == 0) {
        if (btn.pl_led0_state != LED_ON) {
            XGpioPs_WritePin(&Gpio, PL_LED0, LED_ON);
            btn.pl_led0_state = LED_ON;

            PL_LED_0_flag = 1;  /* Update flag */

            /* Send LED status update */
			sprintf(buffer, "LED:PL_LED0:1\r\n");
			tcp_control_send_response(buffer);
        }
    } else {
        if (btn.pl_led0_state != LED_OFF) {
            XGpioPs_WritePin(&Gpio, PL_LED0, LED_OFF);
            btn.pl_led0_state = LED_OFF;

            PL_LED_0_flag = 0;  /* Update flag */

            /* Send LED status update */
			sprintf(buffer, "LED:PL_LED0:0\r\n");
			tcp_control_send_response(buffer);
        }
    }

    /* PL_LED1: ON when PL_KEY1 is pressed */
    if (btn.pl_key1 == 0) {
        if (btn.pl_led1_state != LED_ON) {
            XGpioPs_WritePin(&Gpio, PL_LED1, LED_ON);
            btn.pl_led1_state = LED_ON;

            PL_LED_1_flag = 1;  /* Update flag */

            /* Send LED status update */
			sprintf(buffer, "LED:PL_LED1:1\r\n");
			tcp_control_send_response(buffer);
        }
    } else {
        if (btn.pl_led1_state != LED_OFF) {
            XGpioPs_WritePin(&Gpio, PL_LED1, LED_OFF);
            btn.pl_led1_state = LED_OFF;

            PL_LED_1_flag = 0;  /* Update flag */

            /* Send LED status update */
			sprintf(buffer, "LED:PL_LED1:0\r\n");
			tcp_control_send_response(buffer);
        }
    }

    /* PS_LED0: ON when PS_KEY0 is pressed */
	if (btn.ps_key0 == 0) {
		if (btn.ps_led0_state != LED_ON) {
			XGpioPs_WritePin(&Gpio, PS_LED0, LED_ON);
			btn.ps_led0_state = LED_ON;

			PS_LED_0_flag = 1;  /* Update flag */

			/* Send LED status update */
			char buffer[64];
			sprintf(buffer, "LED:PS_LED0:1\r\n");
			tcp_control_send_response(buffer);
		}
	} else {
		if (btn.ps_led0_state != LED_OFF) {
			XGpioPs_WritePin(&Gpio, PS_LED0, LED_OFF);
			btn.ps_led0_state = LED_OFF;

			PS_LED_0_flag = 0;  /* Update flag */

			/* Send LED status update */
			char buffer[64];
			sprintf(buffer, "LED:PS_LED0:0\r\n");
			tcp_control_send_response(buffer);
		}
	}
}

static void handle_button_actions(void)
{
	char buffer[64];
    /* PS_KEY1: Play/Stop Toggle */
    if (btn.ps_key1_pressed) {
        btn.ps_key1_pressed = 0;

        int current_state = XAxiDma_Audio_GetState();
        if (current_state == STATE_PLAYING) {
            xil_printf("[INFO] Playback stopped\r\n");
            XGpioPs_WritePin(&Gpio, PS_LED1, LED_OFF);
            XAxiDma_Audio_Play_Stop();

            PS_LED_1_flag = 0;  /* Update flag */

            /* Send LED status update */
            sprintf(buffer, "LED:PS_LED1:0\r\n");
            tcp_control_send_response(buffer);
        } else {
            xil_printf("[INFO] Starting playback...\r\n");
            XGpioPs_WritePin(&Gpio, PS_LED1, LED_ON);
            XAxiDma_Audio_Play_Start(&AudioInst, WavLength, AudioInst.TxBufferPtr);

            PS_LED_1_flag = 1;  /* Update flag */

            /* Send LED status update */
            sprintf(buffer, "LED:PS_LED1:1\r\n");
            tcp_control_send_response(buffer);
        }

        /* Send play status */
        int state = XAxiDma_Audio_GetState();
        tcp_control_send_play_status(state == STATE_PLAYING ? 1 : 0);
    }

    /* PL_KEY0: Volume Up */
    if (btn.pl_key0_pressed) {
        btn.pl_key0_pressed = 0;
        audio_volume_up(&AudioInst, &ps_i2c0);
        int vol = audio_volume_get(&AudioInst, &ps_i2c0);
        xil_printf("[INFO] Volume = %d%%\r\n", vol);

        /* Send volume status */
        tcp_control_send_volume_status((u16_t)vol);
    }

    /* PL_KEY1: Volume Down */
    if (btn.pl_key1_pressed) {
        btn.pl_key1_pressed = 0;
        audio_volume_down(&AudioInst, &ps_i2c0);
        int vol = audio_volume_get(&AudioInst, &ps_i2c0);
        xil_printf("[INFO] Volume = %d%%\r\n", vol);

        /* Send volume status */
        tcp_control_send_volume_status((u16_t)vol);
    }
}

static void print_system_help(void)
{
    xil_printf("\r\n--- System Ready ---\r\n");
    xil_printf("========================================\r\n");
    xil_printf("PS_KEY1: Play/Stop Toggle\r\n");
    xil_printf("PL_KEY0: Volume Up\r\n");
    xil_printf("PL_KEY1: Volume Down\r\n");
    xil_printf("----------------------------------------\r\n");
    xil_printf("PS_LED1: ON when playing\r\n");
    xil_printf("PL_LED0: ON when Volume Up pressed\r\n");
    xil_printf("PL_LED1: ON when Volume Down pressed\r\n");
    xil_printf("========================================\r\n");
}

/* ========================================================================= */
/*                               MAIN FUNCTION                               */
/* ========================================================================= */
int main(void)
{
    int Status;

    xil_printf("\r\n\r\n");
    xil_printf("----- Remote Music Player via lwIP Application -----\r\n");

    /* Initialize SD card */
    xil_printf("[INFO] Mount SD card...\r\n");
    if (SD_Init()) {
        xil_printf("[ERROR] SD card initialization failed!\r\n");
        return XST_FAILURE;
    }

    /* Initialize I2C */
    xil_printf("[INFO] Initialize I2C...\r\n");
    i2c_init(&ps_i2c0, XPAR_XIICPS_0_DEVICE_ID, 40000);
    usleep(500000);

    /* Initialize audio codec */
    xil_printf("[INFO] Initialize WM8960 Audio Codec...\r\n");
    audio_init(&AudioInst, &ps_i2c0);
    AudioInst.TxBufferPtr = WriteBuf;
    memset(AudioInst.TxBufferPtr, 0, PLAY_LEN * sizeof(u32));
    audio_txrx_disable(&AudioInst, TX_ENABLE_MASK | RX_ENABLE_MASK);

    /* Initialize GPIO */
    xil_printf("[INFO] Initialize GPIO...\r\n");
    Status = GpioSetup(&Gpio, GPIO_DEVICE_ID);
    if (Status != XST_SUCCESS) {
        xil_printf("[ERROR] GPIO initialization failed!\r\n");
        return XST_FAILURE;
    }

    /* Initialize all LEDs to OFF */
    XGpioPs_WritePin(&Gpio, PS_LED0, LED_OFF);
    XGpioPs_WritePin(&Gpio, PS_LED1, LED_OFF);
    XGpioPs_WritePin(&Gpio, PL_LED0, LED_OFF);
    XGpioPs_WritePin(&Gpio, PL_LED1, LED_OFF);

    /* Initialize LWIP */
    xil_printf("[INFO] Initialize LWIP...\r\n");
    Status = lwip_music_init();
    if (Status != XST_SUCCESS) {
        xil_printf("[ERROR] LWIP initialization failed! Error: %lu\n\r", Status);
        return XST_FAILURE;
    }

    /* Initialize FTP server */
    xil_printf("[INFO] Initializing FTP server on port %d...\r\n", FTPD_PORT);
    ftpd_set_timezone(28800);  // Set your timezone offset (e.g., 28800 for UTC+8)
    if (ftpd_init() == 0) {
        xil_printf("[INFO] FTP server started successfully\r\n");
        xil_printf("[INFO] Connect using: ftp %s\r\n", inet_ntoa(netif->ip_addr));
    } else {
        xil_printf("[ERROR] FTP server failed to start\r\n");
    }

    /* Load WAV file from SD card */
    xil_printf("[INFO] Loading test audio.wav...\r\n");
    if (SD_LoadFile("0:/test audio.wav") == XST_SUCCESS) {
        xil_printf("[INFO] Loaded audio.wav successfully\r\n");
        strcpy(g_current_music_file, "test audio.wav");
    } else {
    	xil_printf("[INFO] Loading Careless Whisper.wav...\r\n");
    	SD_LoadFile("0:/Careless Whisper.wav");
    }

    /* Flush cache for DMA */
    Xil_DCacheFlushRange((UINTPTR)AudioInst.TxBufferPtr, PLAY_LEN * sizeof(u32));

    /* Initialize DMA for playback (ready for button press) */
    XAxiDma_Audio_Play_Start(&AudioInst, WavLength, AudioInst.TxBufferPtr);

    /* Print system help */
    print_system_help();

    /* ==================== MAIN LOOP ==================== */
    while (1) {
        /* Network processing */
        process_network();

        /* Button processing */
        read_all_buttons();
        detect_button_presses();
        update_leds();
        handle_button_actions();

        /* Check if a new file has been uploaded via TFTP */
		if (g_new_file_available) {
			xil_printf("[INFO] New file detected: %s, loading...\r\n", g_current_music_file);

			/* Stop current playback if playing */
			if (XAxiDma_Audio_GetState() == STATE_PLAYING) {
				XAxiDma_Audio_Play_Stop();
			}

			/* Load the new file */
			if (SD_LoadFile(g_current_music_file) == XST_SUCCESS) {
				/* Flush cache for DMA */
				Xil_DCacheFlushRange((UINTPTR)AudioInst.TxBufferPtr, PLAY_LEN * sizeof(u32));

				/* Prepare for playback but don't start automatically */
				XAxiDma_Audio_Play_Start(&AudioInst, WavLength, AudioInst.TxBufferPtr);

				/* Send status update to GUI that file is ready */
				char buffer[256];
				sprintf(buffer, "[INFO] Loaded %s successfully\r\n", g_current_music_file);
				tcp_control_send_response(buffer);
				xil_printf(buffer);
			} else {
				char buffer[256];
				sprintf(buffer, "[ERROR] Failed to play music: %s\r\n", g_current_music_file);
				tcp_control_send_response(buffer);
				xil_printf(buffer);
			}

			g_new_file_available = 0;
		}

        /* DMA processing */
        XAxiDma_Audio_Play_Process();

        /* Small delay to prevent CPU hogging */
        usleep(1000);
    }

    return 0;
}


