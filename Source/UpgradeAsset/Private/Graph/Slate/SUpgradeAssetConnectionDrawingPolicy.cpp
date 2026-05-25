#include "Graph/Slate/UpgradeAssetConnectionDrawingPolicy.h"

FUpgradeConnectionDrawingPolicy::FUpgradeConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float ZoomFactor, const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements, UEdGraph* InGraphObj)
    : FConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, ZoomFactor, InClippingRect, InDrawElements)
    , CachedDrawElements(InDrawElements) 
{
    // This unfortunately required those two super's above.
}

void FUpgradeConnectionDrawingPolicy::DrawSplineWithArrow(const FGeometry& StartGeom, const FGeometry& EndGeom, const FConnectionParams& Params)
{
    // We want to overwrite the engine's default drawing behaviour, so we actually want this to do nothing. Not even supercede.
}

void FUpgradeConnectionDrawingPolicy::DrawConnection(int32 LayerId, const FVector2D& Start, const FVector2D& End, const FConnectionParams& Params)
{
    // Likewise with the above function.
}
