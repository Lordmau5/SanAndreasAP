#pragma once
#include <CVector.h>

namespace Teleport
{
	void toGround(const CVector& t_destination, bool t_bringVehicle);
	void toExact(const CVector& t_destination, bool t_bringVehicle, int t_areaCode);
}
