#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UpgradeNodeStyleDataAsset.generated.h"

UCLASS(Blueprintable, BlueprintType)
class UPGRADEASSETRUNTIME_API UUpgradeNodeStlyeDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Node|Colour")
	FLinearColor node_colour_purchased;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Node|Colour")
	FLinearColor node_colour_afford;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Node|Colour")
	FLinearColor node_colour_unafford;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Node|Colour")
	FLinearColor node_colour_outOfReach;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Node|Line")
	TObjectPtr<UMaterialInterface> node_line_texture_dashed;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Node|Line")
	TObjectPtr<UTexture2D> node_line_texture_default;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Node|Sounds")
	TObjectPtr<USoundBase> node_sound_purchase;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Node|Sounds")
	TObjectPtr<USoundBase> node_sound_purchase_full;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Node|Sounds")
	TObjectPtr<USoundBase> node_sound_hover;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Node|Sounds")
	TObjectPtr<USoundBase> node_sound_error;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Background|Texture")
	TObjectPtr<UTexture2D> background_texture;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Node|Style")
	double specialNode_size;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Node|Style")
	bool NoOutOfReach;
};
