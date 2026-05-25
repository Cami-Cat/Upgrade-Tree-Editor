# Upgrade Tree Editor
Unreal Engine 5.5.4

# Notes:

You are currently forced into a specific data schema. This will change with the next coming update to allow for more customizability. But you will at the very least keep Icons and Borders. At least it will be easier to extend from and change for your purposes!

# How to use!

Once you've dragged the plugin into your project's `/plugin` directory and enabled it, you should see a neat little button at the top of the level editor that upon clicking, should either tell you no upgrade tree exists, or opens one up. You can create a new upgrade tree very quickly by heading to the plugin content folder and accessing: `Plugins\UpgradeAsset\Content\Tree`. If you make your asset in here, it should work! I'll update this to include the content folder recursively in `v1.02`.

For our use-case, I created this with a widget in mind that stores this data. This can be stored whever you would like, but because of this I have created an actor component that is attached to a c++ widget class by default, that you can inherit from! This class is called `UpgradeTreeWidget`, and contains our component `UpgradeTracker`.

### Upgrade Tracker

Upgrade tracker stores some key information for the end-user:
Firstly, it stores our UpgradeTree. You can access `NodeData` from this tree, which is an array of `FUpgradeNodeInfo`. There should be an included widget in the project that has further steps on how to use that to your advantage!

<img width="2108" height="421" alt="image" src="https://github.com/user-attachments/assets/a5898c66-68b6-4cc4-aee7-ad7e79387ad2" />

Secondly, it stores our event delegates. It has `On Progress Changed` `On Progress Maxed` and `On Unlock Node Requirements Met`, these three activate different things.
- Firstly, when the progress changes, it emits no matter what, including when the progress is maxed out.
- When the progress is maxed out, you can run some other functions off-of this that enable node locking and change the style of the node should you like (Node Widget not included)
- Finally, once you've met your threshold to unlock your other nodes (such as being upgraded once or being fully upgraded), you will then unlock your other nodes with `On Unlock Node Requirements Met`

 ```c++
TObjectPtr<UUpgradeAsset> UpgradeTree;
FOnComponentUpgradeHandler OnNodeProgressChanged;
FOnComponentUpgradedFullyHandler OnNodeMaxedOut;
FOnUnlockConnectedNodeRequirementsMet OnUnlockNodeRequirementsMet;
```

<img width="783" height="1053" alt="image" src="https://github.com/user-attachments/assets/aec5aac1-f264-4193-86e4-ea19a6cd3f0c" />

These are publicly accessible functions.

```c++
// Return how many times this specific node has been upgraded.
int32 GetUpgradeCount(FGuid NodeID) const;
// Return whether the node is unlocked and able to be upgraded.
bool IsNodeUnlocked(FGuid NodeID) const;
// Return whether the node is maxed.
bool IsNodeMaxLevel(FGuid NodeID) const;
// Get the progress struct of the node.
// - bIsNodeUpgraded
// - upgradeCount
FUpgradeNodeRuntimeProgress GetUpgradeProgress(FGuid NodeID);
// Update the node progress statically to force an upgrade count (0-Max) or to unlock the node (bNewIsUnlcoked) 
void SetUpgradeProgress(FGuid NodeID, int32 NewUpgradeCount, bool bNewIsUnlocked);
// Try to increment the upgrade count if you can. Requires your own currency integration logic if successful (bool)
bool TryIncrementUpgrade(FGuid NodeID);
```

You can connect the public delegates to do whatever you would like:

<img width="1550" height="1085" alt="image" src="https://github.com/user-attachments/assets/3d96037e-da05-452c-a71e-4411efb2e9e4" />

Note that this implementation is just an example, and you can use your own logic if you would like. Hence why `SetUpgradeProgress` is publically available as is `GetUpgradeProgress`. If you encounter any bugs, please raise them in the Issues tab and I will see if I can fix this issue! Please provide crash logs or ways to recreate an issue that you discover.
If you have suggestions, then by all means, please suggest additions, changes or extra built-in functions that might help you do as you would like with this tool!
