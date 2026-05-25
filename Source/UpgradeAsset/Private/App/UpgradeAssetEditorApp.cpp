#include "App/UpgradeAssetEditorApp.h"
#include "Graph/Node/UpgradeAssetGraphNode.h"
#include "Framework/Commands/GenericCommands.h"
#include "ScopedTransaction.h"
#include "GraphEditor.h"
#include "Editor.h"
#include "UObject/ObjectSaveContext.h"
#include "Editor/EditorEngine.h"

void UpgradeAssetEditorApp::RegisterTabSpawners(const TSharedRef<class FTabManager>& tabManager) {
    FWorkflowCentricApplication::RegisterTabSpawners(tabManager);
}

// We need to initialize the editor, this means opening up the editor window after we clicked on the object in our content browser.
// THE most important function in the plugin.
void UpgradeAssetEditorApp::InitEditor(const EToolkitMode::Type mode, const TSharedPtr<class IToolkitHost>& initToolkitHost, UObject* inObject) {
    TArray<UObject*> objectsToEdit;
    objectsToEdit.Add(inObject);
    
    // This is the most important part of the entire system. This is where we enforce a base class for the graph (UEdGraph for our purposes) and force the Schema (Node draw, context and data handling)
    // It requires an asset to be working on (Our UpgradeTree is our editable asset) and instantiates a graph based on that.
    _workingAsset = Cast<UUpgradeAsset>(inObject);
    _workingGraph = FBlueprintEditorUtils::CreateNewGraph(
        _workingAsset,
        NAME_None,
        UEdGraph::StaticClass(),
        UUpgradeGraphSchema::StaticClass()
    );
    
    // Then we instantiate the editor. This opens up our window using our registered _editorPath. This is our FName& AppIdentifier.
    InitAssetEditor(
        mode,
        initToolkitHost,
        FName(_editorPath),
        FTabManager::FLayout::NullLayout,
        true,
        true,
        objectsToEdit
    );
    
    // Then we force the application mode through our _modePath and create a TSharedPtr<UpgradeAssetAppMode> to handle our Tab Managers and factory content.
    AddApplicationMode(FName(_modePath), MakeShareable(new UpgradeAssetAppMode(SharedThis(this))));
    // Then we force the current mode, there can be more than one mode but for our purposes I believe we only need the one. Unless we want a different type of editor in the future like a Data Table that contains our imported Data Assets.
    SetCurrentMode(FName(_modePath));
    
    UpdateEditorGraphFromWorkingAsset();

    // Create a lambda (It wouldn't work when trying to bind it to a function) to handle updating on PreSave (ctrl+s call)
    TWeakPtr<UpgradeAssetEditorApp> WeakSelf = SharedThis(this);
    _saveDelegateHandle = FCoreUObjectDelegates::OnObjectPreSave.AddLambda([WeakSelf](UObject* InObject, FObjectPreSaveContext SaveContext) {
        if (TSharedPtr<UpgradeAssetEditorApp> SharedSelf = WeakSelf.Pin()) {
            if (InObject && InObject == SharedSelf->GetWorkingAsset()) {
                SharedSelf->UpdateWorkingAssetFromGraph();
            }
        }
    });

    BindGraphCommands();
    GEditor->RegisterForUndo(this);
}

void UpgradeAssetEditorApp::OnClose() {
    GEditor->UnregisterForUndo(this);
    if (_saveDelegateHandle.IsValid()) {
        FCoreUObjectDelegates::OnObjectPreSave.Remove(_saveDelegateHandle);
    }
    UpdateWorkingAssetFromGraph();
    FCoreUObjectDelegates::OnObjectPreSave.RemoveAll(this);
    FAssetEditorToolkit::OnClose();
}

// This is how we save our graph to the asset.
void UpgradeAssetEditorApp::UpdateWorkingAssetFromGraph() {
    // Should there be no asset, nor graph, we cannot save anything.
    if (_workingAsset == nullptr || _workingGraph == nullptr) { return; }
    // Otherwise, we can modify our asset.
    _workingAsset->Modify();

    // If we have no runtime graph (to store the data for further editing) we need to make one.
    if (_workingAsset->_graph == nullptr) {
        _workingAsset->_graph = NewObject<UUpgradeRuntimeGraph>(_workingAsset, TEXT("RuntimeGraph"), RF_Transactional | RF_Public);
    }

    // We'll then "cast" and set the graph to this runtime graph. 
    UUpgradeRuntimeGraph* runtimeGraph = _workingAsset->_graph;
    runtimeGraph->Modify();
    // Empty the graph's existing data.
    runtimeGraph->Nodes.Empty();
    // And the asset's stored node data.
    _workingAsset->NodeData.Empty();

    // Then iterate over our current graph's nodes.
    for (UEdGraphNode* uiNode : _workingGraph->Nodes) {
        // We'll cast to UUpgradeAssetGraphNode to ensure that what we're saving is what we want.
        UUpgradeAssetGraphNode* castNode = Cast<UUpgradeAssetGraphNode>(uiNode);
        if (!castNode) continue;
        // We'll then get the outer that we'll store the node on.
        UObject* NodeOuter = runtimeGraph;
        // We'll make a unique name for the object so it is stored in memory correctly and not overwritten by anything else.
        FName UniqueNodeName = MakeUniqueObjectName(NodeOuter, UUpgradeRuntimeNode::StaticClass(), *castNode->GetNodeId().ToString());
        // Then finally create the node. Store it as RF_Transactional for undo-redo and RF_Public so it's visible.
        UUpgradeRuntimeNode* runtimeNode = NewObject<UUpgradeRuntimeNode>(NodeOuter, UniqueNodeName, RF_Transactional | RF_Public);
        // Then we can FINALLY set the data.
        runtimeNode->position = FVector2D(castNode->NodePosX, castNode->NodePosY);
        runtimeNode->unlocks = castNode->GetNodeUnlocks();
        runtimeNode->nodeId = castNode->GetNodeId();
        runtimeNode->attachedData = castNode->GetAttachedData();
        // And add the runtime node to the graph.
        runtimeGraph->Nodes.Add(runtimeNode);
        // Now we store the node info to access this through blueprints.
        FUpgradeNodeInfo BPNodeInfo;
        BPNodeInfo.Position = FVector2D(castNode->NodePosX, castNode->NodePosY);
        BPNodeInfo.Unlocks = castNode->GetNodeUnlocks();
        BPNodeInfo.NodeId = castNode->GetNodeId();
        BPNodeInfo.attachedData = castNode->GetAttachedData();
        // And add that to our node data array.
        _workingAsset->NodeData.Add(BPNodeInfo);
    }
    // We can now mark it as clean to remove the dirty flag.
    MarkAsClean();
}

void UpgradeAssetEditorApp::UpdateEditorGraphFromWorkingAsset() {
    if (_workingAsset->_graph == nullptr || _workingGraph == nullptr) { return; }
    
    // Here we'll check whether it is dirty, if it was already dirty we wont force a clean. If not, we will.
    const bool bWasInitiallyClean = !_workingAsset->GetPackage()->IsDirty();
    
    // Modify the graph for Transactional handling.
    _workingGraph->Modify();
    
    // For each node, lets update the node data to what it was saved as.
    for (UUpgradeRuntimeNode* runtimeNode : _workingAsset->_graph->Nodes) {
        if (!runtimeNode) continue;

        // Lets create a new node with no name and set it to be RF_Transactional so that we can track undo/redo
        UUpgradeAssetGraphNode* newNode = NewObject<UUpgradeAssetGraphNode>(_workingGraph, UUpgradeAssetGraphNode::StaticClass(), NAME_None, RF_Transactional);
        
        // And set the positions.
        newNode->NodePosX = runtimeNode->position.X;
        newNode->NodePosY = runtimeNode->position.Y;
        newNode->SetNodeId(runtimeNode->nodeId);
        if (runtimeNode->attachedData) { 
            newNode->_attachedData = runtimeNode->attachedData;
        }
        

        // Bind our delegates. This is important for old assets being reminded that they can be updated.
        if (newNode->GetAttachedData()) {
            newNode->GetAttachedData()->OnIconChanged.RemoveDynamic(newNode, &UUpgradeAssetGraphNode::HandleIconChanged);
            newNode->GetAttachedData()->OnIconChanged.AddDynamic(newNode, &UUpgradeAssetGraphNode::HandleIconChanged);
            newNode->GetAttachedData()->OnBorderChanged.RemoveDynamic(newNode, &UUpgradeAssetGraphNode::HandleBorderChanged);
            newNode->GetAttachedData()->OnBorderChanged.AddDynamic(newNode, &UUpgradeAssetGraphNode::HandleBorderChanged);
        }

        // Force connections through this method in order to notify changes (This allows the engine to keep up and store the previously mentioned undo/redo Transactions)
        for (FGuid unlock : runtimeNode->unlocks) {
            newNode->AddNewUnlockConnection(unlock);
        }
        
        // Add the node to the graph
        _workingGraph->AddNode(newNode, true, true);
    }
    // Mark the node as clean since we're just loading data, but some of these functions would actually mark us as dirty.
    if (bWasInitiallyClean)
    {
        MarkAsClean();
    }
}

// We bind these commands to allow bulk deletion.
void UpgradeAssetEditorApp::BindGraphCommands() const {
    FGenericCommands::Register();

    ToolkitCommands->MapAction(
        FGenericCommands::Get().Delete,
        FExecuteAction::CreateSP(this, &UpgradeAssetEditorApp::DeleteSelectedNodes),
        FCanExecuteAction::CreateSP(this, &UpgradeAssetEditorApp::CanDeleteSelectedNodes)
    );
}

// Perform our bulk deletion.
void UpgradeAssetEditorApp::DeleteSelectedNodes() const {
    if (!_workingGraph || !_workingAsset) { return; }
    if (!GraphEditorWidget.IsValid()) { 
        UE_LOG(LogTemp, Warning, TEXT("GraphEditorWidget is not valid."));
        return; }

    // We get all of the selected nodes from the widget (this is stored in the Slate widget rather than the underlying graph that stores the data.)
    FGraphPanelSelectionSet SelectedNodes = GraphEditorWidget->GetSelectedNodes();
    // And we create a transaction.
    const FScopedTransaction Transaction(FGenericCommands::Get().Delete->GetDescription());
    // We then modify the graph and the asset.
    _workingGraph->Modify();
    _workingAsset->Modify();
    // And delete the nodes using the node's custom UUpgradeAssetGraphNode::DeleteNode() function.
    for (UObject* NodeObj : SelectedNodes) {
        UUpgradeAssetGraphNode* node = Cast<UUpgradeAssetGraphNode>(NodeObj);
        if (node && node->CanUserDeleteNode()) {
            node->DeleteNode();
        }
    }
    // Then we invalidate our selection and change the graph.
    GraphEditorWidget->ClearSelectionSet();
    GraphEditorWidget->NotifyGraphChanged();
}

// We'll just ensure that all nodes can be deleted at all times.
bool UpgradeAssetEditorApp::CanDeleteSelectedNodes() const {
    return true;
}

// Undo and redo handling, just to refresh the graph.
void UpgradeAssetEditorApp::PostUndo(bool bSuccess) {
    if (bSuccess) {
        if (_workingGraph) {
            _workingGraph->NotifyGraphChanged();
        }
        RefreshGraphUI();
    }
}

void UpgradeAssetEditorApp::PostRedo(bool bSuccess) {
    if (bSuccess) {
        if (_workingGraph) {
            _workingGraph->NotifyGraphChanged();
        }
        RefreshGraphUI();
    }
}

// And here the refresh occurs.
void UpgradeAssetEditorApp::RefreshGraphUI() {
    if (!GraphEditorWidget.IsValid()) { return; }
    GraphEditorWidget->ClearSelectionSet();
    GraphEditorWidget->NotifyGraphChanged();
}

// When we change our selection, we want to update the details panel to match the object(s) selected.
void UpgradeAssetEditorApp::OnSelectionChanged(const TSet<UObject*>& NewSelection) 
{
    // So firstly we'll ensure there is a details view.
    if (!detailsView.IsValid()) { 
        return; 
    }
    // If we have a selection greater than 0, we'll iterate over and check the selected nodes.
    if (NewSelection.Num() > 0) {
        // Should it be an object that we want, we'll access it through a const iterator. (Stores the value each iteration)
        UObject* selectedObject = *NewSelection.CreateConstIterator();
        // We'll then validate to see if it's a graph node.
        if (UUpgradeAssetGraphNode* graphNode = Cast<UUpgradeAssetGraphNode>(selectedObject)) {
            // Should it be, we'll store the target asset as our attached data.
            UObject* targetAsset = graphNode->GetAttachedData(); 

            if (targetAsset) {
                // Force clear the details view, then create an array. This being the node and the attachedData on the node.
                detailsView->SetObject(nullptr); 
                TArray<UObject*> ObjectsToView;
                ObjectsToView.Add(graphNode);
                ObjectsToView.Add(targetAsset);
                // We'll then set the objects in the details view to these objects.
                detailsView->SetObjects(ObjectsToView, true);
                // And add a lambda to update the details view when the data has changed.
                graphNode->OnAttachedDataChanged.AddLambda([this](UUpgradeAssetGraphNode* graphNode, UUpgradeNodeDataAsset* newAsset) {
                    TArray<UObject*> ObjectsToView;

                    ObjectsToView.Add(graphNode);
                    ObjectsToView.Add(newAsset);
                    detailsView->SetObjects(ObjectsToView, true);
                });
            }
            else {
                // Otherwise it's just going to be the data of the node, regardless of type.
                detailsView->SetObject(graphNode, true);
            }
        }
    }
    // If nothing is selected, we'll just clear the details panel.
    else {
        detailsView->SetObject(nullptr, true);
    }
}

// We want to sometimes force our editor to mark as clean when no changes have been made, despite some functions practically being read only, some changes will mark the graph as dirty.
void UpgradeAssetEditorApp::MarkAsClean() {
    // Therefore we get the outermost part of the asset and remove the dirty flag and broadcast the state change to update the UI.
    UPackage* AssetPackage = _workingAsset->GetOutermost();
    if (AssetPackage)
    {
        AssetPackage->SetDirtyFlag(false);
        AssetPackage->PackageDirtyStateChangedEvent.Broadcast(AssetPackage);
    }
}

