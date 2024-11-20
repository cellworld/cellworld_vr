// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

// custom log category
DECLARE_LOG_CATEGORY_EXTERN(LogBotEvade, Log, All);

// // custom log macro
// #define LOG(x, ...) UE_LOG(LogBotEvade, Log, TEXT(x), __VA_ARGS__)
// #define LOG_WARNING(x, ...) UE_LOG(Sandbox, Warning, TEXT(x), __VA_ARGS__)
// #define LOG_ERROR(x, ...) UE_LOG(Sandbox, Error, TEXT(x), __VA_ARGS__)
//


// custom log category

// custom log macro
#define LOG(x, ...) UE_LOG(LogBotEvade, Log, TEXT(x), __VA_ARGS__)
#define LOG_WARNING(x, ...) UE_LOG(LogBotEvade, Warning, TEXT(x), __VA_ARGS__)
#define LOG_ERROR(x, ...) UE_LOG(LogBotEvade, Error, TEXT(x), __VA_ARGS__)