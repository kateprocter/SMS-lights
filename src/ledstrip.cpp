#include <neopixel.h>
#include <ledstrip.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>


#define PIXEL_TYPE          WS2812B
#define PIN                 D2
#define NUMPIXELS           150
#define MAX_COLOURS         10
#define MAX_CHANGE_STEPS    20
#define FADE_STEPS          16
#define BLEND_STEPS         8
#define BRIGHTNESS          64

#define RGB_RED     0xFF0000
#define RGB_BLUE    0x0000FF
#define RGB_GREEN   0x00FF00
#define RGB_YELLOW  0xFFFF00
#define RGB_CYAN    0x00FFFF
#define RGB_MAGENTA 0xFF00FF
#define RGB_WHITE   0xFFFFFF
#define RGB_PINK    0xFF66FF
#define RGB_PURPLE  0x4d0066
#define RGB_ORANGE  0xFF6600
#define RGB_VIOLET  0x9900FF
#define RGB_OFF     0x000000

#define FLASH_RATE  500
#define FADE_RATE   200
#define CHASE_RATE  100
#define CYCLE_RATE  500

static void setupLights();
static void updateLights();
static void updateLightsStatic(void);
static void updateLightsBlend(void);
static void updateLightsFlash(void);
static void updateLightsChase(void);
static void updateLightsFade(void);
static void updateLightsCycle(void);
static bool parseMessage(char * message);
static bool isRGBColour(char * str);
static bool isNumber(char * str);
static void lowerCase(char * str);

typedef enum
{
    LED_NONE,
    LED_BLEND,
    LED_FADE,
    LED_FLASH,
    LED_CHASE,
    LED_CYCLE

}LED_EFFECT;

typedef struct
{
    long rgb;
    int  repeat;

}LED_COLOUR;


const char space[2] = " ";

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, PIXEL_TYPE);
Timer updateTimer(200, updateLights, FALSE);

static LED_COLOUR colours[MAX_COLOURS];
static int numColours;

static LED_EFFECT ledEffect;

static int updateTime;


void ledStripInit(void)
{

  pixels.begin(); // This initializes the NeoPixel library.
  pixels.setBrightness(BRIGHTNESS);

  //Initial LED sequence

  for(int l=0; l< NUMPIXELS; l++)
  {
      pixels.setPixelColor(l,RGB_CYAN);
      delay(50);
      pixels.show();

  }


}

//Take new message and attempt to parse. If it contains any recognised
//colours, lights are updated.
//Unrecognised words in message are ignored.

bool ledStripNewMessage(char * message)
{

    if(parseMessage(message))
    {
        updateTimer.stop();
        setupLights();
        if(updateTime != 0)
        {
            updateTimer.changePeriod(updateTime);
            updateTimer.start();
        }
        return TRUE;
    }

    return FALSE;

}


void setupLights()
{

    switch(ledEffect)
    {
        case LED_NONE:
            updateLightsStatic();
            updateTime = 0;
            break;

        case LED_BLEND:
            if(numColours == 1)
            {
                colours[numColours++].rgb = RGB_OFF;
            }
            updateLightsBlend();
            updateTime = 0;
            break;

        case LED_FADE:
            if(numColours == 1 )
            {
                colours[numColours++].rgb = RGB_OFF;
            }
            updateLightsFade();
            updateTime = FADE_RATE;
            break;

        case LED_FLASH:
            updateLightsFlash();
            updateTime = FLASH_RATE;
            break;

        case LED_CHASE:
            updateLightsChase();
            updateTime = CHASE_RATE;
            break;

        case LED_CYCLE:
             updateLightsCycle();
             updateTime = CYCLE_RATE;
             break;

        default:
             break;

    }
}


static void updateLights()
{

    switch(ledEffect)
    {
        case LED_NONE:
            break;

        case LED_BLEND:
            break;

        case LED_FADE:
            updateLightsFade();
            break;

        case LED_FLASH:
            updateLightsFlash();
            break;

        case LED_CHASE:
            updateLightsChase();
            break;

        case LED_CYCLE:
            updateLightsCycle();
            break;

        default:
            break;
    }
}

void updateLightsStatic(void)
{

    int led=0;
    int colour=0;
    int count = 0;
    long ledColour;


    while(led < NUMPIXELS)
    {
        ledColour = colours[colour].rgb;
        count++;

        pixels.setPixelColor(led++, ledColour);

        if(count == colours[colour].repeat)
        {
            count = 0;
            colour = (colour + 1) % numColours;
        }
    }

    pixels.show();

}

void updateLightsFlash(void)
{

    int led=0;
    int colour=0;
    int count = 0;
    long ledColour;
    static bool lightsOn = false;


    if(lightsOn)
    {
        for(led=0; led< NUMPIXELS; led++)
        {
            pixels.setPixelColor(led, RGB_OFF);
        }
        pixels.show();
        lightsOn = false;
    }
    else
   {

        while(led < NUMPIXELS)
        {
            ledColour = colours[colour].rgb;
            count++;

            pixels.setPixelColor(led++, ledColour);

            if(count == colours[colour].repeat)
            {
                count = 0;
                colour = (colour + 1) % numColours;
            }
        }
        pixels.show();
        lightsOn = true;
   }

}

void updateLightsChase(void)
{


    int led=0;
    int colour=0;
    int count = 0;
    static int startLed = 0;
    long ledColour;


    while(led < NUMPIXELS)
    {
        ledColour = colours[colour].rgb;
        count++;

        pixels.setPixelColor(((led + startLed) % NUMPIXELS), ledColour);
        led++;

        if(count == colours[colour].repeat)
        {
            count = 0;
            colour = (colour + 1) % numColours;
        }
    }

    pixels.show();
    startLed = (startLed + 1) % NUMPIXELS;
}

void updateLightsBlend(void)
{
    int blendCount=0;
    int led;
    long ledColour;
    int colour=0;
    long RStart, REnd, RChange;
    long BStart, BEnd, BChange;
    long GStart, GEnd, GChange;

    for(led=0; led < NUMPIXELS; led++)
    {
        if(blendCount == 0)
        {

            ledColour = colours[colour].rgb;
            RStart = colours[colour].rgb >> 16;
            REnd = colours[(colour+1) % numColours].rgb >> 16;
            RChange = (REnd - RStart) / BLEND_STEPS;

            GStart = (colours[colour].rgb >> 8) & 0xFF;
            GEnd = (colours[(colour+1) % numColours].rgb >> 8) & 0xFF;
            GChange = (GEnd - GStart) / BLEND_STEPS;

            BStart = colours[colour].rgb & 0xFF;
            BEnd = colours[(colour+1) % numColours].rgb & 0xFF;
            BChange = (BEnd - BStart) / BLEND_STEPS;

         }
        else
        {

            ledColour = ((RStart + (RChange * blendCount)) << 16) +
                        ((GStart + (GChange * blendCount)) << 8) +
                        (BStart + (BChange * blendCount));

        }

        pixels.setPixelColor(led, ledColour);

        blendCount++;
        if(blendCount == BLEND_STEPS)
        {
            blendCount = 0;
            colour = (colour + 1) % numColours;
        }
    }

    pixels.show();

}

void updateLightsFade(void)
{
    static int fadeCount=0;
    int led;
    long ledColour;
    static int colour = 0;
    static long RStart, REnd, RChange;
    static long BStart, BEnd, BChange;
    static long GStart, GEnd, GChange;

    if(fadeCount == 0)
    {
        ledColour = colours[colour].rgb;
        RStart = colours[colour].rgb >> 16;
        REnd = colours[(colour+1) % numColours].rgb >> 16;
        RChange = (REnd - RStart) / FADE_STEPS;

        GStart = (colours[colour].rgb >> 8) & 0xFF;
        GEnd = (colours[(colour+1) % numColours].rgb >> 8) & 0xFF;
        GChange = (GEnd - GStart) / FADE_STEPS;

        BStart = colours[colour].rgb  & 0xFF;
        BEnd = colours[(colour+1) % numColours].rgb & 0xFF;
        BChange = (BEnd - BStart) / FADE_STEPS;
    }
    else
    {

        ledColour = ((RStart + (RChange * fadeCount)) << 16) +
                    ((GStart + (GChange * fadeCount)) << 8) +
                    (BStart + (BChange * fadeCount));

    }

    for(led=0; led < NUMPIXELS; led++)
    {

        pixels.setPixelColor(led, ledColour);
    }

    fadeCount++;

    if(fadeCount == FADE_STEPS)
    {
        fadeCount = 0;
        colour = (colour + 1) % numColours;
    }

    pixels.show();


}


void updateLightsCycle(void)
{

    int led=0;
    static int colour = 0;
    static int count = 0;
    long ledColour;

    ledColour = colours[colour].rgb;

    for(led=0; led< NUMPIXELS; led++)
    {
        pixels.setPixelColor(led, ledColour);
    }

    pixels.show();

    count++;

    if(count == colours[colour].repeat)
    {
        count = 0;
        colour = (colour + 1) % numColours;
    }

}



static bool parseMessage(char * message)
{

    char * token;
    bool prevTokenColour = TRUE;
    int repeat;

    int numNewColours = 0;
    LED_COLOUR newColours[MAX_COLOURS];
    LED_EFFECT newLedEffect = LED_NONE;

    token = strtok(message, space);


    while(token != NULL)
    {

        lowerCase(token);

        if(isRGBColour(token))
        {
            newColours[numNewColours].rgb = strtoul(token, NULL, 16);
            newColours[numNewColours++].repeat = 1;
            prevTokenColour = TRUE;
        }
        else if(strcmp(token, "red")==0 && numNewColours < MAX_COLOURS)
        {
            newColours[numNewColours].rgb = RGB_RED;
            newColours[numNewColours++].repeat = 1;
            prevTokenColour = TRUE;
        }

        else if(strcmp(token, "blue")==0 && numNewColours < MAX_COLOURS)
        {
            newColours[numNewColours].rgb= RGB_BLUE;
            newColours[numNewColours++].repeat = 1;
            prevTokenColour = TRUE;
        }
        else if(strcmp(token, "green")==0 && numNewColours < MAX_COLOURS)
        {
            newColours[numNewColours].rgb = RGB_GREEN;
            newColours[numNewColours++].repeat = 1;
            prevTokenColour = TRUE;
        }
        else if(strcmp(token, "yellow")==0 && numNewColours < MAX_COLOURS)
        {
            newColours[numNewColours].rgb = RGB_YELLOW;
            newColours[numNewColours++].repeat = 1;
            prevTokenColour = TRUE;
        }
        else if(strcmp(token, "magenta")==0 && numNewColours < MAX_COLOURS)
        {
            newColours[numNewColours].rgb = RGB_MAGENTA;
            newColours[numNewColours++].repeat = 1;
            prevTokenColour = TRUE;
        }
        else if(strcmp(token, "cyan")==0 && numNewColours < MAX_COLOURS)
        {
            newColours[numNewColours].rgb = RGB_CYAN;
            newColours[numNewColours++].repeat = 1;
            prevTokenColour = TRUE;
        }
        else if(strcmp(token, "pink")==0 && numNewColours < MAX_COLOURS)
        {
            newColours[numNewColours].rgb = RGB_PINK;
            newColours[numNewColours++].repeat = 1;
            prevTokenColour = TRUE;
        }
        else if(strcmp(token, "purple")==0 && numNewColours < MAX_COLOURS)
        {
            newColours[numNewColours].rgb = RGB_PURPLE;
            newColours[numNewColours++].repeat = 1;
            prevTokenColour = TRUE;
        }
        else if(strcmp(token, "white")==0 && numNewColours < MAX_COLOURS)
        {
            newColours[numNewColours].rgb = RGB_WHITE;
            newColours[numNewColours++].repeat = 1;
            prevTokenColour = TRUE;
        }
        else if(strcmp(token, "violet")==0 && numNewColours < MAX_COLOURS)
        {
            newColours[numNewColours].rgb = RGB_VIOLET;
            newColours[numNewColours++].repeat = 1;
            prevTokenColour = TRUE;
        }
            else if(strcmp(token, "orange")==0 && numNewColours < MAX_COLOURS)
        {
            newColours[numNewColours].rgb = RGB_ORANGE;
            newColours[numNewColours++].repeat = 1;
            prevTokenColour = TRUE;
        }
        else if(strcmp(token, "off")==0 && numNewColours < MAX_COLOURS)
        {
            newColours[numNewColours].rgb = RGB_OFF;
            newColours[numNewColours++].repeat = 1;
            prevTokenColour = TRUE;
        }
        else if(strcmp(token, "blend")==0)
        {
            newLedEffect = LED_BLEND;
            prevTokenColour = FALSE;
        }
        else if(strcmp(token, "flash")==0)
        {
            newLedEffect = LED_FLASH;
            prevTokenColour = FALSE;

        }
        else if(strcmp(token, "chase")==0)
        {
            newLedEffect = LED_CHASE;
            prevTokenColour = FALSE;

        }
        else if(strcmp(token, "fade")==0)
        {
            newLedEffect = LED_FADE;
            prevTokenColour = FALSE;

        }
        else if(strcmp(token, "cycle")==0)
        {
            newLedEffect = LED_CYCLE;
            prevTokenColour = FALSE;

        }
        else if(isNumber(token) && prevTokenColour)
        {
            repeat = atoi(token);

            repeat = min(repeat, NUMPIXELS);
            repeat = max(repeat, 0);

            newColours[numNewColours-1].repeat = repeat;
            prevTokenColour = FALSE;

        }

        token = strtok(NULL, space);
  }

  if(numNewColours > 0)
  {
      numColours = numNewColours;

      for(int i=0; i<numColours; i++)
      {
          colours[i] = newColours[i];
      }

      ledEffect = newLedEffect;

      return TRUE;
  }

  return FALSE;


}

static bool isRGBColour(char * str)
{
    int i=0;

    if(strlen(str) != 6)
    {
        return false;
    }

    while(str[i])
    {
        if(!isxdigit(str[i++]))
        {
            return false;
        }
    }

    return true;
}

static bool isNumber(char * str)
{
    int i =0;

    while(str[i])
    {
        if(!isdigit(str[i++]))
        {
            return false;
        }
    }

    return true;
}

static void lowerCase(char * str)
{
    int i=0;

    while(str[i])
    {
        str[i] = tolower(str[i]);
        i++;
    }
}
