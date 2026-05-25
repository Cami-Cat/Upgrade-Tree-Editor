#include "UpgradeNodeDataAsset.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraph.h"
#include "Subsystems/AssetEditorSubsystem.h" // Required to fetch the active editor instance
#include "Editor.h"

void UUpgradeNodeDataAsset::SetIcon(UTexture2D* newIcon) {
    nodeIcon = newIcon;
    OnIconChanged.Broadcast(newIcon);
}

void UUpgradeNodeDataAsset::SetBorder(UTexture2D* newIcon) {
    nodeBorder = newIcon;
    OnBorderChanged.Broadcast(newIcon);
}

void UUpgradeNodeDataAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

    if (PropertyName == FName("nodeIcon")) {
        OnIconChanged.Broadcast(nodeIcon);
        FCoreUObjectDelegates::OnObjectPropertyChanged.Broadcast(this, PropertyChangedEvent);
    }
    if (PropertyName == FName("nodeBorder")) {    
        OnBorderChanged.Broadcast(nodeBorder);
        
        FCoreUObjectDelegates::OnObjectPropertyChanged.Broadcast(this, PropertyChangedEvent);
    }
}