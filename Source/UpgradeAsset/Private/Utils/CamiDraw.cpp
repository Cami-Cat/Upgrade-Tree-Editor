#include "Utils/CamiSlateUtils.h"
#include "Utils/Maths/CamiMaths.h"

void CamiSlateUtils::DrawGridLineToMouse(FVector2D pointA, FVector2D pointB, FVector2D boxExtent, FSlateWindowElementList& outDrawElements, FGeometry geometry, int32 maxLayerId) {
    // We get the direction that the two points are going in
    FVector2D headingDirection = (pointB - pointA).GetSafeNormal();
    FVector2D lineStart = pointA;
    FVector2D lineEnd = pointB;

    if (!headingDirection.IsNearlyZero())
    {
        // We get the closest point on our box and set that to the line start.
        lineStart = CamiMath::GetClosestPointAABB(pointA, headingDirection, boxExtent, 1.0f);
    }
    // We store the points as we had done before.
    TArray<FVector2D> connectedLinePoints = { lineStart, lineEnd };
    // We now render our solid persistent wire.
    FSlateDrawElement::MakeLines(outDrawElements, maxLayerId, geometry.ToPaintGeometry(), connectedLinePoints, ESlateDrawEffect::None, wireColour, true, wireThickness);
    // Increment the max layer ID to prevent Slate from complaining. (It needs to know how many layers to draw)
    DrawArrow(lineEnd, headingDirection, outDrawElements, geometry, maxLayerId);
    maxLayerId++;
}

void CamiSlateUtils::DrawGridLineBetweenNodes(FVector2D pointA, FVector2D pointB, FVector2D boxExtent, FSlateWindowElementList& outDrawElements, FGeometry geometry, int32& maxLayerId) {
    // We get the direction that the two points are going in
    FVector2D headingDirection = (pointB - pointA).GetSafeNormal();
    FVector2D lineStart = pointA;
    FVector2D lineEnd = pointB;

    if (!headingDirection.IsNearlyZero())
    {
        // We get the closest point on our box and set that to the line start.
        lineStart = CamiMath::GetClosestPointAABB(pointA, headingDirection, boxExtent, 1.0f);
        // The opposite direction is just the inverse of the direction that we are heading, so we'll just invert it.
        FVector2D opposingDirection = -headingDirection;
        // We get the closest point on our target box and set that to the line end.
        lineEnd = CamiMath::GetClosestPointAABB(pointB, opposingDirection, boxExtent, 1.0f);
    }
    // We store the points as we had done before.
    TArray<FVector2D> connectedLinePoints = { lineStart, lineEnd };
    // We now render our solid persistent wire.
    FSlateDrawElement::MakeLines(outDrawElements, maxLayerId, geometry.ToPaintGeometry(), connectedLinePoints, ESlateDrawEffect::None, wireColour, true, wireThickness);
    // Increment the max layer ID to prevent Slate from complaining. (It needs to know how many layers to draw)
    DrawArrow(lineEnd, headingDirection, outDrawElements, geometry, maxLayerId);
    maxLayerId++;
}

void CamiSlateUtils::DrawArrow(FVector2D arrowTipPoint, FVector2D direction, FSlateWindowElementList& outDrawElements, FGeometry geometry, int32& maxLayerId) {
    float arrowAngleRad = GetArrowAngleRadians();

    FVector2D rightDirection(-direction.Y, direction.X);
    FVector2D arrowLeftDir = -direction * FMath::Cos(arrowAngleRad) + rightDirection * FMath::Sin(arrowAngleRad);
    FVector2D arrowRightDir = -direction * FMath::Cos(arrowAngleRad) - rightDirection * FMath::Sin(arrowAngleRad);

    FVector2D arrowTip = arrowTipPoint;
    FVector2D arrowLeftWing = arrowTip + (arrowLeftDir * arrowLength);
    FVector2D arrowRightWing = arrowTip + (arrowRightDir * arrowLength);

    TArray<FVector2D> arrowPoints = { arrowLeftWing, arrowTip, arrowRightWing };
            
    FSlateDrawElement::MakeLines(outDrawElements, maxLayerId, geometry.ToPaintGeometry(), arrowPoints, ESlateDrawEffect::None, wireColour, true, wireThickness);
    maxLayerId++;
}
