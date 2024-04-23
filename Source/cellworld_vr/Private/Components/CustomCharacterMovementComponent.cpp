#include "Components/CustomCharacterMovementComponent.h"

UCustomCharacterMovementComponent::UCustomCharacterMovementComponent()
{
	this->SetActive(true);
}

void UCustomCharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);
	UE_LOG(LogTemp, Warning, TEXT("UCustomCharacterMovementComponent::OnMovementUpdated"));
}
