#include "Object/UpgradeAssetAction.h"

// An action is how we register with the engine what to do when we double click on a UObject, which our UUpgradeAsset inherits from.
UpgradeAssetAction::UpgradeAssetAction(EAssetTypeCategories::Type category){
       _assetCategory = category;
}

// This defines the name that the asset should be given, this is the subscript when you're viewing an asset that usually says something like "Blueprint", "Blueprint Interface", "Static Mesh", or "Skeletal Mesh" as examples.
FText UpgradeAssetAction::GetName() const {
    return FText::FromString(TEXT("Upgrade Tree"));
}

// Here we can define the colour of our custom asset, this will allow us to make the asset stand out in the editor.
FColor UpgradeAssetAction::GetTypeColor() const {
    return FColor::Red;
}

// This is how we define which class we want to edit. In our case it's UUpgradeAsset. And therefore this action is then attached to that asset.
UClass* UpgradeAssetAction::GetSupportedClass() const {
    return UUpgradeAsset::StaticClass();
}

// Now we want to open the editor, because we've double clicked on the asset.
void UpgradeAssetAction::OpenAssetEditor(const TArray<UObject*>& inObjects, TSharedPtr<class IToolkitHost> editWithinLevelEditor) {
    // First thing's first, we change the EToolkitMode. This changes whether its hould be WorldCentric or Standalone.
    EToolkitMode::Type mode = editWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;
    // Now we iterate over every object that we've "opened." We only really want to edit one Upgrade Tree at a time, but this allows us to handle multiple. And is basically a requirement.
    for (UObject* object : inObjects) {
        // Then we need to cast it to UUpgradeAsset to ensure that it is valid.
        UUpgradeAsset* upgradeAsset = Cast<UUpgradeAsset>(object);
        // If it is
        if (upgradeAsset != nullptr) {
            // We'll create a new "app" for each asset that we have opened.
            TSharedRef<UpgradeAssetEditorApp> editor(new UpgradeAssetEditorApp());
            // And then initialize the editor in our editorApp.
            editor->InitEditor(mode, editWithinLevelEditor, upgradeAsset);
        }
    }
}

// Boilerplate.
uint32 UpgradeAssetAction::GetCategories() {
    return _assetCategory;
}
