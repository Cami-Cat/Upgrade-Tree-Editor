#pragma once

#include "CoreMinimal.h"
#include "IDetailsView.h"
#include "UpgradeAsset.h"
#include "PropertyEditorModule.h"
#include "App/UpgradeAssetEditorApp.h"
#include "WorkflowOrientedApp/WorkflowTabFactory.h"


class UpgradeAssetPropertiesTabFactory : public FWorkflowTabFactory {
public:
    UpgradeAssetPropertiesTabFactory(TSharedPtr<class UpgradeAssetEditorApp> app);
    
    virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& info) const override;
    virtual FText GetTabToolTipText(const FWorkflowTabSpawnInfo& info) const override;

private:
    TWeakPtr<class UpgradeAssetEditorApp> _app;
};