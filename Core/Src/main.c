/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c  LAB6
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_host.h"
#include "seg7.h"

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
I2C_HandleTypeDef hi2c1;

I2S_HandleTypeDef hi2s3;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim7;

/* USER CODE BEGIN PV */
int DelayValue = 50;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM7_Init(void);
void MX_USB_HOST_Process(void);

/* USER CODE BEGIN PFP */
//void Play_Note(int note,int size,int tempo,int space);
//extern void Seven_Segment_Digit (unsigned char digit, unsigned char hex_char, unsigned char dot);
//extern void Seven_Segment(unsigned int HexValue);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

int timer = 0;

char ramp = 0;
char RED_BRT = 0;
char GREEN_BRT = 0;
char BLUE_BRT = 0;
char RED_STEP = 1;
char GREEN_STEP = 2;
char BLUE_STEP = 3;
char DIM_Enable = 0;
char Music_ON = 0;
int TONE = 0;
int COUNT = 0;
int INDEX = 0;
int Note = 0;
int Save_Note = 0;
int Vibrato_Depth = 1;
int Vibrato_Rate = 40;
int Vibrato_Count = 0;
char Animate_On = 0;
char Message_Length = 0;
char *Message_Pointer;
char *Save_Pointer;
int Delay_msec = 0;
int Delay_counter = 0;

int CRC_Tx = 0x7bc26311;
int CRC_Rx = 0;

/* GAME VARS */

typedef enum Level {
	OFF,
	DIM,
	ON,
	BLINK
} Level;

typedef enum Phase {
	TITLE,
	P1PLACE,
	P2PLACE,
	P1TURN,
	P2TURN,
	P1WIN,
	P2WIN
} Phase;

typedef struct {
	Level vertical[2][16];		// 16 top row, 16 bottom row
	Level horizontal[3][8];	// 8 top, 8 mid, 8 bottom
} Map;


Map p1_ships, p1_shots, p2_ships, p2_shots;
Map cursor;
const Map nullMap;

int cursorX = 0;
int cursorY = 0;
int cursorVH = 0;

int doubleMode = 0;

int unplacedSingles = 3;
int unplacedDoubles = 2;

int p1Hits = 0;
int p2Hits = 0;

int hitsToWin = 7;

int turnOver = 0;

char TitleMessage[] = { SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, CHAR_B, CHAR_A, CHAR_T, CHAR_T, CHAR_L, CHAR_E, CHAR_S, CHAR_H, CHAR_I, CHAR_P, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE };
char P1PlaceMessage[] = { CHAR_P, 1, SPACE, CHAR_P, CHAR_L, CHAR_A, CHAR_C, CHAR_E };
char P2PlaceMessage[] = { CHAR_P, 2, SPACE, CHAR_P, CHAR_L, CHAR_A, CHAR_C, CHAR_E };
char P1TurnMessage[] = { CHAR_P, 1, SPACE, CHAR_T, CHAR_U, CHAR_R, CHAR_N, SPACE};
char P2TurnMessage[] = { CHAR_P, 2, SPACE, CHAR_T, CHAR_U, CHAR_R, CHAR_N, SPACE};
char P1WinMessage[] = { CHAR_P, 1, SPACE, CHAR_W, CHAR_I, CHAR_N, CHAR_S, SPACE};
char P2WinMessage[] = { CHAR_P, 2, SPACE, CHAR_W, CHAR_I, CHAR_N, CHAR_S, SPACE};

void Display_Map(Map map);
void Game_Loop_Control();
void Player_Place_Loop(Phase phase);
int Player_Shoot(Phase phase);
char Boolean_Brightness(Level brightness);
void Composite_Display(Map maps[], int count);
int Button_Pressed();


/* Declare array for Song */
Music Song[100];


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

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  //MX_I2C1_Init();
  //MX_I2S3_Init();
  //MX_SPI1_Init();
  //MX_USB_HOST_Init();
  MX_TIM7_Init();
  /* USER CODE BEGIN 2 */

  /*** Configure GPIOs ***/
  GPIOD->MODER = 0x55555555; // set all Port D pins to outputs
  GPIOA->MODER |= 0x000000FF; // Port A mode register - make A0 to A3 analog pins
  GPIOE->MODER |= 0x55555555; // Port E mode register - make E0 to E15 outputs
  GPIOC->MODER |= 0x0; // Port C mode register - all inputs
  GPIOE->ODR = 0xFFFF; // Set all Port E pins high

  /*** Configure ADC1 ***/
  RCC->APB2ENR |= 1<<8;  // Turn on ADC1 clock by forcing bit 8 to 1 while keeping other bits unchanged
  ADC1->SMPR2 |= 1; // 15 clock cycles per sample
  ADC1->CR2 |= 1;        // Turn on ADC1 by forcing bit 0 to 1 while keeping other bits unchanged

  /*** Turn on CRC Clock in AHB1ENR to enable CRC hardware ***/
  RCC->AHB1ENR |= 1 << 12;

  /*****************************************************************************************************
  These commands are handled as part of the MX_TIM7_Init() function and don't need to be enabled
  RCC->AHB1ENR |= 1<<5; // Enable clock for timer 7
  __enable_irq(); // Enable interrupts
  NVIC_EnableIRQ(TIM7_IRQn); // Enable Timer 7 Interrupt in the NVIC controller
  *******************************************************************************************************/

  TIM7->PSC = 199; //250Khz timer clock prescaler value, 250Khz = 50Mhz / 200
  TIM7->ARR = 1; // Count to 1 then generate interrupt (divide by 2), 125Khz interrupt rate to increment byte counter for 78Hz PWM
  TIM7->DIER |= 1; // Enable timer 7 interrupt
  TIM7->CR1 |= 1; // Enable timer counting

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
	  Game_Loop_Control();


    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/* GAME HELPER FUNCTIONS */

void Set_7SEG_Section(int section, char hex) {
	GPIOE->ODR = (0xFF00 | ~hex) & ~(1 << (section + 8));

	// Set all selects high to latch-in character
	GPIOE->ODR |= 0xFF00;
	return;
}

void Composite_Display(Map maps[], int count) {
	Map combinedMap = nullMap;

	for (int i = 0; i < count; i++) {
		for (int j = 0; j < 3; j++) {
			for (int k = 0; k < 8; k++) {
				if (maps[i].horizontal[j][k] == OFF) {
					continue;
				}
				combinedMap.horizontal[j][k] = maps[i].horizontal[j][k];
			}
		}
		for (int j = 0; j < 2; j++) {
			for (int k = 0; k < 16; k++) {
				if (maps[i].vertical[j][k] == OFF) {
					continue;
				}
				combinedMap.vertical[j][k] = maps[i].vertical[j][k];
			}
		}
	}
	Display_Map(combinedMap);
}

void Display_Map(Map map) {
	char board[8] = {0};

	int i = 0;

	// Horizontal
	for (i = 0; i < 8; i++) {
		// Top
		board[i] |= Boolean_Brightness(map.horizontal[0][i]) << 0;
		// Middle
		board[i] |= Boolean_Brightness(map.horizontal[1][i]) << 6;
		// Bottom
		board[i] |= Boolean_Brightness(map.horizontal[2][i]) << 3;
	}

	// Vertical
	for (i = 0; i < 8; i++) {
		// Top Left
		board[i] |= Boolean_Brightness(map.vertical[0][2 * i + 1]) << 5;
		// Top Right
		board[i] |= Boolean_Brightness(map.vertical[0][2 * i]) << 1;
		// Bottom Left
		board[i] |= Boolean_Brightness(map.vertical[1][2 * i + 1]) << 4;
		// Bottom Right
		board[i] |= Boolean_Brightness(map.vertical[1][2 * i]) << 2;
	}

	for (i = 0; i < 8; i++) {
		Set_7SEG_Section(i, board[i]);
	}
}

int Button_Pressed() {
	return !((GPIOC->IDR & (1 << 10)) == (1 << 10));
}

void Animate_Message(char *message) {

	Message_Pointer = &message[0];
	Save_Pointer = &message[0];
	Message_Length = sizeof(message)/sizeof(message[0]);
	Delay_msec = 200;
	Animate_On = 1;

	HAL_Delay(2000);

	Animate_On = 0;
}

void Game_Loop_Control() {
	Phase phase = TITLE;

	while(1) {

		switch (phase) {
			case TITLE:
				// Display title on board (scrolling?)
				Message_Pointer = &TitleMessage[0];
				Save_Pointer = &TitleMessage[0];
				Message_Length = sizeof(TitleMessage)/sizeof(TitleMessage[0]);
				Delay_msec = 200;
				Animate_On = 1;

				unplacedDoubles = 2;
				unplacedSingles = 3;
				doubleMode = 0;

				p1Hits = 0;
				p2Hits = 0;

				p1_ships = p1_shots = p2_ships = p2_shots = nullMap;

				// wait for button press
				while (!Button_Pressed()) {}
				Animate_On = 0;

				// Move game phase to P1PLACE

				phase = P1PLACE;

				break;
			case P1PLACE:

				// Display P1 Place Message
				Animate_Message(P1PlaceMessage);

				// Loop placing function until done with ships
				while(unplacedDoubles > 0) {
					Player_Place_Loop(P1PLACE);
				}

				// Update global variables for P2 Place Phase
				unplacedSingles = 3;
				unplacedDoubles = 2;
				doubleMode = 0;

				timer = 0;

				while (timer < 1500) {
					Display_Map(p1_ships);
				}

				phase = P2PLACE;
				break;
			case P2PLACE:
				// Loop placing function until done with ships
				Animate_Message(P2PlaceMessage);

				while(unplacedDoubles > 0) {
					Player_Place_Loop(P2PLACE);
				}

				doubleMode = 0;

				timer = 0;

				while (timer < 1500) {
					Display_Map(p2_ships);
				}

				phase = P1TURN;
				break;
			case P1TURN:
				Animate_Message(P1TurnMessage);

				while(Player_Shoot(P1TURN) != 1) {}

				if (p1Hits >= hitsToWin) {
					phase = P1WIN;
					break;
				}

				// Wait while maintaining PWM
				timer = 0;

				while (timer < 1000) {
					Display_Map(p1_shots);
				}

				// Delay to show only hit ships
				HAL_Delay(1000);

				phase = P2TURN;
				break;
			case P2TURN:
				Animate_Message(P2TurnMessage);

				while(Player_Shoot(P2TURN) != 1) {}

				if (p2Hits >= hitsToWin) {
					phase = P2WIN;
					break;
				}

				timer = 0;

				while (timer < 1000) {
					Display_Map(p2_shots);
				}

				// Delay to show only hit ships
				HAL_Delay(1000);

				phase = P1TURN;
				break;
			case P1WIN:
				// Show victory screen, wait for button press
				Animate_Message(P1WinMessage);
				HAL_Delay(5000);

				phase = TITLE;
				break;
			case P2WIN:
				// Show victory screen, wait for button press
				Animate_Message(P2WinMessage);
				HAL_Delay(5000);

				phase = TITLE;
				break;
		}
	}

	// ...

}

void Potentiometer_Init(int index) {
	if (index > 3 || index < 1) return;
	ADC1->SQR3 = index; // select ADC channel
	HAL_Delay(1);
	/**** START ADC1 CONVERSION ****/    // Start a conversion on ADC1 by forcing bit 30 in CR2 to 1 while keeping other bits unchanged
	ADC1->CR2 |= 1 << 30;

	HAL_Delay(1);

}

int Place_Ship(Map *map) {
	if (cursorVH) {
		if (map->horizontal[cursorY][cursorX] == ON) {
			return -1;
		} else if (doubleMode && map->horizontal[cursorY][cursorX + 1] == ON) {
			return -1;
		} else {
			map->horizontal[cursorY][cursorX] = ON;
			if (doubleMode) {
				map->horizontal[cursorY][cursorX + 1] = ON;
			}
		}
	} else {
		if (map->vertical[cursorY][cursorX] == ON) {
			return -1;
		} else if (doubleMode && map->vertical[1][cursorX] == ON) {
			return -1;
		} else {
			map->vertical[cursorY][cursorX] = ON;
			if (doubleMode) {
				map->vertical[1][cursorX] = ON;
			}
		}
	}
}

void Process_Cursor() {
	Potentiometer_Init(3);
	cursor = nullMap;
	if (ADC1->DR >> 11) {
		// Horizontal
		Potentiometer_Init(1);
		if (doubleMode) {
			cursorX = (ADC1->DR * 7 / 4096);
		} else {
			cursorX = (ADC1->DR >> 9);
		}

		Potentiometer_Init(2);

		cursorY = (ADC1->DR * 3 / 4096);
		cursor.horizontal[cursorY][cursorX] = BLINK;
		if (doubleMode) {
			cursor.horizontal[cursorY][cursorX + 1] = BLINK;
		}

		cursorVH = 1;
	} else {
		// Vertical
		Potentiometer_Init(1);
		cursorX = (ADC1->DR >> 8);

		Potentiometer_Init(2);
		if (doubleMode) {
			cursorY = 0;
		} else {
			cursorY = (ADC1->DR >> 11);
		}
		cursor.vertical[cursorY][cursorX] = BLINK;
		if (doubleMode) {
			cursor.vertical[1][cursorX] = BLINK;
		}

		cursorVH = 0;
	}
}

void Player_Place_Loop(Phase phase) {

	if (unplacedSingles <= 0) {
		doubleMode = 1;
	}

	Process_Cursor();

	if (Button_Pressed()) {
		if (phase == P1PLACE) {
			if (Place_Ship(&p1_ships) != -1) {
				if (doubleMode) {
					unplacedDoubles--;
				} else {
					unplacedSingles--;
				}
			}
		} else {
			if (Place_Ship(&p2_ships) != -1) {
				if (doubleMode) {
					unplacedDoubles--;
				} else {
					unplacedSingles--;
				}
			}
		}
	}

	Map maps[] = {
			((phase == P1PLACE) ? p1_ships : p2_ships),
			cursor
	};

	Composite_Display(maps, 2);
}

// Player shoot
int Player_Shoot(Phase phase) {
	Map *currentShotMap;
	Map *currentShipMap;
	int *playersHits;
	switch (phase) {
	case (P1TURN):
		currentShotMap = &p1_shots;
		currentShipMap = &p2_ships;
		playersHits = &p1Hits;
		break;
	case (P2TURN):
		currentShotMap = &p2_shots;
		currentShipMap = &p1_ships;
		playersHits = &p2Hits;
		break;
	default:
		break;
		// Throw error
	}

	Process_Cursor();

	// Shoot

	int success = -1;

	if (Button_Pressed()) {
		if (cursorVH) {
			if (currentShotMap->horizontal[cursorY][cursorX] != OFF) {
				return -1;
			}
			if (currentShipMap->horizontal[cursorY][cursorX] == ON) {
				currentShotMap->horizontal[cursorY][cursorX] = ON;
				(*playersHits)++;
			} else {
				currentShotMap->horizontal[cursorY][cursorX] = DIM;
			}
		} else {
			if (currentShotMap->vertical[cursorY][cursorX] != OFF) {
				return -1;
			}
			if (currentShipMap->vertical[cursorY][cursorX] == ON) {
				currentShotMap->vertical[cursorY][cursorX] = ON;
				(*playersHits)++;
			} else {
				currentShotMap->vertical[cursorY][cursorX] = DIM;
			}
		}
		success = 1;
	}

	Map maps[] = {
			*currentShotMap,
			cursor
	};

	Composite_Display(maps, 2);

	return success;
}

char Boolean_Brightness(Level brightness) {
	switch (brightness) {
		case OFF:
			return 0;
		case ON:
			return 1;
		case DIM:
			return ((timer % 8) <= 2);
		case BLINK:
			return ((timer % 500) <= 250);
	}
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */

/**
  * @brief I2S3 Initialization Function
  * @param None
  * @retval None
  */


/**
  * @brief TIM7 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM7_Init(void)
{

  /* USER CODE BEGIN TIM7_Init 0 */

  /* USER CODE END TIM7_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 0;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 65535;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM7_Init 2 */

  /* USER CODE END TIM7_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CS_I2C_SPI_GPIO_Port, CS_I2C_SPI_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(OTG_FS_PowerSwitchOn_GPIO_Port, OTG_FS_PowerSwitchOn_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, LD4_Pin|LD3_Pin|LD5_Pin|LD6_Pin
                          |Audio_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : CS_I2C_SPI_Pin */
  GPIO_InitStruct.Pin = CS_I2C_SPI_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(CS_I2C_SPI_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : OTG_FS_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = OTG_FS_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(OTG_FS_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PDM_OUT_Pin */
  GPIO_InitStruct.Pin = PDM_OUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(PDM_OUT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : BOOT1_Pin */
  GPIO_InitStruct.Pin = BOOT1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BOOT1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : CLK_IN_Pin */
  GPIO_InitStruct.Pin = CLK_IN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(CLK_IN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD4_Pin LD3_Pin LD5_Pin LD6_Pin
                           Audio_RST_Pin */
  GPIO_InitStruct.Pin = LD4_Pin|LD3_Pin|LD5_Pin|LD6_Pin
                          |Audio_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : OTG_FS_OverCurrent_Pin */
  GPIO_InitStruct.Pin = OTG_FS_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(OTG_FS_OverCurrent_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : MEMS_INT2_Pin */
  GPIO_InitStruct.Pin = MEMS_INT2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(MEMS_INT2_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

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
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
