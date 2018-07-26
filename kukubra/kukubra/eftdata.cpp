#include "eftdata.hpp"
#include "xorstr.hpp"
#include "visuals.hpp"
#include <xmmintrin.h>  
#include <emmintrin.h>
#include <fstream>

EFTData* EFTData::Instance()
{
	static EFTData instance;
	return &instance;
}

bool EFTData::Initialize(uint32_t processId)
{
	if (!process.Open(processId, PROCESS_ALL_ACCESS))
		return false;

	return process.GetId() != 0 &&
		process.CopyModule(_xor_(L"EscapeFromTarkov.exe"), &module);
}

HWND EFTData::GetWindow()
{
	return process.GetMainWindow();
}

FVector EFTData::GetPosition(uint64_t transform)
{
	auto transform_internal = this->process.ReadMemory<uint64_t>(transform + 0x10);
	if (!transform_internal)
		return FVector();

	auto matrices = this->process.ReadMemory<uint64_t>(transform_internal + 0x38);
	auto index = this->process.ReadMemory<uint32_t>(transform_internal + 0x38 + sizeof(uint64_t));
	if (!matrices || index < 0)
		return FVector();

	return GetPosition(matrices, index);
}

// Could be optimized for less reads, for more info read 64com's post:
// https://www.unknowncheats.me/forum/other-fps-games/280145-unity-external-bone-position-transform.html
FVector EFTData::GetPosition(uint64_t pMatrix, uint32_t index)
{
	uint64_t matrix_list_base = 0;
	if (!this->process.ReadMemory((uintptr_t)(pMatrix + 0x8), &matrix_list_base) || !matrix_list_base)
		return FVector();

	uint64_t dependency_index_table_base = 0;
	if (!this->process.ReadMemory((uintptr_t)(pMatrix + 0x10), &dependency_index_table_base) || !dependency_index_table_base)
		return FVector();

	static auto get_dependency_index = [this](uint64_t base, int32_t index) {
		if (!this->process.ReadMemory((uintptr_t)(base + index * 4), &index))
			return -1;
		return index;
	};

	static auto get_matrix_blob = [this](uint64_t base, uint64_t offs, float* blob, uint32_t size) {
		this->process.ReadMemory((uintptr_t)(base + offs), blob, size);
	};

	int32_t index_relation = get_dependency_index(dependency_index_table_base, index);

	FVector ret_value;
	{
		float* base_matrix3x4 = (float*)malloc(64),
			*matrix3x4_buffer0 = (float*)((uint64_t)base_matrix3x4 + 16),
			*matrix3x4_buffer1 = (float*)((uint64_t)base_matrix3x4 + 32),
			*matrix3x4_buffer2 = (float*)((uint64_t)base_matrix3x4 + 48);

		get_matrix_blob(matrix_list_base, index * 48, base_matrix3x4, 16);

		__m128 xmmword_1410D1340 = { -2.f, 2.f, -2.f, 0.f };
		__m128 xmmword_1410D1350 = { 2.f, -2.f, -2.f, 0.f };
		__m128 xmmword_1410D1360 = { -2.f, -2.f, 2.f, 0.f };

		while (index_relation >= 0) {
			uint32_t matrix_relation_index = 6 * index_relation;

			// paziuret kur tik 3 nureadina, ten translationas, kur 4 = quatas ir yra rotationas.
			get_matrix_blob(matrix_list_base, 8 * matrix_relation_index, matrix3x4_buffer2, 16);
			__m128 v_0 = *(__m128*)matrix3x4_buffer2;

			get_matrix_blob(matrix_list_base, 8 * matrix_relation_index + 32, matrix3x4_buffer0, 16);
			__m128 v_1 = *(__m128*)matrix3x4_buffer0;

			get_matrix_blob(matrix_list_base, 8 * matrix_relation_index + 16, matrix3x4_buffer1, 16);
			__m128i v9 = *(__m128i*)matrix3x4_buffer1;

			__m128 *v3 = (__m128*)base_matrix3x4; // r10@1
			__m128 v10; // xmm9@2
			__m128 v11; // xmm3@2
			__m128 v12; // xmm8@2
			__m128 v13; // xmm4@2
			__m128 v14; // xmm2@2
			__m128 v15; // xmm5@2
			__m128 v16; // xmm6@2
			__m128 v17; // xmm7@2

			v10 = _mm_mul_ps(v_1, *v3);
			v11 = _mm_castsi128_ps(_mm_shuffle_epi32(v9, 0));
			v12 = _mm_castsi128_ps(_mm_shuffle_epi32(v9, 85));
			v13 = _mm_castsi128_ps(_mm_shuffle_epi32(v9, -114));
			v14 = _mm_castsi128_ps(_mm_shuffle_epi32(v9, -37));
			v15 = _mm_castsi128_ps(_mm_shuffle_epi32(v9, -86));
			v16 = _mm_castsi128_ps(_mm_shuffle_epi32(v9, 113));

			v17 = _mm_add_ps(
				_mm_add_ps(
					_mm_add_ps(
						_mm_mul_ps(
							_mm_sub_ps(
								_mm_mul_ps(_mm_mul_ps(v11, (__m128)xmmword_1410D1350), v13),
								_mm_mul_ps(_mm_mul_ps(v12, (__m128)xmmword_1410D1360), v14)),
							_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(v10), -86))),
						_mm_mul_ps(
							_mm_sub_ps(
								_mm_mul_ps(_mm_mul_ps(v15, (__m128)xmmword_1410D1360), v14),
								_mm_mul_ps(_mm_mul_ps(v11, (__m128)xmmword_1410D1340), v16)),
							_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(v10), 85)))),
					_mm_add_ps(
						_mm_mul_ps(
							_mm_sub_ps(
								_mm_mul_ps(_mm_mul_ps(v12, (__m128)xmmword_1410D1340), v16),
								_mm_mul_ps(_mm_mul_ps(v15, (__m128)xmmword_1410D1350), v13)),
							_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(v10), 0))),
						v10)),
				v_0);

			*v3 = v17;

			index_relation = get_dependency_index(dependency_index_table_base, index_relation);
		}

		ret_value = *(FVector*)base_matrix3x4;
		delete[] base_matrix3x4;
	}

	return ret_value;
}

bool EFTData::Read()
{
	this->players.clear();

	// Accumulate players.
	{
		uint64_t registeredPlayers = this->process.ReadMemory<uint64_t>(this->offsets.localGameWorld + this->offsets.localGameWorld_offsets.registeredPlayers);
		if (!registeredPlayers)
			return false;

		uint64_t list_base = this->process.ReadMemory<uint64_t>(registeredPlayers + offsetof(EFTStructs::List, listBase));
		uint32_t player_count = this->process.ReadMemory<uint32_t>(registeredPlayers + offsetof(EFTStructs::List, itemCount));
		if (player_count <= 0 || !list_base)
			return false;

		constexpr auto BUFFER_SIZE = 128;
		if (player_count > BUFFER_SIZE)
			std::runtime_error("Too many players, extend buffer.");

		uint64_t player_instance_buffer[BUFFER_SIZE];
		if (!this->process.ReadMemory(list_base + offsetof(EFTStructs::ListInternal, firstEntry), player_instance_buffer, sizeof(uint64_t) * player_count))
			return false;

		EFTPlayer player;
		for (uint32_t i = 0; i < player_count; i++)
		{
			player.instance = player_instance_buffer[i];

			if (uint64_t pedometer = this->process.ReadMemory<uint64_t>(player.instance + this->offsets.Player.pedometer))
			{
				player.slowOrigin = this->process.ReadMemory<FVector>(pedometer + this->offsets.pedometer.origin);
				if (player.slowOrigin.z > 800) // Poli's meme, too afraid to change it incase it breaks smtn.
					player.slowOrigin.z -= 1000;
				if (player.slowOrigin.z < -800)
					player.slowOrigin.z += 1000;
			}

			// playerbody] skeleton_root_joints] bone_transform_list] list_base] transform]
			static std::vector<uintptr_t> offs_head_transform{ this->offsets.Player.playerBody, 0x28, 0x28, 0x10, 0x20 + 133/*bone id*/ * 8 };
			if (uint64_t bone_transform = this->process.ReadPointerChain(player.instance, offs_head_transform))
			{
				// Get head bone pos.
				player.headPos = this->GetPosition(bone_transform);

				// Get fast origin.
				if (uint64_t bone_transform_internal = this->process.ReadMemory<uint64_t>(bone_transform + 0x10))
					if (auto matrices = this->process.ReadMemory<uint64_t>(bone_transform_internal + 0x38))
						player.fastOrigin = this->process.ReadMemory<FVector>(matrices + 0x90);
			}

			uint64_t movement_ctx = this->process.ReadMemory<uint64_t>(player.instance + this->offsets.Player.movementContext);
			if (movement_ctx)
			{
				player.aimAngles = this->process.ReadMemory<FRotator>(movement_ctx + this->offsets.movementContext.angles_0);
				player.aimAngles.ToSourceAngles(); // yaw = -yaw + 90.f; Clamp();
			}

			uint64_t playerName = this->process.ReadPointerChain(player.instance, { this->offsets.Player.profile, this->offsets.profile.information, this->offsets.information.playerName});
			if (playerName)
			{
				int32_t nameLength = this->process.ReadMemory<int32_t>(playerName + this->offsets.unicodeString.length);
				if (nameLength > 128)
					break;

				player.name.resize(nameLength + 1);
				this->process.ReadMemory(playerName + this->offsets.unicodeString.stringBase, (uint64_t*)player.name.data(), nameLength * 2 + 2);
			}

			// Leave this at the end to have all the data.
			if (this->process.ReadMemory<uint64_t>(player.instance + this->offsets.Player.onSpeedChangedEvent))
				this->localPlayer = player;

			this->players.emplace_back(player);
		}
	}

	// Accumulate loot.
	{
		uint64_t lootList = this->process.ReadMemory<uint64_t>(this->offsets.localGameWorld + this->offsets.localGameWorld_offsets.itemList);
		if (!lootList)
			return false;

		uint64_t list_base = this->process.ReadMemory<uint64_t>(lootList + offsetof(EFTStructs::List, listBase));
		uint32_t loot_count = this->process.ReadMemory<uint32_t>(lootList + offsetof(EFTStructs::List, itemCount));
		if (loot_count <= 0 || !list_base)
			return false;

		constexpr auto BUFFER_SIZE = 1024;
		if (loot_count > BUFFER_SIZE)
			std::runtime_error("Too much loot, extend buffer.");

		uint64_t loot_instance_buffer[BUFFER_SIZE];
		if (!this->process.ReadMemory(list_base + offsetof(EFTStructs::ListInternal, firstEntry), loot_instance_buffer, sizeof(uint64_t) * loot_count))
			return false;

		EFTLoot loot_item;
		std::array<char, 64> name_buffer;
		for (uint32_t i = 0; i < loot_count; i++)
		{
			loot_item.instance = loot_instance_buffer[i];

			static std::vector<uintptr_t> offs_name{ 0x10, 0x30, 0x60 };
			if (auto name_ptr = this->process.ReadPointerChain(loot_item.instance, offs_name))
				name_buffer = this->process.ReadMemory<std::array<char, 64>>(name_ptr);
			name_buffer[63] = '\0';

			loot_item.name = name_buffer.data();
			// You can filter out items here, for example:
			// if (loot_item.name.find("bitc") != std::string::npos)
			//	   loot_item.name = "bitcoin";
			// else
			//	   continue;

			static std::vector<uintptr_t> offs_origin{0x10, 0x30, 0x30, 0x8, 0x38};
			if (auto addr = this->process.ReadPointerChain(loot_item.instance, offs_origin))
				loot_item.origin = this->process.ReadMemory<FVector>(addr + 0x90);

			this->loot.emplace_back(loot_item);
		}
	}

	// Do local player manipulation.
	{
		// Open locked objects.
		uint64_t worldInteractiveObject = this->process.ReadMemory<uint64_t>(this->localPlayer.instance + this->offsets.Player.worldInteractiveObject);
		if (worldInteractiveObject) // What we're looking at.
		{
			if (HIBYTE(GetAsyncKeyState(VK_F2)))
			{
				uint8_t can_be_breached = 1;
				uint8_t state_open = 2;
				this->process.WriteMemory(worldInteractiveObject + this->offsets.worldInteractiveObject.canBeBreached, &can_be_breached);
				this->process.WriteMemory(worldInteractiveObject + this->offsets.worldInteractiveObject.operatable, &can_be_breached);
				this->process.WriteMemory(worldInteractiveObject + this->offsets.worldInteractiveObject.openState, &state_open);
			}
		}

		// Weapon mods.
		if (this->localPlayer.instance)
		{
			auto proceduralWeaponAnimation = this->process.ReadMemory<uint64_t>(this->localPlayer.instance + this->offsets.Player.proceduralWeaponAnimation);
			if (proceduralWeaponAnimation)
			{
				// No recoil.
				if (auto ShotEffector = this->process.ReadMemory<uint64_t>(proceduralWeaponAnimation + 0x48))
				{
					FVector recoil{ 0.f, 0.f, 0.f };
					this->process.WriteMemory(ShotEffector + 0x38, &recoil);
				}
				// No sway.
				if (auto BreathEffector = this->process.ReadMemory<uint64_t>(proceduralWeaponAnimation + 0x28))
				{
					float Intensity = 0.f;
					this->process.WriteMemory(BreathEffector + 0x80, &Intensity);
				}
			}
		}

		// "Speedhack"
		if (GetAsyncKeyState(VK_XBUTTON1) &&
			this->localPlayer.instance)
		{
			// playerBody] skeleton_root_joints] bone_transform_list] list_base] bone_transform] bone_Transform_internal] matrices] ... origin
			static std::vector<uintptr_t> offsets{ 0xb0, 0x28, 0x28, 0x10, 0x20, 0x10, 0x38 };
			if (auto addr = this->process.ReadPointerChain(this->localPlayer.instance, offsets))
			{
				FVector fw, rh, lh;
				this->localPlayer.aimAngles.AngleVectors(&fw, &rh, &lh);

				FVector new_location = this->localPlayer.fastOrigin + fw * 0.5f; // Change 0.5f to adjust speed.
				this->process.WriteMemory(addr + 0x90, &new_location);
			}
		}

		// Too lazy to add in aimbot. Do it yourself.
		{

		}
	}

	// Get view matrix.
	{
		uint64_t temp = this->offsets.fpsCamera;
		if (!(temp = this->process.ReadMemory<uint64_t>(temp + 0x30)) ||
			!(temp = this->process.ReadMemory<uint64_t>(temp + 0x18)))
			return false;

		D3DXMATRIX temp_matrix;
		this->process.ReadMemory(temp + 0xC0, &temp_matrix);
		D3DXMatrixTranspose(&this->viewMatrix, &temp_matrix);
	}

	return true;
}

/* All one time initialization in here*/
bool EFTData::InitOffsets()
{
	if (!process.ReadMemory(this->module.baseAddress + this->offsets.offs_gameObjectManager, &this->offsets.gameObjectManager))
		return false;

	// Read pointer to activeObjects and lastActiveObject with 1 read, then find game world and local game world.
	auto active_objects = process.ReadMemory<std::array<uint64_t, 2>>(this->offsets.gameObjectManager + offsetof(EFTStructs::GameObjectManager, lastActiveObject));
	if (!active_objects[0] || !active_objects[1])
		return false;

	if (!(this->offsets.gameWorld = GetObjectFromList(active_objects[1], active_objects[0], _xor_("GameWorld"))))
		return false;

	// Find fps camera.
	this->offsets.localGameWorld = this->process.ReadPointerChain(this->offsets.gameWorld, { 0x30, 0x18, 0x28 });

	// Get tagged objects and find fps camera.
	auto tagged_objects = process.ReadMemory<std::array<uint64_t, 2>>(this->offsets.gameObjectManager + offsetof(EFTStructs::GameObjectManager, lastTaggedObject));
	if (!tagged_objects[0] || !tagged_objects[1])
		return false;

	if (!(this->offsets.fpsCamera = GetObjectFromList(tagged_objects[1], tagged_objects[0], _xor_("FPS Camera"))))
		return false;

	return true;
}

uint64_t EFTData::GetObjectFromList(uint64_t listPtr, uint64_t lastObjectPtr, const char * objectName)
{
	using EFTStructs::BaseObject;
	char name[64];
	uint64_t classNamePtr = 0x0;

	BaseObject activeObject = process.ReadMemory<BaseObject>(listPtr);
	BaseObject lastObject = process.ReadMemory<BaseObject>(lastObjectPtr);

	if (activeObject.object != 0x0)
	{
		while (activeObject.object != 0 && activeObject.object != lastObject.object)
		{
			classNamePtr = process.ReadMemory<uint64_t>(activeObject.object + 0x60);
			process.ReadMemory(classNamePtr, &name);

			if (strcmp(name, objectName) == 0)
			{
				return activeObject.object;
			}

			activeObject = process.ReadMemory<BaseObject>(activeObject.nextObjectLink);
		}
	}

	if (lastObject.object != 0x0)
	{
		classNamePtr = process.ReadMemory<uint64_t>(lastObject.object + 0x60);
		process.ReadMemory(classNamePtr, &name);

		if (strcmp(name, objectName) == 0)
		{
			return lastObject.object;
		}
	}

	return uint64_t();
}