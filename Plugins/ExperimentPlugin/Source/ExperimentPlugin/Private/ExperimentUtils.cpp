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

/// <summary>
///		Reads in a filePath, which contains the indices of the occlusions to spawn.
///		Requires reading the cell_locations from hexagonal.canonical
/// </summary>
/// <param name="filePath"></param>
/// <param name="readStatus"></param>
/// <returns></returns>
FLocation UExperimentUtils::vrToCanonical(FVector vrCoordinates, float mapLength) {
	FLocation canonical;
	float halfMap = 0.5 * mapLength;
	canonical.x = ( vrCoordinates.X + halfMap )  / mapLength;
	canonical.y = ( halfMap - vrCoordinates.Y  ) / mapLength;
	return canonical;
}

/// <summary>
///		Reads in a filePath, which contains the indices of the occlusions to spawn.
///		Requires reading the cell_locations from hexagonal.canonical
/// </summary>
/// <param name="filePath"></param>
/// <param name="readStatus"></param>
/// <returns></returns>
FVector UExperimentUtils::canonicalToVr(FLocation canonicalCoordinates, float mapLength) {
	FVector vr;
	float halfMap = 0.5 * mapLength;
	vr.X = ( mapLength * canonicalCoordinates.x ) -  halfMap;
	vr.Y = halfMap - ( canonicalCoordinates.y * mapLength);
	vr.Z = 260.0;
	return vr;
}

/// <summary>
///		Reads in a filePath, which contains the indices of the occlusions to spawn.
///		Requires reading the cell_locations from hexagonal.canonical
/// </summary>
/// <param name="filePath"></param>
/// <param name="readStatus"></param>
/// <returns></returns>
int UExperimentUtils::updateFrame(int Frame)
{
	Frame++;
	return Frame;
}

/// <summary>
///		Reads in a filePath, which contains the indices of the occlusions to spawn.
///		Requires reading the cell_locations from hexagonal.canonical
/// </summary>
/// <param name="filePath"></param>
/// <param name="readStatus"></param>
/// <returns></returns>
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

/// <summary>
///		Reads in a filePath, which contains the indices of the occlusions to spawn.
///		Requires reading the cell_locations from hexagonal.canonical
///		Values are HARDCODED at the moment. Can be optimized later.
/// </summary>
/// <param name="filePath"> The File Path to the World with Specified Entropy</param>
/// <param name="readStatus"></param>
/// <returns></returns>
TArray<FLocation> UExperimentUtils::LoadOcclusions(FString filePath, bool& readStatus, float mapLength)
{
	// Get cell_locations from world_implementation
		// 1. Get world_implementation as jsonString
		// 2. Convert to world_implementation
		// 3. Convert to vr locations
	FString jsonString = LoadWorldImplementation("C:/Research/hexagonal.canonical"); // todo: fix hard path 
	FWorldImplementation world_impl = JsonStringToWorldImplementation(jsonString);

	// Properly loads the file with cell_locations
	// UE_LOG(LogTemp, Warning, TEXT("%s"), *jsonString);
	TArray<FLocation> vr_cell_locs;

	//UE_LOG(LogTemp, Warning, TEXT("Loaded Canonical World"));

	// See if file exists
	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*filePath))
	{
		// FWorldImplementation world_info
		readStatus = false;
		UE_LOG(LogTemp, Warning, TEXT("Can't Find Cell Group File"));
		return vr_cell_locs;
	}

	FString retString = "";

	// Attempt a file read, output stored in retString
	if (!FFileHelper::LoadFileToString(retString, *filePath))
	{
		readStatus = false;
		UE_LOG(LogTemp, Warning, TEXT("Can't Load Cell Group File"));
		return vr_cell_locs;
	}

	//UE_LOG(LogTemp, Warning, TEXT("%s"), *retString);
	
	const TCHAR* delims[] = { TEXT(","), TEXT("["), TEXT("]") };
	TArray<FString> parsed;
	retString.ParseIntoArray(parsed, delims, 3);
	TArray<int> indices;

	for (int i = 0; i < parsed.Num(); i++)
	{
		// Add parsed index into array
		indices.Add(UKismetStringLibrary::Conv_StringToInt(parsed[i]) );
		FString idx_str = parsed[i];
	}

	// retString has a string version of all of the indices we need
	FLocation vr_converted;
	FVector vr_vector;
	for (int i = 0; i < indices.Num(); i++) // For idx in FString loaded from file...
	{
		//UE_LOG(LogTemp, Warning, TEXT("Index: %d"), indices[i]);
		//UE_LOG(LogTemp, Warning, TEXT("%f,%f"), world_impl.cell_locations[indices[i]].x, world_impl.cell_locations[indices[i]].y)
		vr_vector = canonicalToVr(world_impl.cell_locations[indices[i]], mapLength);
		vr_converted.x = vr_vector.X;
		vr_converted.y = vr_vector.Y;
		//UE_LOG(LogTemp, Warning, TEXT("%f,%f"), vr_converted.x, vr_converted.y);
		vr_cell_locs.Add(vr_converted);
	}

	UE_LOG(LogTemp, Warning, TEXT("Finished Converting Indices to Canonical"));
	readStatus = true;
	return vr_cell_locs;	
}

