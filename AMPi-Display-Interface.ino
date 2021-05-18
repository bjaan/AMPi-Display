//ICONS - tool to create at http://javl.github.io/image2cpp/ - use 16x16 monochrome images & set white background & set use invert image colors & set plain bytes & set Horizontal 1 byte per pixel draw mode
const unsigned char IMG_POWER [] PROGMEM = { 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x0d, 0xb0, 0x1d, 0xb8, 0x39, 0x9c, 0x31, 0x8e, 0x60, 0x06, 0x60, 0x06, 0x60, 0x06, 0x60, 0x06, 0x30, 0x0c, 0x38, 0x1c, 0x1c, 0x38, 0x0f, 0xf0, 0x03, 0xc0 };
const unsigned char IMG_AIRPLAY [] PROGMEM = { 0x07, 0xe0, 0x08, 0x10, 0x30, 0x0c, 0x43, 0xc2, 0x44, 0x22, 0x49, 0x92, 0x92, 0x49, 0x94, 0x29, 0x94, 0x29, 0x94, 0x29, 0x90, 0x09, 0x49, 0x92, 0x43, 0xc2, 0x27, 0xe4, 0x0f, 0xf0, 0x0f, 0xf0 };
const unsigned char iMG_CHROMECAST_AUDIO [] PROGMEM = { 0x0f, 0xf8, 0x1f, 0xfc, 0x1e, 0x3c, 0x1e, 0x3c, 0x1f, 0x7c, 0x1f, 0xfc, 0x1c, 0x1c, 0x18, 0x0c, 0x11, 0xc4, 0x13, 0xe4, 0x00, 0xe4, 0x1e, 0x64, 0x03, 0x44, 0x19, 0x0c, 0x05, 0x78, 0x15, 0x00 };
const unsigned char IMG_PANDORA_MUSIC [] PROGMEM = { 0x7f, 0xf0, 0x40, 0x08, 0x40, 0x04, 0x40, 0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x04, 0x40, 0x08, 0x47, 0xf0, 0x44, 0x00, 0x44, 0x00, 0x44, 0x00, 0x7c, 0x00 };
const unsigned char IMG_APPLE_MUSIC [] PROGMEM = { 0x3f, 0xfc, 0x7f, 0xfe, 0xff, 0xc7, 0xfc, 0x07, 0xf8, 0x07, 0xf8, 0x27, 0xf9, 0xe7, 0xf9, 0xe7, 0xf9, 0xe7, 0xf9, 0xc7, 0xf1, 0x87, 0xe1, 0x87, 0xe1, 0xcf, 0xf3, 0xff, 0x7f, 0xfe, 0x3f, 0xfc };

//TEXT
const char TXT_TURN_STREAMER_ON [] PROGMEM = "Turn Streamer on";
const char TXT_TURN_STREAMER_ON2 [] PROGMEM = "Turn Streamer on!";
const char TXT_TURN_STREAMER_OFF [] PROGMEM = "Turn Streamer off";
const char TXT_AMPI [] PROGMEM = "AMPi";
const char TXT_YES [] PROGMEM = "Yes!";
const char TXT_NO_DONT [] PROGMEM = "No, don't.";
const char TXT_PANDORA_MUSIC [] PROGMEM = "Pandora Music";
const char TXT_APPLE_MUSIC [] PROGMEM = "Apple Music";
const char TXT_TURNING_ON [] PROGMEM = "Turning on.";
const char TXT_TURNING_OFF [] PROGMEM = "Turning off.";
const char TXT_WAIT_FIFTEEN_SECONDS [] PROGMEM = "Wait 15 seconds...";
const char TXT_TURN_ON_NOW [] PROGMEM = "Turn on now?";
const char TXT_STREAMER_IS_READY [] PROGMEM = "Streamer is ready";

#include <Adafruit_ST7735.h> //see https://github.com/adafruit/Adafruit-ST7735-Library
#include <Fonts/FreeSans9pt7b.h> //font available in the above library
#include <RotaryEncoder.h> //see http://www.mathertel.de/Arduino/RotaryEncoderLibrary.aspx

#define RPI_POWER 8
#define ROTENC_CLK 2
#define ROTENC_DT 3
#define ROTENC_SW 4
#define COLOR_RED 64611 /*tft.color565(255, 140, 26)*/
#define COLOR_BLUE 11101 /*tft.color565(40, 106, 237)*/
#define COLOR_ORANGE 0xf366 /*tft.color565(247, 109, 49)*/

// GUI LIBARY DEFINITIONS

#define TYPE_MAIN 100
#define TYPE_LABEL 10
#define TYPE_RECTANGLE 20
#define TYPE_LIST 30
#define TYPE_ICON 40
#define TYPE_ITEM 50

#define MAX_MESSAGE 50
#define MAX_TEXT 35
#define FONT_SHIFT_DOWN 14

struct Control;

typedef void (*Event) (Control* control);
struct Events { Event onClick; Event onEnter; Event onExit; };

struct Control {
  uint8_t type;
  bool selected = false;
  bool visible = true;
  char* text = NULL;
  uint8_t x = 0; uint8_t y = 0;
  uint8_t width = 0; uint8_t height = 0;
  uint16_t color1 = 0; uint16_t color2 = 0;
  Events* events  = NULL;
  const unsigned char* icon = NULL;
  Control* next = NULL; Control* parent = NULL; Control* child = NULL;

  Control(byte type, Control* parent, const char* text_P = NULL, bool maxText = false) {
    this->type = type;
    this->parent = parent;
    if (type != TYPE_RECTANGLE) { color1 = ST7735_WHITE; color2 = ST7735_BLACK; } else { color1 = ST7735_BLACK; color2 = ST7735_BLACK; }
    if (maxText) text = (char*)malloc(MAX_TEXT); else if (text_P) text = (char*)malloc(1+strlen_P(text_P)); //set dynamic text will use MAX_TEXT size instead of needed bytes for fixed label
    if (text_P) strcpy_P(text, text_P); else if (text) text[0] = 0; //optionally with initial text_P text from PROGMEM
  };
  void setText(const char* text_P) {
    if (!text) text = (char*)malloc(MAX_TEXT); //just when we did not initialize by mistake - unnecessary when code is well written, i.e. not setting text when not initialized
    if (text_P) strcpy_P(text, text_P); else text[0] = 0;
  }
  void clearText() { setText(NULL); }
};

struct ControlNode { Control* control; ControlNode* next; };
struct ControlList {
  ControlNode* first = NULL;
  ControlNode* last = NULL;
  void add(Control* c) {
    ControlNode* node = first; //start checking if the control is already added
    while (node) {
      if (node->control == c) return; //found! stop looking and don't add
      node = node->next;
    }
    //not found! create node & add it to list
    node = (ControlNode*)malloc(sizeof(ControlNode));
    node->control = c;
    node->next = NULL;
    if (!first) first = node;
    if (last) last->next = node;
    last = node;
  }
};

// SIMPLE POINT-TO-POINT PROTOCOL FOR SERIAL DEFINITIONS

#define PPP_BEGIN_FLAG 0x3C
#define PPP_END_FLAG 0x3E
#define PPP_NDX_COMMAND 0
#define PPP_NDX_LENGTH 1
#define FIFTEEN_SECONDS 15000
#define PPP_T_NO_OPERATION 0x00
#define PPP_T_POWER_ON_COMPLETE 0x50        //P   <P>
#define PPP_T_POWER_OFF_CONFIRMED 0x70      //p   <p>
#define PPP_T_AIRPLAY_ICON_COLOR 0x52       //R   <R\x03RGB>
#define PPP_T_PANDORA_MUSIC_ICON_COLOR 0x4E //N   <N\x03RGB>
#define PPP_T_APPLE_MUSIC_ICON_COLOR 0x4C   //L   <L\x03RGB>
#define PPP_T_STATUS_TEXT 0x53              //S   <SsizeTEXT>
#define PPP_T_BLOCK 0x42                    //B   <B>
#define PPP_T_UNBLOCK 0x62                  //b   <b>

unsigned char receiveBuffer[MAX_MESSAGE];
bool receiving = false;
unsigned char receiveIndex = 0;
unsigned char receiveSizeLeft = 0;
unsigned char character;

// DISPLAY & ROTARY ENCODE DEFINITIONS

Adafruit_ST7735 tft = Adafruit_ST7735(10 /*cs*/, 9 /*dc*/, 11/*mosi*/, 13 /*sclk*/, -1 /*rst*/);

RotaryEncoder encoder(ROTENC_CLK, ROTENC_DT, RotaryEncoder::LatchMode::TWO03);

// GLOBALS
ControlList renderPipe = ControlList(); //linked list with Controls to (re-)render
bool rpiPower = false;
bool lastRpiPower = true;
bool rpiPoweringDown = false;
bool rpiPoweringUp = false;
bool blocked = false;
unsigned long delayEnd;
bool buttonDown = false;
int encoderPosition = 0;
Control* mainControl; 
Control* focusControl;
Control* statusBar; Control* statusLabel;
Control* rpiPowerIcon; Control* airplayIcon; Control* pandoraIcon; Control* appleMusicIcon;; Control* chromeCastIcon;
Control* mainMenu; Control* rpiPowerItem;
Control* popupControl; Control* popupLabel1; Control* popupConfirmMenu;
Control* playerControl;

// INITIALIZATION

void checkPosition() {
  encoder.tick(); // just call tick() to check the state.
}

void setup(void) {
  Serial.begin(9600); // open the serial port at 9600 bps:
 
 Serial.print(COLOR_ORANGE);

  tft.initR(INITR_BLACKTAB); tft.setRotation(1); tft.setFont(&FreeSans9pt7b); //initialize display & set default font
 
  //RPi Power
  pinMode(RPI_POWER, OUTPUT);    // sets the digital pin 2 as output
  digitalWrite(RPI_POWER, LOW); //off

  //initialize encoder
  attachInterrupt(ROTENC_CLK, checkPosition, CHANGE);
  attachInterrupt(ROTENC_DT, checkPosition, CHANGE);
  pinMode(ROTENC_SW, INPUT_PULLUP);

  //initialize GUI
  mainControl = new Control(TYPE_MAIN, NULL /** root - no parent **/);
  mainControl->color2 = ST7735_BLACK; //background color

  //initialize status bar
  statusBar = new Control(TYPE_RECTANGLE, mainControl);
  statusBar->x = 0; statusBar->y = 111 /*128 - 17*/; statusBar->width = 160; statusBar->height = 17;
  statusBar->color1 = ST7735_BLACK; statusBar->color2 = COLOR_BLUE;

  Control* statusBar2 = new Control(TYPE_RECTANGLE, mainControl);
  statusBar2->x = 0; statusBar2->y = 0; statusBar2->width = 160; statusBar2->height = 13;
  statusBar2->color1 = COLOR_BLUE; statusBar2->color2 = ST7735_BLACK;

  statusLabel = new Control(TYPE_LABEL, mainControl, NULL, true);
  statusLabel->x = 3; statusLabel->y = 110;
  statusLabel->color1 = ST7735_WHITE;
  
  Control* ampiLabelMain = new Control(TYPE_LABEL, mainControl, TXT_AMPI); //size x=y=0 to hide background
  ampiLabelMain->x = 3; ampiLabelMain->y = 3;
  ampiLabelMain->color1 = COLOR_RED; ampiLabelMain->color2 = ST7735_BLACK;
    
  mainMenu = new Control(TYPE_LIST, mainControl);
  mainMenu->x = 3; mainMenu->y = 25; mainMenu->width = 154 /*160 - 6*/; mainMenu->height = 81 /*2*3+25*3*/;
  mainMenu->color1 = ST7735_WHITE; mainMenu->color2 = ST7735_BLACK;
  mainMenu->events = new Events(); mainMenu->events->onEnter = enterMenu;

  rpiPowerItem = new Control(TYPE_ITEM, mainMenu, TXT_TURN_STREAMER_ON, true);
  rpiPowerItem->width = mainMenu->width; rpiPowerItem->height = 25; rpiPowerItem->x = 1;
  rpiPowerItem->color1 = ST7735_BLACK; rpiPowerItem->color2 = COLOR_RED;
  rpiPowerItem->events = new Events(); rpiPowerItem->events->onClick = swithRpiPower;

  Control* pandoraItem = new Control(TYPE_ITEM, mainMenu, TXT_PANDORA_MUSIC);
  pandoraItem->width = mainMenu->width; pandoraItem->height = 25; pandoraItem->x = 1; pandoraItem->y = 27  /*1*2+1*25*/; 
  pandoraItem->color1 = ST7735_BLACK; pandoraItem->color2 = COLOR_RED;
  pandoraItem->events = new Events(); pandoraItem->events->onClick = switchPandora;

  Control* appleItem = new Control(TYPE_ITEM, mainMenu, TXT_APPLE_MUSIC);
  appleItem->width = mainMenu->width; appleItem->height = 25; appleItem->x = 1; appleItem->y = 54 /*2*2+2*25*/; 
  appleItem->color1 = ST7735_BLACK; appleItem->color2 = COLOR_RED;
  appleItem->events = new Events(); appleItem->events->onClick = switchAppleMusic;

  rpiPowerIcon = new Control(TYPE_ICON, mainControl);
  rpiPowerIcon->width = 16; rpiPowerIcon->height = 16; rpiPowerIcon->x = 50 /* 3+45 (AMPi label width) + 2 */; rpiPowerIcon->y = 3;
  rpiPowerIcon->color1 = COLOR_RED; rpiPowerIcon->icon = IMG_POWER;

  airplayIcon = new Control(TYPE_ICON, mainControl);
  airplayIcon->width = 16; airplayIcon->height = 16; airplayIcon->x = 68 /*3+45 (AMPi label width) +2+16 (icon width) +2 */; airplayIcon->y = 3;
  airplayIcon->color1 = COLOR_RED; airplayIcon->icon = IMG_AIRPLAY;

  pandoraIcon = new Control(TYPE_ICON, mainControl);
  pandoraIcon->width = 16; pandoraIcon->height = 16; pandoraIcon->x = 86 /*3+45 (AMPi label width) +2+16 (icon width*2) +2*2 */; pandoraIcon->y = 3;
  pandoraIcon->color1 = COLOR_RED; pandoraIcon->icon = IMG_PANDORA_MUSIC;

  appleMusicIcon = new Control(TYPE_ICON, mainControl);
  appleMusicIcon->width = 16; appleMusicIcon->height = 16; appleMusicIcon->x = 104 /*3+45 (AMPi label width) +2+16 (icon width*3) +2*3 */; appleMusicIcon->y = 3;
  appleMusicIcon->color1 = COLOR_RED; appleMusicIcon->icon = IMG_APPLE_MUSIC;
 
  chromeCastIcon = new Control(TYPE_ICON, mainControl);
  chromeCastIcon->width = 16; chromeCastIcon->height = 16; chromeCastIcon->x = 120 /*3+45 (AMPi label width) +2+16 (icon width*4) +2*3 */; chromeCastIcon->y = 3;
  chromeCastIcon->color1 = COLOR_RED; chromeCastIcon->icon = iMG_CHROMECAST_AUDIO;
  //build main menu
  mainMenu->child = rpiPowerItem; rpiPowerItem->next = pandoraItem; pandoraItem->next = appleItem; 

  //build popup GUI
  popupControl = new Control(TYPE_RECTANGLE, mainControl);
  popupControl->x = 4; popupControl->y = 2; popupControl->width = 152; popupControl->height = 107;
  popupControl->color1 = COLOR_ORANGE; popupControl->color2 = COLOR_BLUE;
  popupControl->visible = false;

  popupLabel1 = new Control(TYPE_LABEL, popupControl); //size x=y=0 to hide background
  popupLabel1->x = 5; popupLabel1->y = 10;
  popupLabel1->color1 = ST7735_WHITE;

  popupConfirmMenu = new Control(TYPE_LIST, popupControl);
  popupConfirmMenu->x = 5; popupConfirmMenu->y = 35; popupConfirmMenu->width = 150; popupConfirmMenu->height = 52 /*2*2+2*25*/;
  popupConfirmMenu->color1 = ST7735_WHITE; popupConfirmMenu->color2 = ST7735_BLACK;
  popupConfirmMenu->events = new Events(); popupConfirmMenu->events->onEnter = enterMenu;
 
  Control* popupConfirmYesItem = new Control(TYPE_ITEM, popupConfirmMenu, TXT_YES);
  popupConfirmYesItem->width = popupConfirmMenu->width; popupConfirmYesItem->height = 25; popupConfirmYesItem->x = 1;
  popupConfirmYesItem->color1 = ST7735_BLACK; popupConfirmYesItem->color2 = COLOR_RED;
  popupConfirmYesItem->events = new Events(); popupConfirmYesItem->events->onClick = closePopupAndSwithRpiPower;

  Control* popupConfirmNoItem = new Control(TYPE_ITEM, popupConfirmMenu, TXT_NO_DONT);
  popupConfirmNoItem->width = popupConfirmMenu->width; popupConfirmNoItem->height = 25; popupConfirmNoItem->x = 1; popupConfirmNoItem->y = 26;
  popupConfirmNoItem->color1 = ST7735_BLACK; popupConfirmNoItem->color2 = COLOR_RED;
  popupConfirmNoItem->events = new Events();  popupConfirmNoItem->events->onClick = closePopup;
  
  popupConfirmMenu->child = popupConfirmYesItem; popupConfirmYesItem->next = popupConfirmNoItem;

  popupControl->child = popupLabel1; popupLabel1->next = popupConfirmMenu; //popupConfirmMenu->next = null;
  
  //build player gui
  playerControl = new Control(TYPE_RECTANGLE, mainControl);
  playerControl->x = 4; playerControl->y = 2; playerControl->width = 152; playerControl->height = 107;
  playerControl->color1 = COLOR_ORANGE; playerControl->color2 = COLOR_ORANGE;
  playerControl->visible = false;

  //connect main gui components
  mainControl->child = statusBar; statusBar->next = statusBar2; statusBar2->next = statusLabel; statusLabel->next = ampiLabelMain; ampiLabelMain->next = mainMenu; mainMenu->next = rpiPowerIcon; rpiPowerIcon->next = airplayIcon; airplayIcon->next = pandoraIcon; pandoraIcon->next = appleMusicIcon; appleMusicIcon->next = chromeCastIcon; chromeCastIcon->next = popupControl; popupControl->next = playerControl; //playerControl->next = null  //build main GUI

  renderPipe.add(mainControl);
  renderQueueAllChildren(mainControl);

  renderRenderPipe();
}

// GUI LIBARY IMPLEMENTATION

void renderText(char* text, uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint16_t color1, uint16_t color2) {
  if (!text) return;
  int16_t textx, texty; uint16_t textwidth, textheight;
  tft.getTextBounds(text, x, y, &textx, &texty, &textwidth, &textheight); //location is assumed top left point of text, but baseline of font
  texty += FONT_SHIFT_DOWN; //need to increase y to put anchor point top left: fixed value as fixed font
  if (!(width == 0 || height == 0)) { // a zero size hides does not render the background
    tft.fillRect(x, y, width, height, color2);
  }
  if (text[0] != 0) {
    tft.setCursor(x, y + FONT_SHIFT_DOWN); //also print text lower as it needs to anchored on the top left
    tft.setTextColor(color1);
    tft.print(text);
  }
}

void renderItem(Control* item)
{
  //item position X = top & bottom & left & right item/list border & text extra right
  //item position Y = text extra downward (~ font size)
  //item size X = width
  //item size Y = height
  //item color X = item text color when not selected
  //item color Y = item text color when selected
  //parent color X = text color (foreground)
  //parent color Y = background
  uint8_t textx = item->parent->x + item->x + item->x;
  uint8_t texty = item->parent->y + item->x + item->x + item->x + item->y;
  uint8_t textwidth = item->width - item->x - item->x - item->x - item->x;
  uint8_t textheight = item->height - item->x - item->x - item->x - item->x; 
  renderText(item->text, textx, texty, textwidth, textheight, item->parent->color1, item->parent->color2);
}

void render(Control* c) {
  uint16_t i; //used for loops
  if (c->visible) {
    switch (c->type) {
      case TYPE_MAIN: tft.fillScreen(c->color2); break;
      case TYPE_LABEL: renderText(c->text, c->x, c->y, c->width, c->height, c->color1, c->color2); break;
      case TYPE_RECTANGLE: 
        if (c->width > 0 && c->height > 0)
          if (c->color1 == c->color2) tft.fillRect(c->x, c->y, c->width, c->height, c->color2);
          else {
            uint8_t r1, g1, b1; color565toRGB(c->color1, r1, g1, b1);
            uint8_t r2, g2, b2; color565toRGB(c->color2, r1, g2, b2);
            for (i = 0; i < c->height; i++) { tft.drawFastHLine( c->x, c->y + i, c->width, tft.color565(map(i, 0, c->height, r1, r2), map(i, 0, c->height, g1, g2), map(i, 0, c->height, b1, b2))); }
          }
        break;
      case TYPE_ICON: tft.drawBitmap(c->x, c->y, c->icon, c->width, c->height, c->color1); break;
      case TYPE_ITEM: renderItem(c); break;
      case TYPE_LIST:
        if (c->parent->visible) {
          Control* item = c->child;
          while (item) {
            tft.drawRect(c->x + item->x, c->y + item->x + item->y, item->width - item->x - item->x, item->height - item->x - item->x, (item->selected ? item->color2 : item->color1));
            item = item->next;
          }
        }
        break;
    }
  } else if (c->type != TYPE_RECTANGLE) tft.fillRect(c->x, c->y, c->width, c->height, mainControl->color2);
}

void renderQueueAllChildren(Control* c) {
  Control* d = c->child;
  while (d != NULL)  {
    if (d->visible) { //add only visible objects
      if (d->type == TYPE_LIST) { //render children first in case of list as the list draws the rectangles around the (non-)selected items
         renderQueueAllChildren(d); 
         renderPipe.add(d);
      } else {
        renderPipe.add(d);
        renderQueueAllChildren(d); 
      }
    }
    d = d->next; //get next child
  }
}

void renderRenderPipe() {
  ControlNode* cn = renderPipe.first;
  ControlNode* cn2;
  while (cn) {
    render(cn->control);
    cn2 = cn;
    cn = cn->next; //get next control node
    free(cn2); //clean up
  }
  renderPipe.first = NULL; renderPipe.last = NULL;
}

void color565toRGB(uint16_t color, uint8_t &r, uint8_t &g, uint8_t &b) { r = (color>>8)&0x00F8; g = (color>>3)&0x00FC; b = (color<<3)&0x00F8; } //converts 565 format 16 bit color to RGB https://github.com/PaulStoffregen/ILI9341_t3/pull/24/commits/7e3789680bd6ecee425edc3d60a2a973ff417ab8
int readRGB88AndConvertRGB656(unsigned char * buffer, uint16_t index) { return tft.color565(buffer[index], buffer[index+1], buffer[index+2]); }

void enterMenu(Control* control) {
  if (focusControl) { //unselect current control & when TYPE_LIST unselect all items
    focusControl->selected = false;
    if (control->type == TYPE_LIST) {
      Control* c = control->child;
      while (c) { c->selected = false; c = c->next; }
    }
  }
  focusControl = control;
  control->selected = true;
  if (control->child) {
    control->child->selected = true;
  }
  renderPipe.add(control);
}

void moveMenu(Control* control, bool directionDown) {
  if (control->child) {
    uint16_t s = 0; //selected index   0 is nothing selected
    uint16_t c = 0; //count
    Control* item = control->child; //find index s that is selected
    uint16_t i = 1;
    while (item) {
      c++;
      if (item->selected) s = i;
      i++;
      item = item->next;
    }
    if (directionDown == false) { if (s > 1) s--; else s = c; } else { if (s < c) s++; else s = 1; } //move according to direction
    item = control->child;
    i = 1;
    while (item) { //switch to other selection
      if (i == s) { if (!item->selected) { item->selected = true; }}
      else { if (item->selected) { item->selected = false; }}
      i++;
      item = item->next;
    }
    renderPipe.add(control);
  }
}

void clickMenu(Control* control) {
    Control* item = control->child;
    while (item) {
        if (item->selected) if (item->events) if (item->events->onClick) item->events->onClick(item);
        item = item->next;
    }
}

void sendUp() {
  if (!focusControl) return; //after start up do nothing
  if (focusControl->type == TYPE_LIST) moveMenu(focusControl, false);
}

void sendDown() {
  if (!focusControl) { //after start up jump into main menu
    sendEnter(mainMenu);
    return;
  }
  if (focusControl->type == TYPE_LIST) moveMenu(focusControl, true);
}

void sendClick() {
  if (!focusControl) return; //after start up do nothing
  if (focusControl->type == TYPE_LIST) clickMenu(focusControl);
}

void sendEnter(Control* control) { 
  focusControl = control;
  if (control->events) if (control->events->onEnter) control->events->onEnter(control);
}

void setStatus() { //render whatever is already in statusLabel->text
  renderPipe.add(statusBar); //background label needs to update
  renderPipe.add(statusLabel); //status label needs to update
}

// MAIN LOOP

void loop() {
  encoder.tick();
  if (!rpiPoweringDown && !rpiPoweringUp) { //normal operation as Raspberry Pi is not powering down or up
    int newPos = encoder.getPosition();
    if (encoderPosition != newPos) {
      if (encoderPosition % 2 == 0) if (encoderPosition < newPos) sendUp(); else sendDown();
      encoderPosition = newPos;
    }
    int btnState = digitalRead(ROTENC_SW); //read the button state
    if (btnState == LOW) { //if we detect LOW signal, button is pressed
      if (!buttonDown) sendClick();
      buttonDown = true;
    } else buttonDown = false;
  } else if (rpiPoweringDown && millis() > delayEnd) rpiPowerOff(); //as the Raspberry Pi powering down we need to wait it out (fixed timer of 15 seconds, not fancy) and at the end of timer change icon colors and change rpiPower booleans
  if (rpiPoweringUp || blocked) waitAnimate();
  if (readSerialData()) processReceiveBuffer(); //process serial data
  if (renderPipe.first) renderRenderPipe();
}

// GUI ACTIONS

void waitAnimate() {
  if (statusLabel->text[0] != '.' || statusLabel->text[15] == '.') {
    memset(statusLabel->text, (char)0, MAX_TEXT) /*clear status text for animation, when not going or when full restart */;
    renderPipe.add(statusBar); //background label needs to update
  }
  delay(500); //wait 500ms
  for (uint8_t i = 0; i < MAX_TEXT-1; i++) {
    if (statusLabel->text[i] == 0) { statusLabel->text[i] = '.' ; statusLabel->text[i+1] == 0; break; }
  }
  renderPipe.add(statusLabel); //status label needs to update
}

void swithRpiPower(Control* control) {
  rpiPower = !rpiPower;
  if(rpiPower) { control->setText(TXT_TURNING_ON); digitalWrite(RPI_POWER, HIGH) /*turn relay ON*/; rpiPoweringUp = true; } else { control->setText(TXT_TURNING_OFF);  Serial.print("<p>") /*Send Power Off command to Raspberry Pi*/ ;}
  lastRpiPower = rpiPower;
  renderPipe.add(control);
  renderPipe.add(control->parent);
}

void rpiPowerOff() {
  rpiPowerIcon->color1 = COLOR_RED; renderPipe.add(rpiPowerIcon);
  rpiPowerItem->setText(TXT_TURN_STREAMER_ON); renderPipe.add(rpiPowerItem); renderPipe.add(rpiPowerItem->parent);
  statusLabel->clearText(); setStatus();
  digitalWrite(RPI_POWER, LOW) /*turn relay OFF*/;
  rpiPoweringDown = false;
}

void setAllServicesRed() {
  airplayIcon->color1 = COLOR_RED; renderPipe.add(airplayIcon);
  pandoraIcon->color1 = COLOR_RED; renderPipe.add(pandoraIcon);
  appleMusicIcon->color1 = COLOR_RED; renderPipe.add(appleMusicIcon);
}

void closePopup(Control* control) {
  popupControl->visible = false;
  statusLabel->clearText();
  setStatus();
  renderPipe.add(mainControl);
  renderQueueAllChildren(mainControl);
  enterMenu(mainMenu);
}

void closePopupAndSwithRpiPower(Control* control) {
  closePopup(control);
  swithRpiPower(rpiPowerItem);
}

void switchPandora(Control* control) { 
  if (rpiPower && !rpiPoweringUp) {
    Serial.print("<N>"); Serial.flush();
  } else {
    popupControl->visible = true; 
    popupLabel1->setText(TXT_TURN_ON_NOW);
    statusLabel->setText(TXT_TURN_STREAMER_ON2); 
    setStatus();
    renderPipe.add(popupControl); renderQueueAllChildren(popupControl);
    sendEnter(popupConfirmMenu);
  }
}

void switchAppleMusic(Control* control) { 
  if (rpiPower && !rpiPoweringUp) {
    Serial.print(F("<L>")); Serial.flush();
  } else {
    popupControl->visible = true; 
    popupLabel1->setText(TXT_TURN_ON_NOW);
    statusLabel->setText(TXT_TURN_STREAMER_ON2); 
    setStatus();
    renderPipe.add(popupControl); renderQueueAllChildren(popupControl);
    sendEnter(popupConfirmMenu);
  }
}

bool readSerialData() {
 while (Serial.available() > 0) {
    character = Serial.read();
    if (!receiving) {
      if (character == PPP_BEGIN_FLAG) { receiving = true; receiveIndex = 0; receiveSizeLeft = 1 /*the command is expected - so 1 byte*/; }
    } else {
      if (character == PPP_END_FLAG && receiveSizeLeft == 0) {
        if (receiveIndex <= PPP_NDX_LENGTH) receiveBuffer[PPP_NDX_LENGTH] = 0; //no length received - 1 byte packet like <P>
        receiving = false;
        return true;
      } else {
        if (receiveIndex < MAX_MESSAGE-1) {
          receiveBuffer[receiveIndex] = character;
          if (receiveIndex == PPP_NDX_LENGTH) receiveSizeLeft = character; else if (receiveSizeLeft == 0) receiving = false; else receiveSizeLeft--;
          receiveIndex++;
        } else receiving = false;
      } 
    }
  }
  return false;
}

void processReceiveBuffer() {
  unsigned char size_ = receiveBuffer[PPP_NDX_LENGTH];

  switch (receiveBuffer[PPP_NDX_COMMAND]) {  //first byte to be expected signal/command type - second (depending on the command) is the size in bytes of the payload
    case PPP_T_POWER_ON_COMPLETE:
      statusLabel->setText(TXT_STREAMER_IS_READY);
      setStatus();
      rpiPowerIcon->color1 = ST7735_WHITE; renderPipe.add(rpiPowerIcon);
      rpiPowerItem->setText(TXT_TURN_STREAMER_OFF); renderPipe.add(rpiPowerItem);
      renderPipe.add(rpiPowerItem->parent); //main menu needs to update
      rpiPoweringUp = false;
      break;

    case PPP_T_POWER_OFF_CONFIRMED: /*Power Off command received by Raspberry Pi and this is confirmation, it is shutting down */ 
      rpiPoweringDown = true;
      delayEnd = millis() + FIFTEEN_SECONDS;
      statusLabel->setText(TXT_WAIT_FIFTEEN_SECONDS);
      setStatus();
      setAllServicesRed();
      break;

    case PPP_T_AIRPLAY_ICON_COLOR:
      airplayIcon->color1 = readRGB88AndConvertRGB656(receiveBuffer,2); renderPipe.add(airplayIcon); //third, forth & fifth byte are new color for the airplay icon
      break;
      
    case PPP_T_PANDORA_MUSIC_ICON_COLOR:
      pandoraIcon->color1 = readRGB88AndConvertRGB656(receiveBuffer,2); renderPipe.add(pandoraIcon); //third, forth & fifth byte are new color for the pandora icon
      break;

    case PPP_T_APPLE_MUSIC_ICON_COLOR:
      appleMusicIcon->color1 = readRGB88AndConvertRGB656(receiveBuffer,2); //third, forth & fifth byte are new color for the airplay icon
      break;
      
    case PPP_T_STATUS_TEXT:
      receiveBuffer[PPP_NDX_LENGTH+size_+1] = 0; //terminate with zero byte
      strcpy(statusLabel->text, (char*)&receiveBuffer[2]); setStatus();
      break;

    case PPP_T_BLOCK:
      blocked = true;
      break;

    case PPP_T_UNBLOCK:
      blocked = false;
      break;
      
    case PPP_T_NO_OPERATION:     
      break;

    default:
      sprintf(statusLabel->text, "%d NOT IMPL'D\n", (byte)receiveBuffer[PPP_NDX_COMMAND]); setStatus(); //render whatever is in statusLabel->text
      break;
  }  
}
