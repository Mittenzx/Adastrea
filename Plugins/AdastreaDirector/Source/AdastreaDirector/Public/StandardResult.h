// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "StandardResult.generated.h"

/**
 * Status enum for operation results
 */
UENUM(BlueprintType)
enum class EAdastreaResultStatus : uint8
{
	Success UMETA(DisplayName = "Success"),
	Error UMETA(DisplayName = "Error")
};

/**
 * Standardized result structure for Adastrea operations.
 * All functions return a consistent result format for better error handling.
 */
USTRUCT(BlueprintType)
struct ADASTREADIRECTOR_API FAdastreaResult
{
	GENERATED_BODY()

	/** Status of the operation */
	UPROPERTY(BlueprintReadOnly, Category = "Adastrea|Result")
	EAdastreaResultStatus Status = EAdastreaResultStatus::Success;

	/** Human-readable message describing the result */
	UPROPERTY(BlueprintReadOnly, Category = "Adastrea|Result")
	FString Message;

	/** Additional details as key-value pairs */
	UPROPERTY(BlueprintReadOnly, Category = "Adastrea|Result")
	TMap<FString, FString> Details;

	/** Default constructor */
	FAdastreaResult() = default;

	/** Convenience constructor for success result */
	static FAdastreaResult MakeSuccess(const FString& InMessage)
	{
		FAdastreaResult Result;
		Result.Status = EAdastreaResultStatus::Success;
		Result.Message = InMessage;
		return Result;
	}

	/** Convenience constructor for error result */
	static FAdastreaResult MakeError(const FString& InMessage)
	{
		FAdastreaResult Result;
		Result.Status = EAdastreaResultStatus::Error;
		Result.Message = InMessage;
		return Result;
	}

	/** Check if the operation was successful */
	bool IsSuccess() const { return Status == EAdastreaResultStatus::Success; }

	/** Check if the operation failed */
	bool IsError() const { return Status == EAdastreaResultStatus::Error; }

	/** Add a detail to the result */
	void AddDetail(const FString& Key, const FString& Value)
	{
		Details.Add(Key, Value);
	}

	/** Convert to JSON object */
	TSharedPtr<FJsonObject> ToJson() const;

	/** Create from JSON object */
	static FAdastreaResult FromJson(const TSharedPtr<FJsonObject>& JsonObject);
};
