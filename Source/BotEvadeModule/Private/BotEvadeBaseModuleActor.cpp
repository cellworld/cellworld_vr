#include "BotEvadeBaseModuleActor.h"

// Sets default values
ABotEvadeBaseModuleActor::ABotEvadeBaseModuleActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABotEvadeBaseModuleActor::BeginPlay() {
	Super::BeginPlay();
}

// Called every frame
// ReSharper disable once CppParameterMayBeConst
void ABotEvadeBaseModuleActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

