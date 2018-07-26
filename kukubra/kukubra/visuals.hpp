#pragma once
#include <Windows.h>
#include <d2d1.h>
#include "math.hpp"

#include "eftdata.hpp"
#include <list>

class Visuals
{
public:
	static Visuals* Instance();

	void Render();

private:
	bool WorldToScreen(const FVector& point3D, D2D1_POINT_2F& point2D);
};

