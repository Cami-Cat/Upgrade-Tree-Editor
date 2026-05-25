#pragma once

#include "CoreMinimal.h"
#include "GraphEditor.h"
#include "UpgradeAsset.h"
#include "IDetailsView.h"
#include "PropertyEditorModule.h"
#include "App/UpgradeAssetEditorApp.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "WorkflowOrientedApp/WorkflowTabFactory.h"
#include "Editor/UnrealEd/Public/Kismet2/BlueprintEditorUtils.h"

class UpgradeAssetPrimaryTabFactory : public FWorkflowTabFactory {
public:
    UpgradeAssetPrimaryTabFactory(TSharedPtr<class UpgradeAssetEditorApp> app);
    
    virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& info) const override;
    virtual FText GetTabToolTipText(const FWorkflowTabSpawnInfo& info) const override;


private:
    TWeakPtr<class UpgradeAssetEditorApp> _app;
};