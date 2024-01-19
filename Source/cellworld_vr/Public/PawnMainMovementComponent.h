#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "PawnMainMovementComponent.generated.h"

/**
 *
 */
UCLASS()
class CELLWORLD_VR_API UPawnMainMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()
public:
	UPawnMainMovementComponent();
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
