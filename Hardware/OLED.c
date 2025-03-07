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
	OLED_W_SDA(1);		//�ͷ�SDA��ȷ��SDAΪ�ߵ�ƽ
	OLED_W_SCL(1);		//�ͷ�SCL��ȷ��SCLΪ�ߵ�ƽ
	OLED_W_SDA(0);		//��SCL�ߵ�ƽ�ڼ䣬����SDA��������ʼ�ź�
	OLED_W_SCL(0);		//��ʼ���SCLҲ���ͣ���Ϊ��ռ�����ߣ�ҲΪ�˷�������ʱ���ƴ��
}


void OLED_I2C_Stop(void)
{
	OLED_W_SDA(0);		//����SDA��ȷ��SDAΪ�͵�ƽ
	OLED_W_SCL(1);		//�ͷ�SCL��ʹSCL���ָߵ�ƽ
	OLED_W_SDA(1);		//��SCL�ߵ�ƽ�ڼ䣬�ͷ�SDA��������ֹ�ź�
}


void OLED_I2C_SendByte(uint8_t Byte)
{
	uint8_t i;
	
	/*ѭ��8�Σ��������η������ݵ�ÿһλ*/
	for (i = 0; i < 8; i++)
	{
		OLED_W_SDA(Byte & (0x80 >> i));
		OLED_W_SCL(1);	//�ͷ�SCL���ӻ���SCL�ߵ�ƽ�ڼ��ȡSDA
		OLED_W_SCL(0);	//����SCL��������ʼ������һλ����
	}
	
	OLED_W_SCL(1);		//�����һ��ʱ�ӣ�������Ӧ���ź�
	OLED_W_SCL(0);
}


void OLED_WriteCommand(uint8_t Command)
{
	OLED_I2C_Start();				//I2C��ʼ
	OLED_I2C_SendByte(0x78);		//����OLED��I2C�ӻ���ַ
	OLED_I2C_SendByte(0x00);		//�����ֽڣ���0x00����ʾ����д����
	OLED_I2C_SendByte(Command);		//д��ָ��������
	OLED_I2C_Stop();				//I2C��ֹ
}


void OLED_WriteData(uint8_t Data)
{
	OLED_I2C_Start();				//I2C��ʼ
	OLED_I2C_SendByte(0x78);		//����OLED��I2C�ӻ���ַ
	OLED_I2C_SendByte(0x40);		//�����ֽڣ���0x40����ʾ����д����
	OLED_I2C_SendByte(Data);
	OLED_I2C_Stop();				//I2C��ֹ
}


void OLED_SetCursor(uint8_t Y, uint8_t X)
{
	OLED_WriteCommand(0xB0 | Y);					//����ҳλ��
	OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));	//����Xλ�ø�4λ
	OLED_WriteCommand(0x00 | (X & 0x0F));			//����Xλ�õ�4λ
}


void OLED_Clear(void)
{
	uint8_t i, j;
	for (j = 0; j < 8; j ++)				//����8ҳ
	{
		OLED_SetCursor(j,0);
		for (i = 0; i < 128; i++)			//����128��
		{
			OLED_WriteData(0x00);	//���Դ���������ȫ������
		}
	}
}


void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char)
{
	uint8_t i;
	OLED_SetCursor((Line - 1) * 2, (Column - 1) * 8);
	for (i = 0; i < 8; i++)		//����Ϊ��8���أ���16����
	{
		OLED_WriteData(OLED_F8x16[Char - ' '][i]);
	}
	OLED_SetCursor((Line - 1) * 2 + 1,(Column - 1) * 8);
	for (i = 0; i < 8; i++)	//����Ϊ��6���أ���8����
	{
		OLED_WriteData(OLED_F8x16[Char - ' '][i + 8]);
	}
}


void OLED_ShowString(uint8_t Line, uint8_t Column, char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i++)		//�����ַ�����ÿ���ַ�
	{
		OLED_ShowChar(Line, Column + i, String[i]);
	}
}


uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;	//���Ĭ��Ϊ1
	while (Y--)			//�۳�Y��
	{
		Result *= X;		//ÿ�ΰ�X�۳˵������
	}
	return Result;
}


void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i++)		//�������ֵ�ÿһλ							
	{
		OLED_ShowChar(Line, Column + i, Number / OLED_Pow(10, Length - i - 1) % 10 + '0');
	}
}



void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i;
	uint32_t Number1;
	if (Number >= 0)						//���ִ��ڵ���0
	{
		OLED_ShowChar(Line, Column, '+');	//��ʾ+��
		Number1 = Number;					//Number1ֱ�ӵ���Number
	}
	else									//����С��0
	{
		OLED_ShowChar(Line, Column, '-');	//��ʾ-��
		Number1 = -Number;					//Number1����Numberȡ��
	}
	
	for (i = 0; i < Length; i++)			//�������ֵ�ÿһλ								
	{
		OLED_ShowChar(Line, Column + i + 1, Number1 / OLED_Pow(10, Length - i - 1) % 10 + '0');
	}
}


void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i, SingleNumber;
	for (i = 0; i < Length; i++)		//�������ֵ�ÿһλ
	{
		SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;	
		if (SingleNumber < 10)			//��������С��10
		{
			OLED_ShowChar(Line, Column + i, SingleNumber + '0');
		}
		else							//�������ִ���10
		{
			OLED_ShowChar(Line, Column + i, SingleNumber - 10 + 'A');
		}
	}
}



void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i++)		//�������ֵ�ÿһλ	
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
	
	if (Number >= 0)						//���ִ��ڵ���0
	{
		OLED_ShowChar(Line, Column, '+');	//��ʾ+��
		Number1 = Number;
	}
	else									//����С��0
	{
		OLED_ShowChar(Line, Column, '-');	//��ʾ-��
		Number1 = -Number;					//Numberȡ��
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
	for (i = 0; i < 16; i++)		//�������ִ�
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
	
	for (i = 0; i < 1000; i++)      //�ϵ���ʱ
	{
		for ( j = 0; j < 1000; j++);
	}
	
	OLED_I2C_Init();
	
	/*д��һϵ�е������OLED���г�ʼ������*/
	OLED_WriteCommand(0xAE);	//������ʾ����/�رգ�0xAE�رգ�0xAF����
	
	OLED_WriteCommand(0xD5);	//������ʾʱ�ӷ�Ƶ��/����Ƶ��
	OLED_WriteCommand(0x80);	//0x00~0xFF
	
	OLED_WriteCommand(0xA8);	//���ö�·������
	OLED_WriteCommand(0x3F);	//0x0E~0x3F
	
	OLED_WriteCommand(0xD3);	//������ʾƫ��
	OLED_WriteCommand(0x00);	//0x00~0x7F
	
	OLED_WriteCommand(0x40);	//������ʾ��ʼ�У�0x40~0x7F
	
	OLED_WriteCommand(0xA1);	//�������ҷ���0xA1������0xA0���ҷ���
	
	OLED_WriteCommand(0xC8);	//�������·���0xC8������0xC0���·���

	OLED_WriteCommand(0xDA);	//����COM����Ӳ������
	OLED_WriteCommand(0x12);
	
	OLED_WriteCommand(0x81);	//���öԱȶ�
	OLED_WriteCommand(0xCF);	//0x00~0xFF

	OLED_WriteCommand(0xD9);	//����Ԥ�������
	OLED_WriteCommand(0xF1);

	OLED_WriteCommand(0xDB);	//����VCOMHȡ��ѡ�񼶱�
	OLED_WriteCommand(0x30);

	OLED_WriteCommand(0xA4);	//����������ʾ��/�ر�

	OLED_WriteCommand(0xA6);	//��������/��ɫ��ʾ��0xA6������0xA7��ɫ

	OLED_WriteCommand(0x8D);	//���ó���
	OLED_WriteCommand(0x14);

	OLED_WriteCommand(0xAF);	//������ʾ
	
	OLED_Clear();				//���
}


