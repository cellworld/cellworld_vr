// (c) Copyright 2019-2020 HP Development Company, L.P.

#pragma once

#include "UObject/ObjectMacros.h"

//https://answers.unrealengine.com/questions/36107/blueprints-with-multiple-exec-output-pins.html

UENUM(BlueprintType)
enum class EExecution : uint8
{
    //When an execution pin uses the word "Then", it is unlabeled
    //Users will interpret this as the default path.
    Then,
    Error
};