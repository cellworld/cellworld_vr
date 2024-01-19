#pragma once
#include "CoreMinimal.h"

//USTRUCT(BlueprintType)
//struct FMyStruct
//{
//    GENERATED_BODY()
//
//    //~ The following member variable will be accessible by Blueprint Graphs:
//    // This is the tooltip for our test variable.
//    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Variables")
//    int32 MyIntegerMemberVariable;
//
//    //~ The following member variable will be not accessible by Blueprint Graphs:
//    int32 NativeOnlyMemberVariable;
//
//    /**~
//    * This UObject pointer is not accessible to Blueprint Graphs, but
//    * is visible to UE4's reflection, smart pointer, and garbage collection
//    * systems.
//    */
//    UPROPERTY()
//    UObject* SafeObjectPointer;
//};
//
//USTRUCT()
//struct FClientInfo 
//{ 
//	GENERATED_BODY()
//
//	UPROPERTY()
//	FString access_key = "F8OK38DWnRgqJgr5aaUhgcfBPHoEe5toBiDGGREkR2DWeZxgTKFpCF5YvAdnHd-S";
//
//	UPROPERTY()
//	FString client_id = "25b17c6b-3386-45f8-9e1e-88d76259b5bf";
//
//};