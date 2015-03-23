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
////////// BEGIN IMU INSTANTIATIONS ///////////
float angles[3]; // yaw pitch roll
float angles2[3];
float highestAngle2[1];
// Set the FreeSixIMU object
FreeSixIMU sixDOF = FreeSixIMU();

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
  
  // Whew! We got past the tough parts.
 
}

/////////////////// LOOP FUNCTION ///////////////////////// 
void loop() {
  sixDOF.getEuler(angles);
  float anglesTotal[3];
  float highestAngle1[1];
  float changeInAnglesTotals[1];
  float value[5];
  
  value[0] = 30;
  value[1] = 50;
  value[2] = 100;
  value[3] = 200;
  value[4] = 20;
    
  anglesTotal[0] = abs(angles[0] - angles2[0]); //Gives total change in x axis
  anglesTotal[1] = abs(angles[1] - angles2[1]); //Gives total change in y axis
  anglesTotal[2] = abs(angles[2] - angles2[2]); //gives total change in z axis
  
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
  
   changeInAnglesTotals[0] = abs(highestAngle1[0] - highestAngle2[0]);
   Serial.println(highestAngle1[0]);
   Serial.println(highestAngle2[0]);
   Serial.println(changeInAnglesTotals[0]);
   
  if(highestAngle1[0] <= value[0] && changeInAnglesTotals[0] > value[4])
  {
    playfile("Clash1.WAV");
    delay(200);
  }
  else
  {
    if(highestAngle1[0] <=value[0] && changeInAnglesTotals[0] < value[4])
    {  
      playfile("Minus80.WAV");
      delay(1000);
    }
    if(highestAngle1[0] <= value[1] && highestAngle1[0] > value[0])
    {
      playfile("Minus40.WAV");
      delay(1000);
    }
    if(highestAngle1[0] <= value[2] && highestAngle1[0] > value[1])
    {
      playfile("Zero0.WAV");
      delay(1000);
    }
    if(highestAngle1[0] <=value[3] && highestAngle1[0] > value[2])
    {
      playfile("Plus40.WAV");
      delay(1000);
    }
    if(highestAngle1[0] > value[3])
    {
      playfile("Plus80.WAV");
      delay(1000);
    }
  }
  

  angles2[0] = angles[0]; //initializes x angle2 to the previous x angle for total
  angles2[1] = angles[1]; //initializes y angle2 to the previous y angle for total
  angles2[2] = angles[2]; //initializes z angle2 to the previous z angle for total
  highestAngle2[0] = highestAngle1[0]; //initializes change so we know whether to play hum or clash
  // Use playcomplete("SOUND.WAV") or playfile("SOUND.WAV") to call a sound to play 
  Serial.println(highestAngle2[0]);
  Serial.println();
  delay(100); // this delay is to simulate 100ms read increment
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
    Serial.print("Couldn't open file "); Serial.print(name); return;
  }
  // OK read the file and turn it into a wave object
  if (!wave.create(f)) {
    Serial.println("Not a valid WAV"); return;
  }
  
  // ok time to play! start playback
  wave.play();
}

