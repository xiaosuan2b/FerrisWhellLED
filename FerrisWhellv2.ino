#include <IRremote.hpp>


#define RCLK 4  // 74HC595 锁存器  大平台
#define SRCLK 3// 595 时钟         活塞
#define SER 2 // 595 data input    资料

#define big RCLK // 大平台
#define push SRCLK //活塞
#define datain SER //资料

#define R_PIN 9
#define G_PIN 10
#define B_PIN 11

#define IR_PIN 7

#define num595 14 // The number of 595

byte sheet[num595];

int hue = 0;
unsigned long lastRGBUpdate = 0;
// unsigned long lastEffectUpdate = 0;


void Register_update_no_clear(byte lines[num595])
{
  /** Update register at once
    * 
    */
  
  
  
  digitalWrite(RCLK, LOW);

  for(int i = 0; i < num595; i++)
  {
    shiftOut(SER, SRCLK, MSBFIRST, sheet[i]);
  }
  
  digitalWrite(RCLK, HIGH);

}

void Register_clear()
{
  byte lines[num595];
  memset(lines, B11111111, num595);

  Register_update_no_clear(lines);

}

void Register_update()
{
  Register_clear();
  Register_update_no_clear(sheet);

}


// HSV到RGB的转换函数
void hsv_to_rgb(int h, int s, int v, byte &r, byte &g, byte &b) {
  float H = h / 360.0f;
  float S = s / 100.0f;
  float V = v / 100.0f;

  int i = (int)(H * 6.0f);
  float f = H * 6.0f - i;
  float p = V * (1.0f - S);
  float q = V * (1.0f - f * S);
  float t = V * (1.0f - (1.0f - f) * S);

  float r_float, g_float, b_float;
  switch(i % 6) {
    case 0: r_float = V, g_float = t, b_float = p; break;
    case 1: r_float = q, g_float = V, b_float = p; break;
    case 2: r_float = p, g_float = V, b_float = t; break;
    case 3: r_float = p, g_float = q, b_float = V; break;
    case 4: r_float = t, g_float = p, b_float = V; break;
    case 5: r_float = V, g_float = p, b_float = q; break;
  }

  r = (byte)(r_float * 255);
  g = (byte)(g_float * 255);
  b = (byte)(b_float * 255);
}


void rgbThread(){

  
    // 计算并更新RGB颜色
    byte r, g, b;
    hsv_to_rgb(hue, 100, 100, r, g, b);  // 获取当前的RGB颜色
    analogWrite(R_PIN, r);
    analogWrite(G_PIN, g);
    analogWrite(B_PIN, b);
    
    // 增加hue值来使颜色变化
    hue = (hue + 10) % 360;


}
void ir_receive()
{
  if (IrReceiver.decode()) 
  {
    Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);
    IrReceiver.printIRResultShort(&Serial);


  switch(IrReceiver.decodedIRData.decodedRawData)
    {
      case 0xBA45FF00: //Keypad button "power"
        

        break;

      case 0xAD52FF00: //Keypad button "8"

        break;
      
      case 0xB54AFF00: //Keypad button "9"
        break;

      default:
        break;
    }

    
    IrReceiver.resume(); // Enable receiving of the next value
  
  }


}



void setup() {

  pinMode(RCLK, OUTPUT);
  pinMode(SRCLK, OUTPUT);
  pinMode(SER, OUTPUT);

  pinMode(R_PIN, OUTPUT);
  pinMode(G_PIN, OUTPUT);
  pinMode(B_PIN, OUTPUT);

  Serial.begin(9600);
  IrReceiver.begin(IR_PIN, ENABLE_LED_FEEDBACK);
  
}



void loop() {

  unsigned long currentMillis = millis();


  if (currentMillis - lastRGBUpdate >= 100) {
    lastRGBUpdate = currentMillis;
  
    rgbThread();

  }
  // delay(500);
}
