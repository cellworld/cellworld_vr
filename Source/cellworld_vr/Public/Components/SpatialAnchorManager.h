#pragma once

#include "CoreMinimal.h"
#include "AndroidPermissionCallbackProxy.h"
#include "Components/ActorComponent.h"
#include "AndroidPermissionFunctionLibrary.h"
#include "OculusXRAnchorLatentActions.h"
#include "OculusXRAnchors.h"
#include "OculusXRFunctionLibrary.h"
#include "cellworld_vr/cellworld_vr.h"
#include "SpatialAnchorManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAnchorCreated, bool, bResult);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class USpatialAnchorManager : public UActorComponent
{
	GENERATED_BODY()

public:    
	// Sets default values for this component's properties
	USpatialAnchorManager() {
		bLoadPreview = false;
		bSelectionMode = false;
		SpatialAnchorLimit = 6; 
	};

protected:
	FAnchorCreated OnAnchorCreated_Dispatch; 
	// Called when the game starts
	virtual void BeginPlay() override {
		Super::BeginPlay();

		// Check Android Permissions
		// todo: enable android permissions plugin
		if (!UAndroidPermissionFunctionLibrary::CheckPermission(TEXT("android.permission.WRITE_EXTERNAL_STORAGE"))) {
			TArray<FString> Permissions;
			Permissions.Add(TEXT("android.permission.WRITE_EXTERNAL_STORAGE"));
			Permissions.Add(TEXT("android.permission.READ_EXTERNAL_STORAGE"));
		
			// Request Android Permissions
			// UAndroidPermissionCallbackProxy* CallbackProxy = UAndroidPermissionFunctionLibrary::AcquirePermissions(Permissions);
			// CallbackProxy->OnPermissionsGrantedDynamicDelegate.AddDynamic(this,ThisClass::OnPermissionsGrantedEvent); // todo: make variable and unbind
				// FOnPermissionsGrantedDynamicDelegate::CreateUObject(this, &USpatialAnchorManager::OnPermissionsGranted));
			
		}
	}

	void CreateAnchor_Event() {
		UActorComponent* ModelSpawnPositioner = nullptr; // todo: make class 
		if (!ModelSpawnPositioner) {
			UE_LOG(LogExperiment, Warning, TEXT("[SpatialAnchorManager::CreateAnchor_Event] ModelSpawnPositioner is not set."));
			return;
		}

		// FTransform SpawnTransform = ModelSpawnPositioner->GetComponentTransform(); // todo
		FTransform SpawnTransform = FTransform(); 

		// Spawn Anchor Model Actor
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this->GetOwner();

		// AActor* SpawnedAnchor = GetWorld()->SpawnActor<AActor>(UBPSpatialAnchorModel::StaticClass(), SpawnTransform, SpawnParams); // todo
		AActor* SpawnedAnchor = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), SpawnTransform, SpawnParams); 

		if (SpawnedAnchor) {
			// LoadedAnchors.Add(Cast<UBPSpatialAnchorModel>(SpawnedAnchor));
			OnAnchorCreated_Dispatch.Broadcast(true);
			UOculusXRAsyncAction_CreateSpatialAnchor* AsyncAction_CreateSpatialAnchor=  UOculusXRAsyncAction_CreateSpatialAnchor::OculusXRAsyncCreateSpatialAnchor(SpawnedAnchor,SpawnTransform);
			if (AsyncAction_CreateSpatialAnchor) {
				// todo: bind delegates to success or failed result 
			}
			UE_LOG(LogExperiment, Warning, TEXT("[SpatialAnchorManager::CreateAnchor_Event] Anchor successfully created and added to LoadedAnchors."));
		} else {
			UE_LOG(LogExperiment, Warning, TEXT("[SpatialAnchorManager::CreateAnchor_Event] Failed to spawn anchor."));
			OnAnchorCreated_Dispatch.Broadcast(false);
		}
	}
public:
	// Selection Methods
	bool HasSelectedAnchor() const { return false; }
	void SetHovered() {}
	void SetSelected() {}
	void SetSelectedUsingHovered() {}
	void UnsetHovered() {}
	void DeselectAll() {}

	// Helper Methods
	void FindAnchor() {}
	int32 GetAnchorCount() const { return 0; }
	bool CheckSelection() const { return false; }
	void CreateAnchorsFromQuery() {}

	UFUNCTION()
	// ReSharper disable once CppMemberFunctionMayBeStatic

	void OnAnchorCreatedSuccess_Event(UOculusXRAnchorComponent* Anchor, EOculusXRAnchorResult::Type ResultType) {
		UE_LOG(LogExperiment, Log, TEXT("[USpatialAnchorManager::OnAnchorCreatedSuccess_Event] Success."))
		// todo: add anchor to array of anchors 
	};
	
	void OnAnchorCreatedFailed_Event(EOculusXRAnchorResult::Type ResultType) {
		UE_LOG(LogExperiment, Log, TEXT("[USpatialAnchorManager::OnAnchorCreatedFailed_Event] Failed."))
		// todo: handle failed creation
	};
	
	void OnPermissionsGrantedEvent(const TArray<FString>& Permissions, const TArray<bool>& GrantResults) {
		UE_LOG(LogExperiment, Warning, TEXT("[SpatialAnchorManager::OnPermissionsGrantedEvent] "))
		for (int32 Idx = 0; Idx < Permissions.Num(); ++Idx) {
			FString Permission = Permissions[Idx];
			const bool bGranted = GrantResults.IsValidIndex(Idx) ? GrantResults[Idx] : false;
			UE_LOG(LogExperiment, Warning, TEXT("Permission: %s, Granted: %s"), *Permission, bGranted ? TEXT("True") : TEXT("False"));
		}
	}
	
	// todo: create a USpatialAnchorModel class
	// Selection
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Selection")
	TObjectPtr<class AActor> TempSpatialAnchorModel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Selection")
	TObjectPtr<class AActor> HoveredAnchor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Selection")
	bool bSelectionMode;

	// Anchor Containers
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Anchor Containers")
	TArray<TObjectPtr<class AActor>> LoadedAnchors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Anchor Containers")
	TArray<TObjectPtr<class AActor>> SelectedAnchors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Anchor Containers")
	TArray<TObjectPtr<class AActor>> PreviewAnchors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Anchor Containers")
	TArray<TObjectPtr<class AActor>> ToEraseAnchors; 

	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings")
	int32 SpatialAnchorLimit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings")
	bool bLoadPreview;

	// Save Game
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save Game")
	TArray<FString> UUIDsToLoad;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save Game")
	TArray<FString> SavedUUIDs;
};