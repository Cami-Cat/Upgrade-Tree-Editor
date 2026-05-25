#include "App/Factories/UpgradeAssetPropertiesTabFactory.h"
// Another important addition to the tree, this properties tab allows us to edit the properties of whatever asset we tell it to. For now, we default to the property that is opened. But in the future we want to change it to the properties of the data asset
// Inside the editor tree.
UpgradeAssetPropertiesTabFactory::UpgradeAssetPropertiesTabFactory(TSharedPtr<class UpgradeAssetEditorApp> app) : FWorkflowTabFactory(FName("UpgradeAssetPropertiesTab"), app) {
    _app = app;
    // More boilerplate simplicity. View UpgradeAssetPrimaryTabFactory.h for more detail.
    TabLabel = FText::FromString(TEXT("Properties"));
    ViewMenuDescription = FText::FromString(TEXT("Displays a properties view for the current upgrade asset."));
    ViewMenuTooltip = FText::FromString(TEXT("Display properties."));
}

// We're doing the same thing as before, creating the body of the tab, but there are some more steps to be done!
TSharedRef<SWidget> UpgradeAssetPropertiesTabFactory::CreateTabBody(const FWorkflowTabSpawnInfo& info) const {
    TSharedPtr<UpgradeAssetEditorApp> app = _app.Pin();
    // Firstly, we need to get the propertyEditorModule, This is a simple LoadModuleChecked with an FName of "PropertyEditor".
    FPropertyEditorModule& propertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));

    // We can create an FDetailsViewArgs object with these parameters.
    // There's a detailed view of what all of these do inside the FDetailsViewArgs object.
    FDetailsViewArgs detailsViewArgs;
    {
        detailsViewArgs.bAllowSearch = true;
        detailsViewArgs.bHideSelectionTip = true;
        detailsViewArgs.bLockable = false;
        detailsViewArgs.bSearchInitialKeyFocus = true;
        detailsViewArgs.bUpdatesFromSelection = false;
        detailsViewArgs.NotifyHook = nullptr;
        detailsViewArgs.bShowOptions = true;
        detailsViewArgs.bShowScrollBar = true;
        detailsViewArgs.bAllowMultipleTopLevelObjects = true;
        detailsViewArgs.NameAreaSettings = FDetailsViewArgs::ObjectsUseNameArea;
    }

    // Now we can create our details view and construct it with the arguments we just created.
    TSharedPtr<IDetailsView> detailsView = propertyEditorModule.CreateDetailView(detailsViewArgs);
    // We then update the object that it reads to the working asset of the editor.

    app->detailsView = detailsView;
    detailsView->SetObject(app->GetWorkingAsset());

    // And now fill it in with a new SVerticalBox!
    return SNew(SVerticalBox)
        + SVerticalBox::Slot()
            .FillHeight(1.0f)
            .HAlign(HAlign_Fill)
            [
                detailsView.ToSharedRef()
            ];
}

// More boilerplate.
FText UpgradeAssetPropertiesTabFactory::GetTabToolTipText(const FWorkflowTabSpawnInfo& info) const {
    return FText::FromString(TEXT("Display properties."));
}