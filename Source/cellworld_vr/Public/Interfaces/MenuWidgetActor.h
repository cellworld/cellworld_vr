// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MainMenuWidgetBasic.h"
#include "cellworld_vr/cellworld_vr.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Actor.h"
#include "MenuWidgetActor.generated.h"

UCLASS()
class CELLWORLD_VR_API AMenuWidgetActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMenuWidgetActor() {
		UE_LOG(LogExperiment, Log, TEXT("[AMenuWidgetActor] Constructor"))
		
		MenuWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("MenuWidgetComponent"));
        MenuWidgetComponent->SetupAttachment(RootComponent);

		// Set the widget class for the component to use UMainMenuWidget
		// static ConstructorHelpers::FClassFinder<UUserWidget> WidgetClass(TEXT("/Game/UI/MainMenuWidget_BP")); // Update the path to your MainMenuWidget Blueprint class if needed
		// if (WidgetClass.Succeeded()) {
		// 	MenuWidgetComponent->SetWidgetClass(WidgetClass.Class);
		// 	UE_LOG(LogExperiment, Log, TEXT("[AMenuWidgetActor] Find WidgetClass: OK"))
		// } else {
		// 	UE_LOG(LogExperiment, Error, TEXT("[AMenuWidgetActor] Find WidgetClass: Failed"))
		// }

		// Optional: Set widget component properties
		MenuWidgetComponent->SetDrawSize(FVector2D(1920, 1080)); // Set the resolution of the widget
		MenuWidgetComponent->SetWidgetSpace(EWidgetSpace::World); // Display the widget in the world
		MenuWidgetComponent->SetPivot(FVector2D(0.5f, 0.5f)); 
	}

	UPROPERTY(EditAnywhere)
	TObjectPtr<UMainMenuWidgetBasic> MainMenuWidget; 

	UPROPERTY(EditAnywhere)
	TObjectPtr<UWidgetComponent> MenuWidgetComponent; 

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override {
		Super::BeginPlay();
		// MainMenuWidget->SetIsEnabled(true);
		// MainMenuWidget->SetFocus()
	}

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override {
		Super::Tick(DeltaTime);
	}

};
