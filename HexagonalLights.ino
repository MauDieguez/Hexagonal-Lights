/*
 * Created by: Jose Mauricio Munoz Dieguez
 * Date: July 12th 2020
 * 
 */

#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <FastLED.h>
#include <StopWatch.h>
#include <cmath>

const char* ssid = "Network Name";
const char* pass = "Network PASSWORD";
const char* auth = "BLINK AUTH";

// We define some constants to later work with. 
const int STEPS = 100;
const int LED_IN_BOX = 10;
const int NUM_BOXES = 5;
const int LED_PIN = 19;
const int TOTAL_LEDS = LED_IN_BOX * NUM_BOXES;

// We define our LED Array and our primary / secondary color variables.
CRGB leds[TOTAL_LEDS];
CRGB primary_color;
CRGB secondary_color;

// Variables to have a better control of the modes
int wasRandom = -1;
int lastMode = -1;
int hexaMode = 3;
int animTime = 100;

// Specific mode variables
int hueAdd = 50;
int rDelay = 13;
int cycleDelay = 20;
int randomCycle = 0;

int hueControl[NUM_BOXES];
int hueLEDS[TOTAL_LEDS];
int hueCycleMode = 0;

// Delay without pausing the program.
StopWatch animControl;

// We update our array which stores all the current colors in the LED strip
void updateHueControl()
{
  for (int i=0;i<NUM_BOXES;i++)
  {
    hueControl[i] = hueControl[i] + (5 * LED_IN_BOX);
    if (hueControl[i] > 255)hueControl[i]-=255;
  }
}

// We initialize our array 
void hueControlInit()
{
  for (int i=0;i<NUM_BOXES;i++)
  {
    hueControl[i] = 5 * i * LED_IN_BOX;
  }
}


// We initialize our second array that we use for another effect
void hueLEDSInit()
{
  for (int i=0;i<TOTAL_LEDS;i++)
  {
    hueLEDS[i] = i;
  }
}

// This function will exectute while the mode is active. Basically it changes a whole LED Box to its next hue color
void rainbowMode() 
{

  if (animControl.ms() > animTime)
  {
    for (int i=0;i<LED_IN_BOX;i++)
    {
      for (int j=0;j<NUM_BOXES;j++) // 5 veces
      {
        int hue = hueControl[j] + 5 * (i+1);
        if (hue>255)hue-=255;
        leds[j*LED_IN_BOX+i] = CHSV(hue , 255, 255);
        FastLED.show();
      }
      delay(rDelay);
    }
    
    animControl.restart();
    updateHueControl();
  }
}

// This function will execute while the mode is active. Bascially it changes all the hues of all LED Box, making a Rainbow Line Effect.
void rainbowLineMode()
{
  if (animControl.ms() > animTime)
  {
    for (int i=0;i<TOTAL_LEDS;i++)
    {
      hueLEDS[i]+= hueAdd;
      if (hueLEDS[i]>255)hueLEDS[i]-=255;
      leds[i] = CHSV(hueLEDS[i],255,255);
      FastLED.show();
      delay(rDelay);
    }
    animControl.restart();
  }
}

// This function will execute while the mode is active. Just stays static (one color).
void staticMode()
{
  for (int i=0;i<TOTAL_LEDS;i++)
  {
    leds[i]=primary_color;
  }
  FastLED.show();
}

// This function will execute while it is active. It changes between two color, making a good gradient between both.
void cycleMode()
{
  if (lastMode != hexaMode) // If this is the first time we activate the mode, make it static for a second
  {
    staticMode();
  }
  else if (animControl.ms() > animTime)
  {
    if (randomCycle == 1) // If the user chose to make it random
    {
      for (int i=0;i<20;i++) // We just change the current color to the next
      {
        hueCycleMode++;
        if (hueCycleMode > 255)hueCycleMode-=255;
        for (int j=0;j<TOTAL_LEDS;j++)
        {
          leds[j] = CHSV(hueCycleMode,255,255);
        }
        delay(rDelay);
        FastLED.show();
      }
    }
    else
    {
      if (wasRandom == 1) // If it was random before, we initialize the mode by making it the primary color
      {
        staticMode();
      }

      // We get the differences between the first and second color
      
      int r_diff = primary_color.r - secondary_color.r;
      int g_diff = primary_color.g - secondary_color.g;
      int b_diff = primary_color.b - secondary_color.b;

      // How much hue we will add for every step
      float r_add = r_diff / STEPS;
      float g_add = g_diff / STEPS;
      float b_add = b_diff / STEPS;

      // Current RGB values
      float r_curr = primary_color.r;
      float g_curr = primary_color.g;
      float b_curr = primary_color.b;

      // The delay between steps
      int div_delay = rDelay / STEPS;

      // We add gradually the hue
      for (int i=0;i<STEPS;i++)
      {
        r_curr += r_add;
        g_curr += g_add;
        b_curr += b_add;
        
        for (int j=0;j<TOTAL_LEDS;j++)
        {  
          leds[j].red = (int) r_curr;
          leds[j].green = (int) g_curr;
          leds[j].blue = (int) b_curr; 
        }
        FastLED.delay(div_delay);
      }

      delay(animTime);

      // We do the same, but backwards, to complete the animation.
      
      r_add *= -1;
      g_add *= -1;
      b_add *= -1;

      for (int i=0;i<STEPS;i++)
      {
        r_curr += r_add;
        g_curr += g_add;
        b_curr += b_add;
        
        for (int j=0;j<TOTAL_LEDS;j++)
        {  
          leds[j].red = (int) r_curr;
          leds[j].green = (int) g_curr;
          leds[j].blue = (int) b_curr; 
        }
        FastLED.delay(div_delay);
      }
    }
  }
}

// This function will execute while the mode is active. This is a funny one. It changes color randomly like if it were a disco.
void discoMode()
{
  if (randomCycle == 1) // If random buttom is activated, then choose a random color between 1 and 255
  {
    int hueColor = random(1,255);
    for (int i=0;i<TOTAL_LEDS;i++)
    {
      leds[i] = CHSV(hueColor,255,255);
    }
    FastLED.delay(animTime);
  }
  else // Just change between the primary/secondary colors
  {
    for (int i=0;i<TOTAL_LEDS;i++)
    {
      leds[i].r = primary_color.r;
      leds[i].g = primary_color.g;
      leds[i].b = primary_color.b;
    }
    FastLED.delay(animTime);
    for (int i=0;i<TOTAL_LEDS;i++)
    {
      leds[i].r = secondary_color.r;
      leds[i].g = secondary_color.g;
      leds[i].b = secondary_color.b;
    }
    FastLED.delay(animTime);
  }
}

// A function to turn off the lamps.
void off()
{
  for (int i=0;i<TOTAL_LEDS;i++)
  {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
}


// Debug Purposes
void testMode()
{
  for (int i=0;i<LED_IN_BOX;i++)
  {
    leds[i] = CRGB::Red;
    FastLED.show();
    delay(1000);
  }
}

// Debug purposes. Ensure the LEDS (physically) are RGB. 
void rgbTest()
{
  int red = 0;
  int blue = 0;
  int green = 0;
  
  for (int i=0;i<3;i++)
  {
    red = 0;
    blue = 0;
    green = 0;
    
    for (int j=0;j<255;j++)
    {
      if (i==0)red++;
      else if (i==1)green++;
      else if (i==2)blue++;
      
      for (int k=0;k<TOTAL_LEDS;k++)
      {
        leds[k].red = red;
        leds[k].green = green;
        leds[k].blue = blue;
      }
      FastLED.show();
      delay(40);
    }
  }
}

// This funcitons sets the mode based on the phone's input
void setMode()
{
  switch(hexaMode)
  {
    case 1: // Rainbow
    {
      Serial.println("Rainbow Mode ->");
      rainbowMode();
      break;
    }
    case 2: // Line Rainbow
    {
      Serial.println("Smooth Rainbow ->");
      rainbowMode2();
      break;
    }
    case 3: // Static Color
    {
      Serial.println("Static Mode ->");
      staticMode();
      break;
    }
    case 4: // Cycle Mode
    {
      Serial.println("Cycle Mode ->");
      cycleMode();
      break;
    }
    case 5: // Disco Mode
    {
      Serial.println("Disco Mode ->");
      discoMode();
      break;
    }
    case 6: // Off
    {
      Serial.println("Off Mode ->");
      off();
      break;
    }
    case 7:
    {
      Serial.println("RGB Test ->");
      rgbTest();
      break;
    }
  }
}

// Updates the mode
void hexaUpdate()
{
  setMode();
}

// -----------------

void setup() {
  Serial.begin(9600);
  Serial.println("SETUP ->");
  animControl.restart();
  hueControlInit();
  hueLEDSInit();
  FastLED.addLeds<WS2811, LED_PIN, BRG>(leds, TOTAL_LEDS); // Blue Red Green
  Blynk.begin(auth,ssid,pass);
}

// The BLYNK app works with block, so for example V1 is a menu block. We use BLYNK_WRITE to read what's coming from that block.
BLYNK_WRITE(V1) // Read Mode
{
  lastMode = hexaMode;
  hexaMode = param.asInt();
}

BLYNK_WRITE(V2) // Read Primary Color
{
  primary_color.r = param[0].asInt();
  primary_color.g = param[1].asInt();
  primary_color.b = param[2].asInt();
}

BLYNK_WRITE(V3) // Read Secondary Color
{
  secondary_color.r = param[0].asInt();
  secondary_color.g = param[1].asInt();
  secondary_color.b = param[2].asInt();
}

BLYNK_WRITE(V4) // Read duration between animations
{
  animTime = param.asInt();
}

BLYNK_WRITE(V5) // Red how much hue we need to add to animations
{
  hueAdd = param.asInt();
}

BLYNK_WRITE(V6) // Read the delay between every step
{
  rDelay = param.asInt();
}

BLYNK_WRITE(V7) // Read if we want random colors
{
  wasRandom = randomCycle;
  randomCycle = param.asInt();
}

BLYNK_WRITE(V8) // Read how much delay we want for cycle mode
{
  cycleDelay = param.asInt();
}

// Update modes
void loop() {
  Serial.print("Mode ");
  Serial.print(hexaMode);
  Serial.print(" ");
  hexaUpdate();
  Blynk.run();
}
