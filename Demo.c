/**
 * This is the main file of the ESPLaboratory Demo project.
 * It implements simple sample functions for the usage of UART,
 * writing to the display and processing user inputs.
 *
 * @author: Jonathan Müller-Boruttau,
 * 			Tobias Fuchs tobias.fuchs@tum.de
 * 			Nadja Peters nadja.peters@tum.de (RCS, TUM)
 *
 */
#include "includes.h"
// start and stop bytes for the UART protocol
static const uint8_t startByte = 0xAA, stopByte = 0x55;

static const uint16_t displaySizeX = 320, displaySizeY = 240;

QueueHandle_t ESPL_RxQueue; // Already defined in ESPL_Functions.h
SemaphoreHandle_t ESPL_DisplayReady;

// Stores lines to be drawn
QueueHandle_t JoystickQueue;

int main() {
	// Initialize Board functions and graphics
	ESPL_SystemInit();

	// Initializes Draw Queue with 100 lines buffer
	JoystickQueue = xQueueCreate(100, 2 * sizeof(char));

	// Initializes Tasks with their respective priority
	xTaskCreate(drawTask, "drawTask", 1000, NULL, 4, NULL);
	xTaskCreate(checkJoystick, "checkJoystick", 1000, NULL, 3, NULL);
	//xTaskCreate(switchScreen, "switchScreen", 1000, NULL, 1, NULL);

	// Start FreeRTOS Scheduler
	vTaskStartScheduler();
}

/**
 * Example task which draws to the display.
 * Documentation on https://wiki.ugfx.io/index.php/Drawing
 */
void drawTask() {

	int16_t alpha = 0;	//angle for rotation around triangle
	int16_t r = 50;		//radius for rotation around triangle

	int16_t intButtonA = 0,
			intButtonB = 0,
			intButtonC = 0,
			intButtonD = 0,

	int8_t FlagButtonA = 0,
			FlagButtonB = 0,
			FlagButtonC = 0,
			FlagButtonD = 0,
			FlagButtonK = 0;

	char str[100]; // buffer for messages to draw to display
	struct coord joystickPosition; // joystick queue input buffer

	font_t font1; // Load font for ugfx
	font1 = gdispOpenFont("DejaVuSans24*");

	//rectangle coordinates
	uint16_t caveX0 = 150,
			caveY0 = 125,
			caveSize = 20,
			caveX = caveX0,
			caveY = caveY0;
	//circle coordinates
	uint16_t circlePositionX0 = 150,
			circlePositionY0 = 125,
			circlePositionX = circlePositionX0,
			circlePositionY = circlePositionY0;
	//triangle coordinates
	uint16_t trianglePositionX0 = 150,
			trianglePositionY0 = 125,
			trianglePositionX = trianglePositionX0,
			trianglePositionY = trianglePositionY0;

	// Start endless loop
	while (TRUE) {
		while (xQueueReceive(JoystickQueue, &joystickPosition, 0) == pdTRUE);

		//Initializing coordinates to move screen, minus joystick offset
		int16_t ScreenPosX = joystickPosition.x - 129,
				ScreenPosY = joystickPosition.y - 127;

		/**********************
		 *	2.1 Display
		 ***********************/
		// Clear background
		gdispClear(White);

		//Getting new coordinates
		caveX = caveX0 + r * sin(alpha) + ScreenPosX;
		caveY = caveY0 + r * cos(alpha) + ScreenPosY;
		circlePositionX = circlePositionX0 + r * sin(alpha) + ScreenPosX;
		circlePositionY = circlePositionY0 - r * cos(alpha) + ScreenPosY;
		trianglePositionX = trianglePositionX0 + ScreenPosX;
		trianglePositionY = trianglePositionY0 + ScreenPosY;

		//Increasing the angle
		if (alpha > 359) {
			alpha = 0;
		}
		else {
			alpha = alpha + 15;
		}

		// draw rectangle
		gdispFillArea(caveX - 10, caveY - 10, caveSize + 20, caveSize + 20,	Red);
		// color inner white
		gdispFillArea(caveX, caveY, caveSize, caveSize, White);

		//draw circle
		gdispFillCircle(circlePositionX, circlePositionY, 10, Green);

		//draw triangle
		gdispDrawLine(trianglePositionX, trianglePositionY - 15, trianglePositionX + 15, trianglePositionY + 15, Red);
		gdispDrawLine(trianglePositionX + 15, trianglePositionY + 15, trianglePositionX - 15, trianglePositionY + 15, Red);
		gdispDrawLine(trianglePositionX - 15, trianglePositionY + 15, trianglePositionX, trianglePositionY - 15, Red);

		// Generate string
		sprintf(str, "Random Text 1");
		gdispDrawString(20 + ScreenPosX, 50 + ScreenPosY, str, font1, Black);

		// Generate moving string
		sprintf(str, "Random Text 2");
		gdispDrawString(alpha + ScreenPosX, 20 + ScreenPosY, str, font1, Black);

		/**********************
		 *	2.2 Buttons
		 ***********************/
		sprintf(str, "A: %d|B: %d|C %d|D: %d|", intButtonA, intButtonB,
				intButtonC, intButtonD);
		gdispDrawString(150 + ScreenPosX, 50 + ScreenPosY, str, font1, Black);

		//Buttons are Pulled-Up, setting a flag to increase it once per press, no debouncing needed because enough delay
		//Button A
		if (!GPIO_ReadInputDataBit(ESPL_Register_Button_A, ESPL_Pin_Button_A) && !FlagButtonA) {
			intButtonA++;
			FlagButtonA = 1;
		}
		else if (GPIO_ReadInputDataBit(ESPL_Register_Button_A, ESPL_Pin_Button_A)) {
			FlagButtonA = 0;
		}
		//Button B
		if (!GPIO_ReadInputDataBit(ESPL_Register_Button_B, ESPL_Pin_Button_B) && !FlagButtonB) {
			intButtonB++;
			FlagButtonB = 1;
		}
		else if (GPIO_ReadInputDataBit(ESPL_Register_Button_B, ESPL_Pin_Button_B)) {
			FlagButtonB = 0;
		}
		//Button C
		if (!GPIO_ReadInputDataBit(ESPL_Register_Button_C, ESPL_Pin_Button_C) && !FlagButtonC) {
			intButtonC++;
			FlagButtonC = 1;
		}
		else if (GPIO_ReadInputDataBit(ESPL_Register_Button_C, ESPL_Pin_Button_C)) {
			FlagButtonC = 0;
		}
		//Button D
		if (!GPIO_ReadInputDataBit(ESPL_Register_Button_D, ESPL_Pin_Button_D) && !FlagButtonD) {
			intButtonD++;
			FlagButtonD = 1;
		}
		else if (GPIO_ReadInputDataBit(ESPL_Register_Button_D, ESPL_Pin_Button_D)) {
			FlagButtonD = 0;
		}
		//Button K
		if (!GPIO_ReadInputDataBit(ESPL_Register_Button_K, ESPL_Pin_Button_K) && !FlagButtonK) {
			intButtonA = 0;
			intButtonB = 0;
			intButtonC = 0;
			intButtonD = 0;
			FlagButtonK = 1;
		}
		else if (GPIO_ReadInputDataBit(ESPL_Register_Button_K, ESPL_Pin_Button_K)) {
			FlagButtonK = 0;
		}

		/***********************
		 *	2.3 Joystick
		 ***********************/
		sprintf(str, "Joystick Position X: %d Y: %d", joystickPosition.x, joystickPosition.y);
		gdispDrawString(15 + ScreenPosX, 225 + ScreenPosY, str, font1, Black);



		// Wait for display to stop writing
		xSemaphoreTake(ESPL_DisplayReady, portMAX_DELAY);
		// swap buffers
		ESPL_DrawLayer();

	}
}

/**
 * This task polls the joystick value every 20 ticks
 */
void checkJoystick() {
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	struct coord joystickPosition = { 0, 0 };
	const TickType_t tickFramerate = 20;

	while (TRUE) {
		// Remember last joystick values
		joystickPosition.x = (uint8_t) (ADC_GetConversionValue(ESPL_ADC_Joystick_2) >> 4);
		joystickPosition.y = (uint8_t) 255 - (ADC_GetConversionValue(ESPL_ADC_Joystick_1) >> 4);

		xQueueSend(JoystickQueue, &joystickPosition, 100);

		// Execute every 20 Ticks
		vTaskDelayUntil(&xLastWakeTime, tickFramerate);
	}
}


/**
 * Example function to send data over UART
 *
 * Sends coordinates of a given position via UART.
 * Structure of a package:
 *  8 bit start byte
 *  8 bit x-coordinate
 *  8 bit y-coordinate
 *  8 bit checksum (= x-coord XOR y-coord)
 *  8 bit stop byte
 */
void sendPosition(struct coord position) {
	const uint8_t checksum = position.x ^ position.y;

	UART_SendData(startByte);
	UART_SendData(position.x);
	UART_SendData(position.y);
	UART_SendData(checksum);
	UART_SendData(stopByte);
}

/**
 * Example how to receive data over UART (see protocol above)
 */
void uartReceive() {
	char input;
	uint8_t pos = 0;
	char checksum;
	char buffer[5]; // Start byte,4* line byte, checksum (all xor), End byte
	struct coord position = { 0, 0 };
	while (TRUE) {
		// wait for data in queue
		xQueueReceive(ESPL_RxQueue, &input, portMAX_DELAY);

		// decode package by buffer position
		switch (pos) {
		// start byte
		case 0:
			if (input != startByte)
				break;
		case 1:
		case 2:
		case 3:
			// read received data in buffer
			buffer[pos] = input;
			pos++;
			break;
		case 4:
			// Check if package is corrupted
			checksum = buffer[1] ^ buffer[2];
			if (input == stopByte || checksum == buffer[3]) {
				// pass position to Joystick Queue
				position.x = buffer[1];
				position.y = buffer[2];
				xQueueSend(JoystickQueue, &position, 100);
			}
			pos = 0;
		}
	}
}

/*
 *  Hook definitions needed for FreeRTOS to function.
 */
void vApplicationIdleHook() {
	while (TRUE) {
	};
}

void vApplicationMallocFailedHook() {
	while (TRUE) {
	};
}
