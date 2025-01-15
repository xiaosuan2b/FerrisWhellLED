#include <Arduino.h>

#include <IRremote.hpp>
#include <FastLED.h>

#include "HNO3Scheculer.h"

/* PIN declaration */

// 74HC595
#define SER 2	// 595 data input   
#define SRCLK 3 // 595 时钟        
#define RCLK 4	// 74HC595 锁存器 


#define LED_STRIP_PIN 5 // 灯带

#define IR_PIN 7 // 红外针脚

#define SWITCH_PIN 8 // 开关针脚

#define R_PIN 9 // RGB 针脚
#define G_PIN 10
#define B_PIN 11


/*  Other constant value declaration  */

#define num595 14 // The number of 595

#define NUM_LEDS 60         // 定义LED数量，根据实际灯带长度修改
#define BRIGHTNESS 40      // 设置亮度（0到255之间）
#define LED_TYPE WS2812B    // 定义LED类型
#define COLOR_ORDER GRB     // WS2812B的颜色顺序为GRB
#define update_duration 500

byte sheet[num595];

CRGB led_strip[NUM_LEDS];        // 灯带数组


/*  Statue list */
/**
 * total_statue decides other's statue
 * 	depending on statue mechine
 * 	which updates statues every update_duration.
 * 
 */

byte statue_total = 1;
	// 0: off
	// 1:


byte statue_led = 1;
	// 0: off
	// 1: effect1
	// 2: effect2

// byte statue_rgb = 1;
byte statue_rgb = 1;
	// 0: off
	// 1: rainbow
  
byte statue_led_strip = 1;
	// 0: off
	// 1: rainbow


byte lastLEDeffect = 1; // 上一个调用的led effect


// effect functions declaration
void led_effect0();
void led_effect1(); 
void led_effect2();
void led_effect3();
void led_effect4();
void led_effect5();
void led_effect6();
void led_effect7();
void led_effect8();


void (*led_effects[])() = {led_effect0, led_effect1, led_effect2,
							 led_effect3, led_effect4, led_effect5,
							 led_effect6, led_effect7, led_effect8};


// It is considered that 
//	the number of led states 
// 	is the total number of states
byte last_statue = sizeof(led_effects) / sizeof(led_effects[0]) - 1 ;

int hue = 0;


/* LED position list*/
// 外圈
byte circle[][24] = {
	{0, 1, 2, 3, 4, 5, 6, 7,
	 8, 9, 10, 11, 12, 13, 14, 15,
	 16, 17, 18, 19, 20, 21, 22, 23, 
	},
	{
		56, 57, 58, 59, 60, 61, 62, 63,
		64, 65, 66, 67, 68, 69, 70, 71,
		72, 73, 74, 75, 76, 77, 78, 79
	}
};

byte circle_num = 2;


// 辐条
byte spokes[][4] = { 
	{24, 25, 26, 27}, {28, 29, 30, 31},
	{32, 33, 34, 35}, {36, 37, 38, 39},
	{40, 41, 42, 43}, {44, 45, 46, 47},
	{48, 49, 50, 51}, {52, 53, 54, 55},

	{80, 81, 82, 83}, {84, 85, 86, 87},
	{88, 89, 90, 91}, {92, 93, 94, 95},
	{96, 97, 98, 99}, {100, 101, 102, 103},
	{104, 105, 106, 107}, {108, 109, 110, 111},
};

byte spokes_num = sizeof(spokes) / sizeof(spokes[0]);

byte tails[] = {
		23, 0, 2, 3, 5, 6, 8, 9, 11, 12, 14, 15, 17, 18, 20, 21, 
		79, 56, 58, 59, 61, 62, 64, 65, 67, 68, 70, 71, 73, 74, 76, 77
};


byte current_spread_pos = 0;



/* Scheculer timer value */
unsigned long currentMillis = 0;

void set_sheet_bit(byte pos, bool value);

void rgbThread_2();

void statue_machine();
void irThread();
void rgbThread();
void ledThread();
void ledStripThread();
void switchThread();

void spoke_effect_2();
void led_spread_out();
void led_rotate();
void led_spokes_pulse();
void led_rotate_switch1();
void led_rotate_switch2();
void runner5_1();
void runner5_2();
void runner6_1();
void runner6_2();

HNO3Scheculer StatueMachineSch(statue_machine, update_duration);
HNO3Scheculer IRSch(irThread, 50);
HNO3Scheculer RGBSch(rgbThread, 100);
HNO3Scheculer LEDSch(ledThread, 500);
HNO3Scheculer LEDStripSch(ledStripThread, 100);
HNO3Scheculer SwitchSch(switchThread, 100);

HNO3Scheculer LEDSpreadSch(led_spread_out, 100);
HNO3Scheculer LEDRotateSch(led_rotate, 50);
HNO3Scheculer LEDSpokesPulseSch(led_spokes_pulse, 500);
HNO3Scheculer ledRotateSwitch1Sch(led_rotate_switch1, 40);
HNO3Scheculer ledRotateSwitch2Sch(led_rotate_switch2, 100);
HNO3Scheculer runner5_1Sch(runner5_1, 80);
HNO3Scheculer runner5_2Sch(runner5_2, 60);
HNO3Scheculer runner6_1Sch(runner6_1, 60);
HNO3Scheculer runner6_2Sch(runner6_2, 60);
HNO3Scheculer spokeEffect2Sch(spoke_effect_2, 300);



void sheet_clear()
{
	memset(sheet, B11111111, num595);
}

void led_circle_clear()
{
	for (byte i = 0; i < 2; i++)
	{
		for (byte j = 0; j < 24; j++)
		{
			set_sheet_bit(circle[i][j], 1);
		}
		
	}
	
}

void led_spokes_clear()
{
	current_spread_pos = 0;
	for (byte i = 0; i < spokes_num; i++)
	{
		for (byte j = 0; j < 4; j++)
		{
			set_sheet_bit(spokes[i][j], 1);
		}
		
	}
	
}

void set_sheet_bit(byte pos, bool value)
{
	// pos counts from 0
	byte byteIndex = pos / 8;          // 找到目标字节
    byte bitIndex = 7 - (pos % 8);     // 确定位于目标字节中的位位置（从高位到低位）

    if (value) {
        sheet[byteIndex] |= (1 << bitIndex);   // 设置该位为 1
    } else {
        sheet[byteIndex] &= ~(1 << bitIndex);  // 清除该位为 0
    }

}

void invert_sheet_bit(byte pos)
{
	/**
	 * 反转字节的某位
	 * @param pos
	 */

	// pos counts from 0
	byte byteIndex = pos / 8;          // 找到目标字节
    byte bitIndex = 7 - (pos % 8);     // 确定位于目标字节中的位位置（从高位到低位）

    
    sheet[byteIndex] ^= (1 << bitIndex);   // 反转该位
   

}



void Register_update_with_lines(byte lines[num595])
{
	/** 更新寄存器
	 * @param byte lines[num595]
	 * @author 刘向奥 <a3109809244@163.com>
	 */

	digitalWrite(RCLK, LOW); // 拉下锁存器

	for (int i = num595-1; i >= 0; i--)
	{
		shiftOut(SER, SRCLK, LSBFIRST, lines[i]);

	}
 
	digitalWrite(RCLK, HIGH); // 推上锁存器
}

void Register_clear()
{
	for(int i = 0; i < 2; i++){
		byte lines[num595];
		memset(lines, B11111111, num595);
		Register_update_with_lines(lines);
	}

	// for (int i = 0; i < num595; i++)
	// {
	// 	byte l[] = {B11111111};
	// 	Register_update_with_lines(l);
	// 	delay(10);
	// }
	


	
}

void Register_update_no_clear()
{
	// Update the register without clearing it 
	// 		in order to reach higher speed.

	Register_update_with_lines(sheet);
}

void Register_update()
{
	// Update the register.
	// Use this method to update it with file-scope sheet.


	Register_clear();
	Register_update_no_clear();
}

// HSV到RGB的转换函数
void hsv_to_rgb(int h, int s, int v, byte &r, byte &g, byte &b)
{
	float H = h / 360.0f;
	float S = s / 100.0f;
	float V = v / 100.0f;

	int i = (int)(H * 6.0f);
	float f = H * 6.0f - i;
	float p = V * (1.0f - S);
	float q = V * (1.0f - f * S);
	float t = V * (1.0f - (1.0f - f) * S);

	float r_float, g_float, b_float;
	switch (i % 6)
	{
	case 0:
		r_float = V, g_float = t, b_float = p;
		break;
	case 1:
		r_float = q, g_float = V, b_float = p;
		break;
	case 2:
		r_float = p, g_float = V, b_float = t;
		break;
	case 3:
		r_float = p, g_float = q, b_float = V;
		break;
	case 4:
		r_float = t, g_float = p, b_float = V;
		break;
	case 5:
		r_float = V, g_float = p, b_float = q;
		break;
	}

	r = (byte)(r_float * 255);
	g = (byte)(g_float * 255);
	b = (byte)(b_float * 255);
}

void rgbThread()
{
	/**
	 * Update the rgb only by update the value of HSV.
	 * This method provides smoother visual transitions.
	 */

	switch (statue_rgb)
	{
		case 0: // off
			digitalWrite(R_PIN, HIGH);
			digitalWrite(G_PIN, HIGH);
			digitalWrite(B_PIN, HIGH);
			break;

		case 1: // rainbow
			// 计算并更新RGB颜色

			byte r, g, b;
			hsv_to_rgb(hue, 100, 100, r, g, b); // 获取当前的RGB颜色
			analogWrite(R_PIN, r);
			analogWrite(G_PIN, g);
			analogWrite(B_PIN, b);

			// 增加hue值来使颜色变化
			// hue = (hue + 10) % 360;
			hue = (hue + 5) % 360;
			break;
		
		default:
			statue_rgb = 0;
			break;
	}
	
}

byte r = 255, g = 0, b = 0;

void rgbThread_2()
{
	/**
	 * Update the rgb only by update the value of RGB
	 * 		in order to reach higher speed.
	 */


	byte step = 10;
	 if (r == 255 && g < 255 && b == 0) {      // 红 -> 黄
        g += step;
        if (g > 255) g = 255;
    } else if (r > 0 && g == 255 && b == 0) { // 黄 -> 绿
        r -= step;
        if (r < 0) r = 0;
    } else if (r == 0 && g == 255 && b < 255) { // 绿 -> 青
        b += step;
        if (b > 255) b = 255;
    } else if (r == 0 && g > 0 && b == 255) {   // 青 -> 蓝
        g -= step;
        if (g < 0) g = 0;
    } else if (r < 255 && g == 0 && b == 255) { // 蓝 -> 紫
        r += step;
        if (r > 255) r = 255;
    } else if (r == 255 && g == 0 && b > 0) {   // 紫 -> 红
        b -= step;
        if (b < 0) b = 0;
    }

    // 更新RGB引脚的输出
    analogWrite(R_PIN, r);
    analogWrite(G_PIN, g);
    analogWrite(B_PIN, b);

}

void to_previous_statue()
{
	if (statue_total == 0)
	{
		// new count loop
		statue_total = last_statue;

	}else
	{
		statue_total--;
	}
	
	Serial.println("to_previous_statue() triggered");
	Serial.println("Current statue is :");
	Serial.println(statue_total);

}

void to_next_statue()
{
	if (statue_total == last_statue)
	{
		// new count loop
		statue_total = 0;
	}else
	{

		statue_total++;
		
	}
	
	Serial.println("to_next_statue() triggered");
	Serial.println("Current statue is :");
	Serial.println(statue_total);
}

#define reliable_count 3

short trigger_times = 0;

void switchThread()
{
	static byte high_count = 0;

	
    if (digitalRead(SWITCH_PIN) == HIGH) 
	{
		Serial.println("Switch detected");
		high_count++;
		Serial.println(high_count);
	}
	

	if (high_count >= reliable_count) 
	{
		to_next_statue();
		high_count = 0;

		trigger_times++;

		Serial.println("Cumulative trigger times");
		Serial.println(trigger_times);

		
		Serial.println("Current statue");
		Serial.println(statue_led);
	}
}

void switchThread2()
{
	if (digitalRead(SWITCH_PIN) == HIGH)
	{
		to_next_statue();
		Serial.println("Statue Switched");

	}
	
}


void irThread()
{
	if (IrReceiver.decode())
	{

		// Print it to Serial
		Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);
		IrReceiver.printIRResultShort(&Serial);

		switch (IrReceiver.decodedIRData.decodedRawData)
		{
			case 0xBA45FF00: // Keypad button "power"

				switch (statue_total)
				{
				case 0:
					statue_total = 1;
					break;
				
				default:
					statue_total = 0;
					break;
				}
				
				
				break;

			case 0xBF40FF00: // Keypad button "Previous"
				to_previous_statue();
				break;

			case 0xBC43FF00: // Keypad button "Next"
				to_next_statue();
				break;

			case 0xE916FF00: // Keypad button "0"
				statue_total = 0;
				break;

			case 0xF30CFF00: // Keypad button "1"
				statue_total = 1;
				break;
			
			case 0xE718FF00: // Keypad button "2"
				statue_total = 2;
				break;
			
			case 0xA15EFF00: // Keypad button "3"
				statue_total = 3;
				break;
			
			case 0xF708FF00: // Keypad button "4"
				statue_total = 4;
				break;

			case 0xE31CFF00: // Keypad button "5"
				statue_total = 5;
				break;

			case 0xA55AFF00: // Keypad button "6"
				statue_total = 6;
				break;
			
			case 0xBD42FF00: // Keypad button "7"
				statue_total = 7;
				break;
			
			case 0xAD52FF00: // Keypad button "8"
				statue_total = 8;
				break;


			
			default:
				break;
		}

		IrReceiver.resume(); // Enable receiving of the next value
	}
}



void led_effect0()
{
	// setup
	if (lastLEDeffect != 0)
	{
		LEDSch.set_duration(500);
		sheet_clear();
		lastLEDeffect = 0;
		
	}

	Register_clear();
}


void led_effect7()
{
	/* led always on*/
	// setup
	// reset sheet
	if (lastLEDeffect != 7)
	{
		LEDSch.set_duration(500);
		sheet_clear();
		lastLEDeffect = 7;
		
	}

	// light all

	for (int i = 0; i < num595; i++)
	{
		byte ls[num595];
		memset(ls, B00000000, num595);
		Register_update_with_lines(ls);
	}
	

}

void led_effect1()
{
	/**	led 交替闪烁 
	 * @author 刘向奥 <a3109809244@163.com>
	*/
	
	static byte pos = 0; // 位置

	// setup
	if (lastLEDeffect != 1)
	{
		LEDSch.set_duration(1500);
		sheet_clear();
		lastLEDeffect = 1;
		pos = 0;
	}

	sheet_clear();

	// 外圈
	for (byte i = pos; i < 24; i += 2)
	{
		
		set_sheet_bit(circle[0][i], 0);
		set_sheet_bit(circle[1][i], 0);

	}
 
	// 辐条
	for (byte i = 0; i < spokes_num; i++)
	{
		// for (byte j = pos ? 0 : 1; j < 4; j+=2)
		// {
		// 	set_sheet_bit(spokes[i][j], 0);
		// }
		set_sheet_bit(spokes[i][1], 0);
		set_sheet_bit(spokes[i][2], 0);

		set_sheet_bit(spokes[i][0], pos ? 1 : 0);
		set_sheet_bit(spokes[i][3], pos ? 1 : 0);
	}

	Register_update();

	if (pos == 0)
	{
		pos = 1;
	}else
	{
		pos = 0;
	}
	
	
	

}

void led_effect8()
{
	/* Flow forward and back */
	
	// setup
	// reset sheet
	if (lastLEDeffect != 8)
	{
		LEDSch.set_duration(250);
		sheet_clear();
		lastLEDeffect = 8;
		
	}


	static byte i = 0;
	static bool forward = true;

	sheet_clear();

	if (i == (num595*8-1))
	{
		forward = false;
	}else if (i == 0)
	{
		forward = true;
	}
	
	
	
	set_sheet_bit(i, 0);
	Register_update();

	forward ? i++ : i--;
	
}


byte ef2_current_spoke = 0;
byte ef2_cnt = 0;
void spoke_effect_2()
{


	// static byte cnt = 0;
	// static byte current_circle_led = 0;

	ef2_cnt++;
	led_spokes_clear();

	/**
	 * 辐条光剑旋转
	 */
	
	if (ef2_cnt <= 16)
	{
		// 两根转

		for (byte i = ef2_current_spoke; i < ef2_current_spoke +16; i+=4)
		{

			// 辐条
			for (byte j = 0; j < 4; j++)
			{
				set_sheet_bit(spokes[i][j], 0);
			}
			
		}
		
		ef2_current_spoke++;
		if (ef2_current_spoke >= (spokes_num /4))
		{
			ef2_current_spoke = 0;
		}


	}else if (ef2_cnt <= 32)
	{
		// 四根转
		for (int i = ef2_current_spoke; i < spokes_num; i += 2)
		{
			for (int j = 0; j < 4; j++)
			{
				set_sheet_bit(spokes[i][j], 0);
			}
			
		}

		ef2_current_spoke++;
		if (ef2_current_spoke >= 2)
		{
			ef2_current_spoke = 0;
		}


	}else if (ef2_cnt <= 44)
	{
		//闪两下
		if (ef2_cnt % 2 == 1)
		{
			for (int i = 0; i < spokes_num; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					set_sheet_bit(spokes[i][j], 0);
				}
				
			}
			
		}else
		{
			led_spokes_clear();
		}
		
		
	}else
	{
		ef2_cnt = 0;
	}
	
	
}


void led_effect2()
{
	static byte cnt = 0;
	static byte current_spoke = 0;
	// static byte current_circle_led = 0;

	if (lastLEDeffect != 2)
	{
		LEDSch.set_duration(300);
		sheet_clear();
		lastLEDeffect = 2;

		current_spoke = 0;
		// current_circle_led = 0;
		cnt = 0;
	}

	cnt++;
	sheet_clear();

	/**
	 * 辐条光剑旋转
	 */
	
	if (cnt <= 16)
	{
		// 两根转
		// for (int i = 0; i < 4; i++)
		// {
			
		// 	set_sheet_bit(spokes[current_spoke][i], 0);
		// 	set_sheet_bit(spokes[current_spoke + 4][i], 0);
		// 	set_sheet_bit(spokes[current_spoke + 8][i], 0);
		// 	set_sheet_bit(spokes[current_spoke + 12][i], 0);
			
		// }

		for (byte i = current_spoke; i < current_spoke +16; i+=4)
		{

			// 辐条
			for (byte j = 0; j < 4; j++)
			{
				set_sheet_bit(spokes[i][j], 0);
			}
			
			// 尾巴
			set_sheet_bit(tails[i*2], 0);
			// set_sheet_bit(tails[i*2+1], 0);

		}
		

		// for (byte i = 0; i < 2; i++)
		// {
		// 	set_sheet_bit(tails[current_spoke+i], 0);
		// 	set_sheet_bit(tails[current_spoke + 4 + i ], 0);
		// 	set_sheet_bit(tails[current_spoke+2], 0);
		// 	set_sheet_bit(tails[current_spoke+3], 0);
		// }
		

		
		current_spoke++;
		if (current_spoke >= (spokes_num /4))
		{
			current_spoke = 0;
		}

		// spoke_effect_2();


	}else if (cnt <= 32)
	{
		// 四根转
		for (int i = current_spoke; i < spokes_num; i += 2)
		{
			for (int j = 0; j < 4; j++)
			{
				set_sheet_bit(spokes[i][j], 0);
			}
			
			// 尾巴
			set_sheet_bit(tails[i*2], 0);
			// set_sheet_bit(tails[i*2+1], 0);
		}

		current_spoke++;
		if (current_spoke >= 2)
		{
			current_spoke = 0;
		}


	}else if (cnt <= 44)
	{
		//闪两下
		if (cnt % 2 == 1)
		{
			for (int i = 0; i < spokes_num; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					set_sheet_bit(spokes[i][j], 0);
				}
				set_sheet_bit(tails[i*2+1], 0);
				
			}
			
		}else
		{
			sheet_clear();
		}
		
		
	}else
	{
		cnt = 0;
	}
	
	
	
	
	
	
	
	
	
	
	
	
	// 灯旋转
	// for (int i = current_circle_led; i < circle_num; i+=4)
	// {
	// 	set_sheet_bit(circle[0][i], 0);
	// 	set_sheet_bit(circle[1][i], 0);
		
	// }

	// for (int i = 0; i < circle_num; i++)
	// {
	// 	set_sheet_bit(circle[0][i], 0);
	// }

	// current_circle_led++;

	// if (current_circle_led >= 4)
	// {
	// 	current_circle_led = 0;
	// }


	
	Register_update();

}










void led_spread_out()
{	
	/**
	 * The effect that light the led of spokes spreading out
	 * It's your duty to update the sheet
	 */
	if (current_spread_pos == 4)
	{
		led_spokes_clear();
		LEDSpreadSch.sleep_with_duration(1500);
		return;
	}
	
	

	for (int i = 0; i < spokes_num; i++)
	{
		set_sheet_bit(spokes[i][current_spread_pos],0);
	}
	
	current_spread_pos++;
}

void led_spokes_pulse()
{	
	/**
	 * The effect that light the led of spokes pulse out
	 * It's your duty to update the sheet
	 */
	if (current_spread_pos == 4)
	{
		led_spokes_clear();
		LEDSpokesPulseSch.sleep_with_duration(500);
		return;
	}
	
	
	led_spokes_clear();
	for (int i = 0; i < spokes_num; i++)
	{
		set_sheet_bit(spokes[i][current_spread_pos],0);
	}
	
	current_spread_pos++;
}



bool circle_light_forward = true;
byte current_circle_pos = 0;



void led_rotate()
{
	/**
	 * 点亮或者熄灭外圈 led
	 * @author 刘向奥 <a3109809244@163.com>
	 */


	
	// 给 led 状态表的相应位置赋值
	set_sheet_bit(circle[0][current_circle_pos],
					circle_light_forward ? 0 : 1);
	set_sheet_bit(circle[1][current_circle_pos],
					circle_light_forward ? 0 : 1);
	
	current_circle_pos++;
	
	
	if (current_circle_pos >= 24)
	{
		// 改成点亮模式或者熄灭模式
		current_circle_pos = 0;
		circle_light_forward = !circle_light_forward;
	}

    // 将状态表更新到 595
	// Register_update();
	Register_update_no_clear();
}


void led_effect3()
{
	/** 
	 * spread out and rotate
	*/
	
	

	// effect setup
	if (lastLEDeffect != 3)
	{
		sheet_clear();
		Register_clear();
		lastLEDeffect = 3;

		current_spread_pos = 0;
		current_circle_pos = 0;
		circle_light_forward = true;
		LEDSch.set_duration(0);
		LEDSpreadSch.set_duration(100);
	}



	LEDSpreadSch.run(millis());
	
	LEDRotateSch.run(millis());
	
	



	
}

byte current_circle_pos1 = 0;
byte current_circle_pos2 = 0;


void led_rotate_switch1()
{
	/**
	 * 旋转切换 led 状态
	 * @author 刘向奥 <a3109809244@163.com>
	 */


	
	// 反转对应位
	invert_sheet_bit(circle[0][current_circle_pos1]);
	invert_sheet_bit(circle[1][current_circle_pos1]);
	
	current_circle_pos1++;

	if (current_circle_pos1 >= 24)
	{
		current_circle_pos1 = 0;
	}
	

}

void led_rotate_switch2()
{
	/**
	 * 旋转切换 led 状态
	 * @author 刘向奥 <a3109809244@163.com>
	 */


	
	// 反转对应位
	invert_sheet_bit(circle[0][current_circle_pos2]);
	invert_sheet_bit(circle[1][current_circle_pos2]);
	
	current_circle_pos2++;
	
	if (current_circle_pos2 >= 24)
	{
		current_circle_pos2 = 0;
	}

    // 将状态表更新到 595
	Register_update_no_clear();
}



void led_effect4()
{
	/** 
	 * 外圈加载动效式
	 * 内圈 pulse
	*/
	
	

	// effect setup
	if (lastLEDeffect != 4)
	{
		sheet_clear();
		Register_clear();
		lastLEDeffect = 4;

		current_circle_pos1 = 0;
		current_circle_pos2 = 0;

		current_spread_pos = 0;
		LEDSch.set_duration(0);
		LEDSpreadSch.set_duration(3000);
	}


	LEDSpreadSch.run(millis());

	
	ledRotateSwitch1Sch.run(millis());
	ledRotateSwitch2Sch.run(millis());
	
	



	
}


void led_set_tail(byte current_pos, bool direction, byte tail_len)
{
	/**
	 * rotate with tail
	 * @param bool direction, 1:forward 0:back
	 * 		  byte tail_len don't include current_pos
	 * 
	 * It's your duty to clear the register
	 * 
	 * 
	 * if direction == 1, that your you are rotating forward(anticlockwise)
	 * 	the tail will be generate backforward
	 * else is otherwise
	 */

	if (!direction) // 0 : back
	{
		// current_pos = ((current_pos + tail_len) % 24) + 1;
		current_pos = ((current_pos + tail_len) % 24);
	}
	

	// Code below will generate tail backward of it
	if (current_pos + 1 < tail_len) // 当前位置小于尾长
	{
		for (byte i = 0; i < circle_num; i++) // 循环两圈
		{
			for (byte j = 1; j <= current_pos; j++) //从当前到0
			{
				set_sheet_bit(circle[i][current_pos-j], 0);
			}

			for (byte j = 1; j < tail_len - current_pos - 1; j++) // 剩余
			{
				set_sheet_bit(circle[i][24-j], 0);
			}
				
		}
		
		
	}else // 充裕情况
	{
		for (byte i = 0; i < circle_num; i++) // 循环两圈
		{
			for (byte j = 1; j < tail_len; j++)
			{
				set_sheet_bit(circle[i][current_pos-j], 0);
			}

		}
	}
	
	

}

byte current_eft5_1_pos = 0;
byte current_eft5_2_pos = 23;

void runner5_1()
{

	led_circle_clear();
	// sheet_clear();


	// 给 led 状态表的相应位置赋值
	set_sheet_bit(circle[0][current_eft5_1_pos], 0);
	set_sheet_bit(circle[1][current_eft5_1_pos], 0);

	led_set_tail(current_eft5_1_pos, 1, 4);
	
	current_eft5_1_pos++;
	
	
	if (current_eft5_1_pos >= 24)
	{
		current_eft5_1_pos = 0;
	}

	// Register_update();
	// Register_update_no_clear();

}

void runner5_2()
{

	// 给 led 状态表的相应位置赋值
	set_sheet_bit(circle[0][current_eft5_2_pos], 0);
	set_sheet_bit(circle[1][current_eft5_2_pos], 0);

	led_set_tail(current_eft5_2_pos, 0, 4);
	
	current_eft5_2_pos--;
	
	
	if (current_eft5_2_pos <= 0)
	{
		current_eft5_2_pos = 23;
	}

    // 将状态表更新到 595
	// Register_update();
	Register_update_no_clear();
}

void led_effect5()
{
	/**
	 * 两小片乱转
	 * 
	 * one forward one backforward circle
	 * spokes spread
	 * 
	 * @author 刘向奥 <a3109809244@163.com>
	 */
	
	// effect setup
	if (lastLEDeffect != 5)
	{
		sheet_clear();
		Register_clear();
		lastLEDeffect = 5;

		// current_spread_pos = 0;
		ef2_current_spoke = 0;
		ef2_cnt = 0;
		current_eft5_1_pos = 0;
		current_eft5_2_pos = 23;
		LEDSch.set_duration(0);
	}


	runner5_1Sch.run(millis());
	// LEDSpreadSch.run(millis());
	spokeEffect2Sch.run(millis());
	runner5_2Sch.run(millis());



}


byte current_eft6_1_pos = 12;
byte current_eft6_2_pos = 23;
byte tail_len = 0;
bool tail_statue = 1;
// 1: ++
// 0: --

void runner6_1()
{
	
	led_circle_clear();
	// sheet_clear();


	// 给 led 状态表的相应位置赋值
	set_sheet_bit(circle[0][current_eft6_1_pos], 0);
	set_sheet_bit(circle[1][current_eft6_1_pos], 0);

	led_set_tail(current_eft6_1_pos, 0, tail_len);
	
	current_eft6_1_pos--;
	
	
	if (current_eft6_1_pos <= 0)
	{
		current_eft6_1_pos = 23;
	}

	// Register_update();
	// Register_update_no_clear();

}

void runner6_2()
{

	// led_circle_clear();

	// 给 led 状态表的相应位置赋值
	set_sheet_bit(circle[0][current_eft6_2_pos], 0);
	set_sheet_bit(circle[1][current_eft6_2_pos], 0);

	led_set_tail(current_eft6_2_pos, 0, tail_len);
	
	current_eft6_2_pos--;
	
	
	if (current_eft6_2_pos <= 0)
	{
		current_eft6_2_pos = 23;
	}

    // 将状态表更新到 595
	// Register_update();
	Register_update_no_clear();

	if (tail_statue)
	{
		tail_len++;
	}else
	{
		tail_len--;
	}

	if (tail_len >= 9 || tail_len <= 0)
	{
		tail_statue = !tail_statue;
	}
	
	
	
	
}


void led_effect6()
{
	/**
	 * 半圆加载动画
	 * @author 刘向奥 <a3109809244@163.com>
	 */


	// effect setup
	if (lastLEDeffect != 6)
	{
		sheet_clear();
		Register_clear();
		lastLEDeffect = 6;

		
		current_eft6_1_pos = 12;
		current_eft6_2_pos = 23;
		current_spread_pos = 0;
		tail_len = 0;

		LEDSch.set_duration(0);
	}



	
	runner6_1Sch.run(millis());
	LEDSpreadSch.run(millis());
	runner6_2Sch.run(millis());

}



void ledThread()
{

	led_effects[statue_led]();
	// effect1();

}

void ledStripThread()
{
	switch (statue_led_strip)
	{
		case 0: // off
			FastLED.clear();
			FastLED.show();

			break;

		case 1: // rainbow
			static uint8_t hue = 0;   // 定义初始色相，从0开始
			fill_rainbow(led_strip, NUM_LEDS, hue, 7);  // 使用彩虹效果填充灯带，每个LED色相差为7

			FastLED.show();           // 更新灯带显示
			hue++;                    // 增加色相，使颜色逐渐变化

			break;
		
		default:
			statue_led_strip = 0;
			break;
	}
	
}

void setup()
{
	pinMode(SWITCH_PIN, INPUT);

	pinMode(RCLK, OUTPUT);
	pinMode(SRCLK, OUTPUT);
	pinMode(SER, OUTPUT);

	pinMode(R_PIN, OUTPUT);
	pinMode(G_PIN, OUTPUT);
	pinMode(B_PIN, OUTPUT);

	Serial.begin(9600);
	IrReceiver.begin(IR_PIN, ENABLE_LED_FEEDBACK);

	FastLED.addLeds<LED_TYPE, LED_STRIP_PIN, COLOR_ORDER>(led_strip, NUM_LEDS).setCorrection(TypicalLEDStrip);
  	FastLED.setBrightness(BRIGHTNESS);

}



void statue_machine()
{
	// updates all statues
	// statue_led, statue_rgb
	switch (statue_total)
	{
		case 0:
			statue_led = 0;
			statue_rgb = 0;
			statue_led_strip = 0;

			break;
		case 1:
			statue_led = 1;
			statue_rgb = 1;
			statue_led_strip = 1;
			break;
		
		case 2:
			statue_led = 2;
			statue_rgb = 1;
			statue_led_strip = 1;

			break;

		case 3:
			statue_led = 3;
			statue_rgb = 1;
			statue_led_strip = 1;

			break;
		
		case 4:
			statue_led = 4;
			statue_rgb = 1;
			statue_led_strip = 1;

			break;
		
		case 5:
			statue_led = 5;
			statue_rgb = 1;
			statue_led_strip = 1;

			break;

		case 6:
			statue_led = 6;
			statue_rgb = 1;
			statue_led_strip = 1;

			break;

		case 7:
			statue_led = 7;
			statue_rgb = 1;
			statue_led_strip = 1;

			break;
		
		case 8:
			statue_led = 8;
			statue_rgb = 1;
			statue_led_strip = 1;

			break;
		
		
		default:
			statue_total = 0;
			break;
	}
	
}

void loop()
{
	/**
	 *  @author 刘向奥 <a3109809244@163.com>
	 */

	currentMillis = millis(); // 获取时间戳

	StatueMachineSch.run(currentMillis); // 状态机 Scheduler

	SwitchSch.run(currentMillis); // 开关 Scheduler

	IRSch.run(currentMillis); // 红外 Scheduler

	RGBSch.run(currentMillis); // RGB Scheduler

	LEDSch.run(currentMillis); // LED Scheduler

	LEDStripSch.run(currentMillis); // 灯带 Scheduler

}
