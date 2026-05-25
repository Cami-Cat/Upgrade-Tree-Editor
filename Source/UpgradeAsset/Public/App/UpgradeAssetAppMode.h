#pragma once

#include "CoreMinimal.h"
#include "App/UpgradeAssetEditorApp.h"
#include "App/Factories/UpgradeAssetPrimaryTabFactory.h"
#include "App/Factories/UpgradeAssetPropertiesTabFactory.h"
#include "WorkflowOrientedApp/ApplicationMode.h"
#include "WorkflowOrientedApp/WorkflowTabManager.h"

class UpgradeAssetAppMode : public FApplicationMode {
public:
    UpgradeAssetAppMode(TSharedPtr<class UpgradeAssetEditorApp> app);

    virtual void RegisterTabFactories(TSharedPtr<class FTabManager> inTabManager) override;
    virtual void PreDeactivateMode() override;
    virtual void PostActivateMode() override;

private:
    TWeakPtr<class UpgradeAssetEditorApp> _app;
    FWorkflowAllowedTabSet _tabs;

};