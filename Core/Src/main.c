/* USER CODE BEGIN Header */
/*
 Project name: AquaGuard
*/
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
volatile uint8_t button_pressed_flag = 0; // Flag set by button interrupt; cleared in main loop.

uint8_t system_active = 0; // System active state: 1 = ON (monitoring), 0 = OFF (standby).
uint8_t water_present = 0; // Current water detection result: 1 = water present, 0 = dry.
uint8_t last_water_state = 0; // Previous water detection result, used to send alerts only on change.
uint8_t send_system_status = 1; // Flag to request transmission of system status (SYSTEM ON / SYSTEM OFF).
uint32_t periodic_counter = 0; // Counter for periodic status reports.
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  GPIO_InitTypeDef gpio_init;
  gpio_init.Pin = GPIO_PIN_9;
  gpio_init.Mode = GPIO_MODE_OUTPUT_PP;
  gpio_init.Pull = GPIO_NOPULL;
  gpio_init.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &gpio_init);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_GPIO_WritePin(R_GPIO_Port, R_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(G_GPIO_Port, G_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(B_GPIO_Port, B_Pin, GPIO_PIN_RESET);

  HAL_TIM_Base_Start_IT(&htim2);

  HAL_Delay(2000);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  if (send_system_status) {
		  // Handling the first startup or system status change
		  // UART and ADC. HAL
	      if (system_active) {
	          HAL_UART_Transmit(&huart1, (uint8_t*)"SYSTEM ON\n", 10, 100);
	          uint16_t adc_val;
	          HAL_ADC_Start(&hadc1);
	          if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
	              adc_val = HAL_ADC_GetValue(&hadc1);
	              water_present = (adc_val > 500) ? 1 : 0;
	          }
	          HAL_ADC_Stop(&hadc1);

	          if (water_present) {
	              HAL_UART_Transmit(&huart1, (uint8_t*)"WARNING\n", 8, 100);
	          } else {
	              HAL_UART_Transmit(&huart1, (uint8_t*)"GOOD\n", 5, 100);
	          }
	          last_water_state = water_present;
	          periodic_counter = 0;
	      } else {
	          HAL_UART_Transmit(&huart1, (uint8_t*)"SYSTEM OFF\n", 11, 100);
	          last_water_state = 0;
	      }
	      send_system_status = 0;
	  } else {
	      // Standard operation after initialization
	      if (system_active) {
	          uint16_t adc_val;
	          HAL_ADC_Start(&hadc1);
	          if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
	              adc_val = HAL_ADC_GetValue(&hadc1);
	              water_present = (adc_val > 500) ? 1 : 0;
	          }
	          HAL_ADC_Stop(&hadc1);

	          // Sending a message via UART when a change is made
	          if (water_present != last_water_state) {
	              last_water_state = water_present;
	              if (water_present) {
	                  HAL_UART_Transmit(&huart1, (uint8_t*)"WARNING\n", 8, 100);
	              } else {
	                  HAL_UART_Transmit(&huart1, (uint8_t*)"GOOD\n", 5, 100);
	              }
	              periodic_counter = 0;
	          }

	          // Periodic sending
	          /*
	           WARNING - 5 seconds
	           GOOD - 20 seconds
	           */
	          periodic_counter++;
	          if (water_present) {
	              if (periodic_counter >= 50) {
	                  periodic_counter = 0;
	                  HAL_UART_Transmit(&huart1, (uint8_t*)"WARNING\n", 8, 100);
	              }
	          } else {
	              if (periodic_counter >= 200) {
	                  periodic_counter = 0;
	                  HAL_UART_Transmit(&huart1, (uint8_t*)"GOOD\n", 5, 100);
	              }
	          }

	          // Operation indication
	          if (water_present) {
	              HAL_GPIO_WritePin(G_GPIO_Port, G_Pin, GPIO_PIN_RESET);
	              HAL_GPIO_WritePin(B_GPIO_Port, B_Pin, GPIO_PIN_RESET);
	          } else {
	              HAL_GPIO_WritePin(R_GPIO_Port, R_Pin, GPIO_PIN_RESET);
	              HAL_GPIO_WritePin(G_GPIO_Port, G_Pin, GPIO_PIN_SET);
	              HAL_GPIO_WritePin(B_GPIO_Port, B_Pin, GPIO_PIN_RESET);
	          }
	      } else {
	          // SYSTEM OFF
	          HAL_GPIO_WritePin(R_GPIO_Port, R_Pin, GPIO_PIN_RESET);
	          HAL_GPIO_WritePin(G_GPIO_Port, G_Pin, GPIO_PIN_RESET);
	          HAL_GPIO_WritePin(B_GPIO_Port, B_Pin, GPIO_PIN_RESET);
	          last_water_state = 0;
	          periodic_counter = 0;
	      }
	  }

	  HAL_Delay(100);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enables the Clock Security System
  */
  HAL_RCC_EnableCSS();
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == BUTTON_Pin)
    {
        static uint32_t last_time = 0;
        uint32_t now = HAL_GetTick();

        if ((now - last_time) > 150)
        {
            last_time = now;
            system_active = !system_active;
            send_system_status = 1;
        }
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        if (system_active && water_present)
        {
            HAL_GPIO_TogglePin(R_GPIO_Port, R_Pin);   // Flashing red
        }
        else
        {
            // If the system is turned off or there is no water, turn off the red light.
            HAL_GPIO_WritePin(R_GPIO_Port, R_Pin, GPIO_PIN_RESET);
        }
    }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
