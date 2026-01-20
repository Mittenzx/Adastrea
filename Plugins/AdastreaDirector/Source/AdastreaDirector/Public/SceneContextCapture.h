// Copyright (c) 2024 Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SceneContextCapture.generated.h"

/**
 * Utility class for capturing scene context (screenshots and scene data).
 * Enables AI agents to "see" and understand the current level state.
 */
UCLASS()
class ADASTREADIRECTOR_API USceneContextCapture : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Capture viewport screenshot and return as base64 PNG.
	 * @return Base64-encoded PNG string, or empty string on failure
	 */
	UFUNCTION(BlueprintCallable, Category = "Adastrea|Scene")
	static FString CaptureViewportScreenshot();

	/**
	 * Get JSON summary of current scene actors.
	 * @param PageSize Maximum number of actors to include
	 * @return JSON string with actor data
	 */
	UFUNCTION(BlueprintCallable, Category = "Adastrea|Scene")
	static FString GetSceneSummary(int32 PageSize = 100);

	/**
	 * Query scene with filters.
	 * @param FiltersJson JSON object with filter criteria (class_contains, name_contains, etc.)
	 * @return JSON array of matching actors
	 */
	UFUNCTION(BlueprintCallable, Category = "Adastrea|Scene")
	static FString QueryScene(const FString& FiltersJson);

	/**
	 * Get summary of currently selected actors.
	 * @return JSON array of selected actors
	 */
	UFUNCTION(BlueprintCallable, Category = "Adastrea|Scene")
	static FString GetSelectedActorsSummary();

private:
	/**
	 * Internal function to capture viewport to image buffer.
	 * @param OutImageData PNG-encoded image data
	 * @param OutWidth Image width
	 * @param OutHeight Image height
	 * @return true if capture succeeded
	 */
	static bool CaptureViewportToImage(TArray<uint8>& OutImageData, int32& OutWidth, int32& OutHeight);

	/**
	 * Serialize actor to JSON object.
	 * @param Actor Actor to serialize
	 * @return JSON object with actor data
	 */
	static TSharedPtr<FJsonObject> SerializeActor(AActor* Actor);

	/**
	 * Serialize component to JSON object.
	 * @param Component Component to serialize
	 * @return JSON object with component data
	 */
	static TSharedPtr<FJsonObject> SerializeComponent(UActorComponent* Component);
};
