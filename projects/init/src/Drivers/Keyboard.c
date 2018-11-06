#include "Keyboard.h"


int KeyboardDeviceInit(KeyboardDevice* keyboard)
{
	if(IOBaseDeviceInit( &keyboard->super) )
	{
		return 1;
	}

	return 0;
}
