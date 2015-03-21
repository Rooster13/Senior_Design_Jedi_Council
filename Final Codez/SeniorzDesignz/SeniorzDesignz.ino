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
FatVolume vol;    // This holds the information for the partition on the card
FatReader root;   // This holds the information for the filesystem on the card
FatReader f;      // This holds the information for the file we're play
 
WaveHC wave;      // This is the only wave (audio) object, since we will only play one at a time

////////// BEGIN IMU INSTANTIATIONS ///////////
float angles[3]; // yaw pitch roll

// Set the FreeSixIMU object
FreeSixIMU sixDOF = FreeSixIMU();

/////////////////////// BEGIN FUNCTION DEFINITIONS /////////////////////////////

// this handy function will return the number of bytes currently free in RAM, great for debugging!   
int freeRam(void)
{
  Serial.println("FreeRam");
  extern int  __bss_end; 
  extern int  *__brkval; 
  int free_memory; 
  if((int)__brkval == 0) {
    free_memory = ((int)&free_memory) - ((int)&__bss_end); 
  }
  else {
    free_memory = ((int)&free_memory) - ((int)__brkval); 
  }
  return free_memory; 
} 
 
void sdErrorCheck(void)
{
  if (!card.errorCode()) return;
  Serial.print("\n\rSD I/O error: ");
  Serial.print(card.errorCode(), HEX);
  Serial.print(", ");
  Serial.println(card.errorData(), HEX);
  while(1);
}

///////////// SETUP FUNCTION /////////////// 
void setup() {
  // Prescale internal clock for slow-down (for use at 3.3V)
  CLKPR = 0x80;
  CLKPR = 0x1;
  
  // set up serial port
  Serial.begin(9600);
  Serial.println("WaveHC Example Code");
  
  Wire.begin(); //Join the bus as a master
  Serial.println("Hello!");
  
  delay(5);
  sixDOF.init(); //begin the IMU
  delay(5);
  
  Serial.print("Free RAM: ");       // This can help with debugging, running out of RAM is bad
  Serial.println(freeRam());      // if this is under 150 bytes it may spell trouble!
  
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
    Serial.println("Card init. failed!");  // Something went wrong, lets print out why
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
    Serial.println("No valid FAT partition!");
    sdErrorCheck();      // Something went wrong, lets print out why
    while(1);                            // then 'halt' - do nothing!
  }
  
  // Lets tell the user about what we found
  Serial.print("Using partition ");
  Serial.print(part, DEC);
  Serial.print(", type is FAT");
  Serial.println(vol.fatType(),DEC);     // FAT16 or FAT32?
  
  // Try to open the root directory
  if (!root.openRoot(vol)) {
    Serial.println("Can't open root dir!"); // Something went wrong,
    while(1);                             // then 'halt' - do nothing!
  }
  
  // Whew! We got past the tough parts.
  Serial.println("Ready!");
}

/////////////////// LOOP FUNCTION ///////////////////////// 
void loop() {
  sixDOF.getEuler(angles);
  
  Serial.print(angles[0]);
  Serial.print(" | ");  
  Serial.print(angles[1]);
  Serial.print(" | ");
  Serial.println(angles[2]);
  
  delay(100); 
 // Use playcomplete("SOUND.WAV") or playfile("SOUND.WAV") to call a sound to play 
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
