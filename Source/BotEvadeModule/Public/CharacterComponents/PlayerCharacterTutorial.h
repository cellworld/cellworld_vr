// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerCharacterTutorial.generated.h"

UCLASS()
class BOTEVADEMODULE_API APlayerCharacterTutorial : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacterTutorial();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/* components */
	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "Components")
	class UCameraComponent* Camera;

protected:

	/* override virtual functions */
	void MoveForward(const float InAxisValue); 	
	void MoveRight(const float InAxisValue); 	
	void LookUp(const float InAxisValue); 	
	void Turn(const float InAxisValue);

	/* multiplayer / replication */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;

	/* state management */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString StateCurrent = "None";
	
	UFUNCTION(Blueprintable)
	virtual void OnChangedState(const FString InState);

	UFUNCTION(Blueprintable)
	virtual void OnRep_SetState(const FString& InState);

	UFUNCTION(Blueprintable)
	void DebugServerSetState();
	
	UFUNCTION(Server, Reliable)
	void Server_SetState(const FString& InState);
	
	virtual void Server_SetState_Implementation(const FString& InState);
};
