#pragma once

#include "CoreMinimal.h"
#include "UpgradeAsset.h"
#include "UpgradeAssetFactory.generated.h"

UCLASS()
class UUpgradeAssetFactory : public UFactory {
    GENERATED_BODY()

public:
    UUpgradeAssetFactory(const FObjectInitializer& objectInitializer);

public:
    virtual UObject* FactoryCreateNew(UClass* uclass, UObject* inParent, FName name, EObjectFlags flags, UObject* context, FFeedbackContext* warn);
    virtual bool CanCreateNew() const override;

};