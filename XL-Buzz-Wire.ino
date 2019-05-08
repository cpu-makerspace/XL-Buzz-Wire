/* XL-Buzz Wire for Arduino
 * ======================================= 
 * Built by: CPU - Computer Pool Unterland 
 * Contributors: Richy (richard@gaun.eu)
 * Info: https://computerpool.at
 * v 0.1 - 05.05.2019
*/
const String info = "CPU-Heisser Draht";
const String version = "v 0.2 - 07.05.2019";

#include <SPI.h>
#include <MD_Parola.h>  // https://github.com/MajicDesigns/MD_Parola
#include <MD_MAX72xx.h> // https://github.com/MajicDesigns/MD_MAX72XX
#include "Font_Data.h"  // Font for MD_Parola
#include <StopWatch.h>  
#include <Button.h>


// SETTINGS
Button  btnStart(2);     // Start-Button
Button  btnWireHit(9);   // Wire-Hit
Button  btnGoal(8);      // Reached Goal

// DISPLAY-Settings
// Define the number of devices we have in the chain and the hardware interface
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 8
#define CLK_PIN   11
#define DATA_PIN  12
#define CS_PIN    10

const int ledPin = 13; // integrated LED (debugging only)

// GAME-SETTINGS
int iCountDownStart = 3; // Number of CountDown-Seconds
int iMaxHitsAllowed = 3; // Max. Hits allowed





// DISPLAY-Settings
MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
// Default-Settings
uint8_t scrollSpeed = 20;    // default frame delay value
textEffect_t scrollEffect = PA_SCROLL_LEFT;
textPosition_t scrollAlign = PA_LEFT;
uint16_t scrollPause = 2000; // in milliseconds


// Global message buffers shared by Serial and Scrolling functions
#define  BUF_SIZE  75

// Display Messages in Idle-State
const char* arrMessages[] = { "CPU-Heisser Draht!!",
                              "Drueck' den Button!"};
                          
int iCurMessage = 0;
char curMessage[BUF_SIZE] = {arrMessages[iCurMessage]}; // holds current Message in text-Zone

StopWatch stopwatch; // Init Stop-Watch

// GLOBAL VARS
int iCountDown = iCountDownStart; 
char cCountDown[5];

// Buffer Current Game-State
static uint8_t curState = 0;  // 0=idle / 1=countdown / 2=game started / 3=game ended

static uint32_t iCountdownStartBuffer = 0; // millis() buffer
static uint32_t lastTime = 0; // millis() buffer for Stopwatch

char cTime[9]; // Char for Clock in Timer

int iHitCount = 0; // Wire-Hit Counter


 
void setup(void)
{
  // Set Buttons
  btnStart.begin();
  btnWireHit.begin();
  btnGoal.begin();

  pinMode(ledPin, OUTPUT); // Integrated LED

  // Setup Serial
  Serial.begin(9600);

  // Output Info
  Serial.println(info);
  Serial.println(version);

  
  // Display-Setup
  P.begin(2);  
  P.setZone(0, 0, 7);
  P.displayZoneText(0, curMessage, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
  P.displayReset(0);

  P.synchZoneStart();
}



void startGame()
{
  Serial.println("Start Game");
  curState = 2; // Set Game-State
  iHitCount = 0; // Reset Hit-counter
  // Init Zones
  P.setZone(0, 0, 3);
  P.setZone(1, 4, 7);
  P.setFont(0, numeric7Seg);

  stopwatch.start();
  
  strcpy(curMessage, "LOS!");
  
  strcpy(cTime, "00:00");
  P.displayZoneText(0, cTime, PA_CENTER, scrollSpeed, 0, PA_PRINT, PA_NO_EFFECT);
  P.displayZoneText(1, curMessage, PA_CENTER, scrollSpeed, 0, PA_PRINT, PA_NO_EFFECT);
  //P.displayReset(0);
  //P.displayReset(1);

  //P.synchZoneStart();
}


void startCountdown()
{
  Serial.println("Start CountDown");
  curState = 1; // Set State to Countdown-Mode
  iCountDown = iCountDownStart; 
  itoa(iCountDown, cCountDown, 10);
  
  lastTime = millis(); // Buffer First Time
    
  // Init Zones
  P.setZone(0, 0, 1);
  P.setZone(1, 2, 7);

  strcpy(curMessage, "Start in");
  
  P.displayZoneText(0, cCountDown, PA_CENTER, scrollSpeed, scrollPause, PA_PRINT, PA_NO_EFFECT);
  P.displayZoneText(1, curMessage, PA_CENTER, scrollSpeed, 0, PA_FADE, PA_NO_EFFECT);

  P.displayReset(0);
  P.displayReset(1);

  // start Timer
  iCountdownStartBuffer = millis();
}


void printTime()
{
  lastTime = millis();
  uint32_t  tmil;
  int tsec, tmin ;

  // Get and calulate Values
  tmil = stopwatch.ms();
  tsec = tmil/1000;
  tmin = tsec/60;
  tmin %= 60;
  tsec %= 60;
  tmil %= 1000;
  
  String curTimer;
  curTimer = String(tmin) + ":";
  if (tsec <= 9) // Prepend 0 to seconds
    curTimer += "0";
  curTimer += String(tsec) + "." + String(tmil)[0]; 
  curTimer.toCharArray(cTime, 75);
  //cTime[75] = {tmin + ":" + tsec };
  
  //Serial.println(cTime);
  
  strcpy(curMessage, arrMessages[iCurMessage]);
  P.displayReset(0);
        
  P.displayReset(0);
  P.displayReset(1);
}

void wireHit()
{
  Serial.println("HIT!");
  iHitCount++;  

  String strHitCount = "HIT: " + iHitCount;
  strHitCount.toCharArray(curMessage, 30);  
  
  //strcpy(curMessage, strHitCount);
  P.displayReset(1);  
}


void goalReached()
{
  Serial.println("Bravo!");
  curState = 3; // Set Mode to Goal reached
    
  // Stop Timer
  stopwatch.stop();

  strcpy(curMessage, "Bravo!");
  P.displayReset(1);
}


// MAIN-Loop
void loop(void)
{
  if (btnStart.pressed())     // Check for Start-Botton  
  {  
    startCountdown();  
  }

  if (btnWireHit.pressed() && curState == 2) // Wire Hit in active Game
  {  
    wireHit();
  }

  if (btnGoal.pressed() && curState == 2)  // Goal Reached
  {  
    goalReached();    
  }

  
  if (P.displayAnimate()) // animates and returns true when an animation is completed
  {
    boolean bAllDone = true;

    for (uint8_t i=0; i < MAX_ZONES && bAllDone; i++)
      bAllDone = bAllDone && P.getZoneStatus(i);

    if (bAllDone) // do something as all zones have completed
    {
      switch (curState)
      {
        case 0: // Idle-Mode
          P.setZone(0, 0, 7); // use all elements for one big display

          // ToDo: use more elegant/flexible solution (
          if (iCurMessage == 0)
          {
            iCurMessage = 1;
          } else {
             iCurMessage = 0;
          }        
          strcpy(curMessage, arrMessages[iCurMessage]);
          P.displayReset(0);
          break;
          
        case 1: // Countdown-Mode
          if (iCountDown == 0)
          {
            startGame();
            break;
          }
          if (millis() - lastTime >= 1000)
          {
            lastTime = millis();
            iCountDown--;
  
            itoa(iCountDown, cCountDown, 10);
            P.displayReset(0);
            P.displayReset(1);
          }       
          break;
          
        case 2: // Game-Mode
          if (millis() - lastTime >= 100) // every 100ms
          {
            printTime();
          }   
          //P.displayReset(0);         
          break;
          
        case 3: // Goal-Mode
          // ToDo: Switch back to ide-mode after X-Time?!?
          break;
      }
    }
  }

}
