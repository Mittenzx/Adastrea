// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#include "StandardResult.h"
#include "Serialization/JsonSerializer.h"

TSharedPtr<FJsonObject> FAdastreaResult::ToJson() const
{
	TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
	
	JsonObject->SetStringField(TEXT("status"), 
		Status == EAdastreaResultStatus::Success ? TEXT("ok") : TEXT("error"));
	JsonObject->SetStringField(TEXT("message"), Message);
	
	// Add details
	TSharedPtr<FJsonObject> DetailsObject = MakeShared<FJsonObject>();
	for (const auto& Detail : Details)
	{
		DetailsObject->SetStringField(Detail.Key, Detail.Value);
	}
	JsonObject->SetObjectField(TEXT("details"), DetailsObject);
	
	return JsonObject;
}

FAdastreaResult FAdastreaResult::FromJson(const TSharedPtr<FJsonObject>& JsonObject)
{
	FAdastreaResult Result;
	
	if (!JsonObject.IsValid())
	{
		return MakeError(TEXT("Invalid JSON object"));
	}
	
	// Parse status
	FString StatusStr;
	if (JsonObject->TryGetStringField(TEXT("status"), StatusStr))
	{
		Result.Status = StatusStr.Equals(TEXT("ok"), ESearchCase::IgnoreCase) 
			? EAdastreaResultStatus::Success 
			: EAdastreaResultStatus::Error;
	}
	
	// Parse message
	JsonObject->TryGetStringField(TEXT("message"), Result.Message);
	
	// Parse details
	const TSharedPtr<FJsonObject>* DetailsObject;
	if (JsonObject->TryGetObjectField(TEXT("details"), DetailsObject))
	{
		for (const auto& Pair : (*DetailsObject)->Values)
		{
			FString Value;
			if (Pair.Value->TryGetString(Value))
			{
				Result.Details.Add(Pair.Key, Value);
			}
		}
	}
	
	return Result;
}
