#include "process.hpp"
#include <TlHelp32.h>
#include <Psapi.h>

#include <algorithm>

uint32_t Process::FindByWindowName(const wchar_t* windowName, HWND* windowHandle)
{
	*windowHandle = FindWindowW(nullptr, windowName);

	if (*windowHandle)
	{
		DWORD processId = 0U;
		GetWindowThreadProcessId(*windowHandle, &processId);

		return processId;
	}

	return 0U;
}

uint32_t Process::FindByProcessName(const wchar_t* processName)
{
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (snapshot == INVALID_HANDLE_VALUE) return 0U;
	
	PROCESSENTRY32W processEntry;
	processEntry.dwSize = sizeof(PROCESSENTRY32W);

	DWORD processId = 0U;

	if (Process32FirstW(snapshot, &processEntry))
	{
		do
		{
			if (lstrcmpiW(processEntry.szExeFile, processName) == 0)
			{
				processId = processEntry.th32ProcessID;
				break;
			}
		} 
		while (Process32NextW(snapshot, &processEntry));
	}

	CloseHandle(snapshot);

	return processId;
}

Process::Process() : processHandle(NULL) {}

Process::~Process()
{
	//Close();
}

void Process::SetHandle(HANDLE processHandle)
{
	this->processHandle = processHandle;
}

bool Process::Open(const uint32_t processId, const uint32_t accessRights)
{
	this->processHandle = OpenProcess(accessRights, FALSE, processId);

	return this->processHandle != 0;
}

void Process::Close()
{
	CloseHandle(this->processHandle);
}

uint32_t Process::GetId()
{
	return GetProcessId(this->processHandle);
}

/*
// TLHELP implementation - doesn't work for be protected games
uintptr_t Process::GetModuleBase(const wchar_t* moduleName, uintptr_t* moduleSize)
{
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetId());
	if (snapshot == INVALID_HANDLE_VALUE) return 0U;

	MODULEENTRY32W moduleEntry;
	moduleEntry.dwSize = sizeof(MODULEENTRY32W);

	uintptr_t moduleBase = 0U;

	if (Module32FirstW(snapshot, &moduleEntry))
	{
		do
		{
			if (lstrcmpiW(moduleEntry.szModule, moduleName) == 0)
			{
				moduleBase = reinterpret_cast<uintptr_t>(moduleEntry.modBaseAddr);

				if (moduleSize)
				{
					*moduleSize = moduleEntry.modBaseSize;
				}

				break;
			}
		} 
		while (Module32NextW(snapshot, &moduleEntry));
	}

	CloseHandle(snapshot);

	return moduleBase;
}
*/

uintptr_t Process::GetModuleBase(const wchar_t* moduleName, uintptr_t* moduleSize)
{
	uintptr_t moduleBase = 0;

	DWORD neededSize = 0;
	if (EnumProcessModules(processHandle, nullptr, 0, &neededSize))
	{
		DWORD sizeModules = neededSize;
		HMODULE* modules = new HMODULE[neededSize / sizeof(HMODULE)];

		if (EnumProcessModules(processHandle, modules, sizeModules, &neededSize))
		{
			if (sizeModules == neededSize)
			{
				for (int i = 0; i < neededSize/sizeof(HMODULE); i++)
				{
					HMODULE currentModule = modules[i];

					wchar_t moduleName[MAX_PATH] = {};
					if (GetModuleBaseNameW(processHandle, currentModule, moduleName, MAX_PATH))
					{
						if (lstrcmpiW(moduleName, moduleName) == 0)
						{
							MODULEINFO moduleInfo;
							if (GetModuleInformation(processHandle, currentModule, &moduleInfo, sizeof(MODULEINFO)))
							{
								moduleBase = reinterpret_cast<uintptr_t>(moduleInfo.lpBaseOfDll);

								if (moduleSize)
								{
									*moduleSize = moduleInfo.SizeOfImage;
								}

								break;
							}
						}
					}
				}
			}
		}

		delete[] modules;
	}

	return moduleBase;
}

bool Process::CopyModule(const wchar_t* moduleName, ProcessModule* module)
{
	uintptr_t moduleSize = 0;
	uintptr_t moduleBase = GetModuleBase(moduleName, &moduleSize);

	if (!moduleBase || !moduleSize) return false;

	module->baseAddress = moduleBase;
	module->bytes.resize(moduleSize);

	if (!this->ReadMemory(moduleBase, module->bytes.data(), module->bytes.size()))
	{
		module->baseAddress = 0;
		module->bytes.clear();
		module->bytes.resize(0);

		return false;
	}

	return true;
}

uintptr_t Process::ReadPointerChain(const uintptr_t address, const std::vector<uintptr_t> offsets)
{
	uintptr_t result = address;
	for (auto const& offset : offsets)
	{
		if (!this->ReadMemory(result + offset, &result))
			return uintptr_t();

		if (result <= 0x0)
			return uintptr_t();;
	}

	return result;
}

bool Process::ReadMemory(const uintptr_t address, void* buffer, const size_t size)
{
	SIZE_T bytesRead;

	if (ReadProcessMemory(this->processHandle, reinterpret_cast<LPCVOID>(address), buffer, size, &bytesRead))
	{
		return bytesRead == size;
	}

	return false;
}

bool Process::WriteMemory(const uintptr_t address, const void * buffer, const size_t size)
{
	SIZE_T bytesWritten;

	if (WriteProcessMemory(this->processHandle, reinterpret_cast<LPVOID>(address), buffer, size, &bytesWritten))
	{
		return bytesWritten == size;
	}

	return false;
}

ProcessModule::ProcessModule() : bytes(){}

ProcessModule::ProcessModule(const ProcessModule& other)
{
	this->bytes = other.bytes;
	this->baseAddress = other.baseAddress;
}

ProcessModule::ProcessModule(ProcessModule&& other)
{
	this->bytes = std::move(other.bytes);
	this->baseAddress = other.baseAddress;
}

bool ProcessModule::CompareBytes(const char* data, const char* pattern, const char* mask)
{
	while (*mask != '\0')
	{
		if (*mask == 'x' && *data != *pattern)
		{
			return false;
		}

		data++;
		pattern++;
		mask++;
	}

	return true;
}

uintptr_t ProcessModule::FindPattern(const char* pattern, const char* mask)
{
	for (uint8_t* bytes = this->bytes.data(); bytes < this->bytes.data() + this->bytes.size(); bytes++)
	{
		if (CompareBytes((const char*)bytes, pattern, mask))
		{
			return baseAddress + (bytes - this->bytes.data());
		}
	}

	return 0;
}