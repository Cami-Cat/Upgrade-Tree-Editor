#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphNode.h"
#include "UpgradeNodeDataAsset.h"
#include "UpgradeAsset.h"
#include "UpgradeAssetGraphNode.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAttachedDataChanged, UUpgradeAssetGraphNode*, UUpgradeNodeDataAsset*);

// This is the default container for the node we're going to use.
UCLASS()
class UUpgradeAssetGraphNode : public UEdGraphNode {
    friend class UpgradeAssetEditorApp;
    GENERATED_BODY()
public:
    // Constructor and Defaults
    UUpgradeAssetGraphNode(const FObjectInitializer& ObjectInitializer);
    virtual void AllocateDefaultPins() override;
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
    virtual void ValidateGuid();
    virtual UUpgradeAssetGraphNode* GetNodeWithGUID(FGuid guid) const;
    virtual void GetNodeContextMenuActions(UToolMenu* menu, class UGraphNodeContextMenuContext* context) const override;
    virtual bool CanUserDeleteNode() const override { return true; }
    virtual void DeleteNode();
    virtual void AddNewUnlockConnection(FGuid _nodeId);
    virtual bool RemoveUnlockConnection(FGuid _nodeId);
    virtual bool RemoveAllUnlockConnections(); 
    
    virtual void SaveAttachedAsset();
    FString CleanStringForAsset(const FString& InString);
    
    UFUNCTION()
    virtual void HandleIconChanged(UTexture2D* newIcon);
    FOnAttachedDataChanged OnAttachedDataChanged;
    UFUNCTION()
    virtual void HandleBorderChanged(UTexture2D* newBorder);
public:


    UFUNCTION()
    virtual FGuid GetNodeId();
    UFUNCTION()
    virtual const TArray<FGuid>& GetNodeUnlocks();
    UFUNCTION()
    virtual UUpgradeNodeDataAsset* GetAttachedData();
    UFUNCTION()
    virtual UObject* GetRuntimeNode();
    
    virtual void SetNodeId(FGuid newNodeId);
    virtual void SetAttachedData(UUpgradeNodeDataAsset* newData);
    virtual void SetRuntimeNode(UObject* runtimeNodeReference);
    
    UPROPERTY()
    TArray<FGuid> _nodeParents;

private:
    UPROPERTY()
    FGuid _nodeId;
    UPROPERTY()
    TArray<FGuid> _unlocks;
    UPROPERTY()
    UObject* _runtimeNode = nullptr;
    UPROPERTY(EditAnywhere, Category = "Node Data", meta = (AllowedClasses = "/Script/UpgradeAssetRuntime.UpgradeNodeDataAsset"))
    UUpgradeNodeDataAsset* _attachedData = nullptr; 
};
