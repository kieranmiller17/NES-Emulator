#ifndef Controller_H
#define Controller_H


#include "typedef.hpp"
#include <SDL.h>


class Controller {

public:

	void pressButton(SDL_Keycode, u8);
	u8 read();
	void write(u8);
	u8 getStatus(u8);

private:

	struct {
		u8 up;
		u8 down;
		u8 left;
		u8 right;
		u8 a;
		u8 b;
		u8 select;
		u8 start;
	} ControllerStatus {};

	bool strobe {};
	u8 strobeStatus {};
};



#endif