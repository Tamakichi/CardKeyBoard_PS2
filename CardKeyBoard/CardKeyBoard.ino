//
// CardKeyBoard.ino
// PS/2キーボード IFバージョン by たま吉さん
// 2019/03/08 
// 必要ライブラリ
//  ps2dev https://playground.arduino.cc/uploads/ComponentLib/ps2dev.zip
//  Adafruit_NeoPixel 
// 
// 2019/03/10 PS/2イニシャル処理の修正（IchigoLatte対応、0x00は送信しない）
//

#include <Adafruit_NeoPixel.h>
#include <ps2dev.h>
#define KB_CLK      A4  // PS/2 CLK  IchigoJamのKBD1に接続
#define KB_DATA     A5  // PS/2 DATA IchigoJamのKBD2に接続

#define PIN           13
#define NUMPIXELS      1

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
PS2dev keyboard(KB_CLK, KB_DATA); // PS/2デバイス

//#include <Wire.h>

#define Set_Bit(val, bitn)    (val |=(1<<(bitn)))
#define Clr_Bit(val, bitn)     (val&=~(1<<(bitn)))
#define Get_Bit(val, bitn)    (val &(1<<(bitn)) )

//       d0   d1     d2  d3 d4 d5 d6 d7 d8 d9 d10 d11
//A3：   esc   1      2  3  4  5  6  7  8  9  0   del
//A2:    tab   q      w  e  r  t  y  u  i  o  p
//A1:   left   up     a  s  d  f  g  h  j  k  l   enter
//A0:   down   right  z  x  c  v  b  n  m  ,  .   space
//sym: d15
//shift: d12
//fn: d14

unsigned char KeyMap[48][7] =
{ //nor, shift,long_shift, sym,long_sym,fn,long_fn,
  {  27,  27,  27,  27,  27, 128, 128},//esc
  { '1', '1', '1', '!', '!', 129, 129},//1
  { '2', '2', '2', '@', '@', 130, 130},//2
  { '3', '3', '3', '#', '#', 131, 131},//3
  { '4', '4', '4', '$', '$', 132, 132},//4
  { '5', '5', '5', '%', '%', 133, 133},//5
  { '6', '6', '6', '^', '^', 134, 134},//6
  { '7', '7', '7', '&', '&', 135, 135},//7
  { '8', '8', '8', '*', '*', 136, 136},//8
  { '9', '9', '9', '(', '(', 137, 137},//9
  { '0', '0', '0', ')', ')', 138, 138},//0
  {  8 , 127 , 127,  8 ,  8 , 139, 139}, //del
  {  9 ,  9 ,  9 ,  9 ,  9 , 140, 140},//tab
  { 'q', 'Q', 'Q', '{', '{', 141, 141},//q
  { 'w', 'W', 'W', '}', '}', 142, 142},//w
  { 'e', 'E', 'E', '[', '[', 143, 143},//e
  { 'r', 'R', 'R', ']', ']', 144, 144},//r
  { 't', 'T', 'T', '/', '/', 145, 145},//t
  { 'y', 'Y', 'Y', '\\', '\\', 146, 146}, //y
  { 'u', 'U', 'U', '|', '|', 147, 147},//u
  { 'i', 'I', 'I', '~', '~', 148, 148},//i
  { 'o', 'O', 'O', '\'', '\'', 149, 149}, //o
  { 'p', 'P', 'P', '"', '"', 150, 150},//p
  {  0 ,  0 ,  0 ,  0 ,  0 , 0, 0},    //  no key
  {  180,  180,  180,  180,  180, 152, 152},//LEFT
  {  181,  181,  181,  181,  181, 153, 153},//UP
  { 'a', 'A', 'A', ';', ';', 154, 154},//a
  { 's', 'S', 'S', ':', ':', 155, 155},//s
  { 'd', 'D', 'D', '`', '`', 156, 156},//d
  { 'f', 'F', 'F', '+', '+', 157, 157},//f
  { 'g', 'G', 'G', '-', '-', 158, 158},//g
  { 'h', 'H', 'H', '_', '_', 159, 159},//h
  { 'j', 'J', 'J', '=', '=', 160, 160},//j
  { 'k', 'K', 'K', '?', '?', 161, 161},//k
  { 'l', 'L', 'L',   0,   0, 162, 162},//l
  { 13 , 13 ,  13,  13,  13, 163, 163},//enter
  {  182, 182 ,  182,  182,  182, 164, 164},//DOWN
  {  183, 183 ,  183,  183,  183, 165, 165},//RIGHT
  { 'z', 'Z', 'Z',  0 ,  0 , 166, 166},//z
  { 'x', 'X', 'X',  0 ,  0 , 167, 167},//x
  { 'c', 'C', 'C',  0 ,  0 , 168, 168},//c
  { 'v', 'V', 'V',  0 ,  0 , 169, 169},//v
  { 'b', 'B', 'B',  0 ,  0 , 170, 170},//b
  { 'n', 'N', 'N',  0 ,  0 , 171, 171},//n
  { 'm', 'M', 'M',  0 ,  0 , 172, 172},//m
  { ',', ',', ',', '<', '<', 173, 173},//,
//  { '.', '.', '.', '<', '>', 174, 174},//.
  { '.', '.', '.', '>', '>', 174, 174},//.
  { ' ' , ' ', ' ', ' ', ' ', 175, 175}//space
};

// アスキーコード to スキャンコード変換テーブル
unsigned char ScanMap[121][3] = {
// 文字コード,2バイト|シフト,スキャンコード
  { 8, 0, 0x66 } , // Backspace
  { 9, 0, 0x0D } , // TAB
  { 13, 0, 0x5A } , // Enter
  { 27, 0, 0x76 } , // Esc
  { 32, 0, 0x29 } , // Space
  { 33, 1, 0x16 } , // !
  { 34, 1, 0x1E } , // "
  { 35, 1, 0x26 } , // #
  { 36, 1, 0x25 } , // $
  { 37, 1, 0x2E } , // %
  { 38, 1, 0x36 } , // &
  { 39, 1, 0x3D } , // '
  { 40, 1, 0x3E } , // (
  { 41, 1, 0x46 } , // )
  { 42, 1, 0x52 } , // *
  { 43, 1, 0x4C } , // +
  { 44, 0, 0x41 } , // ,
  { 45, 0, 0x4E } , // -
  { 46, 0, 0x49 } , // .
  { 47, 0, 0x4A } , // /
  { 48, 0, 0x45 } , // 0
  { 49, 0, 0x16 } , // 1
  { 50, 0, 0x1E } , // 2
  { 51, 0, 0x26 } , // 3
  { 52, 0, 0x25 } , // 4
  { 53, 0, 0x2E } , // 5
  { 54, 0, 0x36 } , // 6
  { 55, 0, 0x3D } , // 7
  { 56, 0, 0x3E } , // 8
  { 57, 0, 0x46 } , // 9
  { 58, 0, 0x52 } , // :
  { 59, 0, 0x4C } , // ;
  { 60, 1, 0x41 } , // <
  { 61, 1, 0x4E } , // =
  { 62, 1, 0x49 } , // >
  { 63, 1, 0x4A } , // ?
  { 64, 0, 0x54 } , // @
  { 65, 1, 0x1C } , // A
  { 66, 1, 0x32 } , // B
  { 67, 1, 0x21 } , // C
  { 68, 1, 0x23 } , // D
  { 69, 1, 0x24 } , // E
  { 70, 1, 0x2B } , // F
  { 71, 1, 0x34 } , // G
  { 72, 1, 0x33 } , // H
  { 73, 1, 0x43 } , // I
  { 74, 1, 0x3B } , // J
  { 75, 1, 0x42 } , // K
  { 76, 1, 0x4B } , // L
  { 77, 1, 0x3A } , // M
  { 78, 1, 0x31 } , // N
  { 79, 1, 0x44 } , // O
  { 80, 1, 0x4D } , // P
  { 81, 1, 0x15 } , // Q
  { 82, 1, 0x2D } , // R
  { 83, 1, 0x1B } , // S
  { 84, 1, 0x2C } , // T
  { 85, 1, 0x3C } , // U
  { 86, 1, 0x2A } , // V
  { 87, 1, 0x1D } , // W
  { 88, 1, 0x22 } , // X
  { 89, 1, 0x35 } , // Y
  { 90, 1, 0x1A } , // Z
  { 91, 0, 0x5B } , // [
  { 92, 0, 0x6A } , // \
  { 92, 0, 0x51 } , // ＼
  { 93, 0, 0x5D } , // ]
  { 94, 0, 0x55 } , // ^
  { 95, 1, 0x51 } , // _
  { 96, 1, 0x54 } , // `
  { 97, 0, 0x1C } , // a
  { 98, 0, 0x32 } , // b
  { 99, 0, 0x21 } , // c
  { 100, 0, 0x23 } , // d
  { 101, 0, 0x24 } , // e
  { 102, 0, 0x2B } , // f
  { 103, 0, 0x34 } , // g
  { 104, 0, 0x33 } , // h
  { 105, 0, 0x43 } , // i
  { 106, 0, 0x3B } , // j
  { 107, 0, 0x42 } , // k
  { 108, 0, 0x4B } , // l
  { 109, 0, 0x3A } , // m
  { 110, 0, 0x31 } , // n
  { 111, 0, 0x44 } , // o
  { 112, 0, 0x4D } , // p
  { 113, 0, 0x15 } , // q
  { 114, 0, 0x2D } , // r
  { 115, 0, 0x1B } , // s
  { 116, 0, 0x2C } , // t
  { 117, 0, 0x3C } , // u
  { 118, 0, 0x2A } , // v
  { 119, 0, 0x1D } , // w
  { 120, 0, 0x22 } , // x
  { 121, 0, 0x35 } , // y
  { 122, 0, 0x1A } , // z
  { 123, 1, 0x5B } , // {
  { 124, 1, 0x6A } , // |
  { 125, 1, 0x5D } , // }
  { 126, 1, 0x55 } , // ~
  { 127, 2, 0x71 } , // Delete
  { 129, 0, 0x05 } , // F1
  { 130, 0, 0x06 } , // F2
  { 131, 0, 0x04 } , // F3
  { 132, 0, 0x0C } , // F4
  { 133, 0, 0x03 } , // F5
  { 134, 0, 0x0B } , // F6
  { 135, 0, 0x83 } , // F7
  { 136, 0, 0x0A } , // F8
  { 137, 0, 0x01 } , // F9
  { 138, 0, 0x09 } , // F10
  { 139, 2, 0x70 } , // Insert
  { 152, 2, 0x6C } , // Home
  { 153, 2, 0x7D } , // PgUp
  { 161, 0, 0x13 } , // Kana
  { 164, 2, 0x7A } , // PgDn
  { 165, 2, 0x69 } , // End
  { 180, 2, 0x6B } , // ←
  { 181, 2, 0x75 } , // ↑
  { 182, 2, 0x72 } , // ↓
  { 183, 2, 0x74 } , // →
};

uint8_t enabled =0;               // PS/2 ホスト送信可能状態

// スキャンコード変換テーブルの検索
int16_t findScanMap(uint8_t code) {
  int16_t no = -1;
  for (int16_t i=0; i < 121; i++) {
      if (ScanMap[i][0] == code) {
        no = i;
        break;
      }
  }
  return no;
}

// PS/2 ホストにack送信
void ack() {
  while(keyboard.write(0xFA));
}

// PS/2 ホストから送信されるコマンドの処理
int keyboardcommand(int command) {
  unsigned char val;
  uint32_t tm;
  switch (command) {
  case 0xFF:  ack();// Reset: キーボードリセットコマンド。正しく受け取った場合ACKを返す。
    //keyboard.write(0xAA);
    break;
  case 0xFE: // 再送要求
    ack();
    break;
  case 0xF6: // 起動時の状態へ戻す
    //enter stream mode
    ack();
    break;
  case 0xF5: //起動時の状態へ戻し、キースキャンを停止する
    //FM
    enabled = 0;
    ack();
    break;
  case 0xF4: //キースキャンを開始する
    //FM
    enabled = 1;
    ack();
    break;
  case 0xF3: //set typematic rate/delay : 
    ack();
    keyboard.read(&val); //do nothing with the rate
    ack();
    break;
  case 0xF2: //get device id : 
    ack();
    keyboard.write(0xAB);
    keyboard.write(0x83);
    break;
  case 0xF0: //set scan code set
    ack();
    keyboard.read(&val); //do nothing with the rate
    ack();
    break;
  case 0xEE: //echo :キーボードが接続されている場合、キーボードはパソコンへ応答（ECHO Responce）を返す。
    //ack();
    keyboard.write(0xEE);
    break;
  case 0xED: //set/reset LEDs :キーボードのLEDの点灯/消灯要求。これに続くオプションバイトでLEDを指定する。 
    ack();
    keyboard.read(&val); //do nothing with the rate
    ack();
    break;
  }
}

// スキャンコードの送信
// 引数
//  no : スキャンコード・テーブル レコード番号
void sendScanCode(int16_t no) {

  // Makeコードの送信
  if (ScanMap[no][1] & 1) {
    // シフトキー併用の場合は、右シフトキーのスキャンコードを先に送信する
    keyboard.write(0x59);
  }
  //delay(10);

  if (ScanMap[no][1] & 2) {
    // 2バイトスキャンコード
    keyboard.write(0xe0);
  }
  keyboard.write(ScanMap[no][2]);
  //delay(10);
  
  // Breakコードの送信
  if (ScanMap[no][1] & 2) {
    // 2バイトスキャンコード
    keyboard.write(0xe0);
  }
  keyboard.write(0xf0);
  keyboard.write(ScanMap[no][2]);
  //delay(10);

  if (ScanMap[no][1] & 1) {
    // シフトキー併用の場合は、右シフトキーのスキャンコードを最後に送信する
    keyboard.write(0xf0);
    keyboard.write(0x59);
  } 
  //delay(10);
}

#define shiftPressed (PINB & 0x10 ) != 0x10
#define symPressed (PINB & 0x80 ) != 0x80
#define fnPressed (PINB & 0x40 ) != 0x40

int _shift = 0, _fn = 0, _sym = 0, idle = 0;
unsigned char KEY = 0, hadPressed = 0;
int Mode = 0; //0->normal.1->shift 2->long_shift, 3->sym, 4->long_shift 5->fn,6->long_fn

void flashOn() {
  pixels.setPixelColor(0, pixels.Color(3, 3, 3)); pixels.show();
}

void flashOff() {
  pixels.setPixelColor(0, pixels.Color(0, 0, 0)); pixels.show();
}

// I2Cによるキー情報送信
void requestEvent() {
  if (hadPressed == 1)   {
    //Wire.write(KeyMap[KEY - 1][Mode]);
    //KEY=0;
    if ((Mode == 1) || (Mode == 3) || (Mode == 5)) {
      Mode = 0;
      _shift = 0;
      _sym = 0;
      _fn = 0;
    }

    hadPressed = 0;
    return;
  }
}

//SoftwareSerial mySerial(12, 11); // RX, TX
void setup() {
  //Serial.begin(115200);
  pinMode(A3, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A0, OUTPUT);
  digitalWrite(A0, HIGH);
  digitalWrite(A1, LOW);
  digitalWrite(A2, LOW);
  digitalWrite(A3, LOW);
  DDRB = 0x00;
  PORTB = 0xff;
  DDRD = 0x00;
  PORTD = 0xff;

  pixels.begin();
  for (int j = 0; j < 3; j++)   {
    for (int i = 0; i < 30; i++)     {
      pixels.setPixelColor(0, pixels.Color(i, i, i)); pixels.show();
      delay(6);
    }
    for (int i = 30; i > 0; i--)     {
      pixels.setPixelColor(0, pixels.Color(i, i, i)); pixels.show();
      delay(6);
    }

  }
  pixels.setPixelColor(0, pixels.Color(0, 0, 0));
  //Wire.begin(0x5f);
  //Wire.onRequest(requestEvent);
/*
  while(keyboard.write(0xAA)!=0);  
  while(keyboard.write(0x00)!=0);  
*/
  uint32_t tm;
  tm = millis();  
  while(keyboard.write(0xAA)!=0)  {
    if ( millis() > tm+1000)
        break;
  }

  //Serial.println("Start");

}

unsigned char GetInput() {
  digitalWrite(A3, LOW);
  digitalWrite(A2, HIGH);
  digitalWrite(A1, HIGH);
  digitalWrite(A0, HIGH);
  delay(2);
  switch (PIND)   {
    case 254: while (PIND != 0xff) {
        flashOn();
        //delay(1);
      } flashOff();   hadPressed = 1; return  1; break;
    case 253: while (PIND != 0xff) {
        flashOn();
        // delay(1);
      } flashOff();  hadPressed = 1; return 2; break;
    case 251: while (PIND != 0xff) {
        flashOn();
        // delay(1);
      } flashOff();  hadPressed = 1;  return 3; break;
    case 247: while (PIND != 0xff) {
        flashOn();
        // delay(1);
      } flashOff();  hadPressed = 1;  return 4; break;
    case 239: while (PIND != 0xff) {
        flashOn();
        //delay(1);
      } flashOff();  hadPressed = 1; return 5; break;
    case 223: while (PIND != 0xff) {
        flashOn();
        //   delay(1);
      } flashOff();  hadPressed = 1; return 6; break;
    case 191: while (PIND != 0xff) {
        flashOn();
        // delay(1);
      } flashOff();  hadPressed = 1; return 7; break;
    case 127: while (PIND != 0xff) {
        flashOn();
        // delay(1);
      } flashOff();  hadPressed = 1; return  8; break;
  }
  
  switch (PINB)   {
    case 222: while (PINB != 223) {
        flashOn();
        //delay(1);
      } flashOff();  hadPressed = 1; return 9; break;
    case 221: while (PINB != 223) {
        flashOn();
        //delay(1);
      } flashOff();  hadPressed = 1; return 10; break;
    case 219: while (PINB != 223) {
        flashOn();
        // delay(1);
      } flashOff();  hadPressed = 1; return 11; break;
    case 215: while (PINB != 223) {
        flashOn();
        // delay(1);
      } flashOff();  hadPressed = 1; return 12; break;
  }

  digitalWrite(A3, HIGH);
  digitalWrite(A2,  LOW);
  digitalWrite(A1, HIGH);
  digitalWrite(A0, HIGH);
  delay(2);
  switch (PIND)   {
    case 254: while (PIND != 0xff) {
        flashOn();
        //  delay(1);
      } flashOff();  hadPressed = 1; return  13; break;
    case 253: while (PIND != 0xff) {
        flashOn();
        //delay(1);
      } flashOff();  hadPressed = 1; return 14; break;
    case 251: while (PIND != 0xff) {
        flashOn();
        // delay(1);
      } flashOff();  hadPressed = 1;  return 15; break;
    case 247: while (PIND != 0xff) {
        flashOn();
        delay(1);
      } flashOff();  hadPressed = 1;  return 16; break;
    case 239: while (PIND != 0xff) {
        flashOn();
        // delay(1);
      } flashOff();  hadPressed = 1; return 17; break;
    case 223: while (PIND != 0xff) {
        flashOn();
        // delay(1);
      } flashOff();  hadPressed = 1; return 18; break;
    case 191: while (PIND != 0xff) {
        flashOn();
        //delay(1);
      } flashOff();  hadPressed = 1; return 19; break;
    case 127: while (PIND != 0xff) {
        flashOn();
        //  delay(1);
      } flashOff();  hadPressed = 1; return  20; break;
  }
  
  switch (PINB)   {
    case 222: while (PINB != 223) {
        flashOn();
        //delay(1);
      } flashOff();  hadPressed = 1; return 21; break;
    case 221: while (PINB != 223) {
        flashOn();
        // delay(1);
      } flashOff();  hadPressed = 1; return 22; break;
    case 219: while (PINB != 223) {
        flashOn();
        //delay(1);
      } flashOff();  hadPressed = 1; return 23; break;
    case 215: while (PINB != 223) {
        flashOn();
        // delay(1);
      } flashOff();  hadPressed = 1; return 24; break;
  }

  digitalWrite(A3, HIGH);
  digitalWrite(A2, HIGH);
  digitalWrite(A1, LOW);
  digitalWrite(A0, HIGH);
  delay(2);
  switch (PIND)   {
    case 254: while (PIND != 0xff) {
        flashOn();
        //delay(1);
      } flashOff();  hadPressed = 1; return  25; break;
    case 253: while (PIND != 0xff) {
        flashOn();
        // delay(1);
      } flashOff();  hadPressed = 1; return 26; break;
    case 251: while (PIND != 0xff) {
        flashOn();
        //delay(1);
      } flashOff();  hadPressed = 1;  return 27; break;
    case 247: while (PIND != 0xff) {
        flashOn();
        // delay(1);
      } flashOff();  hadPressed = 1;  return 28; break;
    case 239: while (PIND != 0xff) {
        flashOn();
        //  delay(1);
      } flashOff();  hadPressed = 1; return 29; break;
    case 223: while (PIND != 0xff) {
        flashOn();
        // delay(1);
      } flashOff();  hadPressed = 1; return 30; break;
    case 191: while (PIND != 0xff) {
        flashOn();
        //  delay(1);
      } flashOff();  hadPressed = 1; return 31; break;
    case 127: while (PIND != 0xff) {
        flashOn();
        //  delay(1);
      } flashOff();  hadPressed = 1; return  32; break;
  }
  
  switch (PINB)   {
    case 222: while (PINB != 223) {
        flashOn();
        //  delay(1);
      } flashOff();  hadPressed = 1; return 33; break;
    case 221: while (PINB != 223) {
        flashOn();
        //delay(1);
      } flashOff();  hadPressed = 1; return 34; break;
    case 219: while (PINB != 223) {
        flashOn();
        //delay(1);
      } flashOff();  hadPressed = 1; return 35; break;
    case 215: while (PINB != 223) {
        flashOn();
        // delay(1);
      } flashOff();  hadPressed = 1; return 36; break;
  }

  digitalWrite(A3, HIGH);
  digitalWrite(A2, HIGH);
  digitalWrite(A1, HIGH);
  digitalWrite(A0, LOW);
  delay(2);
  
  switch (PIND)   {
    case 254: while (PIND != 0xff) {
        flashOn();
        //delay(1);
      } flashOff();  hadPressed = 1; return  37; break;
    case 253: while (PIND != 0xff) {
        flashOn();
        //delay(1);
      } flashOff();  hadPressed = 1; return 38; break;
    case 251: while (PIND != 0xff) {
        flashOn();
        //delay(1);
      } flashOff();  hadPressed = 1;  return 39; break;
    case 247: while (PIND != 0xff) {
        flashOn();
        // delay(1);
      } flashOff();  hadPressed = 1;  return 40; break;
    case 239: while (PIND != 0xff) {
        flashOn();
        // delay(1);
      } flashOff();  hadPressed = 1; return 41; break;
    case 223: while (PIND != 0xff) {
        flashOn();
        // delay(1);
      } flashOff();  hadPressed = 1; return 42; break;
    case 191: while (PIND != 0xff) {
        flashOn();
        //delay(1);
      } flashOff();  hadPressed = 1; return 43; break;
    case 127: while (PIND != 0xff) {
        flashOn();
        //delay(1);
      } flashOff();  hadPressed = 1; return  44; break;
  }
  
  switch (PINB)   {
    case 222: while (PINB != 223) {
        flashOn();
        //delay(1);
      } flashOff();  hadPressed = 1; return 45; break;
    case 221: while (PINB != 223) {
        flashOn();
        // delay(1);
      } flashOff();  hadPressed = 1; return 46; break;
    case 219: while (PINB != 223) {
        flashOn();
        //delay(1);
      } flashOff();  hadPressed = 1; return 47; break;
    case 215: while (PINB != 223) {
        flashOn();
        //delay(1);
      } flashOff();  hadPressed = 1; return 48; break;
  }
  hadPressed = 0;
  return 255;
}


void loop() {
  unsigned char cmd;  // ホストからの送信データ
  if( (digitalRead(KB_CLK)==LOW) || (digitalRead(KB_DATA) == LOW)) {
    while(keyboard.read(&cmd)) ;
    keyboardcommand(cmd);
  } 

  if (shiftPressed)   {
    _sym = 0; _fn = 0; idle = 0;
    while (shiftPressed)delay(1);
    if (_shift == 0)     {
      delay(200);
      if (shiftPressed)       {
        while (shiftPressed)delay(1);
        _shift = 2;
        Mode = 2;
      } else  {
        _shift = 1;
        Mode = 1;
      }
    } else {
      delay(200);
      if (shiftPressed)       {
        while (shiftPressed)delay(1);
        if (_shift == 2)         {
          Mode = 0;
          _shift = 0;
        } else  {
          Mode = 2;
          _shift = 2;
        }
      } else  {
        Mode = 0;
        _shift = 0;
      }
    }
  }

  if (symPressed)   {
    _shift = 0; _fn = 0; idle = 0;
    while (symPressed)delay(1);
    if (_sym == 0)     {
      delay(200);
      if (symPressed)       {
        while (symPressed)delay(1);
        _sym = 2;
        Mode = 4;
      } else  {
        _sym = 1;
        Mode = 3;
      }
    } else {
      delay(200);
      if (symPressed)  {
        while (symPressed)delay(1);
        if (_sym == 2)  {
          Mode = 0;
          _sym = 0;
        } else  {
          Mode = 4;
          _sym = 2;
        }
      } else {
        Mode = 0;
        _sym = 0;
      }
    }
  }

  if (fnPressed) {
    _sym = 0; _shift = 0; idle = 0;
    while (fnPressed)delay(1);
    if (_fn == 0) {
      delay(200);
      if (fnPressed) {
        while (fnPressed)delay(1);
        _fn = 2;
        Mode = 6;
      } else  {
        _fn = 1;
        Mode = 5;
      }
    } else  {
      delay(200);
      if (fnPressed) {
        while (fnPressed)delay(1);
        if (_fn == 2) {
          Mode = 0;
          _fn = 0;
        } else {
          Mode = 6;
          _fn = 2;
        }
      } else {
        Mode = 0;
        _fn = 0;
      }
    }
  }

  switch (Mode)  {
    case 0://normal
      pixels.setPixelColor(0, pixels.Color(0, 0, 0)); break;
    case 1://shift
      if ((idle / 6) % 2 == 1)
        pixels.setPixelColor(0, pixels.Color(0, 0, 0));
      else
        pixels.setPixelColor(0, pixels.Color(5, 0, 0)); break;
    case 2://long_shift
      pixels.setPixelColor(0, pixels.Color(5, 0, 0)); break;
    case 3://sym
      if ((idle / 6) % 2 == 1)
        pixels.setPixelColor(0, pixels.Color(0, 0, 0));
      else
        pixels.setPixelColor(0, pixels.Color(0, 5, 0)); break;
    case 4://long_sym
      pixels.setPixelColor(0, pixels.Color(0, 5, 0)); break;
    case 5://fn
      if ((idle / 6) % 2 == 1)
        pixels.setPixelColor(0, pixels.Color(0, 0, 0));
      else
        pixels.setPixelColor(0, pixels.Color(0, 0, 5)); break;
    case 6://long_fn
      pixels.setPixelColor(0, pixels.Color(0, 0, 5)); break;
  }

  pixels.show(); // This sends the updated pixel color to the hardware.  

  if (hadPressed == 0)   {
    KEY = GetInput();   
  }

  // PS/2 IFによるスキャンコードの送信
  if (hadPressed) {
    uint8_t c = KeyMap[KEY - 1][Mode];
    int16_t no = findScanMap(c);
    if (no >= 0) {
      sendScanCode(no);
    }
    
    if ((Mode == 1) || (Mode == 3) || (Mode == 5)) {
      Mode = 0;
      _shift = 0;
      _sym = 0;
      _fn = 0;
    }
    hadPressed = 0;
  }
  
  idle++;
  delay(10);
}
