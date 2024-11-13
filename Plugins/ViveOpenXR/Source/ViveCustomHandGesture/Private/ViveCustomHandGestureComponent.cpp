// Copyright HTC Corporation. All Rights Reserved.

#include "ViveCustomHandGestureComponent.h"
#include "ViveCustomHandGesture.h"

//DEFINE_LOG_CATEGORY(LogViveCustomHandGesture);


FCustomHandGestureNative UViveCustomGestureComponent::OnCustomHandGestureNative_Right;
FCustomHandGestureNative UViveCustomGestureComponent::OnCustomHandGestureNative_Left;

FDualGestureNative UViveCustomGestureComponent::OnCustomHandGestureNative_Dual;

// Sets default values for this component's properties
UViveCustomGestureComponent::UViveCustomGestureComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UViveCustomGestureComponent::BeginPlay()
{
	Super::BeginPlay();

	UViveCustomGestureComponent::OnCustomHandGestureNative_Right.AddDynamic(this, &UViveCustomGestureComponent::OnCustomHandGestureHandling_Right);
	UViveCustomGestureComponent::OnCustomHandGestureNative_Left.AddDynamic(this, &UViveCustomGestureComponent::OnCustomHandGestureHandling_Left);

	UViveCustomGestureComponent::OnCustomHandGestureNative_Dual.AddDynamic(this, &UViveCustomGestureComponent::OnDualGestureHandling);
}

void UViveCustomGestureComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
}

// Called every frame
void UViveCustomGestureComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

#pragma region
void UViveCustomGestureComponent::OnCustomHandGestureHandling_Right(FString gesture)
{
	UE_LOG(LogViveCustomHandGesture, Log, TEXT("OnCustomHandGestureHandling_Right() gesture: %s"), *gesture);
	OnCustomHandGestureChanged_Right.Broadcast(gesture);
}

void UViveCustomGestureComponent::OnCustomHandGestureHandling_Left(FString gesture)
{
	UE_LOG(LogViveCustomHandGesture, Log, TEXT("OnCustomHandGestureHandling_Left() gesture: %s"), *gesture);
	OnCustomHandGestureChanged_Left.Broadcast(gesture);
}

void UViveCustomGestureComponent::OnDualGestureHandling(FString gesture)
{
	UE_LOG(LogViveCustomHandGesture, Log, TEXT("OnDualGestureHandling() gesture: %s"), *gesture);
	OnCustomHandGestureChange_DualHand.Broadcast(gesture);
}
#pragma endregion Custom Gesture

