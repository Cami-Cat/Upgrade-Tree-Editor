#pragma once

#include "CoreMinimal.h"
#include "Utils/UpgradeTreePalette.h"

class CamiSlateUtils : public FSlateDrawElement {
public:
    static void DrawGridLineToMouse(FVector2D pointA, FVector2D pointB, FVector2D boxExtent, FSlateWindowElementList& outDrawElements, FGeometry geometry, int32 maxLayerId);
    static void DrawGridLineBetweenNodes(FVector2D pointA, FVector2D pointB, FVector2D boxExtent, FSlateWindowElementList& outDrawElements, FGeometry geometry, int32& maxLayerId);
    static void DrawArrow(FVector2D arrowTipPoint, FVector2D direction, FSlateWindowElementList& outDrawElements, FGeometry geometry, int32& maxLayerId);
    static float GetArrowAngleRadians() { return FMath::DegreesToRadians(arrowAngle); }
private:

    static constexpr float wireThickness = 2.0f;
    static constexpr FLinearColor wireColour = UpgradeTreePalette::NodeWireColour;
    static constexpr float arrowLength = 24.0f;
    static constexpr float arrowAngle = 30.0f;
};