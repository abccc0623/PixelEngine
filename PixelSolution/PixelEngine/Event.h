#pragma once


struct KeyEvent 
{
	int keyCode;
	bool Pressed;
};

class GameObject;
struct PhysEvent
{
    bool targetIn;
    GameObject* target1;
    GameObject* target2;
};

struct Event
{
    union
    {
        KeyEvent key;
        PhysEvent Collision;
    };
};
