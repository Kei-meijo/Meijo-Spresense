#include <SDHCI.h>
#include <NetPBM.h>
#include <DNNRT.h>
#include <Camera.h>
#include <File.h>
#include <Audio.h>
#include <Wire.h>
#include "KX126.h"
#include "Vector.h"
#include "BM1383AGLV.h"

KX126 KX126(KX126_DEVICE_ADDRESS_1F);
BM1383AGLV bm1383aglv;
DNNRT dnnrt;
SDClass SD;
//AudioClass *theAudio;
//File music;
int dnn_state;
int pushButton = D2;

#define LENGTH 10
#define DNN_NULL -1
#define DNN_NOW_CALC 0
#define DNN_RESULT_OK 1
#define DNN_RESULT_NG 2

//画像の切り抜きの設定
#define OFFSET_X         0
#define OFFSET_Y         0
#define DNN_IMG_WIDTH   64
#define DNN_IMG_HEIGHT  48
#define RESIZE          4
#define BOX_WIDTH      (DNN_IMG_WIDTH / RESIZE)
#define BOX_HEIGHT     (DNN_IMG_HEIGHT / RESIZE)
DNNVariable input(DNN_IMG_WIDTH*DNN_IMG_HEIGHT);

void printError(enum CamErr err);
DNNVariable dnn(CamImage dnn_image);
int isSit();

bool ErrEnd = false;

static void audio_attention_cb(const ErrorAttentionParam *atprm)
{
  puts("Attention!");
  
  if (atprm->error_code >= AS_ATTENTION_CODE_WARNING)
    {
      ErrEnd = true;
   }
}

void CamCB(CamImage img){
  //Serial.println("Camera");
  if(dnn_state != DNN_NOW_CALC)return;
  //カメラ
  int index = 0;
  if (!img.isAvailable()) return;

  
  img.convertPixFormat(CAM_IMAGE_PIX_FMT_RGB565);  
  Serial.println("start clip and resize");
  CamImage small;
  CamErr camErr = img.clipAndResizeImageByHW(small
              ,OFFSET_X ,OFFSET_Y 
              ,OFFSET_X+BOX_WIDTH-1 ,OFFSET_Y+BOX_HEIGHT-1 
              ,DNN_IMG_WIDTH ,DNN_IMG_HEIGHT);
  if (!small.isAvailable()) {
    Serial.println("Error Occured at making a target image");
    if (camErr) Serial.println("CamErr: " + String(camErr));
    printError(camErr);
    return;
  }

  
  DNNVariable output = dnn(small);
  index = output.maxIndex();

  if(index == 0){
    dnn_state = DNN_RESULT_OK;
  } else {
    dnn_state = DNN_RESULT_NG;
  }
}

void loop() {
  static unsigned long time = 0;
  static long final_time = -1;
  unsigned long nowTime = millis();
  static int play = 0;
  int sit = 0;

  Serial.print("play = ");
  Serial.print(play);
  Serial.print(" dnn_state = ");
  Serial.println(dnn_state);

  if(nowTime - time > 50){
    time = nowTime;
    sit = isSit();
  }

  if(final_time < 0 && sit != 0){
    final_time = nowTime;
    dnn_state = DNN_NOW_CALC;
    Serial.println("DNN_NOW_CALC");
  } else if(final_time >= 0){
    if(nowTime - final_time > 2000){
      final_time = -1;
    }
  }

  if(dnn_state == DNN_RESULT_NG){
    Serial.println("start_audio");
    //theAudio->startPlayer(AudioClass::Player0);
    delay(2000);
    digitalWrite(pushButton,LOW);
    play = 1;
    dnn_state = DNN_NULL;
    Serial.println("start_audio2");
  } else if(dnn_state == DNN_RESULT_OK){
    dnn_state = DNN_NULL;
    digitalWrite(pushButton,HIGH);
    play = 0;
  } 
  
  if(play > 0) {
    delay(1000);
    Serial.println("stop audio");
    digitalWrite(pushButton,HIGH);
    play = 0;
    /*
    int err = theAudio->writeFrames(AudioClass::Player0, music);
    if (err == AUDIOLIB_ECODE_FILEEND || err || ErrEnd) {
      play = 0;
      sleep(1);
      theAudio->stopPlayer(AudioClass::Player0);
    }
    */
  } else if(play == 0){
    digitalWrite(pushButton,HIGH);
  }

  usleep(40000);
}

void setup() {
 
  dnn_state = DNN_NULL;
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  pinMode(pushButton, OUTPUT);
  digitalWrite(pushButton,HIGH);

  //DNN
  
  {
    Serial.println("open nnb");
    File nnbfile = SD.open("model.nnb");
    if (!nnbfile) {
      Serial.print("nnb not found");
      return;
    }
    int ret = dnnrt.begin(nnbfile);
    if (ret < 0) {
      Serial.print("Runtime initialization failure. ");
      if (ret == -16) {
        Serial.println("Please update bootloader!");
      } else {
        Serial.println(ret);
      }
      return;
    }
  }
  

  {
    CamErr err;
    //Camera
    err = theCamera.begin();
    if (err != CAM_ERR_SUCCESS)
    {
      printError(err);
    }

    Serial.println("Start streaming");
    err = theCamera.startStreaming(true, CamCB);
    if (err != CAM_ERR_SUCCESS){
      printError(err);
    }
  }

  Wire.begin();

  if (KX126.init() != 0) {
    Serial.println("KX126 initialization failed");
    Serial.flush();
  }

  if (bm1383aglv.init() != 0) {
    Serial.println("BM1383AGLV initialization failed");
    Serial.flush();
  }  
}

void printError(enum CamErr err){
  Serial.print("Error: ");
  switch (err)
    {
      case CAM_ERR_NO_DEVICE:
        Serial.println("No Device");
        break;
      case CAM_ERR_ILLEGAL_DEVERR:
        Serial.println("Illegal device error");
        break;
      case CAM_ERR_ALREADY_INITIALIZED:
        Serial.println("Already initialized");
        break;
      case CAM_ERR_NOT_INITIALIZED:
        Serial.println("Not initialized");
        break;
      case CAM_ERR_NOT_STILL_INITIALIZED:
        Serial.println("Still picture not initialized");
        break;
      case CAM_ERR_CANT_CREATE_THREAD:
        Serial.println("Failed to create thread");
        break;
      case CAM_ERR_INVALID_PARAM:
        Serial.println("Invalid parameter");
        break;
      case CAM_ERR_NO_MEMORY:
        Serial.println("No memory");
        break;
      case CAM_ERR_USR_INUSED:
        Serial.println("Buffer already in use");
        break;
      case CAM_ERR_NOT_PERMITTED:
        Serial.println("Operation not permitted");
        break;
      default:
        break;
    }
}

DNNVariable dnn(CamImage dnn_image){
  uint16_t* buf = (uint16_t*)dnn_image.getImgBuff();
  float* input_buffer = input.data();
  for (int i = 0; i < DNN_IMG_WIDTH * DNN_IMG_HEIGHT; ++i, ++buf) {
    input_buffer[i] = (float)(((*buf & 0x07E0) >> 5) << 2) ; // extract green
  }

  Serial.println("DNN forward");
  dnnrt.inputVariable(input, 0);
  dnnrt.forward();
  DNNVariable output = dnnrt.outputVariable(0);

  return output;
}


int isSit(){
  float press = 0, temp = 0;
  byte rc;
  rc = bm1383aglv.get_val(&press, &temp);
  
  static int len = 0;
  float acc[3];
  static Vector vec[LENGTH];
  static Vector avg;
  static float acv[LENGTH];
  
  static int l = 0;

  float data = 0;
  static float stopPress = press;
  static float max_val  = 0;
  static int moveNow = 0;


  if(KX126.get_val(acc) == 0){
    Vector v(acc);
    //float accVal = v.val[0] * v.val[1] * v.val[2] * 10;
    float accVal = v.mul(v) * 5;
    
    //
    for(int i = 1;i < LENGTH;i++){
      acv[i - 1] = acv[i];
    }
    acv[LENGTH -1] = accVal;
    
    
    float acvsum;
    for(int i = 0;i < LENGTH;i++){
      acvsum += acv[i];
    }

    acvsum /= LENGTH;
    float d = accVal - acvsum;

    if(d > -1 && d < 1){
          if(l < LENGTH)l++;

          //shift
          for(int i = 1;i < LENGTH;i++){
            vec[i - 1] = vec[i];
          }
          vec[LENGTH - 1] = v;
          

          //avg
          Vector sum;
          for(int k = 0;k < l;k++){
            sum = sum.add(vec[LENGTH - k - 1]);
          }

          sum = sum.div(l);
          avg = sum;

          stopPress = press;
          max_val = 0;
          moveNow--;
          if(moveNow < 0)moveNow = 0;
          //Serial.println(sum.mul(sum));
    } else {    
      float mul = avg.mul(v);
      moveNow++;

      data = mul;

      float tmp = press - stopPress;
      tmp *= 100;

      //max_val = max(max_val,tmp);
      max_val = max_val < tmp ? tmp : max_val;
      //Serial.println(max);
      //Serial.println(max);
    }


    int sit = 0;
    if(max_val > 3 && moveNow > 0){
      sit = 1;
    }

    //Serial.println(sit);
    return sit;
       //Serial.println(data);
  }
}
