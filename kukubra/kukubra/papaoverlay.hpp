#pragma once
#include <Windows.h>
#include "overlay.hpp"
#include "eftdata.hpp"

class PapaOverlay : public Overlay
{
public:
	static PapaOverlay* Instance();

	int Main(HINSTANCE instance, const wchar_t* cmdLine);
};