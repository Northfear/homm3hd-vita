#ifndef __INPUT_H__
#define __INPUT_H__

#include <SDL2/SDL.h>

void ProcessControllerAxisMotion();
void ValidatePointerPosition();
void RequestCursorRedraw();
void handleControllerAxisEvent(const SDL_ControllerAxisEvent* motion);
void handleControllerButtonEvent(SDL_ControllerButtonEvent* button);
int mobProcessSDLEvents(void *userdata, SDL_Event *event);

extern uint32_t lastCursorRedrawTime;
extern float emulatedPointerPosX;
extern float emulatedPointerPosY;

extern float gamepadCursorSpeed;
extern float touchpadCursorSpeed;
extern bool rearTouchEnabled;
extern float rearTouchCursorSpeed;
extern uint16_t gamepadDeadzone;

extern int (* InputTextMessageHandler)(SDL_Event*);
extern int (* MouseMessageHandler)(SDL_Event*);
extern int (* TouchMessageHandler)(SDL_Event*);
extern unsigned int (* MultiTouchMessageHandler)(SDL_Event*);
extern unsigned int (* KeyboardMessageHandler)(SDL_Event*);

extern bool *dotemu_refreshScreen;
extern uint32_t *swipeLastX;
extern uint32_t *swipeLastY;
extern int *swipeDiffX;
extern int *swipeDiffY;

#endif
