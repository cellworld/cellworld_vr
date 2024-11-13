// Copyright HTC Corporation. All Rights Reserved.

#pragma once
#include <stdint.h>
#include "ViveFacialExpressionEnums.generated.h"

UENUM(BlueprintType, Category = "ViveOpenXR|FacialTracking")
enum class EXrFacialTrackingType : uint8
{
	None = 0  UMETA(Hidden),
	Eye = 1,
	Lip = 2
};

/** The avatar's eye relative blend shape.*/
UENUM(BlueprintType, Category = "ViveOpenXR|FacialTracking")
enum class EEyeShape :uint8 {
	Eye_Left_Blink = 0			UMETA(DisplayName = "Eye_Left_Blink"),
	Eye_Left_Wide = 1			UMETA(DisplayName = "Eye_Left_Wide"),
	Eye_Left_Right = 2			UMETA(DisplayName = "Eye_Left_Right"),
	Eye_Left_Left = 3			UMETA(DisplayName = "Eye_Left_Left"),
	Eye_Left_Up = 4				UMETA(DisplayName = "Eye_Left_Up"),
	Eye_Left_Down = 5			UMETA(DisplayName = "Eye_Left_Down"),
	Eye_Right_Blink = 6			UMETA(DisplayName = "Eye_Right_Blink"),
	Eye_Right_Wide = 7			UMETA(DisplayName = "Eye_Right_Wide"),
	Eye_Right_Right = 8			UMETA(DisplayName = "Eye_Right_Right"),
	Eye_Right_Left = 9			UMETA(DisplayName = "Eye_Right_Left"),
	Eye_Right_Up = 10			UMETA(DisplayName = "Eye_Right_Up"),
	Eye_Right_Down = 11			UMETA(DisplayName = "Eye_Right_Down"),
	Eye_Frown = 12				UMETA(DisplayName = "Eye_Frown"),
	Eye_Left_Squeeze = 13		UMETA(DisplayName = "Eye_Left_Squeeze"),
	Eye_Right_Squeeze = 14		UMETA(DisplayName = "Eye_Right_Squeeze"),
	Max = 15					UMETA(Hidden),
	None = 63					UMETA(DisplayName = "None"),
};

/** The prediction result relative blend shape.*/
UENUM(BlueprintType, Category = "ViveOpenXR|FacialTracking")
enum class ELipShape :uint8 {
	Jaw_Right = 0               UMETA(DisplayName = "Jaw_Right"),
	Jaw_Left = 1                UMETA(DisplayName = "Jaw_Left"),
	Jaw_Forward = 2             UMETA(DisplayName = "Jaw_Forward"),
	Jaw_Open = 3                UMETA(DisplayName = "Jaw_Open"),
	Mouth_Ape_Shape = 4         UMETA(DisplayName = "Mouth_Ape_Shape"),
	Mouth_Upper_Right = 5       UMETA(DisplayName = "Mouth_Upper_Right"),
	Mouth_Upper_Left = 6        UMETA(DisplayName = "Mouth_Upper_Left"),
	Mouth_Lower_Right = 7       UMETA(DisplayName = "Mouth_Lower_Right"),
	Mouth_Lower_Left = 8        UMETA(DisplayName = "Mouth_Lower_Left"),
	Mouth_Upper_Overturn = 9    UMETA(DisplayName = "Mouth_Upper_Overturn"),
	Mouth_Lower_Overturn = 10   UMETA(DisplayName = "Mouth_Lower_Overturn"),
	Mouth_Pout = 11             UMETA(DisplayName = "Mouth_Pout"),
	Mouth_Smile_Right = 12      UMETA(DisplayName = "Mouth_Smile_Right"),
	Mouth_Smile_Left = 13       UMETA(DisplayName = "Mouth_Smile_Left"),
	Mouth_Sad_Right = 14        UMETA(DisplayName = "Mouth_Sad_Right"),
	Mouth_Sad_Left = 15         UMETA(DisplayName = "Mouth_Sad_Left"),
	Cheek_Puff_Right = 16       UMETA(DisplayName = "Cheek_Puff_Right"),
	Cheek_Puff_Left = 17        UMETA(DisplayName = "Cheek_Puff_Left"),
	Cheek_Suck = 18             UMETA(DisplayName = "Cheek_Suck"),
	Mouth_Upper_UpRight = 19    UMETA(DisplayName = "Mouth_Upper_UpRight"),
	Mouth_Upper_UpLeft = 20     UMETA(DisplayName = "Mouth_Upper_UpLeft"),
	Mouth_Lower_DownRight = 21  UMETA(DisplayName = "Mouth_Lower_DownRight"),
	Mouth_Lower_DownLeft = 22   UMETA(DisplayName = "Mouth_Lower_DownLeft"),
	Mouth_Upper_Inside = 23     UMETA(DisplayName = "Mouth_Upper_Inside"),
	Mouth_Lower_Inside = 24     UMETA(DisplayName = "Mouth_Lower_Inside"),
	Mouth_Lower_Overlay = 25    UMETA(DisplayName = "Mouth_Lower_Overlay"),
	Tongue_LongStep1 = 26       UMETA(DisplayName = "Tongue_LongStep1"),
	Tongue_Left = 27            UMETA(DisplayName = "Tongue_Left"),
	Tongue_Right = 28           UMETA(DisplayName = "Tongue_Right"),
	Tongue_Up = 29              UMETA(DisplayName = "Tongue_Up"),
	Tongue_Down = 30            UMETA(DisplayName = "Tongue_Down"),
	Tongue_Roll = 31            UMETA(DisplayName = "Tongue_Roll"),
	Tongue_LongStep2 = 32       UMETA(DisplayName = "Tongue_LongStep2"),
	Tongue_UpRight_Morph = 33   UMETA(DisplayName = "Tongue_UpRight_Morph"),
	Tongue_UpLeft_Morph = 34    UMETA(DisplayName = "Tongue_UpLeft_Morph"),
	Tongue_DownRight_Morph = 35 UMETA(DisplayName = "Tongue_DownRight_Morph"),
	Tongue_DownLeft_Morph = 36  UMETA(DisplayName = "Tongue_DownLeft_Morph"),
	Max = 37                    UMETA(Hidden),
	None = 63                   UMETA(DisplayName = "None"),
};