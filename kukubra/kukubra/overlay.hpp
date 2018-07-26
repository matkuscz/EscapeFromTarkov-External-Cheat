#pragma once
#include <Windows.h>
#include <D2d1.h>
#include <Dwrite.h>
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

class Overlay
{
public:
	Overlay();

protected:
	bool Initialize();
	void Deinitialize();
};