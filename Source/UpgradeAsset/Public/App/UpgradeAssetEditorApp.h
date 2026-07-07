#pragma once

#include "CoreMinimal.h"
#include "UpgradeAsset.h"
#include "Graph/UpgradeGraphSchema.h"
#include "App/UpgradeAssetAppMode.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "WorkflowOrientedApp/WorkflowCentricApplication.h"

class UpgradeAssetEditorApp : public FWorkflowCentricApplication, public FEditorUndoClient, public FNotifyHook {
    friend class UUpgradeAssetGraphNode;
public:
    void RegisterTabSpawners(const TSharedRef<class FTabManager>& tabManager) override;
    void InitEditor(const EToolkitMode::Type mode, const TSharedPtr<class IToolkitHost>& initToolkitHost, UObject* inObject);

    class UUpgradeAsset* GetWorkingAsset() { return _workingAsset; }
    class UEdGraph* GetWorkingGraph() { return _workingGraph; }

public:
    virtual FName GetToolkitFName() const override { return FName(_appPath); }
    virtual FText GetBaseToolkitName() const override { return FText::FromString(_appPath); }
    virtual FString GetWorldCentricTabPrefix() const override { return _appPath; }
    virtual FLinearColor GetWorldCentricTabColorScale() const override { return FLinearColor(0.3f, 0.2f, 0.5f, 0.5f); }
    virtual FString GetDocumentationLink() const override { return _docPath; }

    virtual void BindGraphCommands() const;
    virtual void DeleteSelectedNodes() const;
    virtual bool CanDeleteSelectedNodes() const;
    virtual void CopySelectedNodes() const;
    virtual bool CanCopyNodes() const;
    virtual void CutSelectedNodes() const;
    virtual bool CanCutNodes() const;
    virtual void PasteNodes() const;
    virtual bool CanPasteNodes() const;

    virtual void PostUndo(bool bSuccess) override;
    virtual void PostRedo(bool bSuccess) override;
    
    virtual void OnToolkitHostingStarted(const TSharedRef<class IToolkit>& toolkit) override {}
    virtual void OnToolkitHostingFinished(const TSharedRef<class IToolkit>& toolkit) override {}
    virtual void OnClose() override;
    void MarkAsClean();
    
    void OnSelectionChanged(const TSet<UObject*>& newSelection);
    void OnPreSaveAsset(UObject* InObject, FObjectPreSaveContext SaveContext);
    
    UPROPERTY()
    TSharedPtr<SGraphEditor> GraphEditorWidget;
    
    TSharedPtr<IDetailsView> detailsView;
protected:
    void UpdateWorkingAssetFromGraph();
    void UpdateEditorGraphFromWorkingAsset();
    FDelegateHandle _saveDelegateHandle;
    
private:
    UPROPERTY()
    class UUpgradeAsset* _workingAsset = nullptr;
    
    UPROPERTY()
    class UEdGraph* _workingGraph = nullptr;
    
    FDelegateHandle _graphChangeListenerHandler;


    void RefreshGraphUI();
public:

    bool bIsRebuildingGraph = false;
    static inline FString _appPath = TEXT("UpgradeEditorApp");
    static inline FString _editorPath = TEXT("UpgradeAssetEditor");
    static inline FString _modePath = TEXT("UpgradeAssetAppMode");
    static inline FString _docPath = TEXT("TODO : Add Github Documentation Path");
};