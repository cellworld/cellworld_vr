#pragma once
#include "ControllerTypes.generated.h"

UENUM()
enum class EControllerTypes : uint8 {
	DEBUG			UMETA("Debugging"),
	VR				UMETA("Virtual Reality"),
	MOUSE_KEYBOARD	UMETA("Mouse and Keyboard only"),
};