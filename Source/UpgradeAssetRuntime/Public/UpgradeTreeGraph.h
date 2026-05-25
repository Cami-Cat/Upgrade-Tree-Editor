#pragma once

#include "CoreMinimal.h"
#include "UObject/NameTypes.h"
#include "Engine/DataAsset.h"
#include "UpgradeNodeDataAsset.h"
#include "UpgradeTreeGraph.generated.h"

UCLASS()
class UPGRADEASSETRUNTIME_API UUpgradeRuntimeNode : public UObject {
    GENERATED_BODY()

public:
    UPROPERTY()
    FName nodeName;

    UPROPERTY()
    FGuid nodeId;
    
    UPROPERTY()
    TArray<FGuid> unlocks;

    UPROPERTY()
    FVector2D position;

    UPROPERTY(EditAnywhere, Category = "Node Data", meta = (AllowedClasses = "/Script/UpgradeAssetRuntime.UpgradeNodeDataAsset"))
    UUpgradeNodeDataAsset* attachedData;
};

UCLASS()
class UPGRADEASSETRUNTIME_API UUpgradeRuntimeGraph : public UObject {
    GENERATED_BODY()
public:
    UPROPERTY()
    TArray<UUpgradeRuntimeNode*> Nodes;
};