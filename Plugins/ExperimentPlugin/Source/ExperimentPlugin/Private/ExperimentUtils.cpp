#include "ExperimentUtils.h"


FAgentState UExperimentUtils::JsonStringToAgentState(FString jsonString) {
	FAgentState structOutput;
	FJsonObjectConverter::JsonObjectStringToUStruct(jsonString, &structOutput, 0, 0);
	return structOutput;
}

FString UExperimentUtils::AgentStateToJsonString(FAgentState structInput) {
	FString jsonString;
	FJsonObjectConverter::UStructToJsonObjectString(structInput, jsonString, 0, 0, 0);
	return jsonString;
}

FCoordinates UExperimentUtils::JsonStringToCoordinates(FString jsonString) {
	FCoordinates structOutput;
	FJsonObjectConverter::JsonObjectStringToUStruct(jsonString, &structOutput, 0, 0);
	return structOutput;
}

FString UExperimentUtils::CoordinatesToJsonString(FCoordinates structInput) {
	FString jsonString;
	FJsonObjectConverter::UStructToJsonObjectString(structInput, jsonString, 0, 0, 0);
	return jsonString;
}

FLocation UExperimentUtils::JsonStringToLocation(FString jsonString) {
	FLocation structOutput;
	FJsonObjectConverter::JsonObjectStringToUStruct(jsonString, &structOutput, 0, 0);
	return structOutput;
}

FString UExperimentUtils::LocationToJsonString(FLocation structInput) {
	FString jsonString;
	FJsonObjectConverter::UStructToJsonObjectString(structInput, jsonString, 0, 0, 0);
	return jsonString;
}

FShape UExperimentUtils::JsonStringToShape(FString jsonString) {
	FShape structOutput;
	FJsonObjectConverter::JsonObjectStringToUStruct(jsonString, &structOutput, 0, 0);
	return structOutput;
}

FString UExperimentUtils::ShapeToJsonString(FShape structInput) {
	FString jsonString;
	FJsonObjectConverter::UStructToJsonObjectString(structInput, jsonString, 0, 0, 0);
	return jsonString;
}

FSpace UExperimentUtils::JsonStringToSpace(FString jsonString) {
	FSpace structOutput;
	FJsonObjectConverter::JsonObjectStringToUStruct(jsonString, &structOutput, 0, 0);
	return structOutput;
}

FString UExperimentUtils::SpaceToJsonString(FSpace structInput) {
	FString jsonString;
	FJsonObjectConverter::UStructToJsonObjectString(structInput, jsonString, 0, 0, 0);
	return jsonString;
}

FWorldImplementation UExperimentUtils::JsonStringToWorldImplementation(FString jsonString) {
	FWorldImplementation structOutput;
	FJsonObjectConverter::JsonObjectStringToUStruct(jsonString, &structOutput, 0, 0);
	return structOutput;
}

FString UExperimentUtils::WorldImplementationToJsonString(FWorldImplementation structInput) {
	FString jsonString;
	FJsonObjectConverter::UStructToJsonObjectString(structInput, jsonString, 0, 0, 0);
	return jsonString;
}

FLocation3 UExperimentUtils::JsonStringToLocation3(FString jsonString) {
	FLocation3 structOutput;
	FJsonObjectConverter::JsonObjectStringToUStruct(jsonString, &structOutput, 0, 0);
	return structOutput;
}

FString UExperimentUtils::Location3ToJsonString(FLocation3 structInput) {
	FString jsonString;
	FJsonObjectConverter::UStructToJsonObjectString(structInput, jsonString, 0, 0, 0);
	return jsonString;
}

FRotation3 UExperimentUtils::JsonStringToRotation3(FString jsonString) {
	FRotation3 structOutput;
	FJsonObjectConverter::JsonObjectStringToUStruct(jsonString, &structOutput, 0, 0);
	return structOutput;
}

FString UExperimentUtils::Rotation3ToJsonString(FRotation3 structInput) {
	FString jsonString;
	FJsonObjectConverter::UStructToJsonObjectString(structInput, jsonString, 0, 0, 0);
	return jsonString;
}

FTransformation UExperimentUtils::JsonStringToTransformation(FString jsonString) {
	FTransformation structOutput;
	FJsonObjectConverter::JsonObjectStringToUStruct(jsonString, &structOutput, 0, 0);
	return structOutput;
}

FString UExperimentUtils::TransformationToJsonString(FTransformation structInput) {
	FString jsonString;
	FJsonObjectConverter::UStructToJsonObjectString(structInput, jsonString, 0, 0, 0);
	return jsonString;
}

FStartEpisodeRequest UExperimentUtils::JsonStringToStartEpisodeRequest(FString jsonString) {
	FStartEpisodeRequest structOutput;
	FJsonObjectConverter::JsonObjectStringToUStruct(jsonString, &structOutput, 0, 0);
	return structOutput;
}

FString UExperimentUtils::StartExperimentRequestToJsonString(FStartExperimentRequest structInput) {
	FString jsonString;
	FJsonObjectConverter::UStructToJsonObjectString(structInput, jsonString, 0, 0, 0);
	return jsonString;
}

FString UExperimentUtils::FinishExperimentRequestToJsonString(FFinishExperimentRequest structInput) {
	FString jsonString;
	FJsonObjectConverter::UStructToJsonObjectString(structInput, jsonString, 0, 0, 0);
	return jsonString;
}

FStartExperimentResponse UExperimentUtils::JsonStringToStartExperimentResponse(FString jsonString)
{
	FStartExperimentResponse structOutput;
	FJsonObjectConverter::JsonObjectStringToUStruct(jsonString, &structOutput, 0, 0);
	return structOutput;
}

FString UExperimentUtils::StartEpisodeRequestToJsonString(FStartEpisodeRequest structInput) {
	FString jsonString;
	FJsonObjectConverter::UStructToJsonObjectString(structInput, jsonString, 0, 0, 0);
	return jsonString;
}

FString UExperimentUtils::WorldInfoToJsonString(FWorldInfo structInput) {
	FString jsonString;
	FJsonObjectConverter::UStructToJsonObjectString(structInput, jsonString, 0, 0, 0);
	return jsonString;
}

FString UExperimentUtils::GetExperimentRequestToJsonString(FGetExperimentRequest structInput)
{
	FString jsonString;
	FJsonObjectConverter::UStructToJsonObjectString(structInput, jsonString, 0, 0, 0);
	return jsonString;
}

FStartEpisodeResponse UExperimentUtils::JsonStringToStartEpisodeResponse(FString jsonString) {
	FStartEpisodeResponse structOutput;
	FJsonObjectConverter::JsonObjectStringToUStruct(jsonString, &structOutput, 0, 0);
	return structOutput;
}

FString UExperimentUtils::StartEpisodeResponseToJsonString(FStartEpisodeResponse structInput) {
	FString jsonString;
	FJsonObjectConverter::UStructToJsonObjectString(structInput, jsonString, 0, 0, 0);
	return jsonString;
}

FFinishEpisodeRequest UExperimentUtils::JsonStringToFinishEpisodeRequest(FString jsonString) {
	FFinishEpisodeRequest structOutput;
	FJsonObjectConverter::JsonObjectStringToUStruct(jsonString, &structOutput, 0, 0);
	return structOutput;
}

FString UExperimentUtils::FinishEpisodeRequestToJsonString(FFinishEpisodeRequest structInput) {
	FString jsonString;
	FJsonObjectConverter::UStructToJsonObjectString(structInput, jsonString, 0, 0, 0);
	return jsonString;
}

FFinishEpisodeResponse UExperimentUtils::JsonStringToFinishEpisodeResponse(FString jsonString) {
	FFinishEpisodeResponse structOutput;
	FJsonObjectConverter::JsonObjectStringToUStruct(jsonString, &structOutput, 0, 0);
	return structOutput;
}

FString UExperimentUtils::FinishEpisodeResponseToJsonString(FFinishEpisodeResponse structInput) {
	FString jsonString;
	FJsonObjectConverter::UStructToJsonObjectString(structInput, jsonString, 0, 0, 0);
	return jsonString;
}

FUpdateGhostMovementMessage UExperimentUtils::JsonStringToUpdateGhostMovementMessage(FString jsonString) {
	FUpdateGhostMovementMessage structOutput;
	FJsonObjectConverter::JsonObjectStringToUStruct(jsonString, &structOutput, 0, 0);
	return structOutput;
}

FString UExperimentUtils::UpdateGhostMovementMessageToJsonString(FUpdateGhostMovementMessage structInput) {
	FString jsonString;
	FJsonObjectConverter::UStructToJsonObjectString(structInput, jsonString, 0, 0, 0);
	return jsonString;
}

FStep UExperimentUtils::JsonStringToStep(FString jsonString) {
	FStep structOutput;
	FJsonObjectConverter::JsonObjectStringToUStruct(jsonString, &structOutput, 0, 0);
	return structOutput;
}

FCellGroup UExperimentUtils::JsonStringToCellGroup(FString jsonString) {
	FCellGroup structOutput;
	FJsonObjectConverter::JsonObjectStringToUStruct(jsonString, &structOutput, 0);
	return structOutput;
}

FString UExperimentUtils::StepToJsonString(FStep Step) {
	FString jsonString; 
	FJsonObjectConverter::UStructToJsonObjectString(Step, jsonString, 0, 0, 0);
	return jsonString;
}

FLocation UExperimentUtils::VrToCanonical(const FVector VectorIn, const float MapLengthIn, const float WorldScaleIn) {
	FLocation LocationOut;
	LocationOut.x = ((VectorIn.X) / (MapLengthIn * WorldScaleIn));
	LocationOut.y = VectorIn.Y / (-MapLengthIn * WorldScaleIn);
	return LocationOut;
}

FVector UExperimentUtils::CanonicalToVr(const FLocation LocationIn, const float MapLengthIn, const float WorldScaleIn) {
	FVector VectorOut;
	VectorOut.X = (LocationIn.x * MapLengthIn * WorldScaleIn);
	VectorOut.Y = (-LocationIn.y * MapLengthIn * WorldScaleIn);
	VectorOut.Z = 0; // will need to be changed 
	return VectorOut;
}

int UExperimentUtils::updateFrame(int Frame)
{
	Frame++;
	return Frame;
}

float UExperimentUtils::updateTimeStamp(FDateTime episodeStart)
{	
	FTimespan now = FDateTime::UtcNow() - episodeStart;
	float timeStamp = UKismetMathLibrary::GetTotalMilliseconds(now) / 1000.0;
	return timeStamp;
}

FString UExperimentUtils::LoadWorldImplementation(FString filePath)
{
	// Use JsonStringToWorldImplementation()
	// See if File Exists
	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*filePath))
	{
		// FWorldImplementation world_info
		return "";
	}

	FString jsonString = "";

	// Attempt to read file
	if (!FFileHelper::LoadFileToString(jsonString, *filePath))
	{
		return "";
	}

	return jsonString;
}

FWorldInfo UExperimentUtils::GenerateWorldInfo(const FString WorldConfigurationIn, const FString WorldImplementationIn, const FString OcclusionsIn)
{
	FWorldInfo WorldInfo; 
	WorldInfo.occlusions		   = OcclusionsIn; 
	WorldInfo.world_configuration  = WorldConfigurationIn; 
	WorldInfo.world_implementation = WorldImplementationIn;
	return WorldInfo;
}

TArray<FLocation> UExperimentUtils::OcclusionsParseAllLocations(const FString OcclusionLocationsIn)
{
	TArray<FLocation> LocationList;

	// Create a JSON Reader from the JSON string
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(OcclusionLocationsIn);

	// This will hold the parsed JSON array
	TArray<TSharedPtr<FJsonValue>> JsonArray;

	// Deserialize the JSON string into a JSON array
	if (!FJsonSerializer::Deserialize(Reader, JsonArray))
	{ 
		UE_LOG(LogTemp, Fatal, TEXT("[UExperimentUtils::OcclusionsParseAllLocations] Failed to parse deserialize JsonArray. Response: %s"),*OcclusionLocationsIn);
		return LocationList;
	}
	// Iterate over the array
	for (TSharedPtr<FJsonValue> Value : JsonArray)
	{
		// Get the JSON object
		TSharedPtr<FJsonObject> JsonObject = Value->AsObject();
		if (JsonObject.IsValid())
		{
			FLocation Location;
			Location.x = JsonObject->GetNumberField("x");
			Location.y = JsonObject->GetNumberField("y");
			LocationList.Add(Location);
		}
		else {
			return LocationList;
		}
	}

	return LocationList;
}

