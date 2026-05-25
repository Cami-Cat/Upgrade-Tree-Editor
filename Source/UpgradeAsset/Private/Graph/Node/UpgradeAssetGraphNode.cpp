#include "ToolMenu.h"
#include "SGraphNode.h"
#include "EdGraph/EdGraph.h"
#include "Types/SlateEnums.h"
#include "ScopedTransaction.h"
#include "Editor.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "App/UpgradeAssetEditorApp.h"
#include "Framework/Commands/UIAction.h"
#include "Graph/Node/UpgradeAssetGraphNode.h"

// Initialize the node with default attached data rather than a nullptr.
UUpgradeAssetGraphNode::UUpgradeAssetGraphNode(const FObjectInitializer& ObjectInitializer) 
: Super(ObjectInitializer) {
    _attachedData = ObjectInitializer.CreateDefaultSubobject<UUpgradeNodeDataAsset>(this, TEXT("AttachedDataAsset"));
}
// On changing a property, check which property was changed and then broadcast this change so that the details viewer can update itself.
void UUpgradeAssetGraphNode::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{    
    // Run the super of the property first.
    Super::PostEditChangeProperty(PropertyChangedEvent);
    // Check the property name.
    const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
    // Validate that it is _attachedData
    if (PropertyName == GET_MEMBER_NAME_CHECKED(UUpgradeAssetGraphNode, _attachedData))
    {
        if (GetGraph())
        {
            // Then update.
            GetGraph()->NotifyNodeChanged(this);
            GetGraph()->NotifyGraphChanged();
            OnAttachedDataChanged.Broadcast(this, _attachedData);
        }
    }
}
// Allocate the default input and output pins. We'll destroy these anyway when we handle updating the node in our Slate style. But it's important to have a backup!
// Because not having any pins when calling a function later will cause an instantaneous crash.
void UUpgradeAssetGraphNode::AllocateDefaultPins() {
    ValidateGuid();
    CreatePin(EGPD_Input, FName("UpgradePinIn"), FName("InputLink"));
    CreatePin(EGPD_Output, FName("UpgradePinOut"), FName("outputLink"));
}

// Force a new GUID, if we have a nodeId already, we can skip that, otherwise we set it to our new GUID.
void UUpgradeAssetGraphNode::ValidateGuid() {
    CreateNewGuid();
    if (!_nodeId.IsValid()) { SetNodeId(NodeGuid); }
}

// Getters and setters with some simple functions.
FGuid UUpgradeAssetGraphNode::GetNodeId() { return _nodeId; }
void UUpgradeAssetGraphNode::SetNodeId(FGuid newNodeId) { _nodeId = newNodeId; }
const TArray<FGuid>& UUpgradeAssetGraphNode::GetNodeUnlocks() { return _unlocks; }
UUpgradeNodeDataAsset* UUpgradeAssetGraphNode::GetAttachedData() { return _attachedData; }
void UUpgradeAssetGraphNode::SetAttachedData(UUpgradeNodeDataAsset* newData) {
    _attachedData = newData;
    OnAttachedDataChanged.Broadcast(this, newData);
}
UObject* UUpgradeAssetGraphNode::GetRuntimeNode() { return _runtimeNode; }
void UUpgradeAssetGraphNode::SetRuntimeNode(UObject* runtimeNodeReference) { _runtimeNode = runtimeNodeReference; }

// Delegate handling for changing the icon and borders.
void UUpgradeAssetGraphNode::HandleIconChanged(UTexture2D* NewIcon) {
    if (GetGraph()) { GetGraph()->NotifyNodeChanged(this); }
}

void UUpgradeAssetGraphNode::HandleBorderChanged(UTexture2D* NewBorder) {
    if (GetGraph()) { GetGraph()->NotifyNodeChanged(this); }
}

// Add a new (validated) connection to the node.
void UUpgradeAssetGraphNode::AddNewUnlockConnection(FGuid nodeId) {
    // Firstly, prevent adding duplicates.
    if (GetNodeUnlocks().Contains(nodeId)) { return; }
    // Modify the graph.
    if (GetGraph()) { GetGraph()->Modify(); }
    this->Modify();
    
    // Iterate over all of the nodes, find the node of the GUID that you're looking for.
    for (UEdGraphNode* node : this->GetGraph()->Nodes) {
        UUpgradeAssetGraphNode* castNode = Cast<UUpgradeAssetGraphNode>(node);
        if (!castNode || castNode->GetNodeId() != nodeId) { continue; }
        // If it is equal but the other node contains your GUID, return.
        if (castNode->GetNodeId() == nodeId && castNode->GetNodeUnlocks().Contains(GetNodeId())) { return; }
    }
    // Otherwise, add it to our unlocks (Drawn in SUpgradeAssetGraphNode)
    _unlocks.Add(nodeId);
    // Notify the graph that it has changed.
    this->GetGraph()->NotifyGraphChanged(); 
}

// Remove a single connection.
bool UUpgradeAssetGraphNode::RemoveUnlockConnection(FGuid nodeId) {
    // If this isn't even a connection, ignore it.
    if (!_unlocks.Contains(nodeId)) { return false; }
    // Modify.
    if (GetGraph()) { GetGraph()->Modify(); }
    this->Modify();
    // Remove it from the list of unlocks.
    _unlocks.Remove(nodeId);
    // Notify graph change.
    this->GetGraph()->NotifyGraphChanged();
    return true;
}
// Remove every single connection coming to and from this node.
bool UUpgradeAssetGraphNode::RemoveAllUnlockConnections() {
    if (GetGraph()) { GetGraph()->Modify(); }
    this->Modify();
    // Store whether any connections are changed.
    bool bAnyConnectionsChanged = false;
    
    // Store our GUID.
    const FGuid MyNodeId = GetNodeId();

    // If another node has us as a connection, we need to remove it.
    for (UEdGraphNode* Node : this->GetGraph()->Nodes) {
        UUpgradeAssetGraphNode* castNode = Cast<UUpgradeAssetGraphNode>(Node);
        if (!castNode || castNode == this) { continue; }
        // Modify the node that we're changing.
        castNode->Modify();
        // Remove us from that node's connections.
        if (castNode->GetNodeUnlocks().Contains(MyNodeId)) {
            castNode->_unlocks.Remove(MyNodeId);
            bAnyConnectionsChanged = true;
        }
    }

    // Now we clear our own array.
    if (GetNodeUnlocks().Num() > 0) {
        _unlocks.Empty();
        bAnyConnectionsChanged = true;
    }

    // Then we can tell the graph that we've had our changes.
    if (bAnyConnectionsChanged && GetGraph()) {
        this->GetGraph()->NotifyGraphChanged();
    }
    // And return if any changes have been made.
    return bAnyConnectionsChanged;
}

void UUpgradeAssetGraphNode::GetNodeContextMenuActions(UToolMenu* menu, class UGraphNodeContextMenuContext* context) const {
    FToolMenuSection& section = menu->AddSection(TEXT("Section"), FText::FromString(TEXT("Node Actions")));
    
    UUpgradeAssetGraphNode* NonConstMutableNode = const_cast<UUpgradeAssetGraphNode*>(this);
    
    // Here we'll get the active editor app that we're currently working on, using some absolutely fucked up shit.
    UpgradeAssetEditorApp* ActiveEditorApp = nullptr;
    if (GEditor) {
        UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
        if (AssetEditorSubsystem) {
            TArray<UObject*> OpenAssets = AssetEditorSubsystem->GetAllEditedAssets();
            for (UObject* Asset : OpenAssets) {
                if (Asset && Asset->GetClass()->GetName() == TEXT("UpgradeAsset")) {
                    IAssetEditorInstance* AssetEditor = AssetEditorSubsystem->FindEditorForAsset(Asset, false);
                    if (AssetEditor) {
                        // Basically, a shit ton of finding and now we can cast down to our editor app class.
                        ActiveEditorApp = static_cast<UpgradeAssetEditorApp*>(AssetEditor);
                        break;
                    }
                }
            }
        }
    }
    // Now we'll actually add the context menu actions.
    section.AddMenuEntry(
        TEXT("DeleteNode"),
        FText::FromString(TEXT("Delete Node")),
        FText::FromString(TEXT("Deletes the node from the tree.")),
        FSlateIcon(TEXT("UpgradeAssetEditorStyleSet"), TEXT("UpgradeNode.Delete")),
        // We don't really need to make a function for these, so we'll just make a lambda.
        FUIAction(FExecuteAction::CreateLambda(
            [NonConstMutableNode, ActiveEditorApp] () {
                if (NonConstMutableNode) {
                    const FScopedTransaction Transaction(FText::FromString(TEXT("Delete Node")));
                    // Modify the app so that it's snapshotted for our transaction.
                    if (ActiveEditorApp && ActiveEditorApp->GetWorkingAsset()) 
                    { 
                        ActiveEditorApp->GetWorkingAsset()->Modify(); 
                    }
                    // Now we delete the node after it's been stored.
                    NonConstMutableNode->DeleteNode();
                    // Then push this change.
                    if (ActiveEditorApp)
                    {
                        ActiveEditorApp->UpdateWorkingAssetFromGraph();
                    }
                }
            }
        ))
    );
    // And we basically do the same for our remove pins context.
    section.AddMenuEntry(
        TEXT("RemoveAllNodePins"),
        FText::FromString(TEXT("Disconnect Node Pins")),
        FText::FromString(TEXT("Disconnects all node pins to and from this node.")),
        FSlateIcon(TEXT("UpgradeAssetEditorStyleSet"), TEXT("UpgradeNode.Delete")),
        FUIAction(FExecuteAction::CreateLambda(
            [NonConstMutableNode, ActiveEditorApp] () {
                if (NonConstMutableNode) {
                    const FScopedTransaction Transaction(FText::FromString(TEXT("Remove all node pins")));
                    
                    if (ActiveEditorApp && ActiveEditorApp->GetWorkingAsset()) 
                    { 
                        ActiveEditorApp->GetWorkingAsset()->Modify(); 
                    }

                    NonConstMutableNode->RemoveAllUnlockConnections();

                    if (ActiveEditorApp)
                    {
                        ActiveEditorApp->UpdateWorkingAssetFromGraph();
                    }
                }
            }
        ))
    );
}

void UUpgradeAssetGraphNode::DeleteNode() {
    if (GetGraph()) { GetGraph()->Modify(); }    
    RemoveAllUnlockConnections();
    GetGraph()->RemoveNode(this);
}

// Find a specific node with a supplied GUID
UUpgradeAssetGraphNode* UUpgradeAssetGraphNode::GetNodeWithGUID(FGuid guid) const {
    UUpgradeAssetGraphNode* _node = nullptr;

    for (UEdGraphNode* node : GetGraph()->Nodes) {
        if (node == this || !node) { continue; }
        UUpgradeAssetGraphNode* castNode = Cast<UUpgradeAssetGraphNode>(node);
        if (!castNode) { 
            continue; 
        }
        if (castNode->_nodeId == guid) {
            return castNode;
        }
    }
    return _node;
}
