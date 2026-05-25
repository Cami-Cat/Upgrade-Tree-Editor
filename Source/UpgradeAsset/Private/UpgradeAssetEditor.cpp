#include "UpgradeAssetEditor.h"
#include "LevelEditor.h"
#include "ToolMenus.h"
#include "Editor.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "FUpgradeAssetEditorModule"

void FUpgradeAssetEditorModule::StartupModule()
{
	// Define registry objects. Specifically Factories and Actions.
	IAssetTools& assetToolsModule = IAssetTools::Get();
	EAssetTypeCategories::Type assetType = assetToolsModule.RegisterAdvancedAssetCategory(FName(TEXT("UpgradeAssets")), FText::FromString("Upgrade Assets"));	
	TSharedPtr<UpgradeAssetAction> upgradeAssetAction = MakeShareable(new UpgradeAssetAction(assetType));
    
	assetToolsModule.RegisterAssetTypeActions(upgradeAssetAction.ToSharedRef());

	NodeFactory = MakeShareable(new FUpgradeAssetGraphNodeFactory());
	FEdGraphUtilities::RegisterVisualNodeFactory(NodeFactory);

    // Here we can create our own styleset.
	_styleSet = MakeShareable(new FSlateStyleSet(TEXT("UpgradeAssetEditorStyleSet")));
	TSharedPtr<IPlugin> plugin = IPluginManager::Get().FindPlugin("UpgradeAsset");
	// And force a content directory.
    FString contentDir = plugin->GetContentDir();
	_styleSet->SetContentRoot(contentDir);

    // Now we can define our images.
	FSlateImageBrush* thumbnailBrush = new FSlateImageBrush(_styleSet->RootToContentDir(TEXT("UpgradeTreeIcon"), TEXT(".png")), FVector2D(128.0, 128.0));
	FSlateImageBrush* iconBrush = new FSlateImageBrush(_styleSet->RootToContentDir(TEXT("UpgradeTreeIcon"), TEXT(".png")), FVector2D(128.0, 128.0));
	FSlateImageBrush* deleteBrush = new FSlateImageBrush(_styleSet->RootToContentDir(TEXT("DeleteIcon"), TEXT(".png")), FVector2D(128.0, 128.0));
    // And set our path.
    _styleSet->Set(TEXT("ClassThumbnail.UpgradeAsset"), thumbnailBrush);
	_styleSet->Set(TEXT("ClassIcon.UpgradeAsset"), iconBrush);
	_styleSet->Set(TEXT("UpgradeNode.Delete"), deleteBrush);
    // Then register our styleset!
    FSlateStyleRegistry::RegisterSlateStyle(*_styleSet);
    // We'll also now register a menu extension, this adds our button. Yippee.
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FUpgradeAssetEditorModule::RegisterMenuExtensions));
}

// Now we need to clear some things.
void FUpgradeAssetEditorModule::ShutdownModule()
{
    UToolMenus::UnRegisterStartupCallback(this);
    UToolMenus::UnregisterOwner(this);

    FSlateStyleRegistry::UnRegisterSlateStyle(*_styleSet);
    if (NodeFactory.IsValid())
    {
        FEdGraphUtilities::UnregisterVisualNodeFactory(NodeFactory);
    }
}

// Now we can register our menu button extension.
void FUpgradeAssetEditorModule::RegisterMenuExtensions()
{
    // Scope the tool menu.
    FToolMenuOwnerScoped OwnerScoped(this);
    // Find the viewport's toolbar.
    UToolMenu* ToolBar = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.User");
    if (!ToolBar) return;
    // Then we add a new section to the toolbar.
    FToolMenuSection& Section = ToolBar->FindOrAddSection("UpgradeTools");
    // Create a new entry to add to the toolbar.
    FToolMenuEntry Entry = FToolMenuEntry::InitToolBarButton(
        "OpenUpgradeTreeButton",
        // Attach the FExecuteAction to our OnUpgradeAssetClicked function.
        FUIAction(FExecuteAction::CreateRaw(this, &FUpgradeAssetEditorModule::OnOpenUpgradeAssetClicked)),
        LOCTEXT("OpenUpgradeTreeLabel", "Upgrade Tree"),
        LOCTEXT("OpenUpgradeTreeTooltip", "Open the Upgrade Tree Asset Editor"),
        FSlateIcon("UpgradeAssetEditorStyleSet", "ClassIcon.UpgradeAsset") 
    );
    // Now add our button to the section.
    Section.AddEntry(Entry);
}

// On click we can find our asset path.
void FUpgradeAssetEditorModule::OnOpenUpgradeAssetClicked()
{
    // We'll call FUpgradeAssetEditorModule::FindUpgradeAssetPath() to find our asset path. This is specified by the code, rather than the end-user. But I can maybe change this later, not that it should matter when there's a single tree usually.
    FString AssetPath = FindUpgradeAssetPath();
    // If the path is invalid and we couldn't find one, we'll end it here and cause an error.
    if (AssetPath.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("No UpgradeAsset found in the project content directory."));
        return;
    }
    // Otherwise, we'll load our object using our AssetEditorSubsystem, which will force our App to create our asset editor.
    UObject* TargetAsset = StaticLoadObject(UObject::StaticClass(), nullptr, *AssetPath);
    if (TargetAsset)
    {
        UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
        if (AssetEditorSubsystem)
        {
            AssetEditorSubsystem->OpenEditorForAsset(TargetAsset);
        }
    }
}

// And this finds the asset at our specified path!
FString FUpgradeAssetEditorModule::FindUpgradeAssetPath() const
{
    // We'll find our asset registry module, this is necessary to look through our Content Browser.
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    // And we'll create an asset data list.
    TArray<FAssetData> AssetDataList;
    // We'll then create an asset path.
    FTopLevelAssetPath UpgradeAssetClassPath(TEXT("/Script/UpgradeAssetRuntime"), TEXT("UpgradeAsset"));
    // We create a filter, this forces us to look specifically in one folder and in our case, we're looking in the Tree folder.
    FARFilter Filter;
    Filter.ClassPaths.Add(UpgradeAssetClassPath);
    Filter.PackagePaths.Add(TEXT("/UpgradeAsset/Tree"));
    // We also allow recursive paths (paths underneath.)
    Filter.bRecursivePaths = true;
    // Then we get every single asset within the filtered list.
    AssetRegistryModule.Get().GetAssets(Filter, AssetDataList);
    if (AssetDataList.Num() > 0)
    {
        // Now we create a notification
        FText ErrorText = FText::FromString(TEXT("Found upgrade tree data!"));
        FNotificationInfo Info(ErrorText);
       
        Info.ExpireDuration = 5.0f;
        Info.bFireAndForget = true;
        Info.bUseThrobber = false;
        
        TSharedPtr<SNotificationItem> NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);
        if (NotificationItem.IsValid())
        {
            // And we send our notification/
            NotificationItem->SetCompletionState(SNotificationItem::CS_Success);
        }
        // Then we can return the asset path.
        return AssetDataList[0].GetObjectPathString();
    }
    // Otherwise, whoopsies, nothing's there.
    FText ErrorText = FText::FromString(TEXT("Upgrade Tree data not found!"));
    FNotificationInfo Info(ErrorText);
    
    Info.ExpireDuration = 3.0f;
    Info.bFireAndForget = true;
    Info.bUseThrobber = false;
    
    TSharedPtr<SNotificationItem> NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);
    if (NotificationItem.IsValid())
    {
        NotificationItem->SetCompletionState(SNotificationItem::CS_Fail);
    }

    return FString();
}

#undef LOCTEXT_NAMESPACE
IMPLEMENT_MODULE(FUpgradeAssetEditorModule, UpgradeAssetEditor)