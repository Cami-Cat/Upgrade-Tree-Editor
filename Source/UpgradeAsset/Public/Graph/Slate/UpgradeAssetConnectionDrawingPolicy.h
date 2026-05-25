#pragma once

#include "CoreMinimal.h"
#include "ConnectionDrawingPolicy.h"

class FUpgradeConnectionDrawingPolicy : public FConnectionDrawingPolicy
{
public:
    FUpgradeConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float ZoomFactor, const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements, UEdGraph* InGraphObj);

    // Override these functions from FConnectionDrawingPolicy so that the engine doesn't try to draw native pin connectors.
    virtual void DrawSplineWithArrow(const FGeometry& StartGeom, const FGeometry& EndGeom, const FConnectionParams& Params) override;
    virtual void DrawConnection(int32 LayerId, const FVector2D& Start, const FVector2D& End, const FConnectionParams& Params) override;

protected:

    FSlateWindowElementList& CachedDrawElements;
    UEdGraph* GraphObj;
};
