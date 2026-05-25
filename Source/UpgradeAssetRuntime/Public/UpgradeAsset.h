#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UpgradeTreeGraph.h"
#include "UpgradeAsset.generated.h"

class UUpgradeRuntimeGraph;
class UUpgradeNodeDataAsset;

USTRUCT(BlueprintType)
struct FUpgradeNodeInfo {
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Upgrade Tree")
    FVector2D Position = FVector2D::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Upgrade Tree")
    UUpgradeNodeDataAsset* attachedData = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Upgrade Tree")
    FGuid NodeId;

    UPROPERTY(BlueprintReadOnly, Category = "Upgrade Tree")
    TArray<FGuid> Unlocks;
};

USTRUCT(BlueprintType)
struct FUpgradeNodeRuntimeProgress {
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Upgrade Runtime")
    int32 UpgradeCount = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Upgrade Runtime")
    bool bIsUnlocked = false;
};

UCLASS(BlueprintType)
class UPGRADEASSETRUNTIME_API UUpgradeAsset : public UPrimaryDataAsset {
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadOnly, Category = "Upgrade Tree Data")
    TArray<FUpgradeNodeInfo> NodeData;

    UPROPERTY()
    UUpgradeRuntimeGraph* _graph = nullptr;
};