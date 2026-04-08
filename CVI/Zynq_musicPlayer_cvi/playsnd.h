#ifndef PLAYSND_HEADER
#define PLAYSND_HEADER

short CVI_PlaySound   (char *fileName, short asynchronous);
short CVI_PlaySoundEx (char *fileName, short asynchronous, short loop,
                       short playDefault, short stopCurrentSound);
void  CVI_StopCurrentSound (void);

#endif  /* ndef PLAYSND_HEADER */
