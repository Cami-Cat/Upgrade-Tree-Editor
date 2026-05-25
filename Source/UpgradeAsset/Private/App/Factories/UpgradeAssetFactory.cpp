#include "App/Factories/UpgradeAssetFactory.h"

// Factories are the essence of these plugins. This is what tells the editor that UUpgradeAsset is the object that we want to create. Registering the supported class.
UUpgradeAssetFactory::UUpgradeAssetFactory(const FObjectInitializer& objectInitializer) : Super(objectInitializer) {
    SupportedClass = UUpgradeAsset::StaticClass();
}
// This here creates a member of our supported class, UUpgradeAsset.
UObject* UUpgradeAssetFactory::FactoryCreateNew(UClass* uclass, UObject* inParent, FName name, EObjectFlags flags, UObject* context, FFeedbackContext* warn) {
    UUpgradeAsset* asset = NewObject<UUpgradeAsset>(inParent, name, flags);
    return asset;
}
// Here we would usually add some extra functionality, for example however many assets we might want a maximum of, or if we even *want* more to be created. Since we can just update this once one has been made.
// But, we'll just keep it on true for now.
bool UUpgradeAssetFactory::CanCreateNew() const {
    return true;
}