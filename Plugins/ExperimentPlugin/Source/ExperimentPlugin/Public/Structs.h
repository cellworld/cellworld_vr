#pragma once
#include "JsonObjectConverter.h"
#include "Misc/Guid.h"
#include "Structs.generated.h"

#define STRUCTTOJSON(TYPE) \
static F## TYPE JsonStringTo## TYPE (FString jsonString) { \
	F## TYPE structOutput; \
	FJsonObjectConverter::JsonObjectStringToUStruct(jsonString, &structOutput, 0, 0); \
	return structOutput; \
} 

#define JSONTOSTRUCT(TYPE) \
static FString TYPE ##ToJsonString (F##TYPE structInput) { \
	FString jsonString; \
	FJsonObjectConverter::UStructToJsonObjectString(structInput, jsonString, 0, 0, 0); \
	return jsonString; \
} 

USTRUCT(Blueprintable)
struct FLocation
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		float x;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		float y;
};

USTRUCT(Blueprintable)
struct FStep
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
	FLocation location;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
	float rotation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
	int frame;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
	float time_stamp; // delta time in seconds 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
	FString agent_name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
	FString data;

};

USTRUCT(Blueprintable)
struct FCoordinates
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		int x;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		int y;
};

USTRUCT(Blueprintable)
struct FLocation3
{
	GENERATED_BODY()
public:
	FLocation3() = default;
	FLocation3(const FVector &location) {
		x = location.X;
		y = location.Y;
		z = location.Z;
	}
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		float x;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		float y;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		float z;
	bool operator ==(const FLocation3& other) {
		return x == other.x && y == other.y && z == other.z;
	}
	bool operator !=(const FLocation3& other) {
		return !(*(this) == other);
	}
};

USTRUCT(Blueprintable)
struct FRotation3
{
	GENERATED_BODY()
public:
	FRotation3() = default;
	FRotation3(const FRotator& rotator) {
		roll = rotator.Roll;
		pitch = rotator.Pitch;
		yaw = rotator.Yaw;
	}
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		float roll;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		float pitch;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		float yaw;
	bool operator ==(const FRotation3 &other) {
		return roll == other.roll && pitch == other.pitch && yaw == other.yaw;
	}
	bool operator !=(const FRotation3& other) {
		return !(*(this) == other);
	}
};

USTRUCT(Blueprintable)
struct FAgentState
{
	GENERATED_BODY()
public:
	FAgentState() = default;
	FAgentState(int frame, float time_stamp, const FString& AgentName, const FVector& location, const FRotator& rotator) :
	time_stamp(time_stamp),
	frame(frame),
	agent_name(AgentName),
	location(location),
	rotation(rotator){
	}
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		float time_stamp;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		int frame;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		FString agent_name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		FLocation3 location;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		FRotation3 rotation;
	bool operator ==(const FAgentState& other) {
		return location == other.location && rotation == other.rotation;
	}
	bool operator !=(const FAgentState& other) {
		return !(*(this) == other);
	}
};

USTRUCT(Blueprintable)
struct FGetExperimentRequest
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
	FString ExperimentName;
};

USTRUCT(Blueprintable)
struct FCell {
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
	int id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
	FCoordinates coordinates;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
	FLocation location;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
	bool occluded;
};

USTRUCT(Blueprintable)
struct FCellGroup {
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
	TArray<FCell> cell_locations;
};

USTRUCT(Blueprintable)
struct FWorldInfo
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
	FString world_configuration = "hexagonal";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
	FString world_implementation = "canonical";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
	FString occlusions = "21_05";
};

USTRUCT(Blueprintable)
struct FGetExperimentRequestResponse
{
	//    def __init__(self, experiment_name: str = "", world_info: World_info = None, 
	// start_date: datetime = None, subject_name: str = "", 
	// duration: int = 0, remaining_time: float =0.0, episode_count: int=0, 
	// rewards_cells:Cell_group_builder=None):

	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		FString experiment_name = "";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		FWorldInfo world_info;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		FString start_date;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		FString subject_name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		int duration;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		float remaining_time;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		int episode_count;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		FCellGroup reward_cells;

};

USTRUCT(Blueprintable)
struct FStartExperimentResponse {
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		FString experiment_name = "";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		FString start_date = "";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		FWorldInfo world; 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		FString subject_name = "";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		int duration; 
	
};

USTRUCT(Blueprintable)
struct FStartExperimentRequest
{
	GENERATED_BODY()
public:
	// prefix: str, 
	// suffix : str, 
	// world_configuration : str, 
	// world_implementation : str, 
	// occlusions : str, 
	// subject_name : str, 
	// duration : int, 
	// rewards_cells : Cell_group_builder = None, 
	// rewards_orientations : JsonList = None

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		FString prefix = "VR";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		FString suffix = "suffix";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		FWorldInfo world;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		FString subject_name = "vr_dude";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		int duration = 0; // Tmax 30 min (convert to seconds)
};

USTRUCT(Blueprintable)
struct FStartEpisodeRequest
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		FString experiment_name;
};

USTRUCT(Blueprintable)
struct FStartEpisodeResponse
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		TArray<int> occlusions;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		FLocation predator_spawn_location;
};

USTRUCT(Blueprintable)
struct FFinishEpisodeRequest
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		FString experiment_name;
};

USTRUCT(Blueprintable)
struct FFinishEpisodeResponse
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		int participant_id;
};

USTRUCT(Blueprintable)
struct FUpdateGhostMovementMessage
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		float forward;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		float rotation;
};

USTRUCT(Blueprintable)
struct FTransformation
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		float size;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		float rotation;
};

USTRUCT(Blueprintable)
struct FShape
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		int sides;
};

USTRUCT(Blueprintable)
struct FSpace
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		FLocation center;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		FShape shape;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		FTransformation transformation;
};

USTRUCT(Blueprintable)
struct FWorldImplementation
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		TArray<FLocation> cell_locations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		FSpace space;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
		FTransformation cell_transformation;
};

USTRUCT(Blueprintable)
struct FColumnsWrapper {
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Experiment)
	TArray<AActor*> Columns;
};
