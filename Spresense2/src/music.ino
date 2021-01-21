/*
 *  player.ino - Simple sound player example application
 *  Copyright 2018 Sony Semiconductor Solutions Corporation
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <SDHCI.h>
#include <Audio.h>

SDClass theSD;
AudioClass *theAudio;

File myFile;

int pushButton = 2;
bool ErrEnd = false;
bool nowPlay = false;

/**
 * @brief Audio attention callback
 *
 * When audio internal error occurc, this function will be called back.
 */

static void audio_attention_cb(const ErrorAttentionParam *atprm)
{
  puts("Attention!");
  
  if (atprm->error_code >= AS_ATTENTION_CODE_WARNING)
    {
      ErrEnd = true;
   }
}

/**
 * @brief Setup audio player to play mp3 file
 *
 * Set clock mode to normal <br>
 * Set output device to speaker <br>
 * Set main player to decode stereo mp3. Stream sample rate is set to "auto detect" <br>
 * System directory "/mnt/sd0/BIN" will be searched for MP3 decoder (MP3DEC file)
 * Open "Sound.mp3" file <br>
 * Set master volume to -16.0 dB
 */
void setup()
{
  // start audio system
  theAudio = AudioClass::getInstance();
  pinMode(pushButton,INPUT);

  theAudio->begin(audio_attention_cb);

  puts("initialization Audio Library");

  /* Set clock mode to normal */
  theAudio->setRenderingClockMode(AS_CLKMODE_NORMAL);

  /* Set output device to speaker with first argument.
   * If you want to change the output device to I2S,
   * specify "AS_SETPLAYER_OUTPUTDEVICE_I2SOUTPUT" as an argument.
   * Set speaker driver mode to LineOut with second argument.
   * If you want to change the speaker driver mode to other,
   * specify "AS_SP_DRV_MODE_1DRIVER" or "AS_SP_DRV_MODE_2DRIVER" or "AS_SP_DRV_MODE_4DRIVER"
   * as an argument.
   */
  theAudio->setPlayerMode(AS_SETPLAYER_OUTPUTDEVICE_SPHP, AS_SP_DRV_MODE_LINEOUT);

  /*
   * Set main player to decode stereo mp3. Stream sample rate is set to "auto detect"
   * Search for MP3 decoder in "/mnt/sd0/BIN" directory
   */
  err_t err = theAudio->initPlayer(AudioClass::Player0, AS_CODECTYPE_MP3, "/mnt/sd0/BIN", AS_SAMPLINGRATE_AUTO, AS_CHANNEL_STEREO);

  /* Verify player initialize */
  if (err != AUDIOLIB_ECODE_OK)
    {
      printf("Player0 initialize error\n");
      exit(1);
    }

  
}

/**
 * @brief Play stream
 *
 * Send new frames to decode in a loop until file ends
 */
void loop()
{
  bool click_button = (digitalRead(pushButton) == LOW);
  if(!nowPlay && click_button){
    nowPlay = true;
      /* Open file placed on SD card */
    myFile = theSD.open("sansai.mp3");
  
    /* Verify file open */
    if (!myFile)
      {
        printf("File open error\n");
        exit(1);
      }
    printf("Open! %d\n",myFile);
  
    /* Send first frames to be decoded */
    int err = theAudio->writeFrames(AudioClass::Player0, myFile);
  
    if ((err != AUDIOLIB_ECODE_OK) && (err != AUDIOLIB_ECODE_FILEEND))
      {
        printf("File Read Error! =%d\n",err);
        myFile.close();
        exit(1);
      }
  
    /* Main volume set to -16.0 dB */
    theAudio->setVolume(-160);
    theAudio->startPlayer(AudioClass::Player0);
  }

  if(nowPlay){
    /* Send new frames to decode in a loop until file ends */
    int err = theAudio->writeFrames(AudioClass::Player0, myFile);
    if(err || ErrEnd){
      nowPlay = false;
      sleep(1);
      theAudio->stopPlayer(AudioClass::Player0);
      myFile.close();
    }
  }



  usleep(40000);


  /* Don't go further and continue play */
  return;
}
