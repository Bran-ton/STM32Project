#include "Delay.h"
#include "DHT11.h"


void DHT_Init_InPut(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IN_FLOATING;   //浮空输入
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_11;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
}


void DHT_Init_OutPut(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode= GPIO_Mode_IN_FLOATING;   //推挽输出
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_11;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
}

void DHT_Start(void)
{
	DHT_Init_OutPut();
	GPIO_ResetBits(GPIOB,GPIO_Pin_11);  //拉低总线
	Delay_nus(19000);
	GPIO_SetBits(GPIOB, GPIO_Pin_11);	//拉高总线
	Delay_nus(20);
	DHT_Init_InPut();
}

uint16_t DHT_Scan(void)
{
	return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11);
}

uint16_t DHT_ReadBit(void)
{
	while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == RESET);
	Delay_nus(40);
	if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == SET)
	{
		while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == SET);
		return 1;
	}
	else
	{
		return 0;
	}
}

uint16_t DHT_ReadByte(void)
{
	uint16_t i, data = 0;
	for(i = 0; i < 8; i++)
	{
		data <<= 1; //向左进一位
		data |= DHT_ReadBit();
	}
}

uint16_t DHT_ReadData(uint8_t buffer[5])
{
	uint16_t i = 0;
	
	DHT_Start();
	if(DHT_Scan() == RESET)
	{
		while(DHT_Scan() == RESET);
		while(DHT_Scan() == SET);
		for(i = 0; i < 5; i++)
		{
			buffer[i] = DHT_ReadByte();
		}
		//DHT11输出的40位数据
		while(DHT_Scan() == RESET);
		DHT_Init_OutPut();
		GPIO_SetBits(GPIOB, GPIO_Pin_11);
		
		uint8_t check = buffer[0] + buffer[1] + buffer[2] + buffer[3];
		if(check != buffer[4])
		{
			return 1; //数据错误
		}
	}
	return 0;
	
}
