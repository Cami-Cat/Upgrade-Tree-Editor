#pragma once

#include "CoreMinimal.h"
#include "UpgradeAsset.h"
#include "App/UpgradeAssetEditorApp.h"
#include "AssetTypeActions_Base.h"

class UpgradeAssetAction : public FAssetTypeActions_Base {
public:
    UpgradeAssetAction(EAssetTypeCategories::Type category);

public:
    virtual FText GetName() const override;
    virtual FColor GetTypeColor() const override;
    virtual UClass* GetSupportedClass() const override;
    virtual void OpenAssetEditor(const TArray<UObject*>& inObjects, TSharedPtr<class IToolkitHost> editWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;
    virtual uint32 GetCategories() override;

private:
    EAssetTypeCategories::Type _assetCategory;
};