/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <math.h>//sin cos
#include <stdio.h> //sprintf
#include <string.h>//strlen
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
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

SPI_HandleTypeDef hspi3;

TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim11;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
uint8_t DACConfig = 0b0011;
uint8_t State 		=1 ;
uint8_t Mode		=0 ;
uint8_t Slope		= 1;
uint8_t	Duty_C		=50;

uint16_t ADCin 		= 0;
uint16_t dataOut	= 0;

uint64_t _micro 	= 0;

float Freq 			= 1;
float Freq_Saw 		= 1;
float Freq_Sin  	= 1;
float Freq_Square 	= 1;
float VHigh_Sin 	=3.3;
float VLow_Sin		=0;
float VHigh_Square 	=3.3;
float VLow_Square	=0;
float VHigh_Saw 	=3.3;
float VLow_Saw		=0;
float Time 			=0.0;
float Rad			=0;
float Amp			=0;
float Ver_Shift		=0;
float Period_High	=0;
int Sum_Freq 		=0;
//upper 4 bit of DAC


char TxDataBuffer[32] 	={0};
char RxDataBuffer[32] 	={0};
char Menu[1000] 		={0};

enum{
	Start_Menu 				= 1,
	Waitting_Input 			= 10,
	Menu_1 					= 20,
	Menu_1_Waitting_Input	= 30,
	Menu_2 					= 40,
	Menu_2_Waitting_Input	= 50,
	Menu_3 					= 60,
	Menu_3_Waitting_Input	= 70,
};



/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_SPI3_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM11_Init(void);
/* USER CODE BEGIN PFP */
void UARTRecieveAndResponsePolling();
int16_t UARTRecieveIT();
void MCP4922SetOutput(uint8_t Config, uint16_t DACOutput);
uint64_t micros();
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

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_ADC1_Init();
  MX_SPI3_Init();
  MX_TIM3_Init();
  MX_TIM11_Init();
  /* USER CODE BEGIN 2 */
	HAL_TIM_Base_Start(&htim3);
	HAL_TIM_Base_Start_IT(&htim11);
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*) &ADCin, 1);

	HAL_GPIO_WritePin(LOAD_GPIO_Port, LOAD_Pin, GPIO_PIN_RESET);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

	while (1)
	{
		/*Method 2 Interrupt Mode*/
		HAL_UART_Receive_IT(&huart2,  (uint8_t*)RxDataBuffer, 64);

		/*Method 2 W/ 1 Char Received*/
		int16_t inputchar = UARTRecieveIT();
		if(inputchar!=-1)
		{
			//Display Input
			sprintf(TxDataBuffer, "%c\r\n", inputchar);
			HAL_UART_Transmit(&huart2, (uint8_t*)TxDataBuffer, strlen(TxDataBuffer), 1000);
		}
//..............................................................Calculate Menu...........................................................................

		static uint64_t timestamp = 0;
		if (micros() - timestamp > 10000){
			timestamp = micros();
			Time +=0.01;


//..............................................................Calculate Sawtooth Wave...........................................................................

			if(Mode == 1 ){
				if(Slope==1){
					if(Time <= (1/Freq_Saw)){
						dataOut = (((VHigh_Saw*(4096.0/3.3))-(VLow_Saw*(4096.0/3.3)))/(1/Freq_Saw))*Time + (VLow_Saw*(4096.0/3.3));
					}
					else{
						Time = 0.0;
					}
				}
				else{
					if(Time <= (1/Freq_Saw)){
						dataOut = (((VLow_Saw*(4096.0/3.3))-(VHigh_Saw*(4096.0/3.3)))/(1/Freq_Saw))*Time + (VHigh_Saw*(4096.0/3.3));
					}
					else{
						Time = 0.0;
					}
				}
			}

//..............................................................Calculate Sine Wave...........................................................................

			if(Mode == 2)
			{
				Rad += 0.01;
				Amp =((VHigh_Sin*(4096.0/3.3))-(VLow_Sin*(4096.0/3.3)))/2;
				Ver_Shift =((VHigh_Sin*(4096.0/3.3))+(VLow_Sin*(4096.0/3.3)))/2;
				dataOut = (Amp)*sin(2*M_PI*Freq_Sin*Rad)+(Ver_Shift);
			}

//..............................................................Calculate Square Wave...........................................................................

			if(Mode == 3){
				Period_High = Duty_C/(Freq_Square*100.0);
				if(Time <= Period_High){
					dataOut = VHigh_Square*(4096.0/3.3);
				}
				else if(Time > Period_High && Time <=(1/Freq_Square)){
					dataOut = VLow_Square*(4096.0/3.3);
				}

				else {
					Time =0.0 ;
				}
			}

			if (hspi3.State == HAL_SPI_STATE_READY && HAL_GPIO_ReadPin(SPI_SS_GPIO_Port, SPI_SS_Pin) == GPIO_PIN_SET){
				MCP4922SetOutput(DACConfig, dataOut);
			}
		}

//..............................................................Main Menu...........................................................................

		switch(State){
			case Start_Menu:
				sprintf(Menu, "\n\n\n\n\n\nSELECT GENERATOR WAVE\r\n "
						"1.Sawtooth	Wave\n\r "
						"2.Sine   	Wave\n\r "
						"3.Square 	Wave\n\r");
				HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
				State = Waitting_Input;
				break;
			case Waitting_Input:
				switch(inputchar)
				{
				case -1:  //No Input
					break;
				case '1':
					Mode =1;
					State = Menu_1;
					break;
				case '2':
					Mode =2;
					State = Menu_2;
					break;
				case '3':
					Mode =3;
					State = Menu_3;
					break;
				default:
					sprintf(Menu, "Error! You can press only 1. 2. or 3.\r\n");
					HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
					break;
				}
				break;

//..............................................................Generator Sawtooth Wave .............................................................

				case Menu_1:
				sprintf(Menu, 	"\n\nGenerator Sawtooth Wave \r\n"
						"a.Freq Up   (+1Hz) 	A.(+0.1Hz)\r\n"
						"d.Freq Down (-1Hz) 	D.(-0.1Hz)\r\n"
						"\n"
						"q.V High Up (+0.1V)	w.V High Down (-0.1V)\r\n"
						"e.V Low  Up (+0.1V)	r.V Low  Down (-0.1V)\r\n"
						"\n"
						"z.Slope Up		c.Slope down\r\n"
						"p.Print All\r\n"
						"x.Back\r\n");
					HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
					State = Menu_1_Waitting_Input;
					break;

//..............................................................Menu Sawtooth Wave .............................................................

			case Menu_1_Waitting_Input :
				switch(inputchar){
				case -1:  //No Input
					break;
				case 'z':
					Slope = 1;
					sprintf(Menu, "Slope down\r\n");
					HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
					break;
				case 'c':
					Slope = -1;
					sprintf(Menu, "Slope up\r\n");
					HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
					break;


				case 'a':
					if(Freq_Saw>9){
						Freq_Saw =10 ;
					}
					else {
						Freq_Saw+=1;
					}
					sprintf(Menu, "Frequency of Sawtooth Wave :%.1f\r\n",Freq_Saw);
					HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
					break;
				case 'd':
					if(Freq_Saw<1){
						Freq_Saw =0 ;
					}
					else {
						Freq_Saw-=1;
					}
					sprintf(Menu, "Frequency of Sawtooth Wave :%.1f\r\n",Freq_Saw);
					HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
					break;


				case 'A':
					if(Freq_Saw>9.9){
						Freq_Saw =10 ;
					}
					else {
						Freq_Saw+=0.1;
					}
					sprintf(Menu, "Frequency of Sawtooth Wave :%.1f\r\n",Freq_Saw);
					HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
					break;
				case 'D':
					if(Freq_Saw<0.1){
						Freq_Saw =0 ;
					}
					else {
						Freq_Saw-=0.1;
					}
					sprintf(Menu, "Frequency of Sawtooth Wave :%.1f\r\n",Freq_Saw);
					HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
					break;



				case 'q':
					if (VHigh_Saw>3.2){
						VHigh_Saw=3.3;
					}
					else{
						VHigh_Saw +=0.1;
					}
					sprintf(Menu, "V High :%.1f\r\n",VHigh_Saw);
					HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
					break;
				case 'w':
					if (VHigh_Saw<VLow_Saw+0.1){
						VHigh_Saw=VLow_Saw;
					}
					else{
						VHigh_Saw -=0.1;
					}
					sprintf(Menu, "V High :%.1f\r\n",VHigh_Saw);
					HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
					break;


				case 'e':
					if (VLow_Saw>VHigh_Saw-0.1){
						VLow_Saw=VHigh_Saw;
					}
					else{
						VLow_Saw +=0.1;
					}
					sprintf(Menu, "V Low :%.1f\r\n",VLow_Saw);
					HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
					break;
				case 'r':
					if (VLow_Saw<0.1){
						VLow_Saw=0;
					}
					else{
						VLow_Saw -=0.1;
					}
					sprintf(Menu, "V Low :%.1f\r\n",VLow_Saw);
					HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
					break;



				case 'p':
					sprintf(Menu, "Frequency of Sawtooth Wave :%.1f\r\n",Freq_Saw);
					HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
					sprintf(Menu, "V High :%.1f\r\n",VHigh_Saw);
					HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
					sprintf(Menu, "V Low :%.1f\r\n",VLow_Saw);
					HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
					break;
				case 'x':
					State = Start_Menu;
					break;
				default:
					sprintf(Menu, "Error!\r\n");
					HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
					break;
					}
				break;

//..............................................................Generator Sine Wave .............................................................

			case Menu_2:
				sprintf(Menu, 	"\n\nGenerator Sine Wave \r\n"
						"a.Freq Up   (+1Hz) 	A.(+0.1Hz)\r\n"
						"d.Freq Down (-1Hz) 	D.(-0.1Hz)\r\n"
						"\n"
						"q.V High Up (+0.1V)	w.V High Down (-0.1V)\r\n"
						"e.V Low  Up (+0.1V)	r.V Low  Down (-0.1V)\r\n"
						"\n"
						"p.Print All\r\n"
						"x.Back\r\n");
					HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
					State = Menu_2_Waitting_Input;
					break;

//..............................................................Menu Sine Wave .............................................................

			case Menu_2_Waitting_Input :		//Sine Wave
				switch(inputchar){
				case -1:  //No Input
					break;
				case 'a':
					if(Freq_Sin>9){
						Freq_Sin =10 ;
					}
					else {
						Freq_Sin+=1;
					}
					sprintf(Menu, "Frequency of Sawtooth Wave :%.1f\r\n",Freq_Sin);
					HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
					break;
				case 'd':
					if(Freq_Sin<1){
						Freq_Sin =0 ;
					}
					else {
						Freq_Sin-=1;
					}
					sprintf(Menu, "Frequency of Sawtooth Wave :%.1f\r\n",Freq_Sin);
					HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
					break;


				case 'A':
					if(Freq_Sin>9.9){
						Freq_Sin =10 ;
					}
					else {
						Freq_Sin+=0.1;
					}
					sprintf(Menu, "Frequency of Sawtooth Wave :%.1f\r\n",Freq_Sin);
					HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
					break;
				case 'D':
					if(Freq_Sin<0.1){
						Freq_Sin =0 ;
					}
					else {
						Freq_Sin-=0.1;
					}
					sprintf(Menu, "Frequency of Sawtooth Wave :%.1f\r\n",Freq_Sin);
					HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
					break;



				case 'q':
					if (VHigh_Sin>3.2){
						VHigh_Sin=3.3;
					}
					else{
						VHigh_Sin +=0.1;
					}
					sprintf(Menu, "V High :%.1f\r\n",VHigh_Sin);
					HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
					break;
				case 'w':
					if (VHigh_Sin<VLow_Sin+0.1){
						VHigh_Sin=VLow_Sin;
					}
					else{
						VHigh_Sin -=0.1;
					}
					sprintf(Menu, "V High :%.1f\r\n",VHigh_Sin);
					HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
					break;


				case 'e':
					if (VLow_Sin>VHigh_Sin-0.1){
						VLow_Sin=VHigh_Sin;
					}
					else{
						VLow_Sin +=0.1;
					}
					sprintf(Menu, "V Low :%.1f\r\n",VLow_Sin);
					HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
					break;
				case 'r':
					if (VLow_Sin<0.1){
						VLow_Sin=0;
					}
					else{
						VLow_Sin -=0.1;
					}
					sprintf(Menu, "V Low :%.1f\r\n",VLow_Sin);
					HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
					break;



				case 'p':
					sprintf(Menu, "Frequency of Sin Wave :%.1f\r\n",Freq_Sin);
					HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
					sprintf(Menu, "V High :%.1f\r\n",VHigh_Sin);
					HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
					sprintf(Menu, "V Low :%.1f\r\n",VLow_Sin);
					HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
					break;

				case 'x':
					State = Start_Menu;
					break;
				default:
					sprintf(Menu, "Error!\r\n");
					HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
					break;
					}
				break;

//..............................................................Generator Square Wave .............................................................

			case Menu_3:
				sprintf(Menu, 	"\n\nGenerator Square Wave \r\n"
						"a.Freq Up   (+1Hz) 	A.(+0.1Hz)\r\n"
						"d.Freq Down (-1Hz) 	D.(-0.1Hz)\r\n"
						"\n"
						"q.V High Up (+0.1V)	w.V High Down (-0.1V)\r\n"
						"e.V Low  Up (+0.1V)	r.V Low  Down (-0.1V)\r\n"
						"\n"
						"z.Duty cycle up (+10 Percent)	c.Duty cycle up (-10 Percent)\r\n"
						"p.Print All\r\n"
						"x.Back\r\n");
					HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
					State = Menu_3_Waitting_Input;
					break;

//..............................................................Menu Square Wave .............................................................

			case Menu_3_Waitting_Input :
				switch(inputchar){
				case -1:  //No Input
					break;

				case 'z':
					if(Duty_C > 90){
						Duty_C = 100;
					}
					else{
						Duty_C += 10;
					}
					sprintf(Menu, "Duty_Cycle :%d\r\n",Duty_C);
					HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
					break;
				case 'c':
					if(Duty_C < 10){
						Duty_C = 0;
					}
					else{
						Duty_C -= 10;
					}
					sprintf(Menu, "Duty_Cycle :%d\r\n",Duty_C);
					HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
					break;


				case 'a':
					if(Freq_Square>9){
						Freq_Square =10 ;
					}
					else {
						Freq_Square+=1;
					}
					sprintf(Menu, "Frequency of Sawtooth Wave :%.1f\r\n",Freq_Square);
					HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
					break;
				case 'd':
					if(Freq_Square<1){
						Freq_Square =0 ;
					}
					else {
						Freq_Square-=1;
					}
					sprintf(Menu, "Frequency of Sawtooth Wave :%.1f\r\n",Freq_Square);
					HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
					break;


				case 'A':
					if(Freq_Square>9.9){
						Freq_Square =10 ;
					}
					else {
						Freq_Square+=0.1;
					}
					sprintf(Menu, "Frequency of Sawtooth Wave :%.1f\r\n",Freq_Square);
					HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
					break;
				case 'D':
					if(Freq_Square<0.1){
						Freq_Square =0 ;
					}
					else {
						Freq_Square-=0.1;
					}
					sprintf(Menu, "Frequency of Sawtooth Wave :%.1f\r\n",Freq_Square);
					HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
					break;



				case 'q':
					if (VHigh_Square>3.2){
						VHigh_Square=3.3;
					}
					else{
						VHigh_Saw +=0.1;
					}
					sprintf(Menu, "V High :%.1f\r\n",VHigh_Square);
					HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
					break;
				case 'w':
					if (VHigh_Square<VLow_Saw+0.1){
						VHigh_Square=VLow_Saw;
					}
					else{
						VHigh_Square -=0.1;
					}
					sprintf(Menu, "V High :%.1f\r\n",VHigh_Square);
					HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
					break;


				case 'e':
					if (VLow_Square>VHigh_Square-0.1){
						VLow_Square=VHigh_Square;
					}
					else{
						VLow_Square +=0.1;
					}
					sprintf(Menu, "V Low :%.1f\r\n",VLow_Square);
					HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
					break;
				case 'r':
					if (VLow_Square<0.1){
						VLow_Square=0;
					}
					else{
						VLow_Square -=0.1;
					}
					sprintf(Menu, "V Low :%.1f\r\n",VLow_Square);
					HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
					break;



				case 'p':
					sprintf(Menu, "Frequency of Square Wave :%.1f\r\n",Freq_Square);
					HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
					sprintf(Menu, "V High :%.1f\r\n",VHigh_Square);
					HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
					sprintf(Menu, "V Low :%.1f\r\n",VLow_Square);
					HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
					break;



				case 'x':
					State = Start_Menu;
					break;
				default:
					sprintf(Menu, "Error!\r\n");
					HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu), 1000);
					break;
					}
				break;
		}




    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 100;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T3_TRGO;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief SPI3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI3_Init(void)
{

  /* USER CODE BEGIN SPI3_Init 0 */

  /* USER CODE END SPI3_Init 0 */

  /* USER CODE BEGIN SPI3_Init 1 */

  /* USER CODE END SPI3_Init 1 */
  /* SPI3 parameter configuration*/
  hspi3.Instance = SPI3;
  hspi3.Init.Mode = SPI_MODE_MASTER;
  hspi3.Init.Direction = SPI_DIRECTION_2LINES;
  hspi3.Init.DataSize = SPI_DATASIZE_16BIT;
  hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi3.Init.NSS = SPI_NSS_SOFT;
  hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi3.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI3_Init 2 */

  /* USER CODE END SPI3_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 99;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 100;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief TIM11 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM11_Init(void)
{

  /* USER CODE BEGIN TIM11_Init 0 */

  /* USER CODE END TIM11_Init 0 */

  /* USER CODE BEGIN TIM11_Init 1 */

  /* USER CODE END TIM11_Init 1 */
  htim11.Instance = TIM11;
  htim11.Init.Prescaler = 99;
  htim11.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim11.Init.Period = 65535;
  htim11.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim11.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim11) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM11_Init 2 */

  /* USER CODE END TIM11_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SPI_SS_GPIO_Port, SPI_SS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SHDN_GPIO_Port, SHDN_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LOAD_GPIO_Port, LOAD_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD2_Pin LOAD_Pin */
  GPIO_InitStruct.Pin = LD2_Pin|LOAD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : SPI_SS_Pin */
  GPIO_InitStruct.Pin = SPI_SS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SPI_SS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SHDN_Pin */
  GPIO_InitStruct.Pin = SHDN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SHDN_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void MCP4922SetOutput(uint8_t Config, uint16_t DACOutput)
{
	uint32_t OutputPacket = (DACOutput & 0x0fff) | ((Config & 0xf) << 12);
	HAL_GPIO_WritePin(SPI_SS_GPIO_Port, SPI_SS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit_IT(&hspi3, &OutputPacket, 1);
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if (hspi == &hspi3)
	{
		HAL_GPIO_WritePin(SPI_SS_GPIO_Port, SPI_SS_Pin, GPIO_PIN_SET);
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim == &htim11)
	{
		_micro += 65535;
	}
}

inline uint64_t micros()
{
	return htim11.Instance->CNT + _micro;
}

int16_t UARTRecieveIT()
{
	static uint32_t dataPos =0;
	int16_t data=-1;
	if(huart2.RxXferSize - huart2.RxXferCount!=dataPos)
	{
		data=RxDataBuffer[dataPos];
		dataPos= (dataPos+1)%huart2.RxXferSize;
	}
	return data;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	sprintf(TxDataBuffer, "Received:[%s]\r\n", RxDataBuffer);
	HAL_UART_Transmit(&huart2, (uint8_t*)TxDataBuffer, strlen(TxDataBuffer), 1000);
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
