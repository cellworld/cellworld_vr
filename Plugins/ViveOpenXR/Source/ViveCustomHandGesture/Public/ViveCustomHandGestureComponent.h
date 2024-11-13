// Copyright HTC Corporation. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ViveCustomHandGestureUtils.h"
#include "ViveCustomHandGestureComponent.generated.h"



DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCustomHandGestureNative, FString, gesture);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCustomHandGestureBp, FString, gesture);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDualGestureNative, FString, gesture);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDualGestureBp, FString, gesture);


UCLASS( ClassGroup=(Vive), meta=(BlueprintSpawnableComponent) )
class VIVECUSTOMHANDGESTURE_API UViveCustomGestureComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UViveCustomGestureComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


public:
#pragma region
	/// Single Hand
	static FCustomHandGestureNative OnCustomHandGestureNative_Left;
	static FCustomHandGestureNative OnCustomHandGestureNative_Right;

	UFUNCTION()
	void OnCustomHandGestureHandling_Left(FString gesture);
	UFUNCTION()
	void OnCustomHandGestureHandling_Right(FString gesture);

	UPROPERTY(BlueprintAssignable, Category = "Vive|CustomHandGesture")
	FCustomHandGestureBp OnCustomHandGestureChanged_Left;
	UPROPERTY(BlueprintAssignable, Category = "Vive|CustomHandGesture")
	FCustomHandGestureBp OnCustomHandGestureChanged_Right;

	/// Dual Hand
	static FDualGestureNative OnCustomHandGestureNative_Dual;

	UFUNCTION()
	void OnDualGestureHandling(FString gesture);

	UPROPERTY(BlueprintAssignable, Category = "Vive|CustomHandGesture")
	FDualGestureBp OnCustomHandGestureChange_DualHand;
#pragma endregion Custom Gesture

};
