//ICONS - tool to create at http://javl.github.io/image2cpp/ - use 16x16 monochrome images & set white background & set use invert image colors & set plain bytes & set Horizontal 1 byte per pixel draw mode
const unsigned char IMG_POWER [] PROGMEM = { 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x0d, 0xb0, 0x1d, 0xb8, 0x39, 0x9c, 0x31, 0x8e, 0x60, 0x06, 0x60, 0x06, 0x60, 0x06, 0x60, 0x06, 0x30, 0x0c, 0x38, 0x1c, 0x1c, 0x38, 0x0f, 0xf0, 0x03, 0xc0 };
const unsigned char IMG_AIRPLAY [] PROGMEM = { 0x07, 0xe0, 0x08, 0x10, 0x30, 0x0c, 0x43, 0xc2, 0x44, 0x22, 0x49, 0x92, 0x92, 0x49, 0x94, 0x29, 0x94, 0x29, 0x94, 0x29, 0x90, 0x09, 0x49, 0x92, 0x43, 0xc2, 0x27, 0xe4, 0x0f, 0xf0, 0x0f, 0xf0 };
const unsigned char IMG_PANDORA_MUSIC [] PROGMEM = { 0x7f, 0xf0, 0x40, 0x08, 0x40, 0x04, 0x40, 0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x04, 0x40, 0x08, 0x47, 0xf0, 0x44, 0x00, 0x44, 0x00, 0x44, 0x00, 0x7c, 0x00 };
const unsigned char IMG_APPLE_MUSIC [] PROGMEM = { 0x3f, 0xfc, 0x7f, 0xfe, 0xff, 0xc7, 0xfc, 0x07, 0xf8, 0x07, 0xf8, 0x27, 0xf9, 0xe7, 0xf9, 0xe7, 0xf9, 0xe7, 0xf9, 0xc7, 0xf1, 0x87, 0xe1, 0x87, 0xe1, 0xcf, 0xf3, 0xff, 0x7f, 0xfe, 0x3f, 0xfc };

#include <Adafruit_ST7735.h> //see https://github.com/adafruit/Adafruit-ST7735-Library
#include <Fonts/FreeSans9pt7b.h> //font available in the above libary
#include <RotaryEncoder.h> //see http://www.mathertel.de/Arduino/RotaryEncoderLibrary.aspx

#define RPI_POWER 8
#define ROTENC_CLK 2
#define ROTENC_DT 3
#define ROTENC_SW 4
#define COLOR_RED 64611 /*tft.color565(255, 140, 26)*/
#define COLOR_BLUE 11101 /*tft.color565(40, 106, 237)*/

// GUI LIBARY DEFINITIONS

#define TYPE_MAIN 100
#define TYPE_LABEL 10
#define TYPE_RECTANGLE 20
#define TYPE_LIST 30
#define TYPE_ICON 40
#define TYPE_ITEM 50

#define MAX_MESSAGE 200
#define MAX_TEXT 35

struct Control;

struct XY { word x = 0; word y = 0; };
struct Colors { int x = 0; int y = 0; };
typedef void (*Event) (Control* control);
struct Events { Event onClick; Event onEnter; Event onExit; };

struct Control {
  byte type;
  bool selected = false;
  bool visible = true;
  char* text = NULL;
  char* oldText = NULL;
  XY position = XY();
  XY size = XY();
  Colors colors = Colors();
  Events* events  = NULL;
  const unsigned char* icon  = NULL;
  bool uptodate = false;
  Control* next = NULL;
  Control* parent = NULL;
  Control* child = NULL;

  Control(byte type, Control* parent) {
    this->type = type;
    this->parent = parent;
    if (type == TYPE_LABEL || type == TYPE_ITEM) {
      text = (char*)malloc(MAX_TEXT);
      text[0] = 0;
    }
    if (type == TYPE_ITEM) {
      oldText = (char*)malloc(MAX_TEXT);
      oldText[0] = 0;
    }
    if (type != TYPE_RECTANGLE) {
      colors.x = ST7735_WHITE;
      colors.y = ST7735_BLACK;
    } else {
      colors.x = ST7735_BLACK;
      colors.y = ST7735_BLACK;
    }
  };
};

// SIMPLE POINT-TO-POINT PROTOCOL FOR SERIAL DEFINITIONS

#define PPP_BEGIN_FLAG 0x3C
#define PPP_END_FLAG 0x3E
#define PPP_NDX_COMMAND 0
#define PPP_NDX_LENGTH 1
#define PPP_T_NO_OPERATION 0x00
#define PPP_T_POWER_ON_COMPLETE 0x50 //P    <P>
#define PPP_T_POWER_OFF_CONFIRMED 0x70 //p    <p>
#define FIFTEEN_SECONDS 15000
#define PPP_T_AIRPLAY_ICON_COLOR 0x52 //R   <R\x03RGB>
#define PPP_T_STATUS_TEXT 0x53 //S          <SsizeTEXT>

unsigned char receiveBuffer[MAX_MESSAGE];
bool receiving = false;
unsigned char receiveIndex = 0;
unsigned char receiveSizeLeft = 0;
unsigned char character;

// DISPLAY & ROTARY ENCODE DEFINITIONS

Adafruit_ST7735 tft = Adafruit_ST7735(10 /*cs*/, 9 /*dc*/, 11/*mosi*/, 13 /*sclk*/, -1 /*rst*/);

RotaryEncoder encoder(ROTENC_CLK, ROTENC_DT, RotaryEncoder::LatchMode::TWO03);

// GLOBALS
unsigned long lastGUIRender;
bool rpiPower = false;
bool lastRpiPower = true;
bool rpiPoweringDown = false;
unsigned long delayEnd;
bool guiuptodate = false;
bool buttonDown = false;
int encoderPosition = 0;
Control* mainControl;
Control* focusControl;
Control* statusBar;
Control* statusLabel;
Control* rpiPowerIcon;
Control* airplayIcon;
Control* pandoraIcon;
Control* appleMusicIcon;
Control* mainMenu;
Control* rpiPowerItem;

// INITIALIZATION

void checkPosition() {
  encoder.tick(); // just call tick() to check the state.
}

void setup(void) {
  tft.initR(INITR_BLACKTAB);

  tft.setRotation(1);

  tft.setFont(&FreeSans9pt7b);
 
  //RPi Power
  pinMode(RPI_POWER, OUTPUT);    // sets the digital pin 2 as output
  digitalWrite(RPI_POWER, LOW); //off

  //initialize encoder
  attachInterrupt(ROTENC_CLK, checkPosition, CHANGE);
  attachInterrupt(ROTENC_DT, checkPosition, CHANGE);
  pinMode(ROTENC_SW, INPUT_PULLUP);

  //initialize GUI
  mainControl = new Control(TYPE_MAIN, NULL /** root - no parent **/);
  mainControl->colors.y = ST7735_BLACK; //background color
  guiuptodate = false;

  //initialize status bar
  statusBar = new Control(TYPE_RECTANGLE, mainControl);
  statusBar->position.x = 0; statusBar->position.y = 111 /*128 - 17*/;
  statusBar->size.x = 160; statusBar->size.y = 17;
  statusBar->colors.x = ST7735_BLACK; statusBar->colors.y = COLOR_BLUE;

  Control* statusBar2 = new Control(TYPE_RECTANGLE, mainControl);
  statusBar2->position.x = 0; statusBar2->position.y = 0;
  statusBar2->size.x = 160; statusBar2->size.y = 15;
  statusBar2->colors.x = COLOR_BLUE; statusBar2->colors.y = ST7735_BLACK;

  statusLabel = new Control(TYPE_LABEL, mainControl);
  strcpy(statusLabel->text, "\0");
  statusLabel->position.x = 3; statusLabel->position.y = 125;
  statusLabel->colors.x = ST7735_WHITE; statusLabel->colors.y = COLOR_BLUE;
  
  Control* ampiLabelMain = new Control(TYPE_LABEL, mainControl); //size x=y=0 to hide background
  ampiLabelMain->position.x = 3; ampiLabelMain->position.y = 22;
  strcpy(ampiLabelMain->text, "AMPi\0");
  ampiLabelMain->colors.x = COLOR_RED; ampiLabelMain->colors.y = ST7735_BLACK;
    
  mainMenu = new Control(TYPE_LIST, mainControl);
  mainMenu->position.x = 3; mainMenu->position.y = 35;
  mainMenu->size.x = 154 /*160 - 6*/; mainMenu->size.y = 73;
  mainMenu->colors.x = ST7735_WHITE; mainMenu->colors.y = ST7735_BLACK;
  mainMenu->events = new Events(); mainMenu->events->onEnter = enterMenu;

  rpiPowerItem = new Control(TYPE_ITEM, mainMenu);
  rpiPowerItem->size.x = mainMenu->size.x; rpiPowerItem->size.y = 19;
  strcpy(rpiPowerItem->text, "Turn Streamer On\0");
  rpiPowerItem->position.x = 1; rpiPowerItem->position.y = 11;
  rpiPowerItem->colors.x = ST7735_BLACK; rpiPowerItem->colors.y = COLOR_RED;
  rpiPowerItem->events = new Events(); rpiPowerItem->events->onClick = swithRpiPower;

  Control* pandoraItem = new Control(TYPE_ITEM, mainMenu);
  pandoraItem->size.x = mainMenu->size.x; pandoraItem->size.y = 19;
  strcpy(pandoraItem->text, "Pandora Music\0");
  pandoraItem->position.x = 1; pandoraItem->position.y = 11; 
  pandoraItem->colors.x = ST7735_BLACK; pandoraItem->colors.y = COLOR_RED;
  pandoraItem->events = new Events(); pandoraItem->events->onClick = sendSerialText;

  Control* appleItem = new Control(TYPE_ITEM, mainMenu);
  appleItem->size.x = mainMenu->size.x; appleItem->size.y = 19;
  strcpy(appleItem->text, "Apple Music\0");
  appleItem->position.x = 1; appleItem->position.y = 11;
  appleItem->colors.x = ST7735_BLACK; appleItem->colors.y = COLOR_RED;

  rpiPowerIcon = new Control(TYPE_ICON, mainControl);
  rpiPowerIcon->size.x = 16; rpiPowerIcon->size.y = 16;
  rpiPowerIcon->position.x = 50 /* 3+45 (AMPi label width) + 2 */; rpiPowerIcon->position.y = 8;
  rpiPowerIcon->colors.x = COLOR_RED; rpiPowerIcon->icon = IMG_POWER;

  airplayIcon = new Control(TYPE_ICON, mainControl);
  airplayIcon->size.x = 16; airplayIcon->size.y = 16;
  airplayIcon->position.x = 68 /*3+45 (AMPi label width) +2+16 (icon width) +2 */; airplayIcon->position.y = 8;
  airplayIcon->colors.x = COLOR_RED; airplayIcon->icon = IMG_AIRPLAY;

  pandoraIcon = new Control(TYPE_ICON, mainControl);
  pandoraIcon->size.x = 16; pandoraIcon->size.y = 16;
  pandoraIcon->position.x = 86 /*3+45 (AMPi label width) +2+16 (icon width*2) +2*2 */; pandoraIcon->position.y = 8;
  pandoraIcon->colors.x = COLOR_RED; pandoraIcon->icon = IMG_PANDORA_MUSIC;

  appleMusicIcon = new Control(TYPE_ICON, mainControl);
  appleMusicIcon->size.x = 16; appleMusicIcon->size.y = 16;
  appleMusicIcon->position.x = 104 /*3+45 (AMPi label width) +2+16 (icon width*3) +2*3 */; appleMusicIcon->position.y = 8;
  appleMusicIcon->colors.x = COLOR_RED; appleMusicIcon->icon = IMG_APPLE_MUSIC;
  
  //build main menu
  mainMenu->child = rpiPowerItem; rpiPowerItem->next = pandoraItem;
  pandoraItem->next = appleItem; 
  
  //build main GUI
  mainControl->child = statusBar;
  statusBar->next = statusBar2;
  statusBar2->next = statusLabel;
  statusLabel->next = ampiLabelMain;
  ampiLabelMain->next = mainMenu;
  mainMenu->next = rpiPowerIcon;
  rpiPowerIcon->next = airplayIcon;
  airplayIcon->next = pandoraIcon;
  pandoraIcon->next = appleMusicIcon;
  //appleMusicIcon->next = null;

  Serial.begin(9600); // open the serial port at 9600 bps:

  renderGUI();
}

// GUI LIBARY IMPLEMENTATION

void renderText(char* text, XY position, XY size, Colors colors) {
  word x, y, w, h;
  tft.getTextBounds(text, position.x, position.y, &x, &y, &w, &h);
  if (!(size.x == 0 || size.y == 0)) { // a zero size hides does not render the background
    tft.fillRect(x, y, w, h, colors.y);
  }
  if (text[0] != 0) {
    tft.setCursor(position.x, position.y);
    tft.setTextColor(colors.x);
    tft.print(text);
  }
  if (!(size.x == 0 || size.y == 0)) { // a zero size hides does not render the background
    tft.fillRect(x + w, y, size.x - w, size.y, colors.y);
    tft.fillRect(x, y + h, w, size.y - h, colors.y);
  }
}

void renderIcon(Control* c) {
  tft.drawBitmap(c->position.x, c->position.y, c->icon, c->size.x, c->size.y, c->colors.x);
}

word renderItem(Control* item, int y_delta)
{
  if (!item->uptodate) {
    //item position X = top & bottom & left & right item/list border & text extra right
    //item position Y = text extra downward (~ font size)
    //item size X = width
    //item size Y = height
    //item color X = item text color when not selected
    //item color Y = item text color when selected
    //parent color X = text color (foreground)
    //parent color Y = background
    XY textposition;
    XY textsize;
    textposition.x = item->parent->position.x + item->position.x + item->position.x - 1;
    textposition.y = item->parent->position.y + item->position.x + item->position.x + item->position.x + item->position.y + y_delta;
    textsize.x = item->size.x - item->position.x - item->position.x - item->position.x - item->position.x;
    textsize.y = item->size.y - item->position.x - item->position.x - item->position.x - item->position.x;
    if (strcmp(item->text, item->oldText) != 0) {
      tft.fillRect(item->parent->position.x + item->position.x, item->parent->position.y + item->position.x + y_delta, item->size.x - item->position.x - item->position.x, item->size.y - item->position.x - item->position.x, item->parent->colors.y);
      renderText(item->text, textposition, textsize, item->parent->colors);
      strcpy(item->text, item->oldText);
    }    
    tft.drawRect(item->parent->position.x + item->position.x, item->parent->position.y + item->position.x + y_delta, item->size.x - item->position.x - item->position.x, item->size.y - item->position.x - item->position.x, (item->selected ? item->colors.y : item->colors.x));
    item->uptodate = true;
  }
  return item->size.y;
}

void render(Control* c) {
  if (c->visible) {
    switch (c->type) {
      case TYPE_MAIN: tft.fillScreen(c->colors.y); break;
      case TYPE_LABEL: renderText(c->text, c->position, c->size, c->colors); break;
      case TYPE_RECTANGLE:
        if (c->colors.x == c->colors.y) tft.fillRect(c->position.x, c->position.y, c->size.x, c->size.y, c->colors.y);
        else {
          word delta_r = ((c->colors.y >> 11 & 31) - (c->colors.x >> 11 & 31)) / (int)c->size.y;
          word delta_g = ((c->colors.y >> 5 & 63) - (c->colors.x >> 5 & 63)) / (int)c->size.y;
          word delta_b = ((c->colors.y >> 0 & 31) - (c->colors.x >> 0 & 31)) / (int)c->size.y;
          word col_r = (c->colors.x >> 11 & 31);
          word col_g = (c->colors.x >> 5 & 63);
          word col_b = (c->colors.x >> 0 & 31);
          for (word i = 0; i < c->size.y; i++) {
            tft.drawFastHLine( c->position.x, c->position.y + i, c->size.x, (col_r << 11 | col_g << 5 | col_b << 0));
            col_r += delta_r; col_g += delta_g; col_b += delta_b;
          }
        }
        break;
      case TYPE_ICON:
        renderIcon(c);
        break;
      case TYPE_LIST:
        //background not shown tft.fillRect(c->position.x, c->position.y, c->size.x, c->size.y, ST7735_RED);
        Control* i = c->child;
        word y_delta = 0;
        while (i) {
          if (i->visible) y_delta += renderItem(i, y_delta); else y_delta += i->size.y;
          i = i->next;
        }
        tft.drawRect(c->position.x, c->position.y, c->size.x, y_delta, c->colors.y);
        break;
    }
  } else tft.fillRect(c->position.x, c->position.y, c->size.x, c->size.y, mainControl->colors.y);
  c->uptodate = true;
}

void renderGUI() {
  guiuptodate = false;
  if (!mainControl->uptodate) {
    render(mainControl);
    mainControl->uptodate = true;
  }
  Control* c = mainControl->child;
  while (c) {
    if (!c->uptodate) render(c);
    c = c->next; //get next item
  }
  lastGUIRender = millis();
}

void swapXY(XY &xy) {
  word t = xy.x; xy.x = xy.y; xy.y = t;
}

void enterMenu(Control* control) {
  if (focusControl) {
    focusControl->selected = false;
    focusControl->uptodate = false;
  }
  control->selected = true;
  control->uptodate = false;
  if (control->child) {
    control->child->selected = true;
    control->child->uptodate = false;
  }
  guiuptodate = false; //request GUI update
}

void moveMenu(Control* control, bool directionDown) {
  if (control->child) {
    word s = 0; //selected index   0 is nothing selected
    word c = 0; //count
    Control* item = control->child; //find index s that is selected
    word i = 1;
    while (item) {
      c++;
      if (item->selected) s = i;
      i++;
      item = item->next;
    }
    if (directionDown == false) if (s > 1) s--; else s = c; //move according to direction
                           else if (s < c) s++; else s = 1;
    //switch
    item = control->child;
    i = 1;
    while (item) {
      if (i == s) {
        if (!item->selected) {
          item->selected = true;
          item->uptodate = false;
          control->uptodate = false;
          guiuptodate = false; //request GUI update
        }
      } else {
        if (item->selected) {
          item->selected = false;
          item->uptodate = false;
          control->uptodate = false;
          guiuptodate = false; //request GUI update
        }
      }
      i++;
      item = item->next;
    }
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
  if (focusControl->type == TYPE_LIST) { moveMenu(focusControl, false); }
}

void sendDown() {
  if (!focusControl) { //after start up jump into main menu
    focusControl = mainMenu;
    sendEnter(mainMenu);
    return;
  }
  if (focusControl->type == TYPE_LIST) { moveMenu(focusControl, true); }
}

void sendClick() {
  if (!focusControl) return; //after start up do nothing
  if (focusControl->type == TYPE_LIST) clickMenu(focusControl);
}

void sendEnter(Control* control) {
  if (control->events) if (control->events->onEnter) control->events->onEnter(control);
}

void setStatus(char* text) {
  if (text) strcpy(statusLabel->text, text); //else render whatever is already in statusLabel->text
  statusLabel->uptodate = false; //status label needs to update
  statusBar->uptodate = false; //background label needs to update
}

// MAIN LOOP

void loop() {
  encoder.tick();

  if (!rpiPoweringDown) { //normal operation as Raspberry Pi is not powering down
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
  
    if (readSerialData()) processReceiveBuffer(); //process serial data
  } else if (millis() > delayEnd) rpiPowerOff(); //as the Raspberry Pi powering down we need to wait it out (fixed timer of 15 seconds, not fancy) and at the end of timer change icon colors and change rpiPower booleans
     
  if (!guiuptodate) if (millis() - lastGUIRender > 50) renderGUI(); //wait at least 50ms between updates
}

// GUI ACTIONS

void swithRpiPower(Control* control) {
  rpiPower = !rpiPower;
  if(rpiPower) { strcpy(control->text, "Turn Streamer Off\0"); digitalWrite(RPI_POWER, HIGH) /*turn relay OFF*/; } //currently off
          else { strcpy(control->text, "Turning Off.\0");  Serial.print("<p>") /*Send Power Off command to Raspberry Pi*/ ;} //currently on        
  lastRpiPower = rpiPower;
  control->uptodate = false;
  control->parent->uptodate = false;
  guiuptodate = false;
}

void rpiPowerOff() {
  rpiPowerIcon->colors.x = COLOR_RED; rpiPowerIcon->uptodate = false;
  strcpy(rpiPowerItem->text, "Turn Streamer On\0"); rpiPowerItem->uptodate = false; rpiPowerItem->parent->uptodate = false;
  setStatus("\0");
  guiuptodate = false;
  digitalWrite(RPI_POWER, LOW) /*turn relay OFF*/;
  rpiPoweringDown = false;
}

void sendSerialText(Control* control) { 
  Serial.print("HELLO FROM ARDUINO\n");
  Serial.flush();
}

bool readSerialData() {
 while (Serial.available() > 0) {
    character = Serial.read();
    if (!receiving) {
      if (character == PPP_BEGIN_FLAG) {
        receiving = true;
        receiveIndex = 0;
        receiveSizeLeft = 1; //the command is expected - so 1 byte
      }
    } else {
      if (character == PPP_END_FLAG && receiveSizeLeft == 0) {
        if (receiveIndex <= PPP_NDX_LENGTH) receiveBuffer[PPP_NDX_LENGTH] = 0; //no length received - 1 byte packet like <P>
        receiving = false;
        return true;
      } else {
        if (receiveIndex < MAX_MESSAGE-1) {
          receiveBuffer[receiveIndex] = character;
          if (receiveIndex == PPP_NDX_LENGTH) receiveSizeLeft = character;  
                                         else if (receiveSizeLeft == 0) receiving = false; else receiveSizeLeft--;
          receiveIndex++;
        } else receiving = false;
      } 
    }
  }
  return false;
}

void processReceiveBuffer() {
  switch (receiveBuffer[PPP_NDX_COMMAND]) {  //first byte to be expected signal/command type - second (depending on the command) is the size in bytes of the payload
    case PPP_T_POWER_ON_COMPLETE:
      setStatus("Streamer is ready\0");
      rpiPowerIcon->colors.x = ST7735_WHITE; rpiPowerIcon->uptodate = false;
      guiuptodate = false; //request GUI update
      break;

    case PPP_T_POWER_OFF_CONFIRMED: /*Power Off command received by Raspberry Pi and this is confirmation, it is shutting down */ 
      rpiPoweringDown = true;
      delayEnd = millis() + FIFTEEN_SECONDS;
      setStatus("Wait 15 seconds...\0");
      airplayIcon->colors.x = COLOR_RED; airplayIcon->uptodate = false;
      guiuptodate = false; //request GUI update
      break;
      
    case PPP_T_AIRPLAY_ICON_COLOR:
      airplayIcon->colors.x = tft.color565(receiveBuffer[2], receiveBuffer[3], receiveBuffer[4]); airplayIcon->uptodate = false; //third, forth & fifth byte are new color for the airplay icon
      guiuptodate = false; //request GUI update
      break;
      
    case PPP_T_STATUS_TEXT:
      byte size_ = receiveBuffer[PPP_NDX_LENGTH]; receiveBuffer[PPP_NDX_LENGTH+size_+1] = 0; //terminate with zero byte
      setStatus(&receiveBuffer[2]);
      guiuptodate = false; //request GUI update
      break;
      
    case PPP_T_NO_OPERATION:
     
      break;
      
    default:
      sprintf(statusLabel->text, "%d NOT IMPL'D\n", (byte)receiveBuffer[PPP_NDX_COMMAND]);
      setStatus(NULL); //render whatever is in statusLabel->text
      guiuptodate = false; //request GUI update
      break;
  }  
}
