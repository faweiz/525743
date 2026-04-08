/*
 * tcp_control.c
 * Simple text-based TCP protocol for button and LED status
 */

#include "tcp_control.h"
#include "lwip/tcp.h"
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
#define PL_KEY0 58
#endif
#ifndef PL_KEY1
#define PL_KEY1 59
#endif
#ifndef PS_LED0
#define PS_LED0 7
#endif
#ifndef PS_LED1
#define PS_LED1 8
#endif
#ifndef PL_LED0
#define PL_LED0 60
#endif
#ifndef PL_LED1
#define PL_LED1 61
#endif

/* Connected client state */
struct tcp_client_state {
    struct tcp_pcb *pcb;
    u8_t connected;
};

static struct tcp_client_state client = {NULL, 0};

/*
 * Send a text response to client
 */
err_t tcp_control_send_response(const char *response)
{
    err_t err = ERR_OK;

    if (client.connected && client.pcb) {
        err = tcp_write(client.pcb, response, strlen(response), TCP_WRITE_FLAG_COPY);
        if (err == ERR_OK) {
            tcp_output(client.pcb);
        }
    }
    return err;
}

/*
 * Send button status to client (text format)
 */
void tcp_control_send_button_status(u8_t button_id, u8_t state)
{
    char buffer[64];
    const char *btn_name;

    if (!client.connected) return;

    switch (button_id) {
        case 1: btn_name = "PS_KEY0"; break;
        case 2: btn_name = "PS_KEY1"; break;
        case 3: btn_name = "PL_KEY0"; break;
        case 4: btn_name = "PL_KEY1"; break;
        default: btn_name = "UNKNOWN"; break;
    }

    sprintf(buffer, "BUTTON:%s:%d\r\n", btn_name, state);
    tcp_control_send_response(buffer);
    xil_printf("[TCP] Sent: %s\r\n", buffer);
}

/*
 * Send volume status to client (text format)
 */
void tcp_control_send_volume_status(u16_t volume)
{
    char buffer[32];

    if (!client.connected) return;

    sprintf(buffer, "VOLUME:%d%%\r\n", volume);
    tcp_control_send_response(buffer);
    xil_printf("[TCP] Sent: %s\r\n", buffer);
}

/*
 * Send play status to client (text format)
 */
void tcp_control_send_play_status(u8_t state)
{
    char buffer[64];
    const char *state_str = state ? "PLAYING" : "STOPPED";

    if (!client.connected) return;

    sprintf(buffer, "MUSIC:%s\r\n", state_str);
    tcp_control_send_response(buffer);
    xil_printf("[TCP] Sent: %s\r\n", buffer);
}

/*
 * Send all status to client
 */
void tcp_control_send_all_status(void)
{
    char buffer[128];

    if (!client.connected) return;

    /* Send button states (with proper formatting for GUI parsing) */
    u8_t ps_key0 = XGpioPs_ReadPin(&Gpio, PS_KEY0);
    u8_t ps_key1 = XGpioPs_ReadPin(&Gpio, PS_KEY1);
    u8_t pl_key0 = XGpioPs_ReadPin(&Gpio, PL_KEY0);
    u8_t pl_key1 = XGpioPs_ReadPin(&Gpio, PL_KEY1);

    /* Send button statuses */
    sprintf(buffer, "BUTTON:PS_KEY0:%d\r\n", !ps_key0);  /* Invert because buttons are active low */
    tcp_control_send_response(buffer);

    sprintf(buffer, "BUTTON:PS_KEY1:%d\r\n", !ps_key1);
    tcp_control_send_response(buffer);

    sprintf(buffer, "BUTTON:PL_KEY0:%d\r\n", !pl_key0);
    tcp_control_send_response(buffer);

    sprintf(buffer, "BUTTON:PL_KEY1:%d\r\n", !pl_key1);
    tcp_control_send_response(buffer);

    /* Send LED states */
//    u8_t ps_led0 = XGpioPs_ReadPin(&Gpio, PS_LED0);
//    u8_t ps_led1 = XGpioPs_ReadPin(&Gpio, PS_LED1);
//    u8_t pl_led0 = XGpioPs_ReadPin(&Gpio, PL_LED0);
//    u8_t pl_led1 = XGpioPs_ReadPin(&Gpio, PL_LED1);

//    sprintf(buffer, "LED:PS_LED0:%d\r\n", ps_led0);
//    tcp_control_send_response(buffer);
//
//    sprintf(buffer, "LED:PS_LED1:%d\r\n", ps_led1);
//    tcp_control_send_response(buffer);
//
//    sprintf(buffer, "LED:PL_LED0:%d\r\n", pl_led0);
//    tcp_control_send_response(buffer);
//
//    sprintf(buffer, "LED:PL_LED1:%d\r\n", pl_led1);
//    tcp_control_send_response(buffer);


    /* Send LED states - USE THE FLAGS instead of reading pins */
   sprintf(buffer, "LED:PS_LED0:%d\r\n", PS_LED_0_flag);  /* Use flag, not pin read */
   tcp_control_send_response(buffer);

   sprintf(buffer, "LED:PS_LED1:%d\r\n", PS_LED_1_flag);  /* Use flag, not pin read */
   tcp_control_send_response(buffer);

   sprintf(buffer, "LED:PL_LED0:%d\r\n", PL_LED_0_flag);  /* Use flag, not pin read */
   tcp_control_send_response(buffer);

   sprintf(buffer, "LED:PL_LED1:%d\r\n", PL_LED_1_flag);  /* Use flag, not pin read */
   tcp_control_send_response(buffer);

    /* Send volume status */
    int volume = audio_volume_get(&AudioInst, &ps_i2c0);
    sprintf(buffer, "VOLUME:%d%%\r\n", volume);
    tcp_control_send_response(buffer);

    /* Send play status */
    int state = XAxiDma_Audio_GetState();
    sprintf(buffer, "MUSIC:%s\r\n", state == STATE_PLAYING ? "PLAYING" : "STOPPED");
    tcp_control_send_response(buffer);

    //xil_printf("[TCP] Sent all status to client\r\n");
}

/*
 * Process text command from client
 */
static void tcp_control_process_command(char *cmd)
{
    char *token;
    char *saveptr;
    char msg[256];

    if(strcmp(cmd, "STATUS") == 0) xil_printf("[TCP] Received command: %s3\r\n", cmd);

    /* Parse command */
    token = strtok_r(cmd, " \r\n", &saveptr);

    if (token == NULL) {
        tcp_control_send_response("ERROR: Empty command\r\n");
        return;
    }

    /* STATUS command */
    if (strcmp(token, "STATUS") == 0 || strcmp(token, "status") == 0) {
        tcp_control_send_all_status();
        tcp_control_send_response("OK:Status sent\r\n");
    }

//    /* PLAY command */
//    else if (strcmp(token, "PLAY") == 0 || strcmp(token, "play") == 0) {
//        extern u32 WavLength;
//        extern u32 WriteBuf[];
//
//        int current_state = XAxiDma_Audio_GetState();
//        if (current_state != STATE_PLAYING) {
//            XAxiDma_Audio_Play_Start(&AudioInst, WavLength, WriteBuf);
//            XGpioPs_WritePin(&Gpio, PS_LED1, LED_ON);
//            PS_LED_1_flag = 1;  /* Update flag */
//
//            sprintf(msg, "LED:PS_LED1:1\r\n");
//            tcp_control_send_response(msg);
//
//            tcp_control_send_response("OK:Play started\r\n");
//            tcp_control_send_play_status(1);
//        } else {
//            tcp_control_send_response("OK:Already playing\r\n");
//        }
//    }



    /* PLAY command */
    else if (strcmp(token, "PLAY") == 0 || strcmp(token, "play") == 0) {
        extern u32 WavLength;
        extern u32 WriteBuf[];
        extern char g_current_music_file[64];

        int current_state = XAxiDma_Audio_GetState();
        if (current_state != STATE_PLAYING) {
            /* Check if we have a valid file loaded */
            if (WavLength > 0) {
                XAxiDma_Audio_Play_Start(&AudioInst, WavLength, WriteBuf);
                XGpioPs_WritePin(&Gpio, PS_LED1, LED_ON);
                PS_LED_1_flag = 1;  /* Update flag */

                sprintf(msg, "LED:PS_LED1:1\r\n");
                tcp_control_send_response(msg);

                /* Also send which file is playing */
                sprintf(msg, "PLAYING_FILE:%s\r\n", g_current_music_file);
                tcp_control_send_response(msg);

                tcp_control_send_response("OK:Play started\r\n");
                tcp_control_send_play_status(1);
            } else {
                tcp_control_send_response("ERROR:No audio file loaded\r\n");
            }
        } else {
            tcp_control_send_response("OK:Already playing\r\n");
        }
    }



    /* STOP command */
    else if (strcmp(token, "STOP") == 0 || strcmp(token, "stop") == 0) {
        int current_state = XAxiDma_Audio_GetState();
        if (current_state == STATE_PLAYING) {
            XAxiDma_Audio_Play_Stop();
            XGpioPs_WritePin(&Gpio, PS_LED1, LED_OFF);
            PS_LED_1_flag = 0;  /* Update flag */

            sprintf(msg, "LED:PS_LED1:0\r\n");
            tcp_control_send_response(msg);

            tcp_control_send_response("OK:Play stopped\r\n");
            tcp_control_send_play_status(0);
        } else {
            tcp_control_send_response("OK:Already stopped\r\n");
        }
    }

    /* VOLUME command */
    else if (strcmp(token, "VOLUME") == 0 || strcmp(token, "volume") == 0) {
        token = strtok_r(NULL, " \r\n", &saveptr);
        if (token) {
            int volume = atoi(token);
            int previous_volume = audio_volume_get(&AudioInst, &ps_i2c0);
            char led_buf[64];

            /* Determine which button was pressed based on volume change direction */
            if (volume > previous_volume) {
                /* Volume Up - Turn on PL_LED0 */
                XGpioPs_WritePin(&Gpio, PL_LED0, LED_ON);
                PL_LED_0_flag = 1;
                /* Send LED status update */
                sprintf(led_buf, "LED:PL_LED0:1\r\n");
                tcp_control_send_response(led_buf);

                // Turn off LED after 100 ms
                usleep(100000);
                XGpioPs_WritePin(&Gpio, PL_LED0, LED_OFF);
                PL_LED_0_flag = 0;
                sprintf(led_buf, "LED:PL_LED0:0\r\n");
				tcp_control_send_response(led_buf);
            }
            else if (volume < previous_volume) {
                /* Volume Down - Turn on PL_LED1 */
                XGpioPs_WritePin(&Gpio, PL_LED1, LED_ON);
                PL_LED_1_flag = 1;
                /* Send LED status update */
                sprintf(led_buf, "LED:PL_LED1:1\r\n");
                tcp_control_send_response(led_buf);

                // Turn off LED after 100 ms
				usleep(100000);
				XGpioPs_WritePin(&Gpio, PL_LED1, LED_OFF);
				PL_LED_1_flag = 0;
				sprintf(led_buf, "LED:PL_LED0:0\r\n");
				tcp_control_send_response(led_buf);
            }

            /* Clamp volume to 0-100 range */
            if (volume < 0) volume = 0;
            if (volume > 100) volume = 100;

            /* Convert percentage to hardware volume */
            int converted_volume = (double)volume / 100 * (VOLUME_MAX - VOLUME_MIN) + VOLUME_MIN;
            audio_volume_set(&AudioInst, &ps_i2c0, converted_volume);

            /* Get actual volume set (might be rounded) */
            int actual_volume = audio_volume_get(&AudioInst, &ps_i2c0);

            sprintf(msg, "OK:Volume set to %d%%\r\n", actual_volume);
            tcp_control_send_response(msg);
            tcp_control_send_volume_status(actual_volume);

        } else {
            tcp_control_send_response("ERROR:Usage VOLUME <0-100>\r\n");
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
                int led_state = state ? LED_ON : LED_OFF;

                if (strcmp(led_name, "PS_LED0") == 0) {
                    XGpioPs_WritePin(&Gpio, PS_LED0, led_state);
                    PS_LED_0_flag = state;  /* Update flag */

                    sprintf(msg, "LED:PS_LED0:%d\r\n", state);
                    tcp_control_send_response(msg);
                    xil_printf("[TCP] LED PS_LED0 set to %d\r\n", state);
                }
                else if (strcmp(led_name, "PS_LED1") == 0) {
                    XGpioPs_WritePin(&Gpio, PS_LED1, led_state);
                    PS_LED_1_flag = state;  /* Update flag */

                    sprintf(msg, "LED:PS_LED1:%d\r\n", state);
                    tcp_control_send_response(msg);
                    xil_printf("[TCP] LED PS_LED1 set to %d\r\n", state);
                }
                else if (strcmp(led_name, "PL_LED0") == 0) {
                    XGpioPs_WritePin(&Gpio, PL_LED0, led_state);
                    PL_LED_0_flag = state;  /* Update flag */

                    sprintf(msg, "LED:PL_LED0:%d\r\n", state);
                    tcp_control_send_response(msg);
                    xil_printf("[TCP] LED PL_LED0 set to %d\r\n", state);
                }
                else if (strcmp(led_name, "PL_LED1") == 0) {
                    XGpioPs_WritePin(&Gpio, PL_LED1, led_state);
                    PL_LED_1_flag = state;  /* Update flag */

                    sprintf(msg, "LED:PL_LED1:%d\r\n", state);
                    tcp_control_send_response(msg);
                    xil_printf("[TCP] LED PL_LED1 set to %d\r\n", state);
                }
                else {
                    tcp_control_send_response("ERROR:Unknown LED\r\n");
                }
            }
        } else {
            tcp_control_send_response("ERROR:Usage LED <name> ON/OFF\r\n");
        }
    }

    /* HELP command */
    else if (strcmp(token, "HELP") == 0 || strcmp(token, "help") == 0) {
        tcp_control_send_response(
            "Commands:\r\n"
            "  STATUS              - Get all status\r\n"
            "  PLAY                - Start playback\r\n"
            "  STOP                - Stop playback\r\n"
            "  VOLUME <0-100>      - Set volume\r\n"
            "  LED <name> ON/OFF   - Control LED (PS_LED0/1, PL_LED0/1)\r\n"
            "  HELP                - Show this help\r\n"
            "  QUIT                - Disconnect\r\n");
    }

    /* QUIT command */
    else if (strcmp(token, "QUIT") == 0 || strcmp(token, "quit") == 0) {
        tcp_control_send_response("OK:Goodbye\r\n");
        if (client.connected && client.pcb) {
            xil_printf("[TCP] Client disconnected\r\n");
            tcp_close(client.pcb);
            client.pcb = NULL;
            client.connected = 0;
        }
    }

    /* Unknown command */
    else {
        char err_msg[64];
        sprintf(err_msg, "ERROR:Unknown command '%s'\r\n", token);
        tcp_control_send_response(err_msg);
    }
}

/*
 * TCP receive callback
 */
static err_t tcp_control_recv_cb(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    char cmd[256];
    int len;

    if (!p) {
        /* Connection closed by client */
        xil_printf("[TCP] Client disconnected\r\n");
        client.connected = 0;
        client.pcb = NULL;
        tcp_close(tpcb);
        return ERR_OK;
    }

    /* Copy received data */
    len = p->len > 255 ? 255 : p->len;
    memcpy(cmd, p->payload, len);
    cmd[len] = '\0';

    /* Process command */
    tcp_control_process_command(cmd);

    /* Free pbuf */
    pbuf_free(p);

    return ERR_OK;
}

/*
 * TCP accept callback - called when new client connects
 */
static err_t tcp_control_accept_cb(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    xil_printf("[TCP] New client connected\r\n");

    /* Disconnect previous client if any */
    if (client.connected && client.pcb) {
        xil_printf("[TCP] Disconnecting previous client\r\n");
        tcp_close(client.pcb);
        client.connected = 0;
        client.pcb = NULL;
    }

    /* Store new client */
    client.pcb = newpcb;
    client.connected = 1;

    /* Set receive callback */
    tcp_recv(newpcb, tcp_control_recv_cb);

    /* Send welcome message */
    tcp_control_send_response("Connected to Zynq Music Player\r\n");
    tcp_control_send_response("Type HELP for commands\r\n");

    /* Send initial status */
    tcp_control_send_all_status();

    return ERR_OK;
}

/*
 * Initialize TCP control server
 */
void tcp_control_init(void)
{
    struct tcp_pcb *pcb;
    err_t err;

    /* Create TCP PCB */
    pcb = tcp_new();
    if (!pcb) {
        xil_printf("[TCP] Error creating PCB\r\n");
        return;
    }

    /* Bind to control port */
    err = tcp_bind(pcb, IP_ADDR_ANY, TCP_CONTROL_PORT);
    if (err != ERR_OK) {
        xil_printf("[TCP] Unable to bind to port %d\r\n", TCP_CONTROL_PORT);
        tcp_close(pcb);
        return;
    }

    /* Start listening */
    pcb = tcp_listen(pcb);
    if (!pcb) {
        xil_printf("[TCP] Error starting listen\r\n");
        return;
    }

    /* Set accept callback */
    tcp_accept(pcb, tcp_control_accept_cb);

    xil_printf("[TCP] Control server listening on port %d\r\n", TCP_CONTROL_PORT);
}
