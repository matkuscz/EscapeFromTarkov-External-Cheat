#include "visuals.hpp"
#include "papaoverlay.hpp"
#include "xorstr.hpp"
#include <string>

Visuals * Visuals::Instance()
{
	static Visuals instance;
	return &instance;
}

void Visuals::Render()
{
	auto& local_player = EFTData::Instance()->localPlayer;
	for (auto& player : EFTData::Instance()->players)
	{
		if (player.instance == local_player.instance)
			continue;

		// The fast origin gets fucked sometimes, fallback to using the slow one so u don't get rekt from "nowhere".
		{
			float distance_fast = (player.slowOrigin - player.fastOrigin).GetLength();
			if (distance_fast > 12.5f)
				player.fastOrigin = player.slowOrigin;
		}

		D2D1_POINT_2F screen_pos;
		if (WorldToScreen(player.fastOrigin, screen_pos))
		{
			int distance = (local_player.slowOrigin - player.slowOrigin).GetLength();
			std::wstring esp_info = std::to_wstring(distance) + L"m\n" + player.name;
			auto draw_rect = D2D1::RectF(screen_pos.x - 100, screen_pos.y, screen_pos.x + 100, screen_pos.y + 20);
			// Draw esp_info
		}

		if (WorldToScreen(player.headPos, screen_pos))
		{
			auto draw_rect = D2D1::RectF(screen_pos.x - 2, screen_pos.y - 2, screen_pos.x + 2, screen_pos.y + 2);
			// Draw draw_rect
		}
	}
}

bool Visuals::WorldToScreen(const FVector& point3D, D2D1_POINT_2F& point2D)
{
	D3DXVECTOR3 _point3D = D3DXVECTOR3(point3D.x, point3D.z, point3D.y);

	auto& matrix = EFTData::Instance()->viewMatrix;
	D3DXVECTOR3 translationVector = D3DXVECTOR3(matrix._41, matrix._42, matrix._43);
	D3DXVECTOR3 up = D3DXVECTOR3(matrix._21, matrix._22, matrix._23);
	D3DXVECTOR3 right = D3DXVECTOR3(matrix._11, matrix._12, matrix._13);

	float w = D3DXVec3Dot(&translationVector, &_point3D) + matrix._44;

	if (w < 0.098f)
		return false;

	float y = D3DXVec3Dot(&up, &_point3D) + matrix._24;
	float x = D3DXVec3Dot(&right, &_point3D) + matrix._14;

	auto&& size = PapaOverlay::Instance()->GetD2DRenderTarget()->GetSize();

	point2D.x = (size.width * 0.5f) * (1.f + x / w);
	point2D.y = (size.height * 0.5f) * (1.f - y / w);

	return true;
}