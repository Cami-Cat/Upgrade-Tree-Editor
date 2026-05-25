#pragma once

#include "CoreMinimal.h"

class CamiMath : public FMath {
public:

    static FVector2D GetClosestPointAABB(FVector2D Origin, FVector2D Direction, FVector2D BoxHalfExtent, float InternalOffset);

};