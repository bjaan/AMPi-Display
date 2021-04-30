//ICONS - tool to create at http://javl.github.io/image2cpp/ - use 16x16 monochrome images & use invert colors
const unsigned char IMG_POWER [] PROGMEM = { 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x0d, 0xb0, 0x1d, 0xb8, 0x39, 0x9c, 0x31, 0x8e, 0x60, 0x06, 0x60, 0x06, 0x60, 0x06, 0x60, 0x06, 0x30, 0x0c, 0x38, 0x1c, 0x1c, 0x38, 0x0f, 0xf0, 0x03, 0xc0 };
const unsigned char IMG_AIRPLAY [] PROGMEM = { 0x07, 0xe0, 0x08, 0x10, 0x30, 0x0c, 0x43, 0xc2, 0x44, 0x22, 0x49, 0x92, 0x92, 0x49, 0x94, 0x29, 0x94, 0x29, 0x94, 0x29, 0x90, 0x09, 0x49, 0x92, 0x43, 0xc2, 0x27, 0xe4, 0x0f, 0xf0, 0x0f, 0xf0 };

#include <Adafruit_ST7735.h> //see https://github.com/adafruit/Adafruit-ST7735-Library
#include <Fonts/FreeSans9pt7b.h> //font available in the above libary
#include <RotaryEncoder.h> //see http://www.mathertel.de/Arduino/RotaryEncoderLibrary.aspx

#define RPI_POWER 8
#define ROTENC_CLK 2
#define ROTENC_DT 3
#define ROTENC_SW 4
#define COLOR_RED 64611 /*tft.color565(255, 140, 26)*/
#define COLOR_BLUE_DARK 14825 /*tft.color565(60, 62, 77)*/
#define COLOR_BLUE 11101 /*tft.color565(40, 106, 237)*/
#define COLOR_GREY_DARK 2145 /*tft.color565(12, 12, 13)*/

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

#define PPP_BEGIN_FLAG 0x7E
#define PPP_END_FLAG 0x7F
#define PPP_T_NO_OPERATION 0x00
#define PPP_T_POWER_ON_COMPLETE 0x50 //P
#define PPP_T_AIRPLAY_STATUS 0x52 //R

// DISPLAY & ROTARY ENCODE DEFINITIONS

Adafruit_ST7735 tft = Adafruit_ST7735(10 /*cs*/, 9 /*dc*/, 11/*mosi*/, 13 /*sclk*/, -1 /*rst*/);

RotaryEncoder encoder(ROTENC_CLK, ROTENC_DT, RotaryEncoder::LatchMode::TWO03);

// GLOBALS
unsigned long lastGUIRender;
bool rpiPower = false;
bool lastRpiPower = true;
bool guiuptodate = false;
bool buttonDown = false;
int encoderPosition = 0;
unsigned char receiveBuffer[MAX_MESSAGE];
bool receiving = false;
unsigned char receiveIndex = 0;
Control* mainControl;
Control* focusControl;
Control* statusLabel;
Control* rpiPowerIcon;
Control* airplayIcon;
Control* mainMenu;

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
  mainControl->colors.y = COLOR_BLUE_DARK;
  guiuptodate = false;

  //initialize status bar
  statusLabel = new Control(TYPE_LABEL, mainControl);
  strcpy(statusLabel->text, "status: active\0");
  statusLabel->position.x = 3;
  statusLabel->position.y = 125;
  statusLabel->size.x = 154; //160 - 6;
  statusLabel->size.y = 15;
  statusLabel->colors.x = ST7735_WHITE;
  statusLabel->colors.y = COLOR_BLUE;

  Control* statusBar = new Control(TYPE_RECTANGLE, mainControl);
  statusBar->position.x = 0;
  statusBar->position.y = 96;//128 - 32;
  statusBar->size.x = 160;
  statusBar->size.y = 17;
  statusBar->colors.x = COLOR_BLUE_DARK;
  statusBar->colors.y = COLOR_BLUE;

  Control* statusBar2 = new Control(TYPE_RECTANGLE, mainControl);
  statusBar2->position.x = 0;
  statusBar2->position.y = 0;
  statusBar2->size.x = 160;
  statusBar2->size.y = 15;
  statusBar2->colors.x = COLOR_BLUE;
  statusBar2->colors.y = COLOR_GREY_DARK;

  Control* statusBar3 = new Control(TYPE_RECTANGLE, mainControl);
  statusBar3->position.x = 0;
  statusBar3->position.y = 113;
  statusBar3->size.x = 160;
  statusBar3->size.y = 15;
  statusBar3->colors.x = COLOR_BLUE;
  statusBar3->colors.y = COLOR_BLUE;
  
  Control* ampiLabelMain = new Control(TYPE_LABEL, mainControl);
  ampiLabelMain->position.x = 3;
  ampiLabelMain->position.y = 22;
  ampiLabelMain->size.x = 45;
  ampiLabelMain->size.y = 15;
  strcpy(ampiLabelMain->text, "AMPi\0");
  ampiLabelMain->colors.x = COLOR_RED;
  ampiLabelMain->colors.y = COLOR_BLUE_DARK;
    
  mainMenu = new Control(TYPE_LIST, mainControl);
  mainMenu->position.x = 3;
  mainMenu->position.y = 35;
  mainMenu->size.x = 154; // 160 - 6;
  mainMenu->size.y = 73;
  mainMenu->colors.x = ST7735_WHITE;
  mainMenu->colors.y = COLOR_BLUE_DARK;
  mainMenu->events = new Events();
  mainMenu->events->onEnter = enterMenu;

  Control* rpiPowerItem = new Control(TYPE_ITEM, mainMenu);
  rpiPowerItem->size.x = mainMenu->size.x;
  rpiPowerItem->size.y = 19;
  rpiPowerItem->colors.y = ST7735_GREEN;
  strcpy(rpiPowerItem->text, "Turn Streamer On\0");
  rpiPowerItem->position.x = 1;
  rpiPowerItem->position.y = 11;
  rpiPowerItem->colors.x = COLOR_BLUE_DARK;
  rpiPowerItem->colors.y = COLOR_RED;
  rpiPowerItem->events = new Events();
  rpiPowerItem->events->onClick = swithRpiPower;

  Control* pandoraItem = new Control(TYPE_ITEM, mainMenu);
  pandoraItem->size.x = mainMenu->size.x;
  pandoraItem->size.y = 19;
  pandoraItem->colors.y = ST7735_GREEN;
  strcpy(pandoraItem->text, "Pandora Music\0");
  pandoraItem->position.x = 1;
  pandoraItem->position.y = 11;
  pandoraItem->colors.x = COLOR_BLUE_DARK;
  pandoraItem->colors.y = COLOR_RED;
  pandoraItem->events = new Events();
  pandoraItem->events->onClick = sendSerialText;

  Control* appleItem = new Control(TYPE_ITEM, mainMenu);
  appleItem->size.x = mainMenu->size.x;
  appleItem->size.y = 19;
  appleItem->colors.y = ST7735_GREEN;
  strcpy(appleItem->text, "Apple Music\0");
  appleItem->position.x = 1;
  appleItem->position.y = 11;
  appleItem->colors.x = COLOR_BLUE_DARK;
  appleItem->colors.y = COLOR_RED;

  rpiPowerIcon = new Control(TYPE_ICON, mainControl);
  rpiPowerIcon->size.x = 16;
  rpiPowerIcon->size.y = 16;
  rpiPowerIcon->position.x = 50;// 3+45/*AMPi label width*/+2
  rpiPowerIcon->position.y = 8;
  rpiPowerIcon->colors.x = COLOR_RED;
  rpiPowerIcon->icon = IMG_POWER;

  airplayIcon = new Control(TYPE_ICON, mainControl);
  airplayIcon->size.x = 16;
  airplayIcon->size.y = 16;
  airplayIcon->position.x = 68;// 3+45/*AMPi label width*/+2+16/*icon width*/+2
  airplayIcon->position.y = 8;
  airplayIcon->colors.x = COLOR_RED;
  airplayIcon->icon = IMG_AIRPLAY;
  
  //build main menu
  mainMenu->child = rpiPowerItem;
  rpiPowerItem->next = pandoraItem;
  pandoraItem->next = appleItem;

  //build main GUI
  mainControl->child = statusBar;
  statusBar->next = statusBar2;
  statusBar2->next = statusBar3;
  statusBar3->next = statusLabel;
  statusLabel->next = ampiLabelMain;
  ampiLabelMain->next = mainMenu;
  mainMenu->next = rpiPowerIcon;
  rpiPowerIcon->next = airplayIcon;
  //airplayIcon->next = null;

  Serial.begin(9600); // open the serial port at 9600 bps:

  renderGUI();
}

// GUI LIBARY IMPLEMENTATION

void renderText(char* text, char* oldText, XY position, XY size, Colors colors) {
  word x;
  word y;
  word w;
  word h;
  if (text)
    tft.getTextBounds(text, position.x, position.y, &x, &y, &w, &h);
  if (oldText[0] != 0) {
    tft.setCursor(position.x, position.y);
    tft.setTextColor(colors.y);
    tft.print(oldText);
  } else {
    tft.fillRect(x, y, w, h, colors.y);
  }
  if (text[0] != 0) {
    tft.setCursor(position.x, position.y);
    tft.setTextColor(colors.x);
    tft.print(text);
    strcpy(oldText, text);
  } else {
    tft.fillRect(x, y, w, h, colors.y);
  }
  tft.fillRect(x + w, y, size.x - w, size.y, colors.y);
  tft.fillRect(x, y + h, w, size.y - h, colors.y);
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
      renderText(item->text, item->oldText, textposition, textsize, item->parent->colors);
    }    
    if (item->selected) {
      tft.drawRect(item->parent->position.x + item->position.x, item->parent->position.y + item->position.x + y_delta, item->size.x - item->position.x - item->position.x, item->size.y - item->position.x - item->position.x, item->colors.y);
    } else {
      tft.drawRect(item->parent->position.x + item->position.x, item->parent->position.y + item->position.x + y_delta, item->size.x - item->position.x - item->position.x, item->size.y - item->position.x - item->position.x, item->colors.x);
    }
    item->uptodate = true;
  }
  return item->size.y;
}

void render(Control* c) {
  if (c->visible) {
    switch (c->type) {
      case TYPE_MAIN:
        tft.fillScreen(c->colors.y);
        break;
      case TYPE_LABEL:
        renderText(c->text, c->oldText, c->position, c->size, c->colors);
        break;
      case TYPE_RECTANGLE:
        if (c->colors.x == c->colors.y)
          tft.fillRect(c->position.x, c->position.y, c->size.x, c->size.y, c->colors.y);
        else {
          word delta_r = ((c->colors.y >> 11 & 31) - (c->colors.x >> 11 & 31)) / (int)c->size.y;
          word delta_g = ((c->colors.y >> 5 & 63) - (c->colors.x >> 5 & 63)) / (int)c->size.y;
          word delta_b = ((c->colors.y >> 0 & 31) - (c->colors.x >> 0 & 31)) / (int)c->size.y;
          word col_r = (c->colors.x >> 11 & 31);
          word col_g = (c->colors.x >> 5 & 63);
          word col_b = (c->colors.x >> 0 & 31);
          for (word i = 0; i < c->size.y; i++) {
            tft.drawFastHLine( c->position.x, c->position.y + i, c->size.x, (col_r << 11 | col_g << 5 | col_b << 0));
            col_r += delta_r;
            col_g += delta_g;
            col_b += delta_b;
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
          if (i->visible)
            y_delta += renderItem(i, y_delta);
          else
            y_delta += i->size.y;
          i = i->next;
        }
        tft.drawRect(c->position.x, c->position.y, c->size.x, y_delta, c->colors.y);
        break;
    }
  } else {
    tft.fillRect(c->position.x, c->position.y, c->size.x, c->size.y, mainControl->colors.y);
  }
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
    if (!c->uptodate) {
      render(c);
    }
    //next item
    c = c->next;
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
      if (item->selected) {
        s = i;
      }
      i++;
      item = item->next;
    }
    if (directionDown == false) { //move according to direction
      if (s > 1) {
        s--;
      } else {
        s = c;
      }
    } else {
      if (s < c) {
        s++;
      } else {
        s = 1;
      }
    }
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
        if (item->selected) 
          if (item->events)
            if (item->events->onClick) 
              item->events->onClick(item);
        item = item->next;
    }
}

void sendUp() {
  if (!focusControl) //after start up do nothing
    return;
  if (focusControl->type == TYPE_LIST) {
    moveMenu(focusControl, false);
    return;
  }
}

void sendDown() {
  if (!focusControl) { //after start up jump into main menu
    focusControl = mainMenu;
    sendEnter(mainMenu);
    return;
  }
  if (focusControl->type == TYPE_LIST) {
    moveMenu(focusControl, true);
    return;
  }
}

void sendClick() {
  if (!focusControl) //after start up do nothing
    return;
  if (focusControl->type == TYPE_LIST) {
    clickMenu(focusControl);
    return;
  }
}

void sendEnter(Control* control) {
  if (control->events)
    if (control->events->onEnter)
      control->events->onEnter(control);
}

// MAIN LOOP

void loop() {
  encoder.tick();
  int newPos = encoder.getPosition();
  if (encoderPosition != newPos) {
    if (encoderPosition % 2 == 0) {
      if (encoderPosition < newPos) {
        sendUp();
      } else {
        sendDown();
      }
    }
    encoderPosition = newPos;
  }
 
  int btnState = digitalRead(ROTENC_SW); //read the button state

  if (btnState == LOW) { //if we detect LOW signal, button is pressed
    if (!buttonDown) { //if 500ms have passed since last LOW pulse, it means that the button has been pressed, released and pressed again
      sendClick();
    }
    buttonDown = true;
  } else {
    buttonDown = false; 
  }

  if (rpiPower) { //when we know that power is sent to the Pi
    if (Serial.available()) {  //read serial data only 
      unsigned char ch;
      while (Serial.available() > 0) {
        ch = Serial.read();
        switch (ch) {
          case PPP_BEGIN_FLAG:
            receiving = true;
            receiveIndex = 0;
            break;
          case PPP_END_FLAG:
            if(receiving) {
              processReceiveBuffer();
              receiving = false;
            }
            break;
          default:
            if (receiving && receiveIndex < MAX_MESSAGE-1) {
              receiveBuffer[receiveIndex] = ch;
              receiveIndex++;
            } else {
              receiveIndex = 0;
              receiving = false; //failed to find begin flag or too big - ignore packet/stop looking for end
            }
            break;
        }
      }
    } else {
      if (Serial.available()) {  //read serial data only when available
        Serial.read(); //eat up - ignore all serial trafic when Pi is unpowered
      }
    }
  }
  
  if (!guiuptodate) {
    if (millis() - lastGUIRender > 50) { //wait at least 50ms between updates
      renderGUI();
    }
  }
}

// GUI ACTIONS

void swithRpiPower(Control* control) {
  rpiPower = !rpiPower;

  if(rpiPower) {
    strcpy(control->text, "Turn Streamer Off\0");
    digitalWrite(RPI_POWER, HIGH);//off
  } else {
    strcpy(control->text, "Turn Streamer On\0");
    digitalWrite(RPI_POWER, LOW);//on
  }
  lastRpiPower = rpiPower;
  control->uptodate = false;
  control->parent->uptodate = false;
  guiuptodate = false;
}

void sendSerialText(Control* control) { 
  Serial.print("HELLO FROM ARDUINO\n");
  Serial.flush();
}

void processReceiveBuffer() {
  //first byte to be expected signal/command type
  switch (receiveBuffer[0]) {
    case PPP_T_POWER_ON_COMPLETE:
      strcpy(statusLabel->text, "Steamer is ready\n");
      statusLabel->uptodate = false;
      
      rpiPowerIcon->colors.x = ST7735_WHITE;
      rpiPowerIcon->uptodate = false;
      
      guiuptodate = false; //request GUI update
      break;
    case PPP_T_AIRPLAY_STATUS:
      memcpy(&airplayIcon->colors.x, receiveBuffer[1], 2); //third & forth byte are new color for the airplay icon
      airplayIcon->uptodate = false;     
      guiuptodate = false; //request GUI update
      break;
    case PPP_T_NO_OPERATION:
      break;
    default:
      sprintf(statusLabel->text, "%d NOT IMPL'D\n", (byte)receiveBuffer[1]);
      statusLabel->uptodate = false;
      
      guiuptodate = false; //request GUI update
      break;
  }  
}
