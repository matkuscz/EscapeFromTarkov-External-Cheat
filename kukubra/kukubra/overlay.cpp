#include "overlay.hpp"
#include "xorstr.hpp"

Overlay::Overlay()
{

}

bool Overlay::Initialize()
{
	auto wndclass = _xor_("WindowClass");

	// Register Window Class
	WNDCLASSEX wndClass;
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.hInstance = this->hinstance;
	wndClass.lpfnWndProc = StaticWndProc;
	wndClass.lpszClassName = wndclass.c_str();
	wndClass.lpszMenuName = nullptr;
	wndClass.style = NULL;
	wndClass.hbrBackground = NULL;
	wndClass.hCursor = NULL;
	wndClass.hIcon = NULL;
	wndClass.hIconSm = NULL;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;

	this->wndclassAtom = RegisterClassEx(&wndClass);
	if (!this->wndclassAtom) return false;

	// Create Overlay Window
	this->hwnd = CreateWindowEx(/*WS_EX_TOPMOST | */ WS_EX_TOOLWINDOW | (this->blockInput ? 0 : WS_EX_TRANSPARENT) |  WS_EX_LAYERED /* | WS_EX_NOACTIVATE */ , wndClass.lpszClassName, L"",
		WS_POPUP, this->position.x, this->position.y, this->size.cx, this->size.cy, NULL, NULL, this->hinstance, this);

	if (!this->hwnd) return false;

	return true;
}

void Overlay::Deinitialize()
{

}