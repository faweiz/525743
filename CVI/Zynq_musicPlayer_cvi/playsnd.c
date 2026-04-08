/*---------------------------------------------------------------------------*/
/*                                                                           */
/* FILE:    playsnd.c                                                        */
/*                                                                           */
/* PURPOSE: This file contains support functions used by sndplay.c.          */
/*                                                                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Include files                                                             */
/*---------------------------------------------------------------------------*/
#include <windows.h>
#include <mmsystem.h>

/*---------------------------------------------------------------------------*/
/* Play the specified sound file synchronously or asynchronously.            */
/*---------------------------------------------------------------------------*/
short CVI_PlaySound (char *fileName, short asynchronous)
{
    if (asynchronous)
        return (short)sndPlaySound(fileName, SND_ASYNC);
    else
        return (short)sndPlaySound(fileName, SND_SYNC);  
}

/*---------------------------------------------------------------------------*/
/* Play the specified sound file synchronously or asynchronously, with some  */
/* additional options.                                                       */
/*---------------------------------------------------------------------------*/
short CVI_PlaySoundEx (char *fileName, short asynchronous, short loop,
                       short playDefault, short stopCurrentSound)
{
    UINT flags = 0;

    if (loop)
        flags |= SND_ASYNC | SND_LOOP;  
    else if (asynchronous)
        flags |= SND_ASYNC;
    else
        flags |= SND_SYNC;
    if (!playDefault)
        flags |= SND_NODEFAULT;
    if (!stopCurrentSound)
        flags |= SND_NOSTOP;
    return (short)sndPlaySound (fileName, flags);   
}

/*---------------------------------------------------------------------------*/
/* Stop playing the current sound.                                           */
/*---------------------------------------------------------------------------*/
void CVI_StopCurrentSound (void)
{
    sndPlaySound(NULL, 0);
}




