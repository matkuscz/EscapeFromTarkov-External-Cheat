#pragma once
#include "process.hpp"
#include "math.hpp"
#include <vector>

#include <d3dx9math.h>
#pragma comment (lib, "d3dx9.lib")

// This is retarded but fuck it, deal with it, all objects should have their full struct
// in the namespace EFTStructs.
struct EFTOffsets
{
	static constexpr uint64_t offs_gameObjectManager = 0x1432840;

	uint64_t gameObjectManager = 0x0;
	uint64_t gameWorld = 0x0;
	uint64_t localGameWorld = 0x0;
	uint64_t fpsCamera = 0x0;

	struct
	{
		static constexpr uint64_t itemList = 0x58;
		static constexpr uint64_t registeredPlayers = 0x68;
	} localGameWorld_offsets;

	struct
	{
		static constexpr uint64_t length = 0x10;
		static constexpr uint64_t stringBase = 0x14;
	} unicodeString;

	struct
	{
		static constexpr uint64_t canBeBreached = 0x108; // all below are 1 byte
		static constexpr uint64_t operatable = 0xdb;
		static constexpr uint64_t openState = 0xd9;
	} worldInteractiveObject;

	struct
	{
		static constexpr uint64_t information = 0x28;
	} profile;

	struct
	{
		static constexpr uint64_t playerName = 0x10;
	} information;

	struct
	{
		static constexpr uint64_t angles_0 = 0x1f4;
		static constexpr uint64_t angles_1 = 0x1f4 + 0x8;
	} movementContext;

	struct
	{
		static constexpr uint64_t origin = 0x30; // Updates REALLY slowly but is accurate.
	} pedometer;

	struct
	{
		static constexpr uint64_t onSpeedChangedEvent = 0x38; // Local player specific var.
		static constexpr uint64_t movementContext = 0x58;
		static constexpr uint64_t pedometer = 0x60;
		static constexpr uint64_t proceduralWeaponAnimation = 0x90;
		static constexpr uint64_t playerBody = 0xB0;
		static constexpr uint64_t worldInteractiveObject = 0x2f8;
		static constexpr uint64_t profile = 0x428;
		
	} Player;
};

namespace EFTStructs
{
	struct BaseObject
	{
		uint64_t previousObjectLink; //0x0000
		uint64_t nextObjectLink; //0x0008
		uint64_t object; //0x0010
	};

	struct GameObjectManager
	{
		uint64_t lastTaggedObject; //0x0000
		uint64_t taggedObjects; //0x0008
		uint64_t lastActiveObject; //0x0010
		uint64_t activeObjects; //0x0018
	}; //Size: 0x0010

	class ListInternal
	{
	public:
		char pad_0x0000[0x20]; //0x0000
		uintptr_t* firstEntry; //0x0020 
	}; //Size=0x0028

	class List
	{
	public:
		char pad_0x0000[0x10]; //0x0000
		ListInternal* listBase; //0x0010 
		__int32 itemCount; //0x0018 
	}; //Size=0x001C
}

struct EFTPlayer
{
	uintptr_t	 instance;
	std::wstring name;
	FRotator	 aimAngles;
	FVector		 headPos;
	FVector		 slowOrigin;
	FVector		 fastOrigin; // Sometimes breaks. Check distance.
};

struct EFTLoot
{
	uintptr_t	instance;
	std::string name;
	FVector		origin;
};

class EFTData
{
public:
	static EFTData* Instance();

	bool Initialize(uint32_t processId);
	bool InitOffsets();
	HWND GetWindow();

	FVector GetPosition(uint64_t transform);
	FVector GetPosition(uint64_t pMatrix, uint32_t index);

	bool Read();

	D3DXMATRIX viewMatrix;
	EFTPlayer localPlayer;
	std::vector<EFTPlayer> players;
	std::vector<EFTLoot> loot;

private:
	uint64_t GetObjectFromList(uint64_t list, uint64_t lastObject, const char* objectName);

	Process process;
	ProcessModule module;

	EFTOffsets offsets;
	std::vector<std::wstring> names;
};