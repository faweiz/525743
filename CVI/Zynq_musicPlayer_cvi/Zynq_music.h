//==============================================================================
//
// Title:		Zynq_music.h
// Purpose:		A short description of the interface.
//
// Created on:	2/14/2026 at 12:15:42 PM by fawei zhang.
// Copyright:	personal. All Rights Reserved.
//
//==============================================================================

#ifndef __Zynq_music_H__
#define __Zynq_music_H__

#ifdef __cplusplus
    extern "C" {
#endif

//==============================================================================
// Include files

#include "cvidef.h"

//==============================================================================
// Constants

//==============================================================================
// Types
/* Sound status structure */
typedef struct {
    int     async;
    int     loop;
    int     playingAll;
    int     stopPressed;
    int     currentTrack;
    char    currentFile[MAX_PATHNAME_LEN];
} SoundStatusType;

/* Parse functions */
typedef enum {
    PARSE_INT,
    PARSE_FLOAT
} parse_type_t;

//==============================================================================
// External variables

//==============================================================================
// Global functions

/* TCP Callback */
int CVICALLBACK ClientTCPCB(unsigned handle, int event, int error, void *callbackData);

/* Sound player utility functions */
int  GetCurrentlySelectedFile(int panel, int listbox, char *filename);
int  GetPlayOptions(int panel, int *asynchronous, int *loop);
void PlaySingleSound(int panel, int listbox);
void PlayAllSounds(int panel, int listbox);
int  DirectoryExists(char *directory);
void GetInitialDirectory(char *directory);
int  FillListBoxWithWavsInDir(int panel, int listbox, char *directory);

int get_value(const char *buf, const char *key, char delimiter, parse_type_t type, void *out_value);
int TCPReceive_StatusUpdate(char *receiveBuf);

static void CheckTFTPStatus(int panel);

int CreateBatScript(char *bat_file, char *tftpCommand);

#ifdef __cplusplus
    }
#endif

#endif  /* ndef __Zynq_music_H__ */
