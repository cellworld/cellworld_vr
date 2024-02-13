// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
//#include "TCPMessages.h"
//#include "ExperimentPlugin.h"
#include "BehaviorTree/BehaviorTree.h"
#include "CharacterPredator.generated.h"

UCLASS()
class ACharacterPredator : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACharacterPredator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	class UBehaviorTree* BehaviorTreeComponentChar; 

	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "AI")
	class ASmartObject* SmartObject;
};
