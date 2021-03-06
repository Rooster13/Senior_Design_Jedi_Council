/*
  Rough Parks Sabers Code Example
  Kristen Villemez
  Sabrina Thompson
  UTDesign at UT Dallas
  Team Jedi Council:
    Sabrina Thompson
    Hailey McCurry
    Garrett Staas
    Kristen Villemez
  
  This code is a rough outline of an example to demonstrate the ability of the Arduino
  Uno to perform the tasks required for this project.
  
  Hardware setup:
  Breakout ------------ Arduino
  3.3V --------------------- 3.3V
  SDA -------^^(330)^^------- A4
  SCL -------^^(330)^^------- A5
  GND ---------------------- GND
*/

/////// BEGIN WAVEHC INCLUDE STATEMENTS /////////
#include <FatReader.h>
#include <FatStructs.h>
#include <mcpDac.h>
#include <SdInfo.h>
#include <SdReader.h>
#include <WaveHC.h>
#include <Wavemainpage.h>
#include <WavePinDefs.h>
#include <WaveUtil.h>

#define C1 "C1.WAV"
#define ME1 "F.WAV"
#define MF1 "H.WAV"
#define Zero1 "HF.WAV"
#define PE1 "S.WAV"
#define PF1 "TH.WAV"

/////// BEGIN IMU INCLUDE STATEMENTS ///////////
#include <FreeSixIMU.h>
#include <FIMU_ADXL345.h>
#include <FIMU_ITG3200.h>

#include <Wire.h>

///////// BEGIN WAVEHC INSTANTIATIONS /////////
SdReader card;    // This object holds the information for the card
FatReader root;   // This holds the information for the filesystem on the card
FatReader f;      // This holds the information for the file we're play 
WaveHC wave;      // This is the only wave (audio) object, since we will only play one at a time
FatVolume vol;    // This holds the information for the partition on the card 
char *toplay, *toplay1;
////////// BEGIN IMU INSTANTIATIONS ///////////

float angles2[3] ;
float highestAngle2[1];
// Set the FreeSixIMU object
FreeSixIMU sixDOF = FreeSixIMU();
// On/Off state
boolean onoff=true;
//Pin Def
#define TOUCH_PIN 1

/////////////////////// BEGIN FUNCTION DEFINITIONS /////////////////////////////

void sdErrorCheck(void)
{
  if (!card.errorCode()) return;
//  Serial.print("\n\rSD I/O error: ");
//  Serial.print(card.errorCode(), HEX);
//  Serial.print(", ");
//  Serial.println(card.errorData(), HEX);
  while(1);
}

///////////// SETUP FUNCTION /////////////// 
void setup() {
  // Prescale internal clock for slow-down (for use at 3.3V)
 
  CLKPR = 0x80;
  CLKPR = 0x1;
 
  // set up serial port
  Serial.begin(9600);
  
  Wire.begin(); //Join the bus as a master
  
  delay(5);
  sixDOF.init(); //begin the IMU
  delay(5);
  
  // Set the output pins for the DAC control. This pins are defined in the library
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
 
  // pin13 LED
  pinMode(13, OUTPUT);
 
  // enable pull-up resistors on switch pins (analog inputs)
  digitalWrite(14, HIGH);
  digitalWrite(15, HIGH);
  digitalWrite(16, HIGH);
  digitalWrite(17, HIGH);
  digitalWrite(18, HIGH);
  digitalWrite(19, HIGH);
  
  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);

 
  //  if (!card.init(true)) { //play with 4 MHz spi if 8MHz isn't working for you
  if (!card.init()) {         //play with 8 MHz spi (default faster!)  
    //Serial.println("Card init. failed!");  // Something went wrong, lets print out why
    sdErrorCheck();
    while(1);                            // then 'halt' - do nothing!
  }
  
  // enable optimize read - some cards may timeout. Disable if you're having problems
  card.partialBlockRead(true);
 
  // Now we will look for a FAT partition!
  uint8_t part;
  for (part = 0; part < 5; part++) {     // we have up to 5 slots to look in
    if (vol.init(card, part)) 
      break;                             // we found one, lets bail
  }
  if (part == 5) {                       // if we ended up not finding one  :(
    //Serial.println("No valid FAT partition!");
    sdErrorCheck();      // Something went wrong, lets print out why
    while(1);                            // then 'halt' - do nothing!
  }
  
  // Try to open the root directory
  if (!root.openRoot(vol)) {
    //Serial.println("Can't open root dir!"); // Something went wrong,
    while(1);                             // then 'halt' - do nothing!
  }
  
  digitalWrite(A0, HIGH);
  delay(25);
  digitalWrite(A1, HIGH);
  delay(25);
  digitalWrite(6, HIGH);
  delay(25);
  digitalWrite(7, HIGH);
  delay(25);
  digitalWrite(8, HIGH);
  delay(25);
  digitalWrite(9, HIGH);
  playcomplete("SO.WAV");
  
  // Whew! We got past the tough parts.
   toplay = Zero1;
   
   // attach interrupt function for cap touch button
//   attachInterrupt(TOUCH_PIN, onOffISR, RISING);
}

/////////////////// ISR FUNCTION //////////////////////////
//void onOffISR()
//{
//  if (onoff)
//  {
//    //turn off sequence goes here
//    digitalWrite(6, LOW);
//    delay(100);
//    digitalWrite(5, LOW);
//    delay(100);
//    digitalWrite(8, LOW);
//    delay(100);
//    digitalWrite(9, LOW);
//    delay(100);
//    digitalWrite(7, LOW);
//    delay(100);
//    digitalWrite(A0, LOW);
//    delay(100);
//  }
//  else
//  {
//    digitalWrite(A0, HIGH);
//    delay(100);
//    digitalWrite(7, HIGH);
//    delay(100);
//    digitalWrite(9, HIGH);
//    delay(100);
//    digitalWrite(8, HIGH);
//    delay(100);
//    digitalWrite(5, HIGH);
//    delay(100);
//    digitalWrite(6, HIGH);
//    delay(100);
//    
//    //turn on sequence goes here
//  }
//  onoff = !onoff;
//}

/////////////////// LOOP FUNCTION ///////////////////////// 
void loop() {
  if (!onoff)
  {
    return;
  }
  float angles[3]; // yaw pitch roll
  sixDOF.getEuler(angles);
  float anglesTotal[3];
  float highestAngle1[1];
  float changeInAnglesTotals[1], changeInAnglesTotal2[1];
  int value[5];
  int delaylength;
  
  ////////////// BEGIN LIMIT VALUE DEFINITIONS /////////////////
  /// PLEASE DO NOT CHANGE ANY CODE OUTSIDE OF THIS BLOCK!!! ///  
  
  value[0] = 10; //Value for clash and low hum
  value[1] = 20; //Value threshhold for hum 2
  value[2] = 30; //Value threshhold for hum 3
  value[3] = 40; //Value threshhold for hum 4
  value[4] = 50; //Value threshhold for clash
  
  ////////////// END LIMIT VALUE DEFINITIONS ///////////////////
  /// PLEASE DO NOT CHANGE ANY CODE OUTSIDE OF THIS BLOCK!!! ///
    
  anglesTotal[0] = abs(angles[0] - angles2[0]); //Gives total change in x axis
  anglesTotal[1] = abs(angles[1] - angles2[1]); //Gives total change in y axis
  anglesTotal[2] = abs(angles[2] - angles2[2]); //gives total change in z axis
  
//  Serial.println(anglesTotal[0]);
//  Serial.println(anglesTotal[1]);
//  Serial.println(anglesTotal[2]);
  
  
  //Statement to decide what angle is greatest to decide what sound to play
  if(anglesTotal[0] > anglesTotal[1])
  {
     if(anglesTotal[0] > anglesTotal[2])
     {
       highestAngle1[0] = anglesTotal[0]; //Angle change x was highest
     }
     else
     {
       highestAngle1[0] = anglesTotal[2]; //Angle change z was highest
     }
  }
  else
  {
    if(anglesTotal[1] > anglesTotal[2]) 
    {
      highestAngle1[0] = anglesTotal[1]; //Angle change y was highest
    }
    else
    {
      highestAngle1[0] = anglesTotal[2]; //Angle change z was highest
    }
  }
  
   changeInAnglesTotals[0] = abs(highestAngle1[0] - highestAngle2[0]); //tells us how the angles have changed to be used to determine what sound is played
//   Serial.println(changeInAnglesTotals[0]); //lets us see the change in angles

  if(highestAngle1[0] <= value[0] && changeInAnglesTotals[0] > value[4]) //if there is a sudden stop and the change is great signifying a clash 
  {
   toplay = C1; //Plays clash sound
   delaylength = 400; //longer delay length to play entire clash
  }
  
  else
  {
    if(highestAngle1[0] <=value[0] && changeInAnglesTotals[0] < value[4]) //if sounds are between these values shoudl be lowest hum
    {  
      toplay = ME1; //hum1
      delaylength = 50;
    }
    if(highestAngle1[0] <= value[1] && highestAngle1[0] > value[0]) //if sounds are between these values shoudl be low to mid hum
    {
      toplay = MF1; //hum2
      delaylength = 50;
    }
    if(highestAngle1[0] <= value[2] && highestAngle1[0] > value[1]) //if sounds are between these values shoudl be medium hum
    {
      toplay = Zero1; //hum3
      delaylength = 50;
    }
    if(highestAngle1[0] <=value[3] && highestAngle1[0] > value[2]) //if sounds are between these values should be mid to high hum
    {
      toplay = PF1; //hum4
      delaylength = 50;
    }
    if(highestAngle1[0] > value[3]) //if sounds are between these values shoudl be highest hum
    {
      toplay = PE1; //hum5
      delaylength = 50;
    }
  }
  
    if(toplay != toplay1 || !wave.isplaying) //if a different wav file needs to be played then stop the current wav file and play the new one
  {
   wave.stop(); 
   playfile(toplay);
  }  
  
  
  delay(delaylength);
  
  angles2[0] = angles[0]; //initializes x angle2 to the previous x angle for total
  angles2[1] = angles[1]; //initializes y angle2 to the previous y angle for total
  angles2[2] = angles[2]; //initializes z angle2 to the previous z angle for total
  highestAngle2[0] = highestAngle1[0]; //initializes change so we know whether to play hum or clash
  // Use playcomplete("SOUND.WAV") or playfile("SOUND.WAV") to call a sound to play 
    
  anglesTotal[0] = abs(angles[0] - angles2[0]); //Gives total change in x axis
  anglesTotal[1] = abs(angles[1] - angles2[1]); //Gives total change in y axis
  anglesTotal[2] = abs(angles[2] - angles2[2]); //gives total change in z axis
  changeInAnglesTotal2[0] = changeInAnglesTotals[0];
  toplay1 = toplay;
}  
/////////// BEGIN WAVE SHIELD SUPPORT FUNCTIONS ///////////
  
// Plays a full file from beginning to end with no pause. (To be used for CLASH and START/STOP)
void playcomplete(char *name) {
  // call our helper to find and play this name
  playfile(name);
  while (wave.isplaying) {
  // do nothing while its playing
  }
  // now it's done playing
}
 
void playfile(char *name) {
  // see if the wave object is currently doing something
  
  if (wave.isplaying) {// already playing something, so stop it!
    wave.stop(); // stop it
  }
  // look in the root directory and open the file
  if (!f.open(root, name)) {
//    Serial.print("Couldn't open file "); Serial.println(name); return;
  }
  // OK read the file and turn it into a wave object
  if (!wave.create(f)) {
//    Serial.println("Not a valid WAV"); return;
  }
  
  // ok time to play! start playback
  wave.play();
}

