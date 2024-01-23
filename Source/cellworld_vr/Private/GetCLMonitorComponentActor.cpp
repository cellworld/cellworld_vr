#include "GetCLMonitorComponentActor.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

AGetCLMonitorComponentActor::AGetCLMonitorComponentActor()
{
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;
}

bool AGetCLMonitorComponentActor::Calibrate()
{
	if (GEngine) {
		GEngine->XRSystem->GetStereoRenderingDevice()->EnableStereo(false);
		GEngine->XRSystem->GetHMDDevice()->EnableHMD(false);
	}
	FString ENV_VAR = TEXT("HP_OMNICEPT_INSTALL");
	FString eyePath = FWindowsPlatformMisc::GetEnvironmentVariable(*ENV_VAR);
	eyePath = eyePath.Append(TEXT("\\..\\..\\HP Omnicept\\HP Omnicept Eye Tracking Calibration\\ETCal\\Binaries\\Win64\\ETCal-Win64-Shipping.exe"));
	FProcHandle handle = FPlatformProcess::CreateProc(*eyePath, nullptr, true, false, false, nullptr, 0, nullptr, nullptr);

	if (handle.IsValid())
	{
		FPlatformProcess::WaitForProc(handle);

		if (GEngine) {
			GEngine->XRSystem->GetStereoRenderingDevice()->EnableStereo(true);
			GEngine->XRSystem->GetHMDDevice()->EnableHMD(true);
		}

		FString fileContent = "famal";
		FString userName = UKismetSystemLibrary::GetPlatformUserName();
		FString filePath = "C:\\Users\\";
		filePath = filePath.Append(userName);
		filePath = filePath.Append("\\AppData\\Local\\ETCal\\Saved\\etcallog.log");

		FFileHelper::LoadFileToString(fileContent, *filePath);

		return fileContent.Contains("Tobii Eye Calibration finished successfully");
	}

	if (GEngine) {
		GEngine->XRSystem->GetStereoRenderingDevice()->EnableStereo(true);
		GEngine->XRSystem->GetHMDDevice()->EnableHMD(true);
	}

	return false;

}

void AGetCLMonitorComponentActor::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("[AGetCLMonitorComponentActor::BeginPlay] Start."));

	/* run calibration */
	//Calibrate();
	InitThread();
}

void AGetCLMonitorComponentActor::InitThread()
{
	UE_LOG(LogTemp, Warning, TEXT("[AGetCLMonitorComponentActor::InitThread] Start."));

	WorldRef = nullptr;
	if (GEngine) {
		WorldRef = GEngine->GetWorld();
		if (WorldRef == nullptr) {
			WorldRef = GetWorld();
		}
	}

	if (WorldRef == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("[AGetPlayerTraversePathActor::InitThread] World ref from actor null. Aborting."));
		return;
	}

	/* to do: pass pawn reference */
	UE_LOG(LogTemp, Error, TEXT("[AGetPlayerTraversePathActor::InitThread] Before Worker Thread."));

	/* create thread object */
	WorkerThread = new FGetCLMonitorComponentThread(this, SaveDataDirectory);
	CurrentRunningThread = FRunnableThread::Create(WorkerThread, TEXT("t_GetCLMonitorComponentThread"));
}

bool AGetCLMonitorComponentActor::IsVectorAllZeros(const FVector Vec)
{
	return Vec.IsZero(); // true if zeros
}

bool AGetCLMonitorComponentActor::IsVectorAllNegativeOnes(const FVector Vec)
{
	FVector NegOnes = { -1.0f,1.0f,-1.0f };
	return Vec == NegOnes;
}

bool AGetCLMonitorComponentActor::IsEyeDataValid(FVector Vec) {
	if (AGetCLMonitorComponentActor::IsVectorAllNegativeOnes(eye_combined_gaze) ||
		AGetCLMonitorComponentActor::IsVectorAllZeros(eye_combined_gaze)) {
		//UE_LOG(LogTemp, Error, TEXT("[AGetCLMonitorComponentActor::DrawEyeTraceOnPlayer()] eye_combined_gaze not valid."));
		return false;
	}
	else {
		return true;
	}
}

bool AGetCLMonitorComponentActor::IsWorldValid(UWorld*& World) {
	
	if (GetWorld()) {
		World = GetWorld();
		return true;
	}

	//UE_LOG(LogTemp, Error, TEXT("[AGetCLMonitorComponentActor::IsWorldValid] GEngine->GetWorld() not valid."));
	return false;
}

bool AGetCLMonitorComponentActor::GetPlayerCameraComponent(UCameraComponent*& Camera) {
	/* get player controller */
	MouseKeyboardPlayerController = Cast<AMouseKeyboardPlayerController>(UGameplayStatics::GetPlayerController(WorldRef, player_index));
	if (!MouseKeyboardPlayerController) {
		UE_LOG(LogTemp, Error, TEXT("[AGetCLMonitorComponentActor::GetPlayerCameraComponent] PlayerController == nullptr ."));
		return false;
	}

	/* get pawn*/
	Pawn = Cast<APawnMain>(MouseKeyboardPlayerController->GetPawn());
	if (Pawn == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("[AGetCLMonitorComponentActor::GetPlayerCameraComponent] Pawn == nullptr ."));
		return false;
	}

	/* new test */
	Camera = Pawn->GetCameraComponent();
	if (Camera == nullptr) {
		return false;
	}

	return true;
}

/* runs a couple steps before showing line trace */
bool AGetCLMonitorComponentActor::IsTraceAvailable() {
	/* make sure that the eye data array isn't empty */
	/*if (!AGetCLMonitorComponentActor::IsEyeDataValid(eye_combined_gaze) || (!AGetCLMonitorComponentActor::IsWorldValid(WorldRef)) || (!AGetCLMonitorComponentActor::GetPlayerCameraComponent(CameraComponent))) {
		return false;
	}*/

	if (!AGetCLMonitorComponentActor::IsEyeDataValid(eye_left) || (!AGetCLMonitorComponentActor::IsWorldValid(WorldRef)) || (!AGetCLMonitorComponentActor::GetPlayerCameraComponent(CameraComponent))) {
		return false;
	}
	if (!AGetCLMonitorComponentActor::IsEyeDataValid(eye_right) || (!AGetCLMonitorComponentActor::IsWorldValid(WorldRef)) || (!AGetCLMonitorComponentActor::GetPlayerCameraComponent(CameraComponent))) {
		return false;
	}

	return true;
}

bool AGetCLMonitorComponentActor::DrawEyeTraceOnPlayer(float DeltaTime)
{
	if (!AGetCLMonitorComponentActor::IsTraceAvailable()) { return false; }

	/* trace params  */
	const FVector trace_start = CameraComponent->GetComponentLocation(); // from camera component location
	const FTransform trace_transform = CameraComponent->GetComponentTransform();
	const FVector trace_end = trace_start + UKismetMathLibrary::TransformDirection(trace_transform, eye_combined_gaze)*10000*100; // 10 meters = 10*100 u.u. (1 cm = 100 u.u)
	FHitResult hit_result;
	
	const FCollisionQueryParams collision_params;

	GetWorld()->LineTraceSingleByChannel(hit_result, trace_start, trace_end, ECollisionChannel::ECC_Visibility, collision_params);
	
	/* if raycast hit an actor */
	if (hit_result.Actor.Get()) {
		FString name;
		hit_result.Actor.Get()->GetName(name);
		//GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Green, FString::Printf(TEXT("[AGetCLMonitorComponentActor::DrawEyeTraceOnPlayer] hit_result.Actor->UniqueID: %s"), *name));
	}

	/* ======== dual eye line trace ============ */
	FVector viewpoint;
	FRotator rot;
	Pawn->GetActorEyesViewPoint(viewpoint, rot);

	const float IPD = 6.5f; // 65mm -> cm = u.u
	const float eye_position_horizontal_offset = IPD / 2; // left/right from center of HMD 

	//FVector trace_start_left = trace_start;
	FVector trace_start_left = viewpoint;
	trace_start_left.X = trace_start_left.X - eye_position_horizontal_offset;

	//FVector trace_start_right = trace_start;
	FVector trace_start_right = viewpoint;
	trace_start_right.X = trace_start_right.X + eye_position_horizontal_offset;

	/* trace params  */

	const FVector trace_end_left = trace_start_left + UKismetMathLibrary::TransformDirection(CameraComponent->GetComponentTransform(), eye_left) * 1000 * 100; // 10 meters = 10*100 u.u. (1 cm = 100 u.u)
	FHitResult hit_result_left;
	const FCollisionQueryParams collision_params_left;

	const FVector trace_end_right = trace_start_right + UKismetMathLibrary::TransformDirection(CameraComponent->GetComponentTransform(), eye_right) * 1000 * 100; // 10 meters = 10*100 u.u. (1 cm = 100 u.u)
	FHitResult hit_result_right;
	const FCollisionQueryParams collision_params_right;

	/*GetWorld()->LineTraceSingleByChannel(hit_result_left, trace_start_left, trace_end_left, ECollisionChannel::ECC_Visibility, collision_params_left);
	GetWorld()->LineTraceSingleByChannel(hit_result_right, trace_start_right, trace_end_right, ECollisionChannel::ECC_Visibility, collision_params_right);*/

	/* ====== debug circle params ====== */
	const float draw_duration = DeltaTime/2; // same as tick, update per frame
	const uint8 depth = 10; // to do: make this vary with pupil dilation and openess
	const float radius = 1.0f;
	const int segments = 5;
	const uint8 depth_priority = 1;
	const float thickness = 0.25f;

	DrawDebugLine(GetWorld(), trace_start_left, trace_end_left, FColor::Red, true, draw_duration, depth_priority, thickness); // line following view direction
	//DrawDebugLine(GetWorld(), trace_start, trace_end, FColor::Orange, true, draw_duration, depth_priority, thickness); // line following view direction
	DrawDebugLine(GetWorld(), trace_start_right, trace_end_right, FColor::Green, true, draw_duration, depth_priority, thickness); // line following view direction
	//DrawDebugSphere(GetWorld(), trace_end, radius, segments, FColor::Blue, false, 0.5f, draw_duration, thickness); // spehere showing view 

	return true;
}

void AGetCLMonitorComponentActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	AGetCLMonitorComponentActor::DrawEyeTraceOnPlayer(DeltaTime);
}
 
void AGetCLMonitorComponentActor::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (CurrentRunningThread && WorkerThread) {
		// need to suspen thread THEN stop it. You can't stop and kill randomly
		CurrentRunningThread->Suspend(true);
		WorkerThread->bStopThread = true;
		CurrentRunningThread->Suspend(false);
		CurrentRunningThread->Kill(false); // was false
		CurrentRunningThread->WaitForCompletion();
	}

	/* destroy this actor */
	UE_LOG(LogTemp, Log, TEXT("[AGetCLMonitorComponentActor::EndPlay()] Calling Destroy()"));

}

UWorld* AGetCLMonitorComponentActor::GetWorldReferenceFromActor()
{
	return GEngine->GetWorld();
}

void AGetCLMonitorComponentActor::Stop()
{
	UE_LOG(LogTemp, Log, TEXT("[AGetPlayerTraversePathActor::Stop()]"));
}

