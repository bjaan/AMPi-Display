#include <Adafruit_ST7735.h> // Adafruit ST7735-Bibliothek wird benötigt
#include <Fonts/FreeSans9pt7b.h>
#include <RotaryEncoder.h>

#define RPI_POWER 8
#define ROTENC_CLK 2
#define ROTENC_DT 3
#define ROTENC_SW 4

#define TYPE_MAIN 1000
#define TYPE_LABEL 100
#define TYPE_RECTANGLE 200
#define TYPE_LIST 300
#define TYPE_ITEM 400
#define TYPE_ICON 500

struct Control;

struct XY {
  int x;
  int y;
  XY() {
    x = 0; y = 0;
  };
};

typedef void (*Event) (Control* control);

struct Events {
  Event onClick;
  Event onEnter;
  Event onExit;
};

struct Control {
  int type;
  bool selected;
  bool visible;
  char* text;
  char* oldText;
  XY position;
  XY size;
  XY colors;
  Events* events;
  bool uptodate;
  Control* next;
  Control* parent;
  Control* child;

  Control(int type, Control* parent) {
    this->type = type;
    this->parent = parent;
    selected = false;
    visible = true;
    if (type == TYPE_LABEL || type == TYPE_ITEM) {
      text = (char*)malloc(35);
      text[0] = 0;
      oldText = (char*)malloc(35);
      oldText[0] = 0;
    } else {
      text = NULL;
      oldText = NULL;
    }
    position = XY();
    size = XY();
    colors = XY();
    if (type != TYPE_RECTANGLE) {
      colors.x = ST7735_WHITE;
      colors.y = ST7735_BLACK;
    } else {
      colors.x = ST7735_BLACK;
      colors.y = ST7735_BLACK;
    }
    uptodate = false;
    next = NULL;
    child = NULL;
    events = NULL;
  };
};

struct StackedString {
  size_t size;
  char* string;
  StackedString* next;

  StackedString(size_t size) {
    this->size = size;
    string = (char*)malloc(size);
  }
};

//int8_t cs, int8_t dc, int8_t mosi, int8_t sclk, int8_t rst
Adafruit_ST7735 tft = Adafruit_ST7735(10 /*cs*/, 9 /*dc*/, 11/*mosi*/, 13 /*sclk*/, -1 /*rst*/); // ST7735-Bibliothek Setup

RotaryEncoder encoder(ROTENC_CLK, ROTENC_DT, RotaryEncoder::LatchMode::TWO03);

int lastStateCLK;
unsigned long lastGUIRender;
bool rpiPower;
bool lastRpiPower;
bool guiuptodate;
int lastButtonPress;
Control* mainControl;
Control* focusControl;
Control* statusLabel;
Control* rpiPowerLabel;
Control* mainMenu;

void setup(void) {

  /***
     ST7735-Chip initialisieren (INITR_BLACKTAB)
     Muss bei AZ-Delivery 1.77'' 160x128px RGB TFT INITR_GREENTAB sein ansonsten Pixelfehler rechts und unten.
     Hinweis: https://github.com/adafruit/Adafruit-ST7735-Library/blob/master/examples/soft_spitftbitmap/soft_spitftbitmap.ino#L52
     Zeile 52-65
   ***/

  tft.initR(INITR_BLACKTAB);

  tft.setRotation(1);

  tft.setFont(&FreeSans9pt7b);
 
  //RPi Power
  pinMode(RPI_POWER, OUTPUT);    // sets the digital pin 2 as output
  digitalWrite(RPI_POWER, LOW); //off
  rpiPower = false;
  lastRpiPower = true;

  //initialize encoder
  attachInterrupt(ROTENC_CLK, checkPosition, CHANGE);
  attachInterrupt(ROTENC_DT, checkPosition, CHANGE);
  pinMode(ROTENC_SW, INPUT_PULLUP);
  lastButtonPress = millis();

  //initialize GUI
  mainControl = new Control(TYPE_MAIN, NULL /** root - no parent **/);
  mainControl->colors.y = tft.color565(60, 62, 77);
  guiuptodate = false;

  //initialize status bar
  statusLabel = new Control(TYPE_LABEL, mainControl);
  strcpy(statusLabel->text, "status: active\0");
  statusLabel->position.x = 3;
  statusLabel->position.y = 125;
  statusLabel->size.x = 154; //160 - 6;
  statusLabel->size.y = 15;
  statusLabel->colors.x = ST7735_WHITE;
  statusLabel->colors.y = tft.color565(40, 106, 237);

  Control* statusBar = new Control(TYPE_RECTANGLE, mainControl);
  statusBar->position.x = 0;
  statusBar->position.y = 96;//128 - 32;
  statusBar->size.x = 160;
  statusBar->size.y = 17;
  statusBar->colors.x = tft.color565(60, 62, 77);
  statusBar->colors.y = tft.color565(40, 106, 237);

  Control* statusBar2 = new Control(TYPE_RECTANGLE, mainControl);
  statusBar2->position.x = 0;
  statusBar2->position.y = 0;
  statusBar2->size.x = 160;
  statusBar2->size.y = 15;
  statusBar2->colors.x = tft.color565(40, 106, 237);
  statusBar2->colors.y = tft.color565(12, 12, 13);

  Control* statusBar3 = new Control(TYPE_RECTANGLE, mainControl);
  statusBar3->position.x = 0;
  statusBar3->position.y = 113;
  statusBar3->size.x = 160;
  statusBar3->size.y = 15;
  statusBar3->colors.x = tft.color565(40, 106, 237);
  statusBar3->colors.y = tft.color565(40, 106, 237);
  
/*
  rpiPowerLabel = new Control(TYPE_LABEL, mainControl);
  rpiPowerLabel->position.x = 36;
  rpiPowerLabel->position.y = 30;
  rpiPowerLabel->size.x = 30;
  rpiPowerLabel->size.y = 15;
  rpiPowerLabel->colors.x = ST7735_WHITE;
  rpiPowerLabel->colors.y = tft.color565(60, 62, 77);
*/

  Control* ampiLabelMain = new Control(TYPE_LABEL, mainControl);
  ampiLabelMain->position.x = 3;
  ampiLabelMain->position.y = 22;
  ampiLabelMain->size.x = 45;
  ampiLabelMain->size.y = 15;
  strcpy(ampiLabelMain->text, "AMPi\0");
  ampiLabelMain->colors.x = tft.color565(255, 140, 26);
  ampiLabelMain->colors.y = tft.color565(60, 62, 77);
    
  mainMenu = new Control(TYPE_LIST, mainControl);
  mainMenu->position.x = 3;
  mainMenu->position.y = 35;
  mainMenu->size.x = 154; // 160 - 6;
  mainMenu->size.y = 73;
  mainMenu->colors.x = ST7735_WHITE;
  mainMenu->colors.y = tft.color565(60, 62, 77);
  mainMenu->events = new Events();
  mainMenu->events->onEnter = enterMenu;

  Control* rpiPowerItem = new Control(TYPE_ITEM, mainMenu);
  rpiPowerItem->size.x = mainMenu->size.x;
  rpiPowerItem->size.y = 19;
  rpiPowerItem->colors.y = ST7735_GREEN;
  strcpy(rpiPowerItem->text, "Turn Streamer On\0");
  rpiPowerItem->position.x = 1;
  rpiPowerItem->position.y = 11;
  rpiPowerItem->colors.x = tft.color565(60, 62, 77);
  rpiPowerItem->colors.y = tft.color565(255, 140, 26);
  rpiPowerItem->events = new Events();
  rpiPowerItem->events->onClick = swithRpiPower;

  Control* pandoraItem = new Control(TYPE_ITEM, mainMenu);
  pandoraItem->size.x = mainMenu->size.x;
  pandoraItem->size.y = 19;
  pandoraItem->colors.y = ST7735_GREEN;
  strcpy(pandoraItem->text, "Pandora Music\0");
  pandoraItem->position.x = 1;
  pandoraItem->position.y = 11;
  pandoraItem->colors.x = tft.color565(60, 62, 77);
  pandoraItem->colors.y = tft.color565(255, 140, 26);
  pandoraItem->events = new Events();
  pandoraItem->events->onClick = sendSerialText;

  Control* appleItem = new Control(TYPE_ITEM, mainMenu);
  appleItem->size.x = mainMenu->size.x;
  appleItem->size.y = 19;
  appleItem->colors.y = ST7735_GREEN;
  strcpy(appleItem->text, "Apple Music\0");
  appleItem->position.x = 1;
  appleItem->position.y = 11;
  appleItem->colors.x = tft.color565(60, 62, 77);
  appleItem->colors.y = tft.color565(255, 140, 26);

  Control* rpiPowerIcon = new Control(TYPE_ICON, mainControl);
  rpiPowerIcon->size.x = 15;
  rpiPowerIcon->size.y = 15;
  rpiPowerIcon->position.x = 3+45/*AMPi label width*/+2;
  rpiPowerIcon->position.y = 22;
  
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

  Serial.begin(9600); // open the serial port at 9600 bps:

  renderGUI();
}

void renderText(char* text, char* oldText, XY position, XY size, XY colors) {
  int x;
  int y;
  uint16_t w;
  uint16_t h;
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

int renderItem(Control* item, int y_delta)
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
          word delta_r = ((c->colors.y >> 11 & 31) - (c->colors.x >> 11 & 31)) / c->size.y;
          word delta_g = ((c->colors.y >> 5 & 63) - (c->colors.x >> 5 & 63)) / c->size.y;
          word delta_b = ((c->colors.y >> 0 & 31) - (c->colors.x >> 0 & 31)) / c->size.y;
          word col_r = (c->colors.x >> 11 & 31);
          word col_g = (c->colors.x >> 5 & 63);
          word col_b = (c->colors.x >> 0 & 31);
          for (int i = 0; i < c->size.y; i++) {
            tft.drawFastHLine( c->position.x, c->position.y + i, c->size.x, (col_r << 11 | col_g << 5 | col_b << 0));
            col_r += delta_r;
            col_g += delta_g;
            col_b += delta_b;
          }
        }
        break;
      case TYPE_LIST:
        //background not shown tft.fillRect(c->position.x, c->position.y, c->size.x, c->size.y, ST7735_RED);
        Control* i = c->child;
        int y_delta = 0;
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
    int s = -1; //selected index
    int c = 0; //count
    //find index s that is selected
    Control* item = control->child;
    int i = 0;
    while (item) {
      c++;
      if (item->selected) {
        s = i;
      }
      i++;
      item = item->next;
    }
    //move according direction
    if (s == -1) {
      s = 0;
    } else {
      if (directionDown == false) {
        if (s > 0) {
          s--;
        } else {
          s = c - 1;
        }
      } else {
        if (s < c - 1) {
          s++;
        } else {
          s = 0;
        }
      }
    }
    //switch
    item = control->child;
    i = 0;
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

void loop() {
  bool waitASec = false;
  unsigned long ms = millis();

  static int pos = 0;
  encoder.tick();

  int newPos = encoder.getPosition();
  if (pos != newPos) {
    if (pos % 2 == 0) {
      if (pos < newPos) {
        sendUp();
      } else {
        sendDown();
      }
    }
    pos = newPos;
  } // if

  // Read the button state
  int btnState = digitalRead(ROTENC_SW);

  //If we detect LOW signal, button is pressed
  if (btnState == LOW) {
    //if 500ms have passed since last LOW pulse, it means that the
    //button has been pressed, released and pressed again
    if (ms - lastButtonPress > 500) {
      sendClick();
      waitASec = true;
    }

    // Remember last button press event
    lastButtonPress = ms;
  }

  
  if (rpiPower) {
    //read serial data only when we know that power is sent to the Pi
    if (Serial.available()) {
      #define MAX_MESSAGE 200
      static char buffer[MAX_MESSAGE];
      static unsigned char index = 0;
      char inch;
      while (Serial.available() > 0) {
        inch = Serial.read();
        if (inch == '\n') {
          buffer[index] = 0;
          break;
        } else {        
          if (index < MAX_MESSAGE-1) {
            buffer[index++] = inch;
          }
        }
      }
      buffer[index] = 0;
      receiveSerialText(&buffer[0]);
    }
   }

  if (!guiuptodate) {
    if (ms - lastGUIRender > 300) { //wait at least 300ms between updates
      renderGUI();
    }
  }

  if (waitASec)
    delay(1000);

}

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

void checkPosition() {
  encoder.tick(); // just call tick() to check the state.
}


void sendSerialText(Control* control) {
  Serial.print("HELLO FROM ARDUINO\n");
  Serial.flush();
}

void receiveSerialText(char* text) {
   //Serial.print(text);
    /*status update*/
    //sprintf(statusLabel->text, "SIN: %s\0", text);
    //strcpy(statusLabel->text, "RECEIVED!\n");
    strcpy(statusLabel->text, text);
    statusLabel->uptodate = false;
    guiuptodate = false; //request GUI update
    /*end status update*/
}
