#pragma once

#include "CoreMinimal.h"

class UpgradeTreePalette {
public:

    static constexpr FLinearColor NodeSelected = FLinearColor(0.06f, 0.45f, 0.85f, 1.0f);
    static constexpr FLinearColor NodeHighlighted = FLinearColor(0.0f, 1.0f, 0.4f, 1.0f);
    static constexpr FLinearColor NodeBackgroundBase = FLinearColor(0.08f, 0.08f, 0.08f, 0.95f);

    static constexpr FLinearColor NodeWireColour = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
};