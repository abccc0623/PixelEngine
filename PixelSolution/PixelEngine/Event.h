#pragma once


struct KeyEvent 
{
	int keyCode;
	bool Pressed;
};

struct Event
{
    union
    {
        KeyEvent key;
    };
};
