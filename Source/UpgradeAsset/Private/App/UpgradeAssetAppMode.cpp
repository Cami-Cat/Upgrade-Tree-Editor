#include "App/UpgradeAssetAppMode.h"

// This is how we handle our tab factories and actually open the menu as we would like!
UpgradeAssetAppMode::UpgradeAssetAppMode(TSharedPtr<class UpgradeAssetEditorApp> app) : FApplicationMode(FName(UpgradeAssetEditorApp::_modePath)) {
    _app = app;
    // First things first, we need to register our tab factories. We have two in our case, Propeties and Primary.
    // Primary is our grid, this is going to take up most of the screen and therefore should have the larger share.
    // Properties hold our details view, this uses the default Unreal Engine properties viewer. This is how we're going to edit our data assets in the menu!
    _tabs.RegisterFactory(MakeShareable(new UpgradeAssetPrimaryTabFactory(app)));
    _tabs.RegisterFactory(MakeShareable(new UpgradeAssetPropertiesTabFactory(app)));

    // So we need to create a tab layout in our tab manager. This can be named whatever we would like, but for development we should name it something like "v1"
    TabLayout = FTabManager::NewLayout("UpgradeAssetAppMode_Layout_v1")
    // Now we add an area.
    ->AddArea(
        FTabManager::NewPrimaryArea()
            // This is a vertically spliit area.
            ->SetOrientation(Orient_Vertical)
            // Split, in Slate syntax, adds a child to the area, box, panel, whatever.
            ->Split
            (
                // Splitter is then what handles the customizable split between whatever tabs we have inside. Since they're tabs, they can be rearranged, but that's not important here.
                FTabManager::NewSplitter()
                    // So we orient this horizontally and split it along that way. Adding two children stacks that contain our Tab Factories.
                    ->SetOrientation(Orient_Horizontal)
                    ->Split
                    (
                        FTabManager::NewStack()
                            // Here is our largest tab, our primary grid tab.
                            ->SetSizeCoefficient(0.75)
                            // We need to set every tab to opened if we want the tab to display on opening.
                            ->AddTab(FName(TEXT("UpgradeAssetPrimaryTab")), ETabState::OpenedTab)
                    )
                    ->Split
                    (
                        FTabManager::NewStack()
                            // Here is our other tab, our properties tab. This can be made to have a smaller share, but the user can configure this as they like.
                            ->SetSizeCoefficient(0.25)
                            ->AddTab(FName(TEXT("UpgradeAssetPropertiesTab")), ETabState::OpenedTab)
                    )
            )
    );
}


// We need a way to register our tab factories.
void UpgradeAssetAppMode::RegisterTabFactories(TSharedPtr<class FTabManager> inTabManager) {
    // Since we currently hold a WeakPtr to our application, we need to turn it into a shared pointer for this scope by pinning it.
    TSharedPtr<UpgradeAssetEditorApp> app = _app.Pin();
    // We then push the tab factories to the application.
    // What Push Tab Factories does is register a tab spawner using our _tabs property. Which is an FWorkflowAllowedTabSet. Set in our constructor.
    app->PushTabFactories(_tabs);
    // Then our inTabManager is registered in the superceded class.
    FApplicationMode::RegisterTabFactories(inTabManager);
}

// Then we supercede these functions below. But we can add custom pre deactivation and post activation functionality here.
void UpgradeAssetAppMode::PreDeactivateMode() {
    FApplicationMode::PreDeactivateMode();
}

void UpgradeAssetAppMode::PostActivateMode() {
    FApplicationMode::PostActivateMode();
}