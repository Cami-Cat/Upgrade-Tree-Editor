#include "Utils/Maths/CamiMaths.h"

// Here we'll return the closest point from Point A (Origin) in the in direction (Direction) Usually referencing the SafeNormal of a different object or point.
// Returns an FVector2D of this closest point.
FVector2D CamiMath::GetClosestPointAABB(FVector2D Origin, FVector2D Direction, FVector2D BoxHalfExtent, float InternalOffset) {
    // We get the current hard direction that this desired area is going in (Using FMath::Sign() to determine whether it is +1 or -1). Multiply it by our width and divide the result of that calculation by the ray direction.
    // This ensures we calculate the closest point from the box not only in the signed direction (- or +) but also in the EXACT direction (/ RayDirection)
    float _X = (Direction.X != 0.0f) ? (FMath::Sign(Direction.X) * BoxHalfExtent.X) / Direction.X : FLT_MAX;
    float _Y = (Direction.Y != 0.0f) ? (FMath::Sign(Direction.Y) * BoxHalfExtent.Y) / Direction.Y : FLT_MAX;
    // We then get the smallest one and choose between width and height. This means this is the closest point.
    float _Min = FMath::Min(_X, _Y);

    // This is where we adjust it to be from the center of the origin point.
    // We get the origin point, add the direction of the ray multiplied by the MAXIMUM of 0.0f and T_Min.
    return (Origin + (Direction * FMath::Max(0.0f, _Min - InternalOffset)));
}