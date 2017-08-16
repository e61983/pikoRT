#include <kernel/compiler.h>

#include "platform.h"

#define CPU_FREQ_IN_HZ 72000000
#define SYSTICK_FREQ_IN_HZ 1000
#define SYSTICK_PERIOD_IN_MSECS (SYSTICK_FREQ_IN_HZ / 1000)

struct timer_operations;

void config_timer_operations(struct timer_operations *tops);

extern struct timer_operations systick_tops;

extern void stm32f1_init(void);

void rcc_clock_init(void);
void __uart_enable(void);

__weak void __platform_init(void)
{
    config_timer_operations(&systick_tops);

    /* SysTick running at 1kHz */
    SysTick_Config(CPU_FREQ_IN_HZ / SYSTICK_FREQ_IN_HZ);

    /* create /dev/ttyS0, serial interface for QEMU UART0 */
    stm32f1_init();
}

__weak void __platform_halt(void)
{
    for (;;)
        ;
}

void __printk_init(void)
{
    /* Enable peripherals and GPIO Clocks */

    /* Enable GPIO TX/RX clock */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /* Enable USART2 clock */
    __HAL_RCC_USART2_CLK_ENABLE();

    /* GPIO init */
    GPIO_InitTypeDef GPIO_InitStruct;

    /* Configure peripheral GPIO*/

    /* UART TX/RX GPIO pin configuration  */
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;

    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);



    /* USART init */
    UART_HandleTypeDef UartHandle;

    /* Configure the UART peripheral */

    /* Put the USART peripheral in the Asynchronous mode (UART Mode) */
    /* UART2 configured as follow:
        - Word Length = 8 Bits
        - Stop Bit = One Stop bit
        - Parity = None
        - BaudRate = 9600 baud
        - Hardware flow control disabled (RTS and CTS signals) */
    UartHandle.Instance = USART2;

    UartHandle.Init.BaudRate = 115200;
    UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
    UartHandle.Init.StopBits = UART_STOPBITS_1;
    UartHandle.Init.Parity = UART_PARITY_NONE;
    UartHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    UartHandle.Init.Mode = UART_MODE_TX_RX | UART_IT_RXNE;
    UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;

    HAL_UART_Init(&UartHandle);

    NVIC_SetPriority(USART2_IRQn, 0xE);
}

void __printk_putchar(char c)
{
    if (c == '\n')
        __printk_putchar('\r');

    while (!(USART2->SR & USART_SR_TXE))
        ;
    USART2->DR = (0xff) & c;
}
