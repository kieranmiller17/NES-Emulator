#include "Controller.hpp"

void Controller::pressButton(SDL_Keycode key, u8 keyDown) {

	switch (key) {
	case SDLK_UP:
		ControllerStatus.up = keyDown;
		break;
	case SDLK_DOWN:
		ControllerStatus.down = keyDown;
		break;
	case SDLK_RIGHT:
		ControllerStatus.right = keyDown;
		break;
	case SDLK_LEFT:
		ControllerStatus.left = keyDown;
		break;
	case SDLK_a:
		ControllerStatus.a = keyDown;
		break;
	case SDLK_b:
		ControllerStatus.b = keyDown;
		break;
	case SDLK_SPACE:
		ControllerStatus.select = keyDown;
		break;
	case SDLK_RETURN:
		ControllerStatus.start = keyDown;
		break;
	default:
		break;
	}

}


u8 Controller::getStatus (u8 index) {

	switch (index)
	{
	case 0:
		return ControllerStatus.a;
	case 1:
		return ControllerStatus.b;
	case 2:
		return ControllerStatus.select;
	case 3:
		return ControllerStatus.start;
	case 4:
		return ControllerStatus.up;
	case 5:
		return ControllerStatus.down;
	case 6:
		return ControllerStatus.left;
	case 7:
		return ControllerStatus.right;
	}
}

u8 Controller::read() {

	u8 result = getStatus(strobe == true ? 0 : strobeStatus);

	strobeStatus++;

	return 0x40 | result;

}

void Controller::write(u8 data) {

	if (strobe == true && !(data & 0x1)) 
	{
		strobeStatus = 0;
	}

	strobe = data & 0x1;

}