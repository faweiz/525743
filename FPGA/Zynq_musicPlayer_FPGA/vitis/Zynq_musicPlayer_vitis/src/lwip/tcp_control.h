/*
 * tcp_control.h
 * Simple text-based TCP protocol for button and LED status
 */

#ifndef TCP_CONTROL_H_
#define TCP_CONTROL_H_

#include "lwip/tcp.h"
#include "xil_printf.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Custom TCP port for control messages */
#define TCP_CONTROL_PORT       5002

#define VOLUME_MIN 			WM8960_VOLUME_MIN             //  -73.5dB 0x30 (mute)
#define VOLUME_MAX 			WM8960_VOLUME_MAX            //   +6.0dB 0x7F
#define VOLUME_DEFAULT 		WM8960_VOLUME_DEFAULT        // ~ +5.0dB 0x79

// LED
extern u8 PS_LED_0_flag;
extern u8 PS_LED_1_flag;
extern u8 PL_LED_0_flag;
extern u8 PL_LED_1_flag;

//// Button
extern u8 PS_KEY_0_flag;
extern u8 PS_KEY_1_flag;
extern u8 PL_KEY_0_flag;
extern u8 PL_KEY_1_flag;


/* Function prototypes */
void tcp_control_init(void);
void tcp_control_send_button_status(u8_t button_id, u8_t state);
void tcp_control_send_volume_status(u16_t volume);
void tcp_control_send_play_status(u8_t state);
void tcp_control_send_all_status(void);

err_t tcp_control_send_response(const char *response);

#ifdef __cplusplus
}
#endif

#endif /* TCP_CONTROL_H_ */
