#include "notes.h"

#define buzzer 9
#define button A3
#define wrdSize 20

/*#define ditDly 70
#define dahDly ditDly*3
#define rlxDly ditDly
#define ltrDly ditDly*2
#define wrdDly ditDly*4*/

#define ditDly 150
#define dahDly ditDly*3
#define rlxDly ditDly*2
#define ltrDly ditDly*6
#define wrdDly ditDly*9
#define toneHz 880

int firstHeld;
boolean butActive;

char buffer[wrdSize];
byte bufferIndex = 0;
char msg[wrdSize] = "senha";

char dict[][wrdSize] = {
  "banana",
  "alface",
  "balde",
  "crypto",
  "pikachu",
  "pipoca",
  "gato",
  "governo",
  "bolacha",
  "cavalo",
  "morse",
  "caneca"
};

struct mc {
  unsigned char dd;
  unsigned char len;
};

const struct mc mrs[] = { // right to left bit encoded, 1=-; 0=. , only len rightmost bits valid
  /*   = " "       */ { 0b000000, 0},
  /* ! = "-.-.--"  */ { 0b110101, 6},
  /* " = ".-..-."  */ { 0b010010, 6},
  /* # = ""        */ { 0b111111, 0},
  /* $ = ".-..."   */ { 0b000010, 5},
  /* % = ""        */ { 0b111111, 0},
  /* & = ""        */ { 0b111111, 0}, 
  /* ' = ".----."  */ { 0b011110, 6},
  /* ( = "-.--."   */ { 0b001101, 5},
  /* ) = "-.--.-"  */ { 0b101101, 6},
  /* * = ""        */ { 0b111111, 0},
  /* + = ".-.-."   */ { 0b001010, 5},
  /* , = "--..--"  */ { 0b110011, 6},
  /* - = "-....-"  */ { 0b100001, 6},
  /* . = ".-.-.-"  */ { 0b101010, 6},
  /* / = "-..-."   */ { 0b001001, 5},
  /* 0 = "-----"   */ { 0b011111, 5},
  /* 1 = ".----"   */ { 0b011110, 5},
  /* 2 = "..---"   */ { 0b011100, 5},
  /* 3 = "...--"   */ { 0b011000, 5},
  /* 4 = "....-"   */ { 0b010000, 5},
  /* 5 = "....."   */ { 0b000000, 5},
  /* 6 = "-...."   */ { 0b000001, 5},
  /* 7 = "--..."   */ { 0b000011, 5},
  /* 8 = "---.."   */ { 0b000111, 5},
  /* 9 = "----."   */ { 0b001111, 5},
  /* : = "---..."  */ { 0b000111, 6},
  /* ; = "-.-.-."  */ { 0b010101, 6},
  /* < = ""        */ { 0b111111, 0},
  /* = = "-...-"   */ { 0b010001, 5},
  /* > = ""        */ { 0b111111, 0},
  /* ? = "..--.."  */ { 0b001100, 6},
  /* @ = ".--.-."  */ { 0b010110, 6},
  /* A = ".-"      */ { 0b000010, 2},
  /* B = "-..."    */ { 0b000001, 4},
  /* C = "-.-."    */ { 0b000101, 4},
  /* D = "-.."     */ { 0b000001, 3},
  /* E = "."       */ { 0b000000, 1},
  /* F = "..-."    */ { 0b000100, 4},
  /* G = "--."     */ { 0b000011, 3},
  /* H = "...."    */ { 0b000000, 4},
  /* I = ".."      */ { 0b000000, 2},
  /* J = ".---"    */ { 0b001110, 4},
  /* K = "-.-"     */ { 0b000101, 3},
  /* L = ".-.."    */ { 0b000010, 4},
  /* M = "--"      */ { 0b000011, 2},
  /* N = "-."      */ { 0b000001, 2},
  /* O = "---"     */ { 0b000111, 3},
  /* P = ".--."    */ { 0b000110, 4},
  /* Q = "--.-"    */ { 0b001011, 4},
  /* R = ".-."     */ { 0b000010, 3},
  /* S = "..."     */ { 0b000000, 3},
  /* T = "-"       */ { 0b000001, 1},
  /* U = "..-"     */ { 0b000100, 3},
  /* V = "...-"    */ { 0b001000, 4},
  /* W = ".--"     */ { 0b000110, 3},
  /* X = "-..-"    */ { 0b001001, 4},
  /* Y = "-.--"    */ { 0b001101, 4},
  /* Z = "--.."    */ { 0b000011, 4}
};

void sendMorse(char* msg) {
  byte index = 0;
  char ltr = msg[index];
  while(ltr) {
    struct mc ch = mrs[toupper(ltr)-32];
    if(ch.len) {
      byte byt = ch.dd;
      for(byte i=0; i<ch.len; i++, byt>>=1) {
        tone(buzzer, toneHz);
        if(byt&1) delay(dahDly);
        else delay(ditDly);
        noTone(buzzer);
        if(i<ch.len-1) delay(rlxDly);
        else delay(ltrDly);
      }
    } else if(ltr = ' ') {
      delay(wrdDly-ltrDly);
    }
    ltr = msg[++index];
  }
}

void setup() {
  pinMode(buzzer, OUTPUT);
  pinMode(button, INPUT_PULLUP);
  Serial.begin(9600);
  randomSeed(analogRead(0));
}

void loop() {
  if(digitalRead(button) == HIGH) {
    if(butActive == false) {
      butActive = true;
      firstHeld = millis();
    }
  } else {
    if(butActive == true) {
      butActive = false;
      int millisHeld = millis() - firstHeld;
      if(millisHeld > 10) { //debounce
        if(millisHeld > 2500) starWars();
        if(millisHeld > 1000) newWord();
        else sendMorse(msg);
      }
    }
  }
  
  while(Serial.available() > 0) {
    char in = Serial.read();
    if(in == '\n') {
      buffer[bufferIndex] = '\0';
      bufferIndex = 0;
      snprintf(msg, wrdSize, "%s", buffer);
      Serial.println(msg);
    } else {
      buffer[bufferIndex++] = in;
    }
  }
}

void newWord() {
  snprintf(msg, wrdSize, "%s", dict[random(sizeof(dict)/20+1)]);
  Serial.println(msg);
  tone(buzzer, toneHz, 70);
  delay(100);
  tone(buzzer, toneHz, 70);
  delay(100);
}

int melody[] = {
  NOTE_C4,
  NOTE_G4,
  NOTE_F4,
  NOTE_E4,
  NOTE_D4,
  NOTE_C5,
  NOTE_G4,
  NOTE_F4,
  NOTE_E4,
  NOTE_D4,
  NOTE_C5,
  NOTE_G4,
  NOTE_F4,
  NOTE_E4,
  NOTE_F4,
  NOTE_D4
};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteLength[] = {
  16, 8, 2, 2, 2, 16, 8, 2, 2, 2, 16, 8, 2, 2, 2, 16
};

void starWars() {
  // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote < 16; thisNote++) {

    // to calculate the note duration, take one second
    // divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 60 * noteLength[thisNote];
    tone(buzzer, melody[thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration + 20;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(buzzer);
  }
}
