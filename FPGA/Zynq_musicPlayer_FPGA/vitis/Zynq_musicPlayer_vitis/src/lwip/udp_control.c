/*
 * udp_control.c
 * Simple text-based UDP protocol for button and LED status
 */

#include "udp_control.h"
#include "lwip/udp.h"
#include "lwip/ip_addr.h"
#include <string.h>
#include <stdio.h>

/* Include necessary headers */
#include "xgpiops.h"
#include "dma_ctrl.h"
//#include "audio.h"
#include "xiicps.h"

/* External declarations from main */
extern XGpioPs Gpio;
extern Audio AudioInst;
extern XIicPs ps_i2c0;
extern u32 g_current_position;
extern u32 g_total_bytes;

/* GPIO and LED definitions - match your main.c */
#ifndef PS_KEY0
#define PS_KEY0 12
#endif
#ifndef PS_KEY1
#define PS_KEY1 11
#endif
#ifndef PL_KEY0
#define PL_KEY0 58  /* Updated to match your main.c */
#endif
#ifndef PL_KEY1
#define PL_KEY1 59  /* Updated to match your main.c */
#endif
#ifndef PS_LED0
#define PS_LED0 7
#endif
#ifndef PS_LED1
#define PS_LED1 8
#endif
#ifndef PL_LED0
#define PL_LED0 60  /* Updated to match your main.c */
#endif
#ifndef PL_LED1
#define PL_LED1 61  /* Updated to match your main.c */
#endif

#ifndef LED_ON
#define LED_ON   0x0
#endif
#ifndef LED_OFF
#define LED_OFF  0x1
#endif

#ifndef STATE_PLAYING
#define STATE_IDLE      0
#define STATE_PLAYING   1
#define STATE_STOPPED   2
#endif

/* Global variables */
static struct udp_pcb *control_pcb = NULL;
ip_addr_t g_host_addr;
u16_t g_host_port = 0;
int g_host_connected = 0;

/*
 * Send a text response to host
 */
static err_t udp_control_send_response(const char *response, ip_addr_t *addr, u16_t port)
{
    err_t err;
    struct pbuf *p;
    int len = strlen(response);

    p = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_POOL);
    if (!p) {
        return ERR_MEM;
    }

    memcpy(p->payload, response, len);

    if (addr && port) {
        err = udp_sendto(control_pcb, p, addr, port);
    } else if (g_host_connected) {
        err = udp_sendto(control_pcb, p, &g_host_addr, g_host_port);
    } else {
        err = ERR_ARG;
    }

    pbuf_free(p);
    return err;
}

/*
 * Send button status to host (text format)
 */
void udp_control_send_button_status(u8_t button_id, u8_t state)
{
    char buffer[64];
    const char *btn_name;

    if (!g_host_connected) return;

    switch (button_id) {
        case 1: btn_name = "PS_KEY0"; break;
        case 2: btn_name = "PS_KEY1"; break;
        case 3: btn_name = "PL_KEY0"; break;
        case 4: btn_name = "PL_KEY1"; break;
        default: btn_name = "UNKNOWN"; break;
    }

    sprintf(buffer, "BUTTON:%s:%d\r\n", btn_name, state);
    udp_control_send_response(buffer, NULL, 0);

    xil_printf("[UDP] Sent: %s\r\n", buffer);
}

/*
 * Send volume status to host (text format)
 */
void udp_control_send_volume_status(u16_t volume)
{
    char buffer[32];

    if (!g_host_connected) return;

    sprintf(buffer, "VOLUME:%d%%\r\n", volume);
    udp_control_send_response(buffer, NULL, 0);

    xil_printf("[UDP] Sent: %s\r\n", buffer);
}

/*
 * Send play status to host (text format)
 */
void udp_control_send_play_status(u8_t state, u32_t position, u32_t total)
{
    char buffer[64];
    const char *state_str = state ? "PLAYING" : "STOPPED";

    if (!g_host_connected) return;

//    sprintf(buffer, "MUSIC Status:%s:%lu:%lu\n", state_str, position, total);
    sprintf(buffer, "MUSIC:%s\r\n", state_str);
    udp_control_send_response(buffer, NULL, 0);

    xil_printf("[UDP] Sent: %s\r\n", buffer);
}

/*
 * Send all status to host
 */
void udp_control_send_all_status(void)
{
    /* Send button states */
    u8_t ps_key0 = XGpioPs_ReadPin(&Gpio, PS_KEY0);
    u8_t ps_key1 = XGpioPs_ReadPin(&Gpio, PS_KEY1);
    u8_t pl_key0 = XGpioPs_ReadPin(&Gpio, PL_KEY0);
    u8_t pl_key1 = XGpioPs_ReadPin(&Gpio, PL_KEY1);

    udp_control_send_button_status(1, !ps_key0);
    udp_control_send_button_status(2, !ps_key1);
    udp_control_send_button_status(3, !pl_key0);
    udp_control_send_button_status(4, !pl_key1);

    /* Send volume status */
    int volume = audio_volume_get(&AudioInst, &ps_i2c0);
    udp_control_send_volume_status((u16_t)volume);

    /* Send play status */
    int state = XAxiDma_Audio_GetState();
    udp_control_send_play_status(state == STATE_PLAYING ? 1 : 0,
                                 g_current_position, g_total_bytes);
}

/*
 * Process text command from host
 */
static void udp_control_process_command(char *cmd, ip_addr_t *addr, u16_t port)
{
    char *token;
    char *saveptr;

    xil_printf("[UDP] Received command: %s\r\n", cmd);

    /* Store host address for responses */
    g_host_addr = *addr;
    g_host_port = port;
    g_host_connected = 1;

    /* Parse command */
    token = strtok_r(cmd, " \r\n", &saveptr);

    if (token == NULL) {
        udp_control_send_response("ERROR: Empty command\r\n", addr, port);
        return;
    }

    /* STATUS command */
    if (strcmp(token, "STATUS") == 0 || strcmp(token, "status") == 0) {
        udp_control_send_all_status();
        udp_control_send_response("OK\n", addr, port);
    }

    /* PLAY command */
    else if (strcmp(token, "PLAY") == 0 || strcmp(token, "play") == 0) {
        extern u32 WavLength;
        extern u32 WriteBuf[];
        XAxiDma_Audio_Play_Start(&AudioInst, WavLength, WriteBuf);
        udp_control_send_response("OK:Play started\r\n", addr, port);
    }

    /* STOP command */
    else if (strcmp(token, "STOP") == 0 || strcmp(token, "stop") == 0) {
        XAxiDma_Audio_Play_Stop();
        udp_control_send_response("OK:Play stopped\r\n", addr, port);
    }

    /* VOLUME command */
    else if (strcmp(token, "VOLUME") == 0 || strcmp(token, "volume") == 0) {
        token = strtok_r(NULL, " \r\n", &saveptr);
        if (token) {
            int volume = atoi(token);
            int converted_volume = (double)volume / 100 * (VOLUME_MAX - VOLUME_MIN) + VOLUME_MIN;
            audio_volume_set(&AudioInst, &ps_i2c0, converted_volume);
            udp_control_send_response("OK:Volume set\r\n", addr, port);
            udp_control_send_volume_status(volume);
        } else {
            udp_control_send_response("ERROR:Usage VOLUME <0-100>\r\n", addr, port);
        }
    }

    /* LED command */
    else if (strcmp(token, "LED") == 0 || strcmp(token, "led") == 0) {
        token = strtok_r(NULL, " \r\n", &saveptr);
        if (token) {
            char *led_name = token;
            token = strtok_r(NULL, " \r\n", &saveptr);
            if (token) {
                int state = (strcmp(token, "ON") == 0 || strcmp(token, "on") == 0) ? 1 : 0;

                if (strcmp(led_name, "PS_LED0") == 0) {
                    XGpioPs_WritePin(&Gpio, PS_LED0, state ? LED_ON : LED_OFF);
                    udp_control_send_response("OK:PS_LED0 set\r\n", addr, port);
                }
                else if (strcmp(led_name, "PS_LED1") == 0) {
                    XGpioPs_WritePin(&Gpio, PS_LED1, state ? LED_ON : LED_OFF);
                    udp_control_send_response("OK:PS_LED1 set\r\n", addr, port);
                }
                else if (strcmp(led_name, "PL_LED0") == 0) {
                    XGpioPs_WritePin(&Gpio, PL_LED0, state ? LED_ON : LED_OFF);
                    udp_control_send_response("OK:PL_LED0 set\r\n", addr, port);
                }
                else if (strcmp(led_name, "PL_LED1") == 0) {
                    XGpioPs_WritePin(&Gpio, PL_LED1, state ? LED_ON : LED_OFF);
                    udp_control_send_response("OK:PL_LED1 set\r\n", addr, port);
                }
                else {
                    udp_control_send_response("ERROR:Unknown LED\r\n", addr, port);
                }
            }
        } else {
            udp_control_send_response("ERROR:Usage LED <name> ON/OFF\r\n", addr, port);
        }
    }

    /* HELP command */
    else if (strcmp(token, "HELP") == 0 || strcmp(token, "help") == 0) {
        udp_control_send_response(
            "Commands:\r\n"
            "  STATUS              - Get all status\r\n"
            "  PLAY                - Start playback\r\n"
            "  STOP                - Stop playback\r\n"
            "  VOLUME <0-100>      - Set volume\r\n"
            "  LED <name> ON/OFF   - Control LED (PS_LED0/1, PL_LED0/1)\r\n"
            "  HELP                - Show this help\r\n",
            addr, port);
    }

    /* Unknown command */
    else {
        char err_msg[64];
        sprintf(err_msg, "ERROR:Unknown command '%s'\r\n", token);
        udp_control_send_response(err_msg, addr, port);
    }
}

/*
 * UDP control receive callback - text mode
 */
static void udp_control_recv_cb(void *arg, struct udp_pcb *upcb, struct pbuf *p,
                                const ip_addr_t *addr, u16_t port)
{
    char cmd[256];
    int len = p->len > 255 ? 255 : p->len;

    /* Copy and null-terminate the received data */
    memcpy(cmd, p->payload, len);
    cmd[len] = '\0';

    /* Process the text command */
    udp_control_process_command(cmd, (ip_addr_t *)addr, port);

    pbuf_free(p);
}

/*
 * Initialize UDP control server - text mode
 */
void udp_control_init(void)
{
    err_t err;

    /* Create UDP PCB */
    control_pcb = udp_new();
    if (!control_pcb) {
        xil_printf("[UDP] Error creating PCB\r\n");
        return;
    }

    /* Bind to control port */
    err = udp_bind(control_pcb, IP_ADDR_ANY, UDP_CONTROL_PORT);
    if (err != ERR_OK) {
        xil_printf("[UDP] Unable to bind to port %d\r\n", UDP_CONTROL_PORT);
        udp_remove(control_pcb);
        return;
    }

    /* Set receive callback */
    udp_recv(control_pcb, udp_control_recv_cb, NULL);

    xil_printf("[UDP] Text control server listening on port %d\r\n", UDP_CONTROL_PORT);
    xil_printf("[UDP] Send 'HELP' for commands\r\n");
}
