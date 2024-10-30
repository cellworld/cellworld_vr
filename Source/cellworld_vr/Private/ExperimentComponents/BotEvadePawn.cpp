#include "ExperimentComponents/BotEvadePawn.h"

ABotEvadePawn::ABotEvadePawn() {
	PrimaryActorTick.bStartWithTickEnabled = true;

	 // Create Default Scene Root
    DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
    RootComponent = DefaultSceneRoot;

    // Create Motion Controllers
    MotionControllerL = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionControllerL"));
    MotionControllerL->SetupAttachment(DefaultSceneRoot);

    MotionControllerR = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionControllerR"));
    MotionControllerR->SetupAttachment(DefaultSceneRoot);

    // Create Camera
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(DefaultSceneRoot);

    // Create Oculus XR Passthrough Layer
    OculusXRPassthroughLayer = CreateDefaultSubobject<UOculusXRPassthroughLayerComponent>(TEXT("OculusXRPassthroughLayer"));
    OculusXRPassthroughLayer->SetupAttachment(DefaultSceneRoot);

    // Create Main Menu Attachment Point
    MainMenuAttachmentPoint = CreateDefaultSubobject<USceneComponent>(TEXT("MainMenuAttachmentPoint"));
    MainMenuAttachmentPoint->SetupAttachment(MotionControllerR);

    // Create Model Spawn Point
    ModelSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("ModelSpawnPoint"));
    ModelSpawnPoint->SetupAttachment(MotionControllerR);

    // Create Anchor Menu Attachment Point
    AnchorMenuAttachmentPoint = CreateDefaultSubobject<USceneComponent>(TEXT("AnchorMenuAttachmentPoint"));
    AnchorMenuAttachmentPoint->SetupAttachment(MotionControllerR);

    // Create Spline
    Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
    Spline->SetupAttachment(DefaultSceneRoot);

    // Create Spline Mesh
    SplineMesh = CreateDefaultSubobject<USplineMeshComponent>(TEXT("SplineMesh"));
    SplineMesh->SetupAttachment(Spline);

    // Create Spatial Anchor Manager
    SpatialAnchorManager_AC = CreateDefaultSubobject<UActorComponent>(TEXT("SpatialAnchorManager_AC"));
    // SpatialAnchorManager_AC->SetupAttachment(DefaultSceneRoot);

    // Create Menu Manager
    MenuManager_AC = CreateDefaultSubobject<UActorComponent>(TEXT("MenuManager_AC"));
    // MenuManager_AC->SetupAttachment(DefaultSceneRoot);
	
}

bool ABotEvadePawn::StartPositionSamplingTimer(const float InRateHz) {
	return false;
}

bool ABotEvadePawn::StopPositionSamplingTimer() {
	return false;
}

void ABotEvadePawn::BeginPlay() {
	Super::BeginPlay();
}

void ABotEvadePawn::Tick(float DeltaTime) { Super::Tick(DeltaTime); }
void ABotEvadePawn::EndPlay(const EEndPlayReason::Type EndPlayReason) { Super::EndPlay(EndPlayReason); }
void ABotEvadePawn::Reset() { Super::Reset(); }
void ABotEvadePawn::UpdateMovementComponent(FVector InputVector, bool bForce) {}
void ABotEvadePawn::MoveForward(float AxisValue) {}
void ABotEvadePawn::MoveRight(float AxisValue) {}
void ABotEvadePawn::Turn(float AxisValue) {}
void ABotEvadePawn::LookUp(float AxisValue) {}
