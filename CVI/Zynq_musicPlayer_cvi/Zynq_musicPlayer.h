/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  MAINPNL                          1       /* callback function: MainPanelCB */
#define  MAINPNL_DIRECTORY                2       /* control type: string, callback function: (none) */
#define  MAINPNL_LISTBOX                  3       /* control type: listBox, callback function: SoundListCallback */
#define  MAINPNL_CHANGEDIR                4       /* control type: command, callback function: ChangeDir */
#define  MAINPNL_PLAYMUSIC                5       /* control type: command, callback function: PlayMusicBtnCB */
#define  MAINPNL_PLAYONE                  6       /* control type: command, callback function: PlayOne */
#define  MAINPNL_STOP                     7       /* control type: command, callback function: Stop */
#define  MAINPNL_QUIT                     8       /* control type: command, callback function: Quit */
#define  MAINPNL_TRANSMIT                 9       /* control type: textBox, callback function: (none) */
#define  MAINPNL_STRING                   10      /* control type: string, callback function: TransmitCB */
#define  MAINPNL_RECEIVE                  11      /* control type: textBox, callback function: (none) */
#define  MAINPNL_CONNECTED                12      /* control type: LED, callback function: (none) */
#define  MAINPNL_CLEAR                    13      /* control type: command, callback function: ClearScreenCB */
#define  MAINPNL_CLIENT_NAME              14      /* control type: string, callback function: (none) */
#define  MAINPNL_CLIENT_IP                15      /* control type: string, callback function: (none) */
#define  MAINPNL_SERVER_NAME              16      /* control type: string, callback function: (none) */
#define  MAINPNL_SERVER_IP                17      /* control type: string, callback function: (none) */
#define  MAINPNL_DECORATION_3             18      /* control type: deco, callback function: (none) */
#define  MAINPNL_DECORATION_4             19      /* control type: deco, callback function: (none) */
#define  MAINPNL_DECORATION_2             20      /* control type: deco, callback function: (none) */
#define  MAINPNL_DECORATION               21      /* control type: deco, callback function: (none) */
#define  MAINPNL_TEXTMSG                  22      /* control type: textMsg, callback function: (none) */
#define  MAINPNL_PS_LED_1                 23      /* control type: LED, callback function: (none) */
#define  MAINPNL_PS_LED_0                 24      /* control type: LED, callback function: (none) */
#define  MAINPNL_PL_LED_1                 25      /* control type: LED, callback function: (none) */
#define  MAINPNL_PL_LED_0                 26      /* control type: LED, callback function: (none) */
#define  MAINPNL_VOLUME                   27      /* control type: scale, callback function: VolSliderCB */
#define  MAINPNL_PS_BTN_1                 28      /* control type: LED, callback function: PlayStopBtnCB */
#define  MAINPNL_PS_BTN_0                 29      /* control type: LED, callback function: PS_LED_BTN_CB */
#define  MAINPNL_TEXTMSG_3                30      /* control type: textMsg, callback function: (none) */
#define  MAINPNL_TEXTMSG_2                31      /* control type: textMsg, callback function: (none) */
#define  MAINPNL_PL_BTN_1                 32      /* control type: command, callback function: VolDownBtnCB */
#define  MAINPNL_PL_BTN_0                 33      /* control type: command, callback function: VolUpBtnCB */
#define  MAINPNL_FILESTATUS               34      /* control type: textMsg, callback function: (none) */
#define  MAINPNL_TIMER                    35      /* control type: timer, callback function: RequestStatusCB */

#define  WAIT                             2
#define  WAIT_TEXTMSG                     2       /* control type: textMsg, callback function: (none) */


     /* Control Arrays: */

          /* (no control arrays in the resource file) */


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */

int  CVICALLBACK ChangeDir(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ClearScreenCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MainPanelCB(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PlayMusicBtnCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PlayOne(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PlayStopBtnCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PS_LED_BTN_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK Quit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK RequestStatusCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SoundListCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK Stop(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK TransmitCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK VolDownBtnCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK VolSliderCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK VolUpBtnCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
