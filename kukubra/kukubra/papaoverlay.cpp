#include "papaoverlay.hpp"
#include "eftdata.hpp"
#include "visuals.hpp"
#include "xorstr.hpp"

PapaOverlay* PapaOverlay::Instance()
{
	static PapaOverlay instance;
	return &instance;
}

int PapaOverlay::Main(HINSTANCE instance, const wchar_t * cmdLine)
{
	auto processId = Process::FindByProcessName(_xor_(L"EscapeFromTarkov.exe"));
	if (!processId)
	{
		MessageBoxW(NULL, _xor_(L"Could not find game process"), _xor_(L"PapaOverlay"), MB_OK);
		return EXIT_FAILURE;
	}

	auto gameData = EFTData::Instance();
	if (!gameData->Initialize(processId))
	{
		MessageBoxW(NULL, _xor_(L"Could not attach to Game"), _xor_(L"PapaOverlay"), NULL);
		return EXIT_FAILURE;
	}

	while (true)
	{
		if (!gameData->Read())
			gameData->InitOffsets();

		// Render with the read data.

		Sleep(20);
	}

	this->Deinitialize();

	return EXIT_SUCCESS;
}

int WINAPI main(HINSTANCE instance, HINSTANCE prevInstance, PWSTR cmdLine, int cmdShow)
{
	return PapaOverlay::Instance()->Main(instance, cmdLine);
}
