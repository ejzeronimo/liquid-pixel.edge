#include <Adafruit_NeoPixel.h>
#include <avr/wdt.h>

#define PIN 6;
#define PIXEL_LEN 58;

int pixelLen = PIXEL_LEN;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(pixelLen, 6, NEO_RGB + NEO_KHZ800);
int brightness = 0;

// sweep variables
int horizEdgeLen = 11;
int vertEdgeLen = 16;
int leftEdgeStart = 0;
int leftWalk = 0;
int rightEdgeStart = 0;
int rightWalk = 0;
bool isWalking = false;
bool isWalkingDone = false;

//*****************************************************************************
//T0C1R255G0B255D50X0W1M0~T0C3R177G201B99M1D100X1~T0C1R77G20B9M1D25X0~
enum SegmentKey : byte
{
  Tk = (byte)'T',
  Ck = (byte)'C',
  Rk = (byte)'R',
  Gk = (byte)'G',
  Bk = (byte)'B',
  Mk = (byte)'M',
  Dk = (byte)'D',
  Wk = (byte)'W',
  Xk = (byte)'X',
  EocK = (byte)'~',
  UnknownK = 0
};

enum Type : byte
{
  Cmd = 0,
  Data,
  Live,
  Ext,
  Wdtr = 99
};

enum Mode : byte
{
  Off = 0,
  Solid,
  RandomCloudy,
  Flash,
  Sweep,
  Twinkle,
  RandomTwinkle,
  Randomflash,
  TheaterChase,
  FadeIO,
  chroma,
  Breath,
  FadeIn,
  FadeOut,
  RandomFadeIO
};

struct LiquidPixelCommand
{
  byte Type;
  byte BoxId;
  byte R;
  byte G;
  byte B;
  byte Mode;
  int DelayMs;
  bool DoWipe;
  bool IsRandom;
};

const byte MAX_SEGMENT_LENGTH_IN_BYTES = 3;
byte segmentKey = (byte)UnknownK;
byte segmentBytesTemp[MAX_SEGMENT_LENGTH_IN_BYTES] = {0, 0, 0};
byte numberOfBytesInSegment = 0;
LiquidPixelCommand lpc;

void ProcessByte(byte commandByte)
{
  switch (commandByte)
  {
    case (byte)Tk:
    case (byte)Ck:
    case (byte)Rk:
    case (byte)Gk:
    case (byte)Bk:
    case (byte)Mk:
    case (byte)Dk:
    case (byte)Wk:
    case (byte)Xk:
      ReconstructSegment();
      segmentKey = (byte)commandByte;
      break;
    case (byte)EocK:
      ReconstructSegment();
      segmentKey = (byte)commandByte;
      ProcessLiquidPixelCommand();
      break;
    default:
      segmentBytesTemp[numberOfBytesInSegment] = commandByte;
      numberOfBytesInSegment++;
      break;
  }
}

void ReconstructSegment()
{
  if (segmentKey != (byte)UnknownK && segmentKey != (byte)EocK)
  {
    byte segmentBytes[numberOfBytesInSegment];
    memcpy(segmentBytes, segmentBytesTemp, numberOfBytesInSegment);
    String segmentString = String((char *)segmentBytes);
    int segmentValue = segmentString.toInt();

    // populate Liquid Pixel Command
    switch (segmentKey)
    {
      case (byte)Tk:
        lpc.Type = segmentValue;
        break;
      case (byte)Ck:
        lpc.BoxId = segmentValue;
        break;
      case (byte)Rk:
        lpc.R = segmentValue;
        break;
      case (byte)Gk:
        lpc.G = segmentValue;
        break;
      case (byte)Bk:
        lpc.B = segmentValue;
        break;
      case (byte)Mk:
        lpc.Mode = segmentValue;
        break;
      case (byte)Dk:
        lpc.DelayMs = segmentValue;
        break;
      case (byte)Wk:
        lpc.DoWipe = (bool)segmentValue;
        break;
      case (byte)Xk:
        lpc.IsRandom = (bool)segmentValue;
        break;
      default:
        break;
    }

    // reset number of bytes since we are done with this segment
    numberOfBytesInSegment = 0;
    segmentBytesTemp[0] = 0;
    segmentBytesTemp[1] = 0;
    segmentBytesTemp[2] = 0;
  }
}

void ProcessLiquidPixelCommand()
{
  //  if(lpc.Type == Wdtr)
  //  {
  //    wdt_reset();
  //    return;
  //  }

  if (lpc.DoWipe)
    off();

  switch (lpc.Mode)
  {
    case Sweep:
      leftEdgeStart = pixelLen - 1;
      leftWalk = leftEdgeStart;
      rightEdgeStart = horizEdgeLen;
      rightWalk = rightEdgeStart;
      break;
    case FadeIn:
      brightness = 0;
      break;
    case FadeOut:
      brightness = 255;
      break;
    default:
      break;
  }
  String commandType = "Command type is: ";
  Serial.println(commandType += lpc.Type);
  String BoxIdNumber = "BoxId number is: ";
  Serial.println(BoxIdNumber += lpc.BoxId);
  String redValue = "Red value is: ";
  Serial.println(redValue += lpc.R);
  String greenValue = "Green value is: ";
  Serial.println(greenValue += lpc.G);
  String blueValue = "Blue value is: ";
  Serial.println(blueValue += lpc.B);
  String mode = "Mode is: ";
  Serial.println(mode += lpc.Mode);
  String DelayMs = "DelayMs is: ";
  Serial.println(DelayMs += lpc.DelayMs);
  String doWipe = "Wipe is: ";
  Serial.println(doWipe += lpc.DoWipe);
  String isRandom = "Random is: ";
  Serial.println(isRandom += lpc.IsRandom);
}
//*****************************************************************************

void off()
{
  for (int i = 0; i <= pixelLen; i++) {
    strip.setPixelColor(i, 0, 0, 0);
  }
  strip.show();
}
void solid(byte r, byte g, byte b) {
  for (int i = 0; i <= pixelLen; i++) {
    strip.setPixelColor(i, r, g, b);
  }
  strip.show();
}

void flash(byte r, byte g, byte b, int delayMs) {
  for (int i = pixelLen; i >= 0; i--) {
    strip.setPixelColor(i, r, g, b);
  }

  strip.show();
  delay(delayMs);

  for (int i = pixelLen; i >= 0; i--) {
    strip.setPixelColor(i, 0, 0, 0);
  }

  strip.show();
  delay(delayMs);
}

void randomflash(byte delayMs) {
  for (int i = 0; i <= pixelLen; i += 3) {
    strip.setPixelColor(i, random(255), random(255), random(255));
    strip.setPixelColor(i + 1, random(255), random(255), random(255));
    strip.setPixelColor(i + 2, random(255), random(255), random(255));
  }

  strip.show();
  delay(delayMs);

  for (int i = pixelLen; i >= 0; i--) {
    strip.setPixelColor(i, 0, 0, 0);

  }

  strip.show();
  delay(delayMs);
}

void sweep(byte r, byte g, byte b, byte delayMs) {
  if (!isWalking && isWalkingDone)
  {
    // top on
    for (int i = rightEdgeStart + vertEdgeLen; i <= rightEdgeStart + vertEdgeLen + horizEdgeLen; i++)
    {
      strip.setPixelColor(i, r, g, b);
    }
    strip.show();

    delay(delayMs);

    // top off
    for (int i = rightEdgeStart + vertEdgeLen; i <= rightEdgeStart + vertEdgeLen + horizEdgeLen; i++)
    {
      strip.setPixelColor(i, 0, 0, 0);
    }
    strip.show();
  }

  if (!isWalking)
  {
    // bottom on
    for (int i = rightEdgeStart - horizEdgeLen; i < rightEdgeStart; i++)
    {
      strip.setPixelColor(i, r, g, b);
    }
    strip.show();

    delay(delayMs);

    // bottom off
    for (int i = rightEdgeStart - horizEdgeLen; i < rightEdgeStart; i++)
    {
      strip.setPixelColor(i, 0, 0, 0);
    }
    strip.show();
  }

  isWalkingDone = false;
  isWalking = true;

  // right walk
  strip.setPixelColor(rightWalk, r, g, b);
  if (rightWalk != horizEdgeLen)
    strip.setPixelColor(rightWalk - 1, 0, 0, 0);

  // left walk
  strip.setPixelColor(leftWalk, r, g, b);
  if (leftWalk != leftEdgeStart)
    strip.setPixelColor(leftWalk + 1, 0, 0, 0);
  strip.show();

  strip.show();
  rightWalk++;
  leftWalk--;
  delay(delayMs);

  // walk is done
  if (rightWalk == rightEdgeStart + vertEdgeLen || leftWalk == leftEdgeStart - vertEdgeLen)
  {
    isWalking = false;
    isWalkingDone = true;

    strip.setPixelColor(rightWalk - 1, 0, 0, 0);
    strip.setPixelColor(leftWalk + 1, 0, 0, 0);
    strip.show();

    rightWalk = rightEdgeStart;
    leftWalk = leftEdgeStart;
  }
}

void theaterChase(byte r, byte g, byte b, byte delayMs) {
  for (int j = 0; j < 10; j++) { //do 10 cycles of chasing
    for (int q = 0; q < 3; q++) {
      for (int i = 0; i < pixelLen; i = i + 3) {
        strip.setPixelColor(i + q, r, g, b);  //turn every third pixel on
      }
      strip.show();

      delay(delayMs);

      for (int i = 0; i < pixelLen; i = i + 3) {
        strip.setPixelColor(i + q, 0, 0, 0);    //turn every third pixel off
      }
    }
  }
}

void twinkle(byte r, byte g, byte bui, byte delayMs) {
  byte pos1 = random(pixelLen);
  byte pos2 = random(pixelLen);
  byte pos3 = random(pixelLen);
  for (int b = 0; b < 255; b++) {
    strip.setPixelColor(pos1, r * b / 255, g * b / 255, bui * b / 255);
    strip.setPixelColor(pos2, r * b / 255, g * b / 255, bui * b / 255);
    strip.setPixelColor(pos3, r * b / 255, g * b / 255, bui * b / 255);
  }
  strip.show();
  delay(delayMs);
  for (int b = 255; b > 0; b--) {
    strip.setPixelColor(pos1, r * b / 255, g * b / 255, bui * b / 255);
    strip.setPixelColor(pos2, r * b / 255, g * b / 255, bui * b / 255);
    strip.setPixelColor(pos3, r * b / 255, g * b / 255, bui * b / 255);
  }
  strip.show();
  delay(delayMs);
}

void randomtwinkle(byte delayMs) {
  int rndPixel = random(pixelLen);

  strip.setPixelColor(rndPixel, 255, 255, 255);
  strip.show();
  delay(delayMs);

  strip.setPixelColor(rndPixel, 0, 0, 0);
  strip.show();
  delay(delayMs);
}

void fadein(byte r, byte g, byte b, byte delayMs) {
  if (brightness <= 255) {

    for (int i = 0; i < 58; i++) {
      strip.setPixelColor(i, r, g, b);
    }

    strip.setBrightness(brightness);
    strip.show();

    ++brightness;
    delay(delayMs);
  }
}

void fadeout(byte r, byte g, byte b, byte delayMs) {
  if (brightness >= 0) {

    for (int i = 0; i < pixelLen; i++) {
      strip.setPixelColor(i, r, g, b);
    }

    strip.setBrightness(brightness);
    strip.show();

    --brightness;
    delay(delayMs);
  }
}

void chooser()
{
  switch (lpc.Mode)
  {
    case Off:
      off();
      break;
    case Solid:
      solid(lpc.R, lpc.G, lpc.B);
      break;
    case Flash:
      flash(lpc.R, lpc.G, lpc.B, lpc.DelayMs);
      break;
    case Sweep:
      sweep(lpc.R, lpc.G, lpc.B, lpc.DelayMs);
      break;
    case Twinkle:
      twinkle(lpc.R, lpc.G, lpc.B, lpc.DelayMs);
      break;
    case RandomTwinkle:
      randomtwinkle(lpc.DelayMs);
      break;
    case Randomflash:
      randomflash(lpc.DelayMs);
      break;
    case TheaterChase:
      theaterChase(lpc.R, lpc.G, lpc.B, lpc.DelayMs);
      break;
    case FadeIn:
      fadein(lpc.R, lpc.G, lpc.B, lpc.DelayMs);
      break;
    case FadeOut:
      fadeout(lpc.R, lpc.G, lpc.B, lpc.DelayMs);
      break;
    default:
      break;
  }
}

void setup()
{
  // immediately disable watchdog timer so set will not get interrupted

  //wdt_disable();

  // I often do serial i/o at startup to allow the user to make config changes of
  // various constants. This is often using fgets which will wait for user input.
  // any such 'slow' activity needs to be completed before enabling the watchdog timer.

  // the following forces a pause before enabling WDT. This gives the IDE a chance to
  // call the bootloader in case something dumb happens during development and the WDT
  // resets the MCU too quickly. Once the code is solid, remove this.

  Serial.begin(9600);
  strip.begin();
  strip.show();
  randomSeed(123);

  //delay(2L * 1000L);

  // enable the watchdog timer. There are a finite number of timeouts allowed (see wdt.h).
  // Notes I have seen say it is unwise to go below 250ms as you may get the WDT stuck in a
  // loop rebooting.
  // The timeouts I'm most likely to use are:
  // WDTO_1S
  // WDTO_2S
  // WDTO_4S
  // WDTO_8S

  //wdt_enable(WDTO_4S);

  // initialize the digital pin as an output.
  // Pin 13 has an LED connected on most Arduino boards:

  //pinMode(onboardLED, OUTPUT);

  // at bootup, flash LED 3 times quick so I know the reboot has occurred.

  //  for (k = 1; k <= 3; k = k + 1) {
  //    digitalWrite(onboardLED, HIGH);
  //    delay(250L);
  //    digitalWrite(onboardLED, LOW);
  //    delay(250L);
  //  }

  // delay a bit more so it is clear we are done with setup
  //solid(255,255,255);
  //delay(750L);
}

void loop()
{
  if (Serial.available() > 0)
  {
    byte byteFromSerialPort = Serial.read();
    ProcessByte(byteFromSerialPort);
  }
  else
  {
    chooser();
  }
}
