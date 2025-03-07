#include "stm32f10x.h"
#include "OLED_Font.h"

#define OLED_W_SCL(x)       GPIO_WriteBit(GPIOB, GPIO_Pin_8, (BitAction)(x))
#define OLED_W_SDA(x)       GPIO_WriteBit(GPIOB, GPIO_Pin_9, (BitAction)(x))


void OLED_I2C_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
 	GPIO_Init(GPIOB, &GPIO_InitStructure);

	OLED_W_SCL(1);
	OLED_W_SDA(1);
}


void OLED_I2C_Start(void)
{
	OLED_W_SDA(1);		//释放SDA，确保SDA为高电平
	OLED_W_SCL(1);		//释放SCL，确保SCL为高电平
	OLED_W_SDA(0);		//在SCL高电平期间，拉低SDA，产生起始信号
	OLED_W_SCL(0);		//起始后把SCL也拉低，即为了占用总线，也为了方便总线时序的拼接
}


void OLED_I2C_Stop(void)
{
	OLED_W_SDA(0);		//拉低SDA，确保SDA为低电平
	OLED_W_SCL(1);		//释放SCL，使SCL呈现高电平
	OLED_W_SDA(1);		//在SCL高电平期间，释放SDA，产生终止信号
}


void OLED_I2C_SendByte(uint8_t Byte)
{
	uint8_t i;
	
	/*循环8次，主机依次发送数据的每一位*/
	for (i = 0; i < 8; i++)
	{
		OLED_W_SDA(Byte & (0x80 >> i));
		OLED_W_SCL(1);	//释放SCL，从机在SCL高电平期间读取SDA
		OLED_W_SCL(0);	//拉低SCL，主机开始发送下一位数据
	}
	
	OLED_W_SCL(1);		//额外的一个时钟，不处理应答信号
	OLED_W_SCL(0);
}


void OLED_WriteCommand(uint8_t Command)
{
	OLED_I2C_Start();				//I2C起始
	OLED_I2C_SendByte(0x78);		//发送OLED的I2C从机地址
	OLED_I2C_SendByte(0x00);		//控制字节，给0x00，表示即将写命令
	OLED_I2C_SendByte(Command);		//写入指定的命令
	OLED_I2C_Stop();				//I2C终止
}


void OLED_WriteData(uint8_t Data)
{
	OLED_I2C_Start();				//I2C起始
	OLED_I2C_SendByte(0x78);		//发送OLED的I2C从机地址
	OLED_I2C_SendByte(0x40);		//控制字节，给0x40，表示即将写数据
	OLED_I2C_SendByte(Data);
	OLED_I2C_Stop();				//I2C终止
}


void OLED_SetCursor(uint8_t Y, uint8_t X)
{
	OLED_WriteCommand(0xB0 | Y);					//设置页位置
	OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));	//设置X位置高4位
	OLED_WriteCommand(0x00 | (X & 0x0F));			//设置X位置低4位
}


void OLED_Clear(void)
{
	uint8_t i, j;
	for (j = 0; j < 8; j ++)				//遍历8页
	{
		OLED_SetCursor(j,0);
		for (i = 0; i < 128; i++)			//遍历128列
		{
			OLED_WriteData(0x00);	//将显存数组数据全部清零
		}
	}
}


void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char)
{
	uint8_t i;
	OLED_SetCursor((Line - 1) * 2, (Column - 1) * 8);
	for (i = 0; i < 8; i++)		//字体为宽8像素，高16像素
	{
		OLED_WriteData(OLED_F8x16[Char - ' '][i]);
	}
	OLED_SetCursor((Line - 1) * 2 + 1,(Column - 1) * 8);
	for (i = 0; i < 8; i++)	//字体为宽6像素，高8像素
	{
		OLED_WriteData(OLED_F8x16[Char - ' '][i + 8]);
	}
}


void OLED_ShowString(uint8_t Line, uint8_t Column, char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i++)		//遍历字符串的每个字符
	{
		OLED_ShowChar(Line, Column + i, String[i]);
	}
}


uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;	//结果默认为1
	while (Y--)			//累乘Y次
	{
		Result *= X;		//每次把X累乘到结果上
	}
	return Result;
}


void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i++)		//遍历数字的每一位							
	{
		OLED_ShowChar(Line, Column + i, Number / OLED_Pow(10, Length - i - 1) % 10 + '0');
	}
}



void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i;
	uint32_t Number1;
	if (Number >= 0)						//数字大于等于0
	{
		OLED_ShowChar(Line, Column, '+');	//显示+号
		Number1 = Number;					//Number1直接等于Number
	}
	else									//数字小于0
	{
		OLED_ShowChar(Line, Column, '-');	//显示-号
		Number1 = -Number;					//Number1等于Number取负
	}
	
	for (i = 0; i < Length; i++)			//遍历数字的每一位								
	{
		OLED_ShowChar(Line, Column + i + 1, Number1 / OLED_Pow(10, Length - i - 1) % 10 + '0');
	}
}


void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i, SingleNumber;
	for (i = 0; i < Length; i++)		//遍历数字的每一位
	{
		SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;	
		if (SingleNumber < 10)			//单个数字小于10
		{
			OLED_ShowChar(Line, Column + i, SingleNumber + '0');
		}
		else							//单个数字大于10
		{
			OLED_ShowChar(Line, Column + i, SingleNumber - 10 + 'A');
		}
	}
}



void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i++)		//遍历数字的每一位	
	{
		OLED_ShowChar(Line, Column + i, Number / OLED_Pow(2, Length - i - 1) % 2 + '0');
	}
}


void OLED_ShowFNum(uint8_t Line, uint8_t Column, float Number, uint8_t Length, uint8_t FLength)
{
	uint8_t i;
	uint8_t flag = 1;
	float Number1;
	uint32_t Number2;
	
	if (Number >= 0)						//数字大于等于0
	{
		OLED_ShowChar(Line, Column, '+');	//显示+号
		Number1 = Number;
	}
	else									//数字小于0
	{
		OLED_ShowChar(Line, Column, '-');	//显示-号
		Number1 = -Number;					//Number取负
	}
	
	Number2 = (int)(Number1*OLED_Pow(10, FLength));
	for(i = Length; i > 0; i--)
	{
		if(i == Length - FLength)
		{
			OLED_ShowChar(Line, Column + i + flag, '.');
			flag = 0;
		}
		OLED_ShowChar(Line, Column + i + flag, Number2/OLED_Pow(10, Length-i)%10+'0');
	}
}


void OLED_ShowCC_F16x16(uint8_t Line, uint8_t Column, uint8_t num)
{
	uint8_t i;
	OLED_SetCursor((Line - 1) * 2, (Column - 1) * 8);
	for (i = 0; i < 16; i++)		//遍历汉字串
	{
		OLED_WriteData(CC_F16x16[num*2][i]);
	}
	OLED_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 8);
	for (i = 0; i < 16; i++)
	{
		OLED_WriteData(CC_F16x16[num*2][i + 16]);
	}
}


void OLED_Init(void)
{
	uint32_t i, j;
	
	for (i = 0; i < 1000; i++)      //上电延时
	{
		for ( j = 0; j < 1000; j++);
	}
	
	OLED_I2C_Init();
	
	/*写入一系列的命令，对OLED进行初始化配置*/
	OLED_WriteCommand(0xAE);	//设置显示开启/关闭，0xAE关闭，0xAF开启
	
	OLED_WriteCommand(0xD5);	//设置显示时钟分频比/振荡器频率
	OLED_WriteCommand(0x80);	//0x00~0xFF
	
	OLED_WriteCommand(0xA8);	//设置多路复用率
	OLED_WriteCommand(0x3F);	//0x0E~0x3F
	
	OLED_WriteCommand(0xD3);	//设置显示偏移
	OLED_WriteCommand(0x00);	//0x00~0x7F
	
	OLED_WriteCommand(0x40);	//设置显示开始行，0x40~0x7F
	
	OLED_WriteCommand(0xA1);	//设置左右方向，0xA1正常，0xA0左右反置
	
	OLED_WriteCommand(0xC8);	//设置上下方向，0xC8正常，0xC0上下反置

	OLED_WriteCommand(0xDA);	//设置COM引脚硬件配置
	OLED_WriteCommand(0x12);
	
	OLED_WriteCommand(0x81);	//设置对比度
	OLED_WriteCommand(0xCF);	//0x00~0xFF

	OLED_WriteCommand(0xD9);	//设置预充电周期
	OLED_WriteCommand(0xF1);

	OLED_WriteCommand(0xDB);	//设置VCOMH取消选择级别
	OLED_WriteCommand(0x30);

	OLED_WriteCommand(0xA4);	//设置整个显示打开/关闭

	OLED_WriteCommand(0xA6);	//设置正常/反色显示，0xA6正常，0xA7反色

	OLED_WriteCommand(0x8D);	//设置充电泵
	OLED_WriteCommand(0x14);

	OLED_WriteCommand(0xAF);	//开启显示
	
	OLED_Clear();				//清空
}


