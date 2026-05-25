#include "App/Factories/UpgradeAssetPrimaryTabFactory.h"
#include "Utils/UpgradeAssetDragDrop.h"
#include "Graph/Node/UpgradeAssetGraphNode.h"
#include "UpgradeNodeDataAsset.h"
#include "ScopedTransaction.h"
#include "Framework/Application/SlateApplication.h"


// This is our primary tab, this is where we'll edit using the grid.
UpgradeAssetPrimaryTabFactory::UpgradeAssetPrimaryTabFactory(TSharedPtr<class UpgradeAssetEditorApp> app) : FWorkflowTabFactory(TEXT("UpgradeAssetPrimaryTab"), app) {
    _app = app;
    // Here we set some important defaults, the title, the description and the tooltip. All that really matters are the title and tooltip.
    TabLabel = FText::FromString(TEXT("Upgrade Tree"));
    ViewMenuDescription = FText::FromString(TEXT("This is where you can edit the values for upgrades in the upgrade tree and move them around!"));
    ViewMenuTooltip = FText::FromString(TEXT("Bring up the Upgrade Tree graph."));
}

TSharedRef<SWidget> UpgradeAssetPrimaryTabFactory::CreateTabBody(const FWorkflowTabSpawnInfo& info) const {
    TSharedPtr<UpgradeAssetEditorApp> app = _app.Pin();
    TSharedPtr<SGraphEditor> SpawnedWidget;

    SGraphEditor::FGraphEditorEvents GraphEvents;

    // Bind selection change events.
    GraphEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(
        app.ToSharedRef(),
        &UpgradeAssetEditorApp::OnSelectionChanged
    );

    // Create our graph.
    TSharedRef<SGraphEditor> GraphWidgetRef = SNew(SGraphEditor)
        .GraphToEdit(app->GetWorkingGraph())
        .GraphEvents(GraphEvents);

    SpawnedWidget = GraphWidgetRef;

    // Create our little drop overlay.
    TSharedRef<SUpgradeGraphCanvasDropOverlay> ActiveDropOverlayRef = SNew(SUpgradeGraphCanvasDropOverlay)
        .GraphToEdit(app->GetWorkingGraph())
        .AppContext(_app);

    // Bind the visibility rule to our safe lambda capture to allow grid inputs through. Otherwise we can't do any of the shit we actually need for our tool.
    ActiveDropOverlayRef->SetVisibility(TAttribute<EVisibility>::Create(
        TAttribute<EVisibility>::FGetter::CreateLambda([ActiveDropOverlayRef]() {
            return ActiveDropOverlayRef->GetVisibility();
        })
    ));

    TSharedRef<SWidget> ActiveDropOverlay = ActiveDropOverlayRef;

    TSharedRef<SWidget> WidgetBody = SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .FillHeight(1.0f)
        .HAlign(HAlign_Fill)
        [
            SNew(SOverlay)
            + SOverlay::Slot()
            [
                GraphWidgetRef // The Graph Panel
            ]
            + SOverlay::Slot()
            [
                ActiveDropOverlay // The drag-drop operation handler.
            ]
        ];
    
    // Set our widget in our app.
    app->GraphEditorWidget = SpawnedWidget;
    // Force keyboard focus to be on our widget.
    FSlateApplication::Get().SetKeyboardFocus(SpawnedWidget);
    return WidgetBody;
}

// Boilerplate.
FText UpgradeAssetPrimaryTabFactory::GetTabToolTipText(const FWorkflowTabSpawnInfo& info) const {
    return FText::FromString(TEXT("Bring up the Upgrade Tree graph."));
}