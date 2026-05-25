#include "Graph/Slate/SUpgradeAssetGraphNode.h"
#include "Utils/CamiSlateUtils.h"

void SUpgradeAssetGraphNode::Construct(const FArguments& InArgs, UEdGraphNode* InNode)
{
    // Store the Node that we're connected to. This allows us to store their information and display it.
    this->GraphNode = InNode;
    this->SetCursor(EMouseCursor::Hand);
    // Update the graph node. This means update pins and its draw state so that Slate knows what to do with it.
    this->UpdateGraphNode();
}

const FSlateBrush* SUpgradeAssetGraphNode::GetNodeBodyBrush() const
{
    UUpgradeAssetGraphNode* castNode = Cast<UUpgradeAssetGraphNode>(GraphNode);
    if (!castNode || !castNode->GetAttachedData() || !castNode->GetAttachedData()->nodeIcon) { return FAppStyle::GetBrush("Graph.Node.Body"); }

    UTexture2D* CurrentIcon = castNode->GetAttachedData()->nodeIcon;

    if (!DynamicNodeIconBrush.IsValid())
    {
        DynamicNodeIconBrush = MakeShareable(new FSlateBrush());
    }

    DynamicNodeIconBrush->DrawAs = ESlateBrushDrawType::Image;
    DynamicNodeIconBrush->SetResourceObject(CurrentIcon);
    DynamicNodeIconBrush->ImageSize.X = CurrentIcon->GetSurfaceWidth();
    DynamicNodeIconBrush->ImageSize.Y = CurrentIcon->GetSurfaceHeight();

    return DynamicNodeIconBrush.Get();
}

const FSlateBrush* SUpgradeAssetGraphNode::GetNodeOutlineBorderBrush() const
{
    UUpgradeAssetGraphNode* castNode = Cast<UUpgradeAssetGraphNode>(GraphNode);
    if (!castNode || !castNode->GetAttachedData()->nodeBorder)
    {
        return FCoreStyle::Get().GetBrush("NoBrush");
    }

    if (!DynamicNodeBorderBrush.IsValid())
    {
        DynamicNodeBorderBrush = MakeShareable(new FSlateBrush());
    }

    DynamicNodeBorderBrush->SetResourceObject(castNode->GetAttachedData()->nodeBorder);
    DynamicNodeBorderBrush->DrawAs = ESlateBrushDrawType::Image;
    DynamicNodeBorderBrush->Margin = FMargin(16.0f, 16.0f, 16.0f, 16.0f);

    return DynamicNodeBorderBrush.Get();
}

FSlateColor SUpgradeAssetGraphNode::GetCustomNodeBodyColor()
{
    // Store what colour we want the body to be. Firstly by getting the panel, then checking whether it is real.
    // If it's currently selected, we want it to ahve a different colour to whether it is unselected, where it is a regular dark grey.
    TSharedPtr<SGraphPanel> OwnerPanel = GetOwnerPanel();
    bIsSelected = OwnerPanel.IsValid() && OwnerPanel->SelectionManager.IsNodeSelected(GraphNode);

    // Selected colour:
    if (bIsSelected) { return UpgradeTreePalette::NodeSelected; }
    // Unselected colour:
    return UpgradeTreePalette::NodeBackgroundBase;
}

FSlateColor SUpgradeAssetGraphNode::GetRimHighlightColor() const
{
    // Decide how we're going to draw the node if we are currently dragging a line from it.
    if (bIsHoveringRim || bIsActivelyDraggingWire)
    {
        // Glowing green neon, stands out for debugging.
        return UpgradeTreePalette::NodeHighlighted; 
    }
    // Otherwise, don't need to change it. Keep it the same grey as the deselected option in GetCustomNodeBodyColor()
    return UpgradeTreePalette::NodeBackgroundBase;
}

FReply SUpgradeAssetGraphNode::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    // Get the position of the mouse relative to the position of the self.
    FVector2D LocalMousePosition = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
    // Get the size of the self in FVector2D.
    FVector2D NodeSize = MyGeometry.GetLocalSize();
    
    // Get how far away the mouse is from the center of the node. Unfortunately, this is circular.
    float DistanceFromCenter = FVector2D::Distance(LocalMousePosition, NodeSize * 0.5f);
    // Create a threshold for large the selectable edge is so that we can drag nodes from it.
    float EdgeDragThreshold = (NodeSize.X * 0.3f);

    // Calculate whether we are hovering over the rim by checking whether the distance from the center is larger than the edge threshold.
    // This changes the draw call method in GetRimHighlightColour()
    // This is also where OnMouseButtonDown() is able to handle ButtonDown events.
    bIsHoveringRim = (DistanceFromCenter > EdgeDragThreshold);

    // Check whether we are currently dragging a wire. This is important for setting where our mouse position is so that has an updated reference to draw to.
    if (bIsActivelyDraggingWire)
    {
        LiveDragMousePos = LocalMousePosition;
    }
    
    // Supercede the call to the parent to get a correct FReply return.
    // Because I'm too lazy to implement my own, and this handles it more than well enough.
    return SGraphNode::OnMouseMove(MyGeometry, MouseEvent);
}

void SUpgradeAssetGraphNode::OnMouseLeave(const FPointerEvent& MouseEvent)
{
    // If we're not actively doing anything (set from OnMouseButtonDown), then we can reset whether we're hovering over the self.
    if (!bIsActivelyDraggingWire)
    {
        bIsHoveringRim = false;
    }
    // Then, supercede everything to the parent once more. This doesn't need a return; Slate will kill me otherwise. Contrary to popular belief, I quite like living.
    SGraphNode::OnMouseLeave(MouseEvent);
}

FReply SUpgradeAssetGraphNode::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    // Is the current button LeftMouseButton and are we currently hovering over the rim? (Set in OnMouseMove)
    if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && bIsHoveringRim)
    {
        // If so, we'll say that we're dragging a wire.
        bIsActivelyDraggingWire = true;
        // Also ensure that we're updatying where our mouse position is in Local Space.
        LiveDragMousePos = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
        
        // Capture the mouse, this keeps things in Sync.
        return FReply::Handled().CaptureMouse(SharedThis(this));
    }
    // Supercede and return an FReply like above. I still like living.
    return SGraphNode::OnMouseButtonDown(MyGeometry, MouseEvent);
}

FReply SUpgradeAssetGraphNode::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    // If the button that we just released was the left mouse button, and we were dragging:
    if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && bIsActivelyDraggingWire)
    {
        // We are no longe dragging.
        bIsActivelyDraggingWire = false;
        // Release mouse capture, we no longer need to hold the mouse.
        FReply ReleaseReply = FReply::Handled().ReleaseMouseCapture();

        // Get the panel that owns us.
        TSharedPtr<SGraphPanel> Panel = GetOwnerPanel();
        // Perform validity checks.
        if (Panel.IsValid() && GraphNode && GraphNode->GetSchema())
        {
            // Get the position of where we dropped our mouse.
            FVector2D DropScreenSpacePos = MouseEvent.GetScreenSpacePosition();
            // Turn it from screen-space position to graph position so that we can check it on our SGraphPanel.
            FVector2D GraphSpacePos = Panel->PanelCoordToGraphCoord(Panel->GetCachedGeometry().AbsoluteToLocal(DropScreenSpacePos));
            
            UEdGraphNode* DropTargetNode = nullptr;
            // Store the UEdGraph that we're a part of.
            UEdGraph* CoreGraph = GraphNode->GetGraph();

            if (CoreGraph)
            {
                // For every Node inside the Graph
                for (UEdGraphNode* CheckNode : CoreGraph->Nodes)
                {
                    // If the node is not us
                    if (CheckNode == GraphNode) { continue; }
                    // Match the extent of the boxes using sizes we already know.
                    if (GraphSpacePos.X >= CheckNode->NodePosX && GraphSpacePos.X <= (CheckNode->NodePosX + 128.0f) &&
                        GraphSpacePos.Y >= CheckNode->NodePosY && GraphSpacePos.Y <= (CheckNode->NodePosY + 128.0f))
                        {
                            // Set the target node to the this node and break so long as the position our event ended matches the inside extent of the box.
                            DropTargetNode = CheckNode;
                            // Break out of the loop.
                            break;
                        }
                }
            }

            // If there was a node that we dropped onto,
            if (DropTargetNode)
            {
                // So long as it has a pin.
                if (GraphNode->Pins.Num() > 0 && DropTargetNode->Pins.Num() > 0)
                {
                    const FScopedTransaction Transaction(FText::FromString(TEXT("Create Connection")));
                    // Try to connect with the pin and notify the graph that there was an update.
                    UUpgradeAssetGraphNode* castNode = Cast<UUpgradeAssetGraphNode>(GraphNode);
                    UUpgradeAssetGraphNode* castTargetNode = Cast<UUpgradeAssetGraphNode>(DropTargetNode);
                    castTargetNode->Modify();
                    castNode->AddNewUnlockConnection(castTargetNode->GetNodeId());
                }
            }
        }
        // Finally release the mouse.
        return ReleaseReply;
    }
    // Supercede.
    return SGraphNode::OnMouseButtonUp(MyGeometry, MouseEvent);
}

void SUpgradeAssetGraphNode::UpdateGraphNode()
{
    UUpgradeAssetGraphNode* castNode = Cast<UUpgradeAssetGraphNode>(GraphNode);

    // Core Slate initialization requirements
    InputPins.Empty();
    OutputPins.Empty();
    SAssignNew(LeftNodeBox, SVerticalBox);
    SAssignNew(RightNodeBox, SVerticalBox);
    this->ContentScale.Bind(this, &SGraphNode::GetContentScale);

    if (GraphNode && GraphNode->Pins.Num() == 0)
    {
        GraphNode->CreatePin(EGPD_Output, TEXT("UpgradePin"), TEXT("Link"));
    }

    // Pull our custom runtime texture brushes
    const FSlateBrush* CustomOutlineFrame = GetNodeOutlineBorderBrush();
    const FSlateBrush* CustomInteriorIcon = GetNodeBodyBrush();

    // I fucking HATE Slate Syntax.
    this->GetOrAddSlot(ENodeZone::Center) 
    .HAlign(HAlign_Fill)
    .VAlign(VAlign_Fill)
    [
        SNew(SBox)
        .WidthOverride(128.0f)
        .HeightOverride(128.0f)
        [
            // Style-less overlay.
            SNew(SOverlay)
            // Here we add our border
            + SOverlay::Slot()
            .HAlign(HAlign_Fill)
            .VAlign(VAlign_Fill)
            [
                SNew(SImage)
                .Image(CustomOutlineFrame)
                .ColorAndOpacity(this, &SUpgradeAssetGraphNode::GetRimHighlightColor)
            ]
            // Here we add our icon, with a margin to make it sit neatly inside.
            + SOverlay::Slot()
            .HAlign(HAlign_Fill)
            .VAlign(VAlign_Fill)
            .Padding(FMargin(16.0f))
            [
                SNew(SImage)
                .Image(CustomInteriorIcon)
                .ColorAndOpacity(FLinearColor::White) 
            ]
            // Then we add some other (un)necessary stuff that will, for some reason upon removal, completely break the node.
            + SOverlay::Slot()
            .HAlign(HAlign_Fill)
            .VAlign(VAlign_Fill)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .HAlign(HAlign_Center)
                .VAlign(VAlign_Center)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(GraphNode ? TEXT(" ") : TEXT(" ")))
                    .TextStyle(FAppStyle::Get(), "Graph.Node.NodeTitle")
                    .ColorAndOpacity(FLinearColor::White)
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(FMargin(2.0f, 2.0f))
                [
                    SNew(SHorizontalBox)
                    .Visibility(EVisibility::Collapsed) 
                    + SHorizontalBox::Slot().AutoWidth()[ LeftNodeBox.ToSharedRef() ]
                    + SHorizontalBox::Slot().AutoWidth()[ RightNodeBox.ToSharedRef() ]
                ]
            ]
        ]
    ];
    CreatePinWidgets();
}


// This is called whenever the engine wants to render whatever we have for it to render.
// There are two bits of logic that we want to handle here.
// Firstly  : Are we currently dragging a wire?
// Secondly : Do wires already exist?
// This allows us to draw persistent wires while we're dragging a wire from a node.
//
// We override the typical OnPaint() call so that we're able to draw in a completely custom way.
int32 SUpgradeAssetGraphNode::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    // So first thing's first, we actually draw everything the engine wants to draw. This is necessary for Slate to render everything that we have, including the node body and text / image.
    int32 MaxLayerId = SGraphNode::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

    FVector2D NodeSize = AllottedGeometry.GetLocalSize();
    FVector2D OriginPoint = NodeSize * 0.5f;
    float BoxHalfWidth = NodeSize.X * 0.5f;
    float BoxHalfHeight = NodeSize.Y * 0.5f;
    bool bDoAntiAliasing = true;

    UUpgradeAssetGraphNode* castNode = Cast<UUpgradeAssetGraphNode>(GraphNode);
    
    // Here we draw our active wire.
    if (bIsActivelyDraggingWire)
    {
        // We first get the direction (normal) of our mouse from the middle point of the node.
        FVector2D RayDirection = (LiveDragMousePos - OriginPoint).GetSafeNormal();
        // We then set the line to begin at the Origin Point, as a fallback.
        FVector2D LineStart = OriginPoint;
        // And we call our custom function to draw for us!
        CamiSlateUtils::DrawGridLineToMouse(LineStart, LiveDragMousePos, FVector2D(BoxHalfWidth, BoxHalfHeight), OutDrawElements, AllottedGeometry, MaxLayerId);
        MaxLayerId++;
    }
    
    // Now that we've drawn the lines that we've been dragging, we also need to draw persistent lines for connections that already exist.
    // Connection handling logic is done in UpgradeAssetGraphSchema. We're just drawing based on that through our classes _unlocks member 
    // TODO (Change this to become a getter that returns a reference) Only in UpgradeAssetGraphSchema do we need to alter it.
    if (GraphNode && castNode->GetNodeUnlocks().Num() > 0)
    {
        for (FGuid unlock : castNode->GetNodeUnlocks()) {
            UUpgradeAssetGraphNode* targetNode = castNode->GetNodeWithGUID(unlock);

            if (!targetNode) { continue; }

            FVector2D targetNodePos = FVector2D(targetNode->NodePosX, targetNode->NodePosY);
            FVector2D sourceNodePos = FVector2D(castNode->NodePosX, castNode->NodePosY);
            FVector2D localTargetCenter = OriginPoint + (targetNodePos - sourceNodePos);

            CamiSlateUtils::DrawGridLineBetweenNodes(OriginPoint, localTargetCenter, FVector2D(BoxHalfWidth, BoxHalfHeight), OutDrawElements, AllottedGeometry, MaxLayerId);
            MaxLayerId++;
        }
    }
    return MaxLayerId;
}