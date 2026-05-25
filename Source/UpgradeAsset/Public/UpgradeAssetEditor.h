#pragma once

#include "CoreMinimal.h"
#include "IAssetTools.h"
#include "AssetToolsModule.h"
#include "Styling/SlateStyle.h"
#include "Object/UpgradeAssetAction.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"
#include "Graph/Node/UpgradeAssetGraphNodeFactory.h"

class FUpgradeAssetEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void RegisterMenuExtensions();
	void OnOpenUpgradeAssetClicked();
	FString FindUpgradeAssetPath() const;

	TSharedPtr<FSlateStyleSet> _styleSet = nullptr;
	TSharedPtr<FUpgradeAssetGraphNodeFactory> NodeFactory = nullptr;
};
