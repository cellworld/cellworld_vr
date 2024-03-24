#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "PawnMainMovementComponent.generated.h"

/**
 *
 */
UCLASS()
class CELLWORLDVR_API UPawnMainMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()
public:
	UPawnMainMovementComponent();
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	class APawnMain* PawnOwner;
	void SetComponentOwner(class APawnMain* Owner);
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity);
};
