#include "stm32f10x.h"                  // Device header

int main(void)
{
	/* 1. 开启 GPIOC 的时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	/* 2. 配置 PC13 为推挽输出 */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* 3. 主循环：让 PC13 上的 LED 闪烁 */
	while (1)
	{
		GPIO_ResetBits(GPIOC, GPIO_Pin_13);		// PC13 输出低电平，LED 亮
		for (volatile uint32_t i = 0; i < 3000000U; i++);	// 粗略延时
		GPIO_SetBits(GPIOC, GPIO_Pin_13);		// PC13 输出高电平，LED 灭
		for (volatile uint32_t i = 0; i < 3000000U; i++);
	}
}
