#ifndef USE_SOFTWARESERIAL
#define USE_SOFTWARESERIAL 1   ///< Set to 1 to use SoftwareSerial library, 0 for native serial port
#endif
#include <MD_YX5300.h>
#include <MD_UISwitch.h>
#if USE_SOFTWARESERIAL
#include <SoftwareSerial.h>

// Connections for serial interface to the YX5300 module
const uint8_t ARDUINO_RX = 4;    // connect to TX of MP3 Player module
const uint8_t ARDUINO_TX = 5;    // connect to RX of MP3 Player module

SoftwareSerial  MP3Stream(ARDUINO_RX, ARDUINO_TX);  // MP3 player serial stream for comms
#define Console Serial           // command processor input/output stream
#else
#define MP3Stream Serial2  // Native serial port - change to suit the application
#define Console   Serial   // command processor input/output stream
#endif
const uint8_t PLAY_FOLDER = 1;   // tracks are all placed in this folder
MD_YX5300 mp3(MP3Stream);

void processVolume(bool bForce = false)
{
  static uint8_t vol;   // current audio volume
  uint8_t newVolume =  mp3.volumeMax();
  
  if (newVolume != vol || bForce)
  {
    vol = newVolume;
    bool b = mp3.volume(vol);
  }
}


void processPlayWelcome(void)
{
  bool b;
  b = mp3.playTrack(1);
}
void processPlayGoodbuy(void)
{
  bool b;
  b = mp3.playTrack(2);
}





int  processCheckInside(void)
{
  
  int echoPinInside = 6;
  int trigPinInside = 9;
  int durationInside, cmInside;


  pinMode(trigPinInside, OUTPUT);
  pinMode(echoPinInside, INPUT);
  
//  digitalWrite(trigPinInside, LOW);
//  delay(5);
  digitalWrite(trigPinInside, HIGH);
  delay(10);
  digitalWrite(trigPinInside, LOW);
  durationInside = pulseIn(echoPinInside, HIGH);
//  pulseIn(echoPinInside, LOW,200);
  cmInside = durationInside / 58;
  return cmInside;
}

int  processCheckOutside(void)
{
  int echoPinOutside = 10;
  int trigPinOutside = 11;
  int durationOutside, cmOutside;

  pinMode(trigPinOutside, OUTPUT);
  pinMode(echoPinOutside, INPUT);

  
//  digitalWrite(trigPinOutside, LOW);
//  delay(5);
  digitalWrite(trigPinOutside, HIGH);
  delay(10);
  digitalWrite(trigPinOutside, LOW);
  durationOutside = pulseIn(echoPinOutside, HIGH);
//  pulseIn(echoPinOutside, LOW,200);
  cmOutside = durationOutside / 58;

  return cmOutside;
  
}



//пины

int counterInside = 10;//счетчик не пора ли делать проверку на стандартное расстояние относительно которого изменение расстояния- движение. в штуках полусекунд. которые нужно отсчитать до обновления
int which_updateInside = 0;// какое расстояние из трех обновляем
int rast0Inside = 0;//расстояний три. тру расстояние - два одинаковых из трех либо первое
int rast1Inside = 0;//
int rast2Inside = 0;//
int rastTrueInside = 0;//дефолтное расстояние


int counterOutside = 10;//счетчик не пора ли делать проверку на стандартное расстояние относительно которого изменение расстояния- движение. в штуках полусекунд. которые нужно отсчитать до обновления
int which_updateOutside = 0;// какое расстояние из трех обновляем
int rast0Outside = 0;//расстояний три. тру расстояние - два одинаковых из трех либо первое
int rast1Outside = 0;//
int rast2Outside = 0;//
int rastTrueOutside = 0;//дефолтное расстояние


int timePeopleMooveBetweenSensors = 1400;


int InsideFixedMoveLast = 0;
int OutsideFixedMoveLast = 0;



void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
//  pinMode(trigPinInside, OUTPUT);
//  pinMode(echoPinInside, INPUT);
//  pinMode(trigPinOutside, OUTPUT);
//  pinMode(echoPinOutside, INPUT);


  pinMode(LED_BUILTIN, OUTPUT);
  Console.begin(9600);

  MP3Stream.begin(MD_YX5300::SERIAL_BPS);
  mp3.begin();
  mp3.setSynchronous(true);
  mp3.playFolderRepeat(PLAY_FOLDER);
  processVolume(true);    // force these to set up the hardware
}




void loop()
{
  mp3.check();        // run the mp3 receiver
  processVolume();    // set the volume if required
  
  // определяем расстояние в сантиметрах
  int durationInside, cmInside;
  int durationOutside, cmOutside;

  if (OutsideFixedMoveLast == 0 and InsideFixedMoveLast == 0)  
  {digitalWrite(LED_BUILTIN, HIGH);}
  else
  {digitalWrite(LED_BUILTIN, LOW);}



  cmOutside = processCheckOutside();
  delay(60);
  cmInside = processCheckInside();
  delay(60);


  //если только запустили программу, то все значения равны первому  
  if (rastTrueOutside == 0) 
  {
    rastTrueOutside = cmOutside;
    rast0Outside = cmOutside;
    rast1Outside = cmOutside;
    rast2Outside = cmOutside;
  }

  if (counterOutside < 2) { //если пора обновить значение базового расстояния

    //обновляем одно из трех
    if (which_updateOutside == 0) 
    {
      rast0Outside = cmOutside;
    } else if (which_updateOutside == 1) 
    {
      rast1Outside = cmOutside;
    } else 
    {
      rast2Outside = cmOutside;
    }

    //определяем правильное расстояние для среднего. делаем выбор двух похожих
    if (abs(rast0Outside - rast1Outside) < 6) {
      rastTrueOutside = (rast0Outside + rast1Outside) / 2;

    } else if (abs(rast1Outside - rast2Outside) < 6) {
      rastTrueOutside = (rast1Outside + rast2Outside) / 2;

    } else if (abs(rast0Outside - rast2Outside) < 6) {
      rastTrueOutside = (rast0Outside + rast2Outside) / 2;

    } else {
      rastTrueOutside = rast0Outside;
    }

    //переключаем на следующий
    if (which_updateOutside < 2) {
      which_updateOutside++;
    } else {
      which_updateOutside = 0;
    }

    //обновляем счетчик
    counterOutside = 8;
  }
  counterOutside--;
  
  

  //управление привычкой расстояния для внутреннего датчика
  //если только запустили программу, то все значения равны первому
  if (rastTrueInside == 0) 
  {
    rastTrueInside = cmInside;
    rast0Inside = cmInside;
    rast1Inside = cmInside;
    rast2Inside = cmInside;
  }

  if (counterInside < 2) { //если пора обновить значение базового расстояния
//    Serial.println("обновление расстояния");

    //обновляем одно из трех
    if (which_updateInside == 0) 
    {
      rast0Inside = cmInside;
    } else if (which_updateInside == 1) 
    {
      rast1Inside = cmInside;
    } else 
    {
      rast2Inside = cmInside;
    }

    //определяем правильное расстояние для среднего. делаем выбор двух похожих
    if (abs(rast0Inside - rast1Inside) < 6) 
    {
      rastTrueInside = (rast0Inside + rast1Inside) / 2;
    } 
    else if (abs(rast1Inside - rast2Inside) < 6) 
    {
      rastTrueInside = (rast1Inside + rast2Inside) / 2;
    } 
    else if (abs(rast0Inside - rast2Inside) < 6) 
    {
      rastTrueInside = (rast0Inside + rast2Inside) / 2;

    } else 
    {
      rastTrueInside = rast0Inside;
    }

    //переключаем на следующий
    if (which_updateInside < 2) 
    {
      which_updateInside++;
    } else 
    {
      which_updateInside = 0;
    }

    //обновляем счетчик
    counterInside = 8;
  }
  counterInside--;



  //  а теперь, когда мы установили стандартные расстояния на датчиках расстояния, мы можем проверить наличие движения
  // rastTrueInside -  cmInside > 5 ===== cmInside < rastTrueInside




  if (rastTrueInside -  cmInside > 20 and InsideFixedMoveLast <= 0 and OutsideFixedMoveLast <= 0)// фиксируем наличие движения со стороны внутри и при этом ни один датчик ещё не фиксировал движение последнее время там
  { 
      InsideFixedMoveLast = timePeopleMooveBetweenSensors;
      OutsideFixedMoveLast = 0;
  }  
  if (rastTrueOutside -  cmOutside > 20 and OutsideFixedMoveLast <= 0 and InsideFixedMoveLast <=0)// фиксируем наличие движения со стороны снаружи и при ни один датчик ещё не фиксировал движение последнее время там
  {
      OutsideFixedMoveLast = timePeopleMooveBetweenSensors;
      InsideFixedMoveLast = 0;
  }
    
  if (rastTrueOutside -  cmOutside > 20 and InsideFixedMoveLast > 0 and InsideFixedMoveLast < timePeopleMooveBetweenSensors-10)// фиксируем наличие движения со стороны снаружи и при этом внешний уже был зафиксирован
  {
      processPlayGoodbuy();
       delay(2300);
  }
  if (rastTrueInside -  cmInside > 20 and OutsideFixedMoveLast > 0 and OutsideFixedMoveLast < timePeopleMooveBetweenSensors-10)// фиксируем наличие движения со стороны внутри и при этом внешний уже был зафиксирован 
  { 
      processPlayWelcome();
       delay(2300);
  }

  
   OutsideFixedMoveLast -= 50;
   InsideFixedMoveLast -= 50;

  if (InsideFixedMoveLast < 0)  
  {
    InsideFixedMoveLast = 0;
  }
  if (OutsideFixedMoveLast < 0)  
  {
    OutsideFixedMoveLast = 0;
  }

  



}
