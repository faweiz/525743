/*
 * udp_control.h
 *
 * Custom UDP protocol for button and LED status communication
 */

#ifndef UDP_CONTROL_H_
#define UDP_CONTROL_H_

#include "lwip/udp.h"
#include "lwip/ip_addr.h"
#include "xil_printf.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Custom UDP port for control messages (different from TFTP port) */
#define UDP_CONTROL_PORT       6969

/* Message types */
#define MSG_TYPE_BUTTON_STATUS 0x01
#define MSG_TYPE_LED_STATUS    0x02
#define MSG_TYPE_VOLUME_STATUS 0x03
#define MSG_TYPE_PLAY_STATUS   0x04
#define MSG_TYPE_COMMAND       0x05

/* Command types from host */
#define CMD_GET_ALL_STATUS     0x01
#define CMD_SET_LED           0x02
#define CMD_SET_VOLUME        0x03
#define CMD_PLAY_STOP         0x04

/* Response codes */
#define RESP_SUCCESS          0x00
#define RESP_ERROR           0xFF

/* Button IDs - match your definitions */
#define BUTTON_PS_KEY0        0x01
#define BUTTON_PS_KEY1        0x02
#define BUTTON_PL_KEY0        0x03
#define BUTTON_PL_KEY1        0x04

/* LED IDs - match your definitions */
#define LED_PS_LED0          0x01
#define LED_PS_LED1          0x02
#define LED_PL_LED0          0x03
#define LED_PL_LED1          0x04

#define VOLUME_MIN 			WM8960_VOLUME_MIN             //  -73.5dB 0x30 (mute)
#define VOLUME_MAX 			WM8960_VOLUME_MAX            //   +6.0dB 0x7F
#define VOLUME_DEFAULT 		WM8960_VOLUME_DEFAULT        // ~ +5.0dB 0x79

/* Message structure */
typedef struct __attribute__((packed)) {
    u8_t msg_type;
    u8_t msg_id;           /* For request/response matching */
    u8_t data_len;
    u8_t data[256];
} udp_control_msg_t;

/* Button status message */
typedef struct __attribute__((packed)) {
    u8_t button_id;
    u8_t state;            /* 0 = released, 1 = pressed */
} button_status_t;

/* LED status message */
typedef struct __attribute__((packed)) {
    u8_t led_id;
    u8_t state;            /* 0 = off, 1 = on */
} led_status_t;

/* Volume status message */
typedef struct __attribute__((packed)) {
    u16_t volume;          /* 0-255 */
} volume_status_t;

/* Play status message */
typedef struct __attribute__((packed)) {
    u8_t state;            /* 0 = stopped, 1 = playing */
    u32_t position;        /* Current position in bytes */
    u32_t total;           /* Total length in bytes */
} play_status_t;

/* Command message from host */
typedef struct __attribute__((packed)) {
    u8_t command;
    u8_t param[8];
} command_t;

/* Function prototypes */
void udp_control_init(void);
void udp_control_send_button_status(u8_t button_id, u8_t state);
void udp_control_send_led_status(u8_t led_id, u8_t state);
void udp_control_send_volume_status(u16_t volume);
void udp_control_send_play_status(u8_t state, u32_t position, u32_t total);
void udp_control_send_all_status(void);

/* External variables to access from main */
extern ip_addr_t g_host_addr;
extern u16_t g_host_port;
extern int g_host_connected;

#ifdef __cplusplus
}
#endif

#endif /* UDP_CONTROL_H_ */
