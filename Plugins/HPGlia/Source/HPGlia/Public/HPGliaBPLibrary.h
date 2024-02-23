// (c) Copyright 2019-2021 HP Development Company, L.P.

#pragma once

#include "HPGliaExecutionEnum.h"
#include "HPGliaClient.h"
#include "HPGliaSettings.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include <string>

#include "HPGliaBPLibrary.generated.h"

/*
    *   Function library class.
    *   Each function in it is expected to be static and represents blueprint node that can be called in any blueprint.
    *
    *   When declaring function you can define metadata for the node. Key function specifiers will be BlueprintPure and BlueprintCallable.
    *   BlueprintPure - means the function does not affect the owning object in any way and thus creates a node without Exec pins.
    *   BlueprintCallable - makes a function which can be executed in Blueprints - Thus it has Exec pins.
    *   DisplayName - full name of the node, shown when you mouse over the node and in the blueprint drop down menu.
    *               Its lets you name the node using characters not allowed in C++ function names.
    *   CompactNodeTitle - the word(s) that appear on the node.
    *   Keywords -  the list of keywords that helps you to find node when you search for it using Blueprint drop-down menu.
    *               Good example is "Print String" node which you can find also by using keyword "log".
    *   Category -  the category your node will be under in the Blueprint drop-down menu.
    *
    *   For more info on custom blueprint nodes visit documentation:
    *   https://wiki.unrealengine.com/Custom_Blueprint_Node_Creation
    */



    UCLASS()
    class UHPGliaBPLibrary : public UBlueprintFunctionLibrary
    {
    public:
        GENERATED_BODY()

            UFUNCTION(BlueprintCallable, meta = (DisplayName = "Connect To Glia Async", Keywords = "Connect Glia", AdvancedDisplay = "connectionSettings"), Category = Glia)
            static HPGLIA_API void ConnectToGliaAsync(FHPGliaConnectionSettings connectionSettings);

        UFUNCTION(BlueprintCallable, meta = (DisplayName = "Disconnect from Glia", Keywords = "Disconnect Glia"), Category = Glia)
            static HPGLIA_API void DisconnectFromGlia();

        UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Subscription", Keywords = "Set Subscription"), Category = Glia)
            static HPGLIA_API void SetSubscriptions(TArray<FSubscription> subscriptionList);

        UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Heart Rate", Keywords = "Get Heart Rate", ExpandEnumAsExecs = "ExecutionBranch"), Category = Glia)
            static HPGLIA_API void GetHeartRate(
                UPARAM(DisplayName = "Heart Rate") int& OutHeartRate, EExecution& ExecutionBranch);

        UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Heart Rate Variability", Keywords = "Get Heart Rate Variability", ExpandEnumAsExecs = "ExecutionBranch"), Category = Glia)
            static HPGLIA_API void GetHeartRateVariability(
                UPARAM(DisplayName = "Heart Rate Variability") FHeartRateVariability& OutHeartRateVariability, EExecution& ExecutionBranch);

        UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Camera Image", Keywords = "Get Camera Image", ExpandEnumAsExecs = "ExecutionBranch"), Category = Glia)
            static HPGLIA_API void GetCameraImage(
                UPARAM(DisplayName = "Camera Image") FCameraImage& OutCameraImage, EExecution& ExecutionBranch);

        UFUNCTION(BlueprintCallable, meta = (DisplayName = "Make Texture2D", Keywords = "Glia Make Texture2D"), Category = Glia)
            static HPGLIA_API void GliaMakeTexture2D(const EPixelFormat PixelFormat, const int Width, const int Height, UTexture2D*& NewTexture);

        UFUNCTION(BlueprintCallable, meta = (DisplayName = "Load Image Bytes Into Texture", Keywords = "Glia Load Image Bytes Into Texture"), Category = Glia)
            static HPGLIA_API void GliaLoadImageBytesIntoTexture(const TArray<uint8>& Buffer, UTexture2D* InTexture, UTexture2D*& OutTexture);

        UFUNCTION(BlueprintCallable, meta = (DisplayName = "Glia Update Texture Resource", Keywords = "Update Texture Resource"), Category = Glia)
            static HPGLIA_API void GliaUpdateTextureResource(UTexture2D* InTexture, UTexture2D*& OutTexture);
        
        UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Eye Tracking", Keywords = "Get Eye Tracking Gaze", ExpandEnumAsExecs = "ExecutionBranch"), Category = Glia)
            static HPGLIA_API void GetEyeTracking(
                UPARAM(DisplayName = "Eye Tracking") FEyeTracking& OutEyeTracking, EExecution& ExecutionBranch);

        UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Cognitive Load", Keywords = "Get Cognitive Load", ExpandEnumAsExecs = "ExecutionBranch"), Category = Glia)
            static HPGLIA_API void GetCognitiveLoad(
                UPARAM(DisplayName = "Cognitive Load Struct") FCognitiveLoad& OutCognitiveLoadStruct,
                EExecution& ExecutionBranch);

        UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get IMU Array", Keywords = "Get IMU Array", ExpandEnumAsExecs = "ExecutionBranch"), Category = Glia)
            static HPGLIA_API void GetIMUArray(
                UPARAM(DisplayName = "IMU Array") FIMUArray& OutIMUArray, EExecution& ExecutionBranch);

        UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get PPG Array", Keywords = "Get PPG Array", ExpandEnumAsExecs = "ExecutionBranch"), Category = Glia)
            static HPGLIA_API void GetPPGArray(
                UPARAM(DisplayName = "PPG Array") FPPGArray& OutPPGArray, EExecution& ExecutionBranch);

        UFUNCTION(BlueprintCallable, meta = (DisplayName = "Send Data Record Request", Keywords = "Send Data Record Request", ExpandEnumAsExecs = "ExecutionBranch"), Category = Glia)
            static HPGLIA_API void SendDataRecordRequest(EExecution& ExecutionBranch);

        UFUNCTION(BlueprintCallable, meta = (DisplayName = "Stop Data Record", Keywords = "Stop Data Record", ExpandEnumAsExecs = "ExecutionBranch"), Category = Glia)
            static HPGLIA_API void StopDataRecord(EExecution& ExecutionBranch);

        UFUNCTION(BlueprintCallable, meta = (Displayname = "Is Glia Connected", Keywords = "Is Glia Connected"), Category = Glia)
            static HPGLIA_API bool isConnected();

        UFUNCTION(BlueprintCallable, meta = (Displayname = "Get Glia Delegates", Keywords = "Get Glia Events Delegates"), Category = Glia)
            static HPGLIA_API void GetGliaDelegates(UHPGliaDelegates *& OutDelegates);
    };
