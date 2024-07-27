#include "stm32l4xx_hal.h"
#include <stdio.h>
#include <stdlib.h>

// Function prototypes
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_LPUART1_UART_Init(void);
static void MX_TIM4_Init(uint32_t pwm_frequency);

// UART handle declaration
UART_HandleTypeDef hlpuart1;
// Timer handle declaration
TIM_HandleTypeDef htim4;

// Variables for storing user input and calculated ARR value
char uart_buf[100];
int uart_buf_len;
char input[10];
uint32_t pwm_frequency;
uint32_t arr_value;

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_LPUART1_UART_Init();

    while (1)
    {
        // Prompt for PWM frequency
        uart_buf_len = sprintf(uart_buf, "Enter PWM Frequency (Hz):\r\n");
        HAL_UART_Transmit(&hlpuart1, (uint8_t*)uart_buf, uart_buf_len, HAL_MAX_DELAY);

        // Receive PWM frequency from UART
        HAL_UART_Receive(&hlpuart1, (uint8_t*)input, sizeof(input), HAL_MAX_DELAY);
        pwm_frequency = atoi(input);

        // Calculate ARR value
        arr_value = (uint32_t)(((80000000)/ (pwm_frequency * (7999 + 1))) - 1);

        // Display calculated ARR value
        uart_buf_len = sprintf(uart_buf, "Value of ARR/Period is: %lu\r\n", arr_value);
        HAL_UART_Transmit(&hlpuart1, (uint8_t*)uart_buf, uart_buf_len, HAL_MAX_DELAY);

        // Initialize Timer 4 with the calculated ARR value
        MX_TIM4_Init(pwm_frequency);

        // Start PWM
        HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);

        // Toggle LED to visualize PWM effect
        for (int i = 0; i < 10; i++)
        {
            HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);
            HAL_Delay(500); // Adjust delay for visible blinking
        }
    }
}

// System Clock Configuration
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = 1;
    RCC_OscInitStruct.PLL.PLLN = 20;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
    {
        Error_Handler();
    }
}

// GPIO Initialization Function
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();

    // Configure GPIO pin PB7 (LED)
    GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

// LPUART1 Initialization Function
static void MX_LPUART1_UART_Init(void)
{
    hlpuart1.Instance = LPUART1;
    hlpuart1.Init.BaudRate = 115200;
    hlpuart1.Init.WordLength = UART_WORDLENGTH_8B;
    hlpuart1.Init.StopBits = UART_STOPBITS_1;
    hlpuart1.Init.Parity = UART_PARITY_NONE;
    hlpuart1.Init.Mode = UART_MODE_TX_RX;
    hlpuart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    hlpuart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    hlpuart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&hlpuart1) != HAL_OK)
    {
        Error_Handler();
    }
}

// Timer 4 Initialization Function
static void MX_TIM4_Init(uint32_t pwm_frequency)
{
    TIM_OC_InitTypeDef sConfigOC = {0};
    uint32_t prescaler = 7999;
    uint32_t arr_value = ((80000000) / (pwm_frequency) * (prescaler + 1)) - 1;
    uint32_t pulse_value = arr_value / 2; // 50% duty cycle

    htim4.Instance = TIM4;
    htim4.Init.Prescaler = prescaler;
    htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim4.Init.Period = arr_value;
    htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
    {
        Error_Handler();
    }

    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = pulse_value;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
    {
        Error_Handler();
    }
}

// Error Handler
void Error_Handler(void)
{
    while (1)
    {
        // Stay in loop
    }
}


#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif /* USE_FULL_ASSERT */
