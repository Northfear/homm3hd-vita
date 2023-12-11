#include <psp2/kernel/clib.h>
#include <stdbool.h>
#include "input.h"
#include "main.h"


const float CONTROLLER_SPEED_MOD = 2000000.0f;
const float CONTROLLER_AXIS_SPEEDUP = 1.03f;
const int32_t CONTROLLER_R_DEADZONE = 25000;
const uint32_t CURSOR_REDRAW_INTERVAL = 17;
const uint32_t MAP_SCROLL_INTERVAL = 100;
const uint32_t REAR_TOUCH_SPEED_MOD = 100;
const uint32_t REAR_LMB_DELAY = 250;

uint32_t lastControllerTime = 0;
uint32_t lastCursorRedrawTime = 0;
uint32_t timeAfterLastScroll = 0;
int16_t controllerLeftXAxis = 0;
int16_t controllerLeftYAxis = 0;
int16_t controllerRightXAxis = 0;
int16_t controllerRightYAxis = 0;
bool leftScrollActive = false;
bool rightScrollActive = false;
bool upScrollActive = false;
bool downScrollActive = false;
float emulatedPointerPosX = 0;
float emulatedPointerPosY = 0;
float gamepadCursorSpeed = 10.f;
float controllerSpeedBoost = 1.f;
uint16_t gamepadDeadzone = 3000;
bool rearTouchEnabled = false;
float rearTouchCursorSpeed = 5.f;
uint8_t rearTouchFingerNum = 0;
uint32_t lastRearTouchTime = 0;
SDL_FingerID rearFirstFingerId = 0;

uint32_t *swipeLastX;
uint32_t *swipeLastY;
int *swipeDiffX;
int *swipeDiffY;


inline void ValidatePointerPosition()
{
	if (emulatedPointerPosX < 0)
		emulatedPointerPosX = 0;
	else if (emulatedPointerPosX >= VITA_WIDTH)
		emulatedPointerPosX = VITA_WIDTH - 1;

	if (emulatedPointerPosY < 0)
		emulatedPointerPosY = 0;
	else if (emulatedPointerPosY >= VITA_HEIGHT)
		emulatedPointerPosY = VITA_HEIGHT - 1;
}

inline void RequestCursorRedraw()
{
	// forced screen update results in visual bugs during combat and town view
	if (ShouldProcessControllerOnTick() && !*dotemu_refreshScreen)
	{
		const uint32_t currentTime = SDL_GetTicks();
		if (currentTime - lastCursorRedrawTime > CURSOR_REDRAW_INTERVAL)
		{
			lastCursorRedrawTime = currentTime;
			*dotemu_refreshScreen = true;
		}
	}
}

inline void EmulateMouseEvent(Uint32 eventType, Uint8 button, Sint32 x, Sint32 y)
{
	SDL_Event ev;
	ev.type = eventType;
	ev.button.button = button;
	ev.button.x = x;
	ev.button.y = y;
	MouseMessageHandler(&ev);
}

void ProcessControllerAxisMotion()
{
	const uint32_t currentTime = SDL_GetTicks();
	const uint32_t deltaTime = currentTime - lastControllerTime;
	lastControllerTime = currentTime;

	SDL_GameControllerUpdate();

	if (controllerLeftXAxis != 0 || controllerLeftYAxis != 0)
	{
		const int16_t xSign = (controllerLeftXAxis > 0) - (controllerLeftXAxis < 0);
		const int16_t ySign = (controllerLeftYAxis > 0) - (controllerLeftYAxis < 0);
		const int oldX = emulatedPointerPosX;
		const int oldY = emulatedPointerPosY;

		float movX = pow(abs(controllerLeftXAxis), CONTROLLER_AXIS_SPEEDUP) * xSign * deltaTime
						* gamepadCursorSpeed * controllerSpeedBoost / CONTROLLER_SPEED_MOD;
		float movY = pow(abs(controllerLeftYAxis), CONTROLLER_AXIS_SPEEDUP) * ySign * deltaTime
						* gamepadCursorSpeed * controllerSpeedBoost / CONTROLLER_SPEED_MOD;

		emulatedPointerPosX += movX;
		emulatedPointerPosY += movY;

		if (((int)emulatedPointerPosX != oldX) || ((int)emulatedPointerPosY != oldY))
		{
			ValidatePointerPosition();
			EmulateMouseEvent(SDL_MOUSEMOTION, 0, emulatedPointerPosX, emulatedPointerPosY);
		}

		RequestCursorRedraw();
	}

	if (upScrollActive || downScrollActive || rightScrollActive || leftScrollActive)
	{
		timeAfterLastScroll += deltaTime;

		if (timeAfterLastScroll > MAP_SCROLL_INTERVAL)
		{
			timeAfterLastScroll = 0;

			*swipeLastX = 0;
			*swipeLastY = 0;
			*swipeDiffX = 0;
			*swipeDiffY = 0;

			if (rightScrollActive)
			{
				*swipeDiffX = -100;
			}
			else if (leftScrollActive)
			{
				*swipeDiffX = 100;
			}

			if (upScrollActive)
			{
				*swipeDiffY = -100;
			}
			else if (downScrollActive)
			{
				*swipeDiffY = 100;
			}

			SDL_Event gesture;
			gesture.type = SDL_MULTIGESTURE;
			gesture.mgesture.numFingers = 2;
			MultiTouchMessageHandler(&gesture);
		}
	}
}

void handleControllerAxisEvent(const SDL_ControllerAxisEvent* motion)
{
	if (motion->axis == SDL_CONTROLLER_AXIS_LEFTX)
	{
		if (abs(motion->value) > gamepadDeadzone)
			controllerLeftXAxis = motion->value;
		else
			controllerLeftXAxis = 0;
	}
	else if (motion->axis == SDL_CONTROLLER_AXIS_LEFTY)
	{
		if (abs(motion->value) > gamepadDeadzone)
			controllerLeftYAxis = motion->value;
		else
			controllerLeftYAxis = 0;
	}
	else if (motion->axis == SDL_CONTROLLER_AXIS_RIGHTX)
	{
		if (abs(motion->value) > CONTROLLER_R_DEADZONE)
			controllerRightXAxis = motion->value;
		else
			controllerRightXAxis = 0;
	}
	else if (motion->axis == SDL_CONTROLLER_AXIS_RIGHTY)
	{
		if (abs(motion->value) > CONTROLLER_R_DEADZONE)
			controllerRightYAxis = motion->value;
		else
			controllerRightYAxis = 0;
	}

	//map scroll
	if (controllerRightXAxis > CONTROLLER_R_DEADZONE)
	{
		if (!rightScrollActive)
		{
			rightScrollActive = true;
		}
	}
	else if (rightScrollActive)
	{
		rightScrollActive = false;
	}

	if (controllerRightXAxis < -CONTROLLER_R_DEADZONE)
	{
		if (!leftScrollActive)
		{
			leftScrollActive = true;
		}
	}
	else if (leftScrollActive)
	{
		leftScrollActive = false;
	}

	if (controllerRightYAxis > CONTROLLER_R_DEADZONE)
	{
		if (!upScrollActive)
		{
			upScrollActive = true;
		}
	}
	else if (upScrollActive)
	{
		upScrollActive = false;
	}

	if (controllerRightYAxis < -CONTROLLER_R_DEADZONE)
	{
		if (!downScrollActive)
		{
			downScrollActive = true;
		}
	}
	else if (downScrollActive)
	{
		downScrollActive = false;
	}
}

void handleControllerButtonEvent(SDL_ControllerButtonEvent* button)
{
	bool mousePress = false;
	Uint8 mouseBtn;

	switch (button->button)
	{
		case SDL_CONTROLLER_BUTTON_A: {
			mousePress = true;
			mouseBtn = SDL_BUTTON_LEFT;
			break;
		}
		case SDL_CONTROLLER_BUTTON_B: {
			mousePress = true;
			mouseBtn = SDL_BUTTON_RIGHT;
			break;
		}
		case SDL_CONTROLLER_BUTTON_X:
			break;
		case SDL_CONTROLLER_BUTTON_Y:
			break;
		case SDL_CONTROLLER_BUTTON_BACK:
			/*
			if (button->type == SDL_CONTROLLERBUTTONDOWN)
			{
				void *gpAdvManager = (void *)(homm3_mod.text_base + 0x0037e84c);
				advManagerDoSystemOptions(gpAdvManager);
			}
			*/
			break;
		case SDL_CONTROLLER_BUTTON_START:
			break;
		case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
			break;
		case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
			controllerSpeedBoost = button->type == SDL_CONTROLLERBUTTONDOWN ? 2.f : 1.f;
			break;
		case SDL_CONTROLLER_BUTTON_DPAD_UP:
			break;
		case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
			break;
		case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
			break;
		case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
			break;
		default:
			break;
	}

	if (mousePress)
	{
		Uint32 eventType = (button->type == SDL_CONTROLLERBUTTONDOWN) ? SDL_MOUSEBUTTONDOWN : SDL_MOUSEBUTTONUP;
		EmulateMouseEvent(eventType, mouseBtn, emulatedPointerPosX, emulatedPointerPosY);
	}
}

void handleRearTouchEvent(SDL_TouchFingerEvent* touch)
{
	if (rearTouchEnabled)
	{
		if (touch->type == SDL_FINGERDOWN)
		{
			++rearTouchFingerNum;
			if (rearTouchFingerNum == 1)
			{
				rearFirstFingerId = touch->fingerId;
				lastRearTouchTime = SDL_GetTicks();
			}
			else if (rearTouchFingerNum == 2)
			{
				EmulateMouseEvent(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_RIGHT, emulatedPointerPosX, emulatedPointerPosY);
			}
			
		}
		else if (touch->type == SDL_FINGERUP)
		{
			--rearTouchFingerNum;
			if ((rearTouchFingerNum == 0) && (SDL_GetTicks() - lastRearTouchTime < REAR_LMB_DELAY))
			{
				EmulateMouseEvent(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, emulatedPointerPosX, emulatedPointerPosY);
				EmulateMouseEvent(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, emulatedPointerPosX, emulatedPointerPosY);
			}
			else if (rearTouchFingerNum == 1)
			{
				EmulateMouseEvent(SDL_MOUSEBUTTONUP, SDL_BUTTON_RIGHT, emulatedPointerPosX, emulatedPointerPosY);
			}
		}
		else if (touch->type == SDL_FINGERMOTION)
		{
			if (rearFirstFingerId == touch->fingerId)
			{
				emulatedPointerPosX += touch->dx * REAR_TOUCH_SPEED_MOD * rearTouchCursorSpeed;
				emulatedPointerPosY += touch->dy * REAR_TOUCH_SPEED_MOD * rearTouchCursorSpeed;

				ValidatePointerPosition();
				EmulateMouseEvent(SDL_MOUSEMOTION, 0, emulatedPointerPosX, emulatedPointerPosY);
				RequestCursorRedraw();
			}
		}
	}
}

int mobProcessSDLEvents(void *userdata, SDL_Event *event)
{
	switch(event->type)
	{
		case SDL_KEYDOWN:
		case SDL_KEYUP: {
			KeyboardMessageHandler(event);
			return 0;
		}

		case SDL_CONTROLLERAXISMOTION: {
			handleControllerAxisEvent(&event->caxis);
			return 0;
		}

		case SDL_CONTROLLERBUTTONDOWN:
		case SDL_CONTROLLERBUTTONUP: {
			handleControllerButtonEvent(&event->cbutton);
			return 0;
		}

		case SDL_TEXTINPUT: {
			InputTextMessageHandler(event);
			return 0;
		}

		case SDL_FINGERDOWN:
		case SDL_FINGERUP:
		case SDL_FINGERMOTION: {
			if (event->tfinger.touchId == 0)
			{
				emulatedPointerPosX = event->tfinger.x * VITA_WIDTH;
				emulatedPointerPosY = event->tfinger.y * VITA_HEIGHT;
				TouchMessageHandler(event);

				if (event->type == SDL_FINGERDOWN)
				{
					RequestCursorRedraw();
				}
			}
			else
			{
				handleRearTouchEvent(&event->tfinger);
			}

			return 1;
		}

		case SDL_MULTIGESTURE: {
			// front touchpad only
			if (event->mgesture.touchId == 0)
			{
				MultiTouchMessageHandler(event);
			}

			return 1;
		}

		case SDL_QUIT: {
			*(uint8_t *)(homm3_mod.text_base + 0x0037ec7c) = 1;
			return 0;
		}

		case SDL_APP_WILLENTERBACKGROUND:
		case SDL_APP_DIDENTERFOREGROUND:
		case SDL_APP_DIDENTERBACKGROUND:
		case SDL_APP_WILLENTERFOREGROUND: {
			return 0;
		}

		case SDL_USEREVENT: {
			*(uint8_t *)(homm3_mod.text_base + 0x0037ec98) = 1;
			return 0;
		}

		default:
			break;
	}

	return 1;
}
