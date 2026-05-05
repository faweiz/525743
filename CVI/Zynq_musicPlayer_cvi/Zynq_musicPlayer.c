//==============================================================================
//
// Title:		Zynq_musicPlayer.c
// Purpose:		ZYNQ Music Player Control By LabWindows/CVI over Ethernet
//
// Created on:	3/14/2026 at 11:56:51 AM by fawei zhang.
// Copyright:	personal. All Rights Reserved.
//
//==============================================================================

//==============================================================================
// Include files

#include "Zynq_musicPlayer.h"

//==============================================================================
// Constants

//==============================================================================
// Types

//==============================================================================
// Static global variables

//==============================================================================
// Static functions

//==============================================================================
// Global variables

//==============================================================================
// Global functions

/*---------------------------------------------------------------------------*/
/* Combined TCP Client with Sound Player                                    */
/*---------------------------------------------------------------------------*/

#include <cvintwrk.h>
#include <cvirte.h>
#include <stdio.h>
#include <stdlib.h>
#include <tcpsupp.h>
#include <string.h>
#include <utility.h>
#include <userint.h>
//#include <windows.h>
//#include <mmsystem.h>
#include "Zynq_music.h" 
#include "playsnd.h"  /* Keep this for sound functions */

/*---------------------------------------------------------------------------*/
/* Macros                                                                   */
/*---------------------------------------------------------------------------*/
#define tcpChk(f) if ((g_TCPError=(f)) < 0) {ReportTCPError(); goto Done;} else
#define TCP_CONTROL_PORT       5002
#define TCP_CONTROL_IP         "192.168.68.11"
//#define MAX_PATHNAME_LEN       256
//#define MAX_FILENAME_LEN       128
#define TRUE    1
#define FALSE   0

/*---------------------------------------------------------------------------*/
/* Module-globals                                                           */
/*---------------------------------------------------------------------------*/
static unsigned int g_hconversation;
static int          g_hmainPanel, wait_panel;
static int          g_connected = 0;
static int          g_TCPError = 0;
static SoundStatusType soundStatus;
static char* defaultDirs[] = {"C:\\Users\\fawei\\Downloads\\z20\\525743\\music",
							  "C:\\Users\\fawei\\Downloads\\z20\\music",};

static char g_selectedWavFile[MAX_PATHNAME_LEN] = "";
//static int g_uploadInProgress = 0;
//static int g_tftpProcessId = -1;
int exe_handle = -1;

/*---------------------------------------------------------------------------*/
/* Internal function prototypes                                             */
/*---------------------------------------------------------------------------*/
static void ReportTCPError(void);

/*---------------------------------------------------------------------------*/
/* Main Entry Point - Combined Application                                  */
/*---------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    int portNum;
    char tempBuf[256] = {0};
    char initialDirectory[MAX_PATHNAME_LEN];
    
    if (InitCVIRTE(0, argv, 0) == 0)
        return -1;
    
    /* Load the combined panel (you need to create this UIR file) */
    if ((g_hmainPanel = LoadPanel(0, "Zynq_musicPlayer.uir", MAINPNL)) < 0)
        goto Done;
	if ((wait_panel   = LoadPanel(0, "Zynq_musicPlayer.uir", WAIT)) < 0)
		return -1;
    
    DisableBreakOnLibraryErrors();

    /* Initialize sound player */
    memset(&soundStatus, 0, sizeof(soundStatus));
    GetInitialDirectory(initialDirectory);
    SetCtrlVal(g_hmainPanel, MAINPNL_DIRECTORY, initialDirectory);
    FillListBoxWithWavsInDir(g_hmainPanel, MAINPNL_LISTBOX, initialDirectory);

    /* TCP Connection setup */
    sprintf(tempBuf, "%s", TCP_CONTROL_IP);
    portNum = TCP_CONTROL_PORT;

    /* Attempt to connect to TCP server */
    SetWaitCursor(1);
    if (ConnectToTCPServer(&g_hconversation, portNum, tempBuf, ClientTCPCB,
                           NULL, 5000) < 0)
        MessagePopup("TCP Client", "Connection to server failed !");
    else
    {
        SetWaitCursor(0);
        g_connected = 1;
        
        /* Update connection info */
        SetCtrlVal(g_hmainPanel, MAINPNL_CONNECTED, 1);
        if (GetTCPHostAddr(tempBuf, 256) >= 0)
            SetCtrlVal(g_hmainPanel, MAINPNL_CLIENT_IP, tempBuf);
        if (GetTCPHostName(tempBuf, 256) >= 0)
            SetCtrlVal(g_hmainPanel, MAINPNL_CLIENT_NAME, tempBuf);
        tcpChk(GetTCPPeerAddr(g_hconversation, tempBuf, 256));
        SetCtrlVal(g_hmainPanel, MAINPNL_SERVER_IP, tempBuf);
        
        /* Display panel and run UI */
        DisplayPanel(g_hmainPanel);
        SetActiveCtrl(g_hmainPanel, MAINPNL_STRING);
        
        /* Request initial status from server */
        char initCmd[] = "STATUS\n";
        ClientTCPWrite(g_hconversation, initCmd, strlen(initCmd), 1000);
        
        RunUserInterface();
    }
    
Done:
    /* Disconnect from TCP server */
    if (g_connected)
        DisconnectFromTCPServer(g_hconversation);

    /* Free resources */
    DiscardPanel(g_hmainPanel);
    CloseCVIRTE();
    return 0;
}

/*---------------------------------------------------------------------------*/
/* TCP Client Callback                                                      */
/*---------------------------------------------------------------------------*/
int CVICALLBACK ClientTCPCB(unsigned handle, int event, int error, void *callbackData)
{
    char receiveBuf[256] = {0};
    ssize_t dataSize = sizeof(receiveBuf) - 1;

    switch (event)
    {
        case TCP_DATAREADY:
            if ((dataSize = ClientTCPRead(g_hconversation, receiveBuf,
                                         dataSize, 1000)) < 0)
            {
                SetCtrlVal(g_hmainPanel, MAINPNL_RECEIVE, "Receive Error\n");
            }
            else
            {
                receiveBuf[dataSize] = '\0';
				// Hide the "status" message
				if (strstr(receiveBuf, "OK:Status sent") == NULL)
                {
                    SetCtrlVal(g_hmainPanel, MAINPNL_RECEIVE, receiveBuf); 
                }
				
                TCPReceive_StatusUpdate(receiveBuf);
                
                /* Handle any sound-related commands from server */
                if (strstr(receiveBuf, "PLAY_SOUND") != NULL)
                {
                    /* Parse sound filename and play */
                    //char soundFile[MAX_PATHNAME_LEN] = "";
                    /* Extract filename from command and play */
                    PlaySingleSound(g_hmainPanel, MAINPNL_LISTBOX);
                }
            }
            break;
            
        case TCP_DISCONNECT:
            MessagePopup("TCP Client", "Server has closed connection!");
            SetCtrlVal(g_hmainPanel, MAINPNL_CONNECTED, 0);
            g_connected = 0;
            MainPanelCB(0, EVENT_CLOSE, 0, 0, 0);
            break;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Transmit Callback - Send text to server                                  */
/*---------------------------------------------------------------------------*/
int CVICALLBACK TransmitCB(int panelHandle, int controlID, int event,
                          void *callbackData, int eventData1, int eventData2)
{
    char transmitBuf[512] = {0};

    switch (event)
    {
        case EVENT_COMMIT:
            GetCtrlVal(panelHandle, MAINPNL_STRING, transmitBuf);
            strcat(transmitBuf, "\n");
            SetCtrlVal(panelHandle, MAINPNL_TRANSMIT, transmitBuf);
            SetCtrlVal(panelHandle, MAINPNL_STRING, "");
            if (ClientTCPWrite(g_hconversation, transmitBuf,
                              strlen(transmitBuf), 1000) < 0)
                SetCtrlVal(panelHandle, MAINPNL_TRANSMIT, "Transmit Error\n");
            break;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Clear Screen Callback                                                    */
/*---------------------------------------------------------------------------*/
int CVICALLBACK ClearScreenCB(int panel, int control, int event,
                             void *callbackData, int eventData1, int eventData2)
{
    if (event == EVENT_COMMIT)
        ResetTextBox(panel, MAINPNL_RECEIVE, "");
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Main Panel Callback - Handle panel closure                               */
/*---------------------------------------------------------------------------*/
int CVICALLBACK MainPanelCB(int panel, int event, void *callbackData,
                           int eventData1, int eventData2)
{
    if (event == EVENT_CLOSE)
        QuitUserInterface(0);
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Quit Callback                                                            */
/*---------------------------------------------------------------------------*/
int CVICALLBACK Quit(int panel, int control, int event, void *callbackData,
                    int eventData1, int eventData2)
{
    switch (event)
    {
        case EVENT_COMMIT:
			if (g_connected) DisconnectFromTCPServer(g_hconversation);
            QuitUserInterface(0);
            break;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/
/* LED Button Callback                                                      */
/*---------------------------------------------------------------------------*/
int CVICALLBACK PS_LED_BTN_CB(int panel, int control, int event,
                              void *callbackData, int eventData1, int eventData2)
{
    char transmitBuf[512] = {0};
    char LEDBTN_NAME[20] = "";
    char LEDBTN_NAME_Str[20] = "";
    unsigned short NumVal = 0;
    int LEDBTN_OpenCloseVal = 0;

    switch (event)
    {
        case EVENT_COMMIT:
            GetCtrlAttribute(panel, control, ATTR_CONSTANT_NAME, LEDBTN_NAME);
            sprintf(LEDBTN_NAME_Str, "%s", (LEDBTN_NAME + 7));
            NumVal = (unsigned short)atoi(LEDBTN_NAME_Str);
            
            GetCtrlAttribute(panel, control, ATTR_CTRL_VAL, &LEDBTN_OpenCloseVal); 
            if(LEDBTN_OpenCloseVal == 1)
            {		   									 
                if(strncmp(LEDBTN_NAME, "PS", 2) == 0) 
                    sprintf(transmitBuf, "LED PS_LED%d ON", NumVal);
                else if(strncmp(LEDBTN_NAME, "PL", 2) == 0) 
                    sprintf(transmitBuf, "LED PL_LED%d ON", NumVal); 
            }
            else
            {		   
                if(strncmp(LEDBTN_NAME, "PS", 2) == 0) 
                    sprintf(transmitBuf, "LED PS_LED%d OFF", NumVal);
                else if(strncmp(LEDBTN_NAME, "PL", 2) == 0) 
                    sprintf(transmitBuf, "LED PL_LED%d OFF", NumVal); 
            } 
                
            strcat(transmitBuf, "\n");
            SetCtrlVal(panel, MAINPNL_TRANSMIT, transmitBuf);
            SetCtrlVal(panel, MAINPNL_STRING, "");
            if (ClientTCPWrite(g_hconversation, transmitBuf,
                              strlen(transmitBuf), 1000) < 0)
                SetCtrlVal(panel, MAINPNL_TRANSMIT, "Transmit Error\n");
            break;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Play/Stop Button Callback                                                */
/*---------------------------------------------------------------------------*/
int CVICALLBACK PlayStopBtnCB(int panel, int control, int event,
                              void *callbackData, int eventData1, int eventData2)
{
    char transmitBuf[512] = {0};
    int current_state = 0;

    switch (event)
    {
        case EVENT_COMMIT:
            GetCtrlAttribute(panel, control, ATTR_CTRL_VAL, &current_state); 
            
            if (current_state == 1) {
                sprintf(transmitBuf, "PLAY");
            } else {
                sprintf(transmitBuf, "STOP");
            }
            							   
            strcat(transmitBuf, "\n");
            SetCtrlVal(panel, MAINPNL_TRANSMIT, transmitBuf);
            
            if (ClientTCPWrite(g_hconversation, transmitBuf,
                              strlen(transmitBuf), 1000) < 0)
                SetCtrlVal(panel, MAINPNL_TRANSMIT, "Transmit Error\n");
            break;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Volume Up Button Callback                                                */
/*---------------------------------------------------------------------------*/
int CVICALLBACK VolUpBtnCB(int panel, int control, int event,
                          void *callbackData, int eventData1, int eventData2)
{
    char transmitBuf[512] = {0};
    int current_volume = 0;

    switch (event)
    {
        case EVENT_COMMIT:
            GetCtrlVal(panel, MAINPNL_VOLUME, &current_volume);
            current_volume += 3;  
            if (current_volume > 100) current_volume = 100;
        
            sprintf(transmitBuf, "VOLUME %d", current_volume);
            strcat(transmitBuf, "\n");
        
            SetCtrlVal(panel, MAINPNL_TRANSMIT, transmitBuf);
            SetCtrlVal(panel, MAINPNL_VOLUME, current_volume);
            SetCtrlVal(g_hmainPanel, MAINPNL_PL_BTN_0, 0);
        
            if (ClientTCPWrite(g_hconversation, transmitBuf,
                              strlen(transmitBuf), 1000) < 0)
                SetCtrlVal(panel, MAINPNL_TRANSMIT, "Transmit Error\n");	
            break;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Volume Down Button Callback                                              */
/*---------------------------------------------------------------------------*/
int CVICALLBACK VolDownBtnCB(int panel, int control, int event,
                            void *callbackData, int eventData1, int eventData2)
{
    char transmitBuf[512] = {0};
    int current_volume = 0;
	
    switch (event)
    {
        case EVENT_COMMIT:
            GetCtrlVal(panel, MAINPNL_VOLUME, &current_volume);
            current_volume -= 2;
            if (current_volume < 0) current_volume = 0;
            
            sprintf(transmitBuf, "VOLUME %d", current_volume);
            strcat(transmitBuf, "\n");
            
            SetCtrlVal(panel, MAINPNL_TRANSMIT, transmitBuf);
            SetCtrlVal(panel, MAINPNL_VOLUME, current_volume);
			
            if (ClientTCPWrite(g_hconversation, transmitBuf,
                              strlen(transmitBuf), 1000) < 0)
                SetCtrlVal(panel, MAINPNL_TRANSMIT, "Transmit Error\n");
            break;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Volume Slider Callback                                                   */
/*---------------------------------------------------------------------------*/
int CVICALLBACK VolSliderCB(int panel, int control, int event,
                           void *callbackData, int eventData1, int eventData2)
{
    char transmitBuf[512] = {0};
    int current_volume = 0;
    
    switch (event)
    {
        case EVENT_COMMIT:	  
            GetCtrlVal(panel, MAINPNL_VOLUME, &current_volume);
           
            sprintf(transmitBuf, "VOLUME %d", current_volume);
            strcat(transmitBuf, "\n");
            
            SetCtrlVal(panel, MAINPNL_TRANSMIT, transmitBuf);
            SetCtrlVal(panel, MAINPNL_VOLUME, current_volume);
            
            if (ClientTCPWrite(g_hconversation, transmitBuf,
                              strlen(transmitBuf), 1000) < 0)
                SetCtrlVal(panel, MAINPNL_TRANSMIT, "Transmit Error\n");
            break;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Status Request Timer Callback                                            */
/*---------------------------------------------------------------------------*/
//int CVICALLBACK RequestStatusCB(int panel, int control, int event,
//                               void *callbackData, int eventData1, int eventData2)
//{
//    char transmitBuf[512] = {0};

//    switch (event)
//    {
//        case EVENT_TIMER_TICK:
//            if (g_connected)
//            {
//                sprintf(transmitBuf, "STATUS\n");
//                SetCtrlVal(panel, MAINPNL_TRANSMIT, transmitBuf);
//                
//                if (ClientTCPWrite(g_hconversation, transmitBuf,
//                                  strlen(transmitBuf), 1000) < 0)
//                    SetCtrlVal(panel, MAINPNL_TRANSMIT, "Transmit Error\n");
//            }
//            break;
//    }
//    return 0;
//}

/*---------------------------------------------------------------------------*/
/* Status Request Timer Callback - Modified to check TFTP status            */
/*---------------------------------------------------------------------------*/
int CVICALLBACK RequestStatusCB(int panel, int control, int event,
                               void *callbackData, int eventData1, int eventData2)
{
    char transmitBuf[512] = {0};

    switch (event)
    {
        case EVENT_TIMER_TICK:
            ///* Check TFTP transfer status if one is in progress */
            //if (g_tftpProcessId == 0) {
            //    CheckTFTPStatus(panel);
            //}
            
            /* Send STATUS command to Zynq */
            if (g_connected)
            {
                sprintf(transmitBuf, "STATUS\n");
                //SetCtrlVal(panel, MAINPNL_TRANSMIT, transmitBuf);
                
                if (ClientTCPWrite(g_hconversation, transmitBuf,
                                  strlen(transmitBuf), 1000) < 0)
                    SetCtrlVal(panel, MAINPNL_TRANSMIT, "Transmit Error\n");
            }
            break;
    }
    return 0;
}





/*---------------------------------------------------------------------------*/
/* Parse Helper Function                                                    */
/*---------------------------------------------------------------------------*/
int get_value(const char *buf, const char *key, char delimiter,
              parse_type_t type, void *out_value)
{
    char *pos = strstr(buf, key);
    if (!pos)
        return -1;

    pos = strchr(pos, delimiter);
    if (!pos)
        return -1;

    pos++; // move past ':' or '='

    // Skip any spaces
    while (*pos == ' ') pos++;

    if (type == PARSE_INT) {
        *(int *)out_value = atoi(pos);
    } else if (type == PARSE_FLOAT) {
        *(float *)out_value = (float)atof(pos);
    } else {
        return -1;
    }

    return 0;
}

/*---------------------------------------------------------------------------*/
/* TCP Receive Status Update                                                */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* TCP Receive Status Update                                                */
/*---------------------------------------------------------------------------*/
//int TCPReceive_StatusUpdate(char *receiveBuf)
//{
//    int ps_key_0 = 0, ps_key_1 = 0, pl_key_0 = 0, pl_key_1 = 0;
//    int volume = 0;
//    int ps_led_0 = 0, ps_led_1 = 0, pl_led_0 = 0, pl_led_1 = 0;
//    char music_state[20] = "";
//    
//    /* Parse button states */
//    get_value(receiveBuf, "PS_KEY0", ':', PARSE_INT, &ps_key_0);
//    get_value(receiveBuf, "PS_KEY1", ':', PARSE_INT, &ps_key_1);
//    get_value(receiveBuf, "PL_KEY0", ':', PARSE_INT, &pl_key_0);
//    get_value(receiveBuf, "PL_KEY1", ':', PARSE_INT, &pl_key_1);
//    
//    /* Parse LED states */
//    get_value(receiveBuf, "PS_LED0", ':', PARSE_INT, &ps_led_0);
//    get_value(receiveBuf, "PS_LED1", ':', PARSE_INT, &ps_led_1);
//    get_value(receiveBuf, "PL_LED0", ':', PARSE_INT, &pl_led_0);
//    get_value(receiveBuf, "PL_LED1", ':', PARSE_INT, &pl_led_1);
//    
//    /* Parse volume */
//    get_value(receiveBuf, "VOLUME", ':', PARSE_INT, &volume);
//    
//    /* Parse music state */
//    if (strstr(receiveBuf, "MUSIC:PLAYING")) {
//        strcpy(music_state, "PLAYING");
//        ps_led_1 = 1;  /* PS_LED1 ON when playing */
//    }
//    if (strstr(receiveBuf, "MUSIC:STOPPED")) {
//        strcpy(music_state, "STOPPED");
//        ps_led_1 = 0;  /* PS_LED1 OFF when stopped */
//    }
//    
//    /* Update Button LED indicators - FIXED: Added missing PS_BTN_1 */
//   	//SetCtrlVal(g_hmainPanel, MAINPNL_PS_BTN_0, ps_key_0);
//    /*SetCtrlVal(g_hmainPanel, MAINPNL_PS_BTN_1, ps_key_1);  // ¡û THIS WAS MISSING
//    SetCtrlVal(g_hmainPanel, MAINPNL_PL_BTN_0, pl_key_0);
//    SetCtrlVal(g_hmainPanel, MAINPNL_PL_BTN_1, pl_key_1);*/
//    
//    /* Update LED status indicators */
//    SetCtrlVal(g_hmainPanel, MAINPNL_PS_LED_0, ps_led_0);
//    SetCtrlVal(g_hmainPanel, MAINPNL_PS_LED_1, ps_led_1);
//    SetCtrlVal(g_hmainPanel, MAINPNL_PL_LED_0, pl_led_0);
//    SetCtrlVal(g_hmainPanel, MAINPNL_PL_LED_1, pl_led_1);
//    
//    /* Update volume display */
//    SetCtrlVal(g_hmainPanel, MAINPNL_VOLUME, volume);
//    
//    return 0;
//}

/*---------------------------------------------------------------------------*/
/* TCP Receive Status Update - Modified to show playing status              */
/*---------------------------------------------------------------------------*/
int TCPReceive_StatusUpdate(char *receiveBuf)
{
    int ps_key_0 = 0, ps_key_1 = 0, pl_key_0 = 0, pl_key_1 = 0;
    int volume = 0;
    int ps_led_0 = 0, ps_led_1 = 0, pl_led_0 = 0, pl_led_1 = 0;
    char music_state[20] = "";
    char statusMsg[100] = "";
    
    /* Parse button states */
    get_value(receiveBuf, "PS_KEY0", ':', PARSE_INT, &ps_key_0);
    get_value(receiveBuf, "PS_KEY1", ':', PARSE_INT, &ps_key_1);
    get_value(receiveBuf, "PL_KEY0", ':', PARSE_INT, &pl_key_0);
    get_value(receiveBuf, "PL_KEY1", ':', PARSE_INT, &pl_key_1);
    
    /* Parse LED states */
    get_value(receiveBuf, "PS_LED0", ':', PARSE_INT, &ps_led_0);
    get_value(receiveBuf, "PS_LED1", ':', PARSE_INT, &ps_led_1);
    get_value(receiveBuf, "PL_LED0", ':', PARSE_INT, &pl_led_0);
    get_value(receiveBuf, "PL_LED1", ':', PARSE_INT, &pl_led_1);
    
    /* Parse volume */
    get_value(receiveBuf, "VOLUME", ':', PARSE_INT, &volume);
    
    /* Parse music state and update status message */
    if (strstr(receiveBuf, "MUSIC:PLAYING")) {
        strcpy(music_state, "PLAYING");
        ps_led_1 = 1;  /* PS_LED1 ON when playing */
        
        /* Show which file is playing (only if we have a selected file) */
        if (strlen(g_selectedWavFile) > 0) {
            char *filenameOnly = strrchr(g_selectedWavFile, '\\');
            if (filenameOnly) {
                filenameOnly++; /* Skip the backslash */
                sprintf(statusMsg, "Playing: %s", filenameOnly);
            } else {
                sprintf(statusMsg, "Playing: %s", g_selectedWavFile);
            }
        } else {
            sprintf(statusMsg, "Playing...");
        }
    }
    if (strstr(receiveBuf, "MUSIC:STOPPED")) {
        strcpy(music_state, "STOPPED");
        ps_led_1 = 0;  /* PS_LED1 OFF when stopped */
        sprintf(statusMsg, "Stopped");
    }
    
    /* Update LED status indicators */
    SetCtrlVal(g_hmainPanel, MAINPNL_PS_LED_0, ps_led_0);
    SetCtrlVal(g_hmainPanel, MAINPNL_PS_LED_1, ps_led_1);
    SetCtrlVal(g_hmainPanel, MAINPNL_PL_LED_0, pl_led_0);
    SetCtrlVal(g_hmainPanel, MAINPNL_PL_LED_1, pl_led_1);
    
    /* Update volume display */
    SetCtrlVal(g_hmainPanel, MAINPNL_VOLUME, volume);
    
    /* Update status message if we have a control for it */
    if (strlen(statusMsg) > 0) {
        SetCtrlVal(g_hmainPanel, MAINPNL_FILESTATUS, statusMsg);
    }
    
    return 0;
}





/*---------------------------------------------------------------------------*/
/* Report TCP Errors                                                        */
/*---------------------------------------------------------------------------*/
static void ReportTCPError(void)
{
    if (g_TCPError < 0)
    {
        char messageBuffer[1024];
        sprintf(messageBuffer, 
            "TCP library error message: %s\nSystem error message: %s", 
            GetTCPErrorString(g_TCPError), GetTCPSystemErrorString());
        MessagePopup("Error", messageBuffer);
        g_TCPError = 0;
    }
}

/*===========================================================================*/
/* SOUND PLAYER FUNCTIONS (from sndplay.c)                                  */
/*===========================================================================*/

/*---------------------------------------------------------------------------*/
/* Sound List Callback                                                      */
/*---------------------------------------------------------------------------*/
int CVICALLBACK SoundListCallback(int panel, int control, int event,
                                  void *callbackData, int eventData1,
                                  int eventData2)
{
    switch (event)
    {
        case EVENT_COMMIT:
            PlaySingleSound(panel, MAINPNL_LISTBOX);
            break;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Play All Sounds                                                          */
/*---------------------------------------------------------------------------*/
int CVICALLBACK PlayAll(int panel, int control, int event,
                       void *callbackData, int eventData1, int eventData2)
{
    switch (event) 
    {
        case EVENT_COMMIT:
            PlayAllSounds(panel, MAINPNL_LISTBOX);
            break;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Play One Sound                                                           */
/*---------------------------------------------------------------------------*/
int CVICALLBACK PlayOne(int panel, int control, int event, void *callbackData,
                        int eventData1, int eventData2)
{
    switch (event)
    {
        case EVENT_COMMIT:
            PlaySingleSound(panel, MAINPNL_LISTBOX);
            break;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Get Currently Selected File                                              */
/*---------------------------------------------------------------------------*/
int GetCurrentlySelectedFile(int panel, int listbox, char *filename)
{
    int numlist;
    int index;
    
    GetCtrlIndex(panel, MAINPNL_LISTBOX, &index);
    GetNumListItems(panel, MAINPNL_LISTBOX, &numlist);
    if (numlist > 0)
        GetValueFromIndex(panel, MAINPNL_LISTBOX, index, filename);
    else
        MessagePopup("Error",
                     "No sound files in selected directory.\nSelect another directory.");
    return 0;   
}

/*---------------------------------------------------------------------------*/
/* Get Play Options                                                         */
/*---------------------------------------------------------------------------*/
int GetPlayOptions(int panel, int *asynchronous, int *loop)
{
    /*GetCtrlVal(panel, MAINPNL_LOOP, loop);
    SetCtrlAttribute(panel, MAINPNL_ASYNCHRONOUS, ATTR_DIMMED, *loop);
    if (*loop) 
        *asynchronous = 1;
    else
        GetCtrlVal(panel, MAINPNL_ASYNCHRONOUS, asynchronous);*/
	
	*loop = 0;
	*asynchronous = 1;
	
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Play Single Sound                                                        */
/*---------------------------------------------------------------------------*/
void PlaySingleSound(int panel, int listbox)
{
    char filename[MAX_PATHNAME_LEN];
    
    GetCurrentlySelectedFile(panel, listbox, filename);
    GetPlayOptions(panel, &soundStatus.async, &soundStatus.loop);
    CVI_PlaySoundEx(filename, (short)soundStatus.async, (short)soundStatus.loop, 0, 1);
    
    /* Update sound status display if you have one */
    strcpy(soundStatus.currentFile, filename);
}

/*---------------------------------------------------------------------------*/
/* Play All Sounds                                                          */
/*---------------------------------------------------------------------------*/
void PlayAllSounds(int panel, int listbox)
{
    int index;
    int count;
    char filename[MAX_PATHNAME_LEN];
    
    GetNumListItems(panel, listbox, &count);
    soundStatus.playingAll = TRUE;
    soundStatus.stopPressed = FALSE;
    
    for (index = 0; index < count; index++)
    {
        ProcessSystemEvents();
        if (soundStatus.stopPressed)
            break;
            
        GetValueFromIndex(panel, listbox, index, filename);
        SetCtrlIndex(panel, listbox, index);
        soundStatus.currentTrack = index;
        strcpy(soundStatus.currentFile, filename);
        
        ProcessSystemEvents();
        CVI_PlaySoundEx(filename, 0, 0, 0, 1);
    }
    soundStatus.playingAll = FALSE;
}

/*---------------------------------------------------------------------------*/
/* Stop Playback                                                            */
/*---------------------------------------------------------------------------*/
int CVICALLBACK Stop(int panel, int control, int event, void *callbackData,
                     int eventData1, int eventData2)
{
	char transmitBuf[512] = {0};
    int current_state = 0;

    switch (event)
    {
        case EVENT_COMMIT:
			// Stop music on PC
			if (soundStatus.playingAll)
                soundStatus.stopPressed = TRUE;
            else if (soundStatus.async || soundStatus.loop)
            {
                CVI_StopCurrentSound();
                soundStatus.async = soundStatus.loop = FALSE;
            }
			
			// Stop music on Zynq
            GetCtrlAttribute(panel, control, ATTR_CTRL_VAL, &current_state); 
            
            if (current_state == 1) {
                sprintf(transmitBuf, "PLAY");
            } else {
                sprintf(transmitBuf, "STOP");
            }
            							   
            strcat(transmitBuf, "\n");
            SetCtrlVal(panel, MAINPNL_TRANSMIT, transmitBuf);
            
            if (ClientTCPWrite(g_hconversation, transmitBuf,
                              strlen(transmitBuf), 1000) < 0)
                SetCtrlVal(panel, MAINPNL_TRANSMIT, "Transmit Error\n");
            break;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Change Directory                                                         */
/*---------------------------------------------------------------------------*/
int CVICALLBACK ChangeDir(int panel, int control, int event, 
                          void *callbackData, int eventData1, int eventData2)
{
    char currentDirectory[MAX_PATHNAME_LEN];
    char newDirectory[MAX_PATHNAME_LEN];
    
    switch (event)
    {
        case EVENT_COMMIT:
            GetCtrlVal(panel, MAINPNL_DIRECTORY, currentDirectory);
            if (DirSelectPopupEx(currentDirectory, "Select Directory",
                                 newDirectory) != VAL_NO_DIRECTORY_SELECTED)
            {
                if (strcmp(currentDirectory, newDirectory))
                {
                    SetCtrlVal(panel, MAINPNL_DIRECTORY, newDirectory);
                    FillListBoxWithWavsInDir(panel, MAINPNL_LISTBOX,
                                             newDirectory);
                }
            }
            break;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Fill ListBox with WAV Files                                              */
/*---------------------------------------------------------------------------*/
int FillListBoxWithWavsInDir(int panel, int listbox, char *directory)
{
    char fileSpec[MAX_PATHNAME_LEN];
    char fileName[MAX_FILENAME_LEN];
    char fullPath[MAX_PATHNAME_LEN];
    
    ClearListCtrl(panel, listbox);
    
    MakePathname(directory, "*.wav", fileSpec);
    if (GetFirstFile(fileSpec, 1, 0, 0, 0, 0, 0, fileName) == 0)
    {
        do
        {
            MakePathname(directory, fileName, fullPath);
            InsertListItem(panel, listbox, -1, fileName, fullPath);
        }
        while (GetNextFile(fileName) == 0);
    }
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Get Initial Directory                                                    */
/*---------------------------------------------------------------------------*/
void GetInitialDirectory(char *directory)
{
    int i, breakOnLibErrState;
    
    breakOnLibErrState = GetBreakOnLibraryErrors();
    DisableBreakOnLibraryErrors();
    
    for (i = 0; i < sizeof(defaultDirs) / sizeof(defaultDirs[0]); i++)
        if (DirectoryExists(defaultDirs[i]))
        {
            strcpy(directory, defaultDirs[i]);
            goto done;
        }
    
    DirSelectPopupEx("c:\\", "Select Waves Directory", directory);

done:
    if (breakOnLibErrState)
        EnableBreakOnLibraryErrors();
    else
        DisableBreakOnLibraryErrors();
}

/*---------------------------------------------------------------------------*/
/* Directory Exists Check                                                   */
/*---------------------------------------------------------------------------*/
int DirectoryExists(char *directory)
{   
    int dummy;
    
    if (GetFileAttrs(directory, &dummy, &dummy, &dummy, &dummy) == 1)
        return TRUE;
    else
        return FALSE;
}








///*---------------------------------------------------------------------------*/
///* Play Music Button Callback - Uses Windows TFTP to upload file to Zynq    */
///*---------------------------------------------------------------------------*/
//int CVICALLBACK PlayMusicBtnCB(int panel, int control, int event,
//                               void *callbackData, int eventData1, int eventData2)
//{
//    char tftpCommand[512] = {0};
//    char statusMsg[256] = {0};
//    char currentDir[MAX_PATHNAME_LEN] = {0};
//	char bat_file[MAX_PATHNAME_LEN] = {""};

//    switch (event)
//    {
//        case EVENT_COMMIT:
//            /* Get the currently selected WAV file */
//            if (GetCurrentlySelectedFile(panel, MAINPNL_LISTBOX, g_selectedWavFile) < 0) {
//                MessagePopup("Error", "Please select a WAV file first");
//                return 0;
//            }
//            
//            /* Check if connected to Zynq */
//            if (!g_connected) {
//                MessagePopup("Error", "Not connected to Zynq board");
//                return 0;
//            }
//            
//            /* Disable the button during upload */
//            SetCtrlAttribute(panel, MAINPNL_PLAYMUSIC, ATTR_DIMMED, 1);
//            SetCtrlVal(panel, MAINPNL_FILESTATUS, "Uploading file to Zynq via TFTP...");
//            
//            /* Get current directory to ensure we can find tftp.exe */
//            GetProjectDir(currentDir);
//			sprintf(bat_file, "%s\\launch.bat", currentDir); 
//            
//            /* Build the TFTP command */
//            /* Format: tftp -i 192.168.68.44 PUT "filename.wav" music.wav */
//            sprintf(tftpCommand, "tftp -i %s PUT %s", 
//                    TCP_CONTROL_IP, g_selectedWavFile);
//            
//            /* Log the command for debugging */
//            sprintf(statusMsg, "Executing: %s", tftpCommand);
//            SetCtrlVal(panel, MAINPNL_TRANSMIT, statusMsg);

//			// create a launch.bat with bat script
//			CreateBatScript(bat_file, tftpCommand);

//			SetDir(currentDir);
//            /* Execute TFTP command asynchronously */
//            g_uploadInProgress = 1;
//            g_tftpProcessId = LaunchExecutableEx(bat_file, LE_SHOWNORMAL, &exe_handle);
//			
//			/* Enable the button after upload */
//			SetCtrlAttribute(panel, MAINPNL_PLAYMUSIC, ATTR_DIMMED, 0);
//            
//            if (g_tftpProcessId < 0) {
//                /* Failed to launch TFTP */
//                SetCtrlVal(panel, MAINPNL_FILESTATUS, "Failed to launch TFTP!");
//                SetCtrlAttribute(panel, MAINPNL_PLAYMUSIC, ATTR_DIMMED, 0);
//                g_uploadInProgress = 0;
//                MessagePopup("Error", "Failed to launch TFTP. Make sure TFTP client is installed.");
//            } else {
//                /* Successfully launched - we'll check completion in timer callback */
//                SetCtrlVal(panel, MAINPNL_FILESTATUS, "TFTP transfer in progress...");
//                
//                /* Start a timer to check when TFTP completes */
//                /* You can use your existing timer or create a new one */
//            }
//            break;
//    }
//    return 0;
//}

///*---------------------------------------------------------------------------*/
///* Check TFTP Process Status - Call this from your timer callback           */
///*---------------------------------------------------------------------------*/
//static void CheckTFTPStatus(int panel)
//{
//    int has_terminated;
//    char transmitBuf[512] = {0};
//	
//	/* Check if process is still running */ 
//	g_uploadInProgress = ExecutableHasTerminated(exe_handle);
//    
//    if (g_uploadInProgress == 0 || g_tftpProcessId < 0)
//        return;
//    
//    /* Check if process has been terminated. */
//    has_terminated = ExecutableHasTerminated(exe_handle);
//    
//    /* if Process completed */
//    if (has_terminated) {
//        /* TFTP successful */
//        SetCtrlVal(panel, MAINPNL_FILESTATUS, "Upload complete! Starting playback...");
//        
//        /* Send PLAY command to Zynq */
//        sprintf(transmitBuf, "PLAY\n");
//        SetCtrlVal(panel, MAINPNL_TRANSMIT, transmitBuf);
//        
//        if (ClientTCPWrite(g_hconversation, transmitBuf,
//                          strlen(transmitBuf), 1000) < 0) {
//            SetCtrlVal(panel, MAINPNL_FILESTATUS, "Upload OK but PLAY command failed");
//        } else {
//            SetCtrlVal(panel, MAINPNL_FILESTATUS, "Playing music on Zynq...");
//        }
//    } else {
//        /* TFTP failed */
//        SetCtrlVal(panel, MAINPNL_FILESTATUS, "TFTP transfer failed!");
//        MessagePopup("TFTP Error", "File transfer failed. Check if TFTP server is running on Zynq.");
//    }
//    
//    /* Clean up */
//    g_tftpProcessId = 1;
//    g_uploadInProgress = 0;
//    
//    /* Re-enable the button */
//    SetCtrlAttribute(panel, MAINPNL_PLAYMUSIC, ATTR_DIMMED, 0);
//}


//int CreateBatScript(char *bat_file, char *tftpCommand){

//	FILE *batFile;
//	
//	remove(bat_file);
//	batFile = fopen(bat_file, "w");
//	if(batFile == NULL) MessagePopup("fopen Error", "Unable to create .bat file");
//	fprintf(batFile, tftpCommand);
//	fclose(batFile);
//	
//	return 0;
//}










































/*---------------------------------------------------------------------------*/
/* Display FTP error message                                                 */
/*---------------------------------------------------------------------------*/
static void ShowFTPError(int errorCode)
{
    char msg[256];
    sprintf(msg, "FTP Error %d: %s", errorCode, InetGetErrorMessage(errorCode));
    MessagePopup("FTP Transfer Error", msg);
}


/*---------------------------------------------------------------------------*/
/* Play Music Button Callback - Uses FTP to upload file to Zynq             */
/*---------------------------------------------------------------------------*/
int CVICALLBACK PlayMusicBtnCB(int panel, int control, int event,
                               void *callbackData, int eventData1, int eventData2)
{
    char localFile[MAX_PATHNAME_LEN] = {0};
    char remoteFile[MAX_PATHNAME_LEN] = "music.wav";
	char msg[1000] = {0};
	int passiveMode = 1,  /* Use passive mode for better firewall compatibility */
    	ftp_handle,
        ftpResult,
		index;
    
    switch (event)
    {
        case EVENT_COMMIT:
            /* Get the currently selected WAV file */
            if (GetCurrentlySelectedFile(panel, MAINPNL_LISTBOX, localFile) < 0) {
                MessagePopup("Error", "Please select a WAV file first");
                return 0;
            }
			
			GetCtrlIndex(panel, MAINPNL_LISTBOX, &index);
			/* Get just the filename for remoteFile */
            GetLabelFromIndex(panel, MAINPNL_LISTBOX, index, remoteFile);
			
            /* Check if connected to Zynq */
            if (!g_connected) {
                MessagePopup("Error", "Not connected to Zynq board");
                return 0;
            }
            
            /* Disable button during transfer */
            SetCtrlAttribute(panel, MAINPNL_PLAYMUSIC, ATTR_DIMMED, 1);
            SetCtrlVal(panel, MAINPNL_FILESTATUS, "Connecting to FTP server...");
            
            /* Login to FTP server */
            ftp_handle = InetFTPLogin(TCP_CONTROL_IP, "admin", "123456");
            
            if (ftp_handle < 0) {
                SetCtrlVal(panel, MAINPNL_FILESTATUS, "FTP login failed!");
                MessagePopup("FTP Error", "Cannot connect to FTP server.\nMake sure FTP server is running on Zynq.");
                SetCtrlAttribute(panel, MAINPNL_PLAYMUSIC, ATTR_DIMMED, 0);
                return 0;
            }
            
            /* Set passive mode for better compatibility */
            InetFTPSetPassiveMode(ftp_handle, passiveMode);
            
            /* Update status */
			sprintf(msg, "Uploading '%s' ... ", remoteFile);
            SetCtrlVal(panel, MAINPNL_FILESTATUS, msg);
			
			// Display "wait" panel
			SetCtrlAttribute (wait_panel, WAIT_TEXTMSG, ATTR_CTRL_VAL, msg);
			InstallPopup (wait_panel);
            
            /* Send the file */
            ftpResult = InetFTPSendFile(ftp_handle, localFile, remoteFile, INET_FTP_FILE_TYPE_BINARY);
            
            /* Close FTP connection */
            InetFTPClose(ftp_handle);
			
			// Remove "wait" panel
			HidePanel (wait_panel);
            
            if (ftpResult < 0) {
                SetCtrlVal(panel, MAINPNL_FILESTATUS, "FTP transfer failed!");
                ShowFTPError(ftpResult);
                SetCtrlAttribute(panel, MAINPNL_PLAYMUSIC, ATTR_DIMMED, 0);
                return 0;
            }
            
            /* Success - send PLAY command */
            SetCtrlVal(panel, MAINPNL_FILESTATUS, "Upload complete! Starting playback...");
            
            char transmitBuf[64];
            sprintf(transmitBuf, "PLAY\n");
            
            if (ClientTCPWrite(g_hconversation, transmitBuf,
                              strlen(transmitBuf), 1000) < 0) {
                SetCtrlVal(panel, MAINPNL_FILESTATUS, "Upload OK but PLAY command failed");
            } else {
                SetCtrlVal(panel, MAINPNL_FILESTATUS, "Playing music on Zynq...");
            }
            
            /* Re-enable button */
            SetCtrlAttribute(panel, MAINPNL_PLAYMUSIC, ATTR_DIMMED, 0);
            
            break;
    }
    return 0;
}
