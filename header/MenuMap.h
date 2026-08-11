#pragma once
#include <CMenuManager.h>
#include <CVector.h>
#include <CVector2D.h>
#include <CSprite2d.h>

namespace MenuMap
{
	inline bool worldToScreen(const CVector& t_worldPos, CVector2D& t_screenPos)
	{
		constexpr float MAP_RANGE = 2990.0f;
		float virtualX = FrontEndMenuManager.m_fMapBaseX + FrontEndMenuManager.m_fMapZoom * (t_worldPos.x / MAP_RANGE);
		float virtualY = FrontEndMenuManager.m_fMapBaseY - FrontEndMenuManager.m_fMapZoom * (t_worldPos.y / MAP_RANGE);

		t_screenPos.x = virtualX * static_cast<float>(RsGlobal.maximumWidth) / 640.0f;
		t_screenPos.y = virtualY * static_cast<float>(RsGlobal.maximumHeight) / 448.0f;

		if (t_screenPos.x < 0.0f || t_screenPos.x > static_cast<float>(RsGlobal.maximumWidth)) return false;
		if (t_screenPos.y < 0.0f || t_screenPos.y > static_cast<float>(RsGlobal.maximumHeight)) return false;
		return true;
	}
}
