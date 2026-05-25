#include "Graph/UpgradeGraphSchema.h"

// Force our drawing policy to be used by returning a constructed version of it with the same arguments.
// This ensures that our Drawing Policy is the one used in our grid editor.
FConnectionDrawingPolicy* UUpgradeGraphSchema::CreateConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements, UEdGraph* InGraphObj) const
{
    return new FUpgradeConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, InZoomFactor, InClippingRect, InDrawElements, InGraphObj);
}

// We need to determine if a connection can be made.
const FPinConnectionResponse UUpgradeGraphSchema::CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const
{
    // We need to ensure that A && B are valid nodes and that A->GetOwningNode() and B->GetOwningNode() are not the same.
    if (A && B && A->GetOwningNode() != B->GetOwningNode())
    {
        // Since our entire body is a single pin, we can force connect to any pin no matter what, so long as the above are valid.
        return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, TEXT("Connect Upgrade Nodes"));
    }
    // Otherwise, we can't connect and we can kill the line.
    return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Invalid Connection"));
}

// This is how we populate the context graph. I'll be honest, I still do not know how this works and how PerformAction knows what it's doing.
void UUpgradeGraphSchema::GetGraphContextActions(FGraphContextMenuBuilder& contextMenuBuilder) const {
    // In any case, we can create a new node action here. We can easily make a long list of something like this. Based on category. Could make functions to separate it.
    TSharedPtr<FNewNodeAction> newNodeAction(
        new FNewNodeAction(
            FText::FromString(TEXT("Nodes")),
            FText::FromString(TEXT("New Node")),
            FText::FromString(TEXT("Create a new upgrade node")),
            0
        )
    );
    // Then all we do is add it to our contextMenuBuilder.
    contextMenuBuilder.AddAction(newNodeAction);
}

/* This perform action function is what creates our node to be edited on the graph in the first place. It takes a few important variables. Firstly, 
    @param parentGraph      // Gets our main graph. This is what the node will be attached to.
    @param fromPin          // Contains information about the pin that we were dragging from.
    @param location         // Is the location for which to spawn the node and where the contextual action was first made.
    @param bSelectNewNode   // Whether we should select the node once we created it.
*/
 UEdGraphNode* FNewNodeAction::PerformAction(UEdGraph* parentGraph, UEdGraphPin* fromPin, const FVector2D location, bool bSelectNewNode) {
    // If we have no graph, somehow, we have to immediately reutrn nullptr; Otherwise we will cause a crash.
    if (!parentGraph) { return nullptr; }
    // Create a transaction. This manages data for undo and redo. Safely recording modifications made.
    // It automatically ends a transaction once it is out of scope.
    const FScopedTransaction Transaction(INVTEXT("Upgrade Tree: Creating New Node"));
    // We need to call modify so that the graph correctly keeps its state and displays as we want it to. There will be asynchronous data and possible memory issues otherwise.
    parentGraph->Modify();
    // We instantiate a new node of type UUpgradeAssetGraphNode. This is what our slate (SUpgradeAssetGraphNode) handler then paints over.
    UUpgradeAssetGraphNode* NewGraphNode = NewObject<UUpgradeAssetGraphNode>(parentGraph, UUpgradeAssetGraphNode::StaticClass(), NAME_None, RF_Transactional);
    // Force set the positions of the node.
    NewGraphNode->NodePosX = location.X;
    NewGraphNode->NodePosY = location.Y;
    // Allocating default pins before adding the node to the graph is important, this ensures that there are no data or draw errors.
    NewGraphNode->AllocateDefaultPins();

    if (NewGraphNode->GetAttachedData()) {
        NewGraphNode->GetAttachedData()->OnIconChanged.RemoveDynamic(NewGraphNode, &UUpgradeAssetGraphNode::HandleIconChanged);
        NewGraphNode->GetAttachedData()->OnIconChanged.AddDynamic(NewGraphNode, &UUpgradeAssetGraphNode::HandleIconChanged);
        NewGraphNode->GetAttachedData()->OnBorderChanged.RemoveDynamic(NewGraphNode, &UUpgradeAssetGraphNode::HandleIconChanged);
        NewGraphNode->GetAttachedData()->OnBorderChanged.AddDynamic(NewGraphNode, &UUpgradeAssetGraphNode::HandleIconChanged);
    }
    parentGraph->AddNode(NewGraphNode, true, bSelectNewNode);
    parentGraph->NotifyGraphChanged();
    return NewGraphNode;
} 

// Try to create a connection. Using response we can guarantee if we CAN create a connection. But there are other things that may get in the way of the process.
bool UUpgradeGraphSchema::TryCreateConnection(UEdGraphPin* PinA, UEdGraphPin* PinB) const {
    FPinConnectionResponse Response = CanCreateConnection(PinA, PinB);

    if (Response.Response == CONNECT_RESPONSE_MAKE)
    {
        return CreateAutomaticConversionNodeAndConnections(PinA, PinB);
    }
    return UEdGraphSchema::TryCreateConnection(PinA, PinB);
}