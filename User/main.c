#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "DHT11.h"
 
int main()
{
	OLED_Init();
	
	uint8_t buffer[5];
	float h, t;
	              
	OLED_ShowCC_F16x16(2, 1, 2); // 温
	OLED_ShowCC_F16x16(2, 3, 4); // 度
	OLED_ShowChar(2, 5, ':');
	
	OLED_ShowCC_F16x16(3, 1, 3); // 湿
	OLED_ShowCC_F16x16(3, 3, 4); // 度
	OLED_ShowChar(3, 5, ':');
	
	while(1)
	{
		if(DHT_ReadData(buffer) == 0)
		{
			h = buffer[0] + buffer[1] / 10.0;
			t = buffer[2] + buffer[3] / 10.0;
 
			OLED_ShowFNum(2, 6, t, 3, 1);
			OLED_ShowCC_F16x16(2, 11, 0); // ℃
 
			OLED_ShowFNum(3, 6, h, 3, 1);
			OLED_ShowCC_F16x16(3, 11, 1); // %
		}
		else
		{
			OLED_Clear();
			OLED_ShowString(2, 6, "ERROR");
		}
	}
}



