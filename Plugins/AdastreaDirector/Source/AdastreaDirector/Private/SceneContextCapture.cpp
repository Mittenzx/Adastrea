// Copyright (c) 2024 Mittenzx. All Rights Reserved.

#include "SceneContextCapture.h"
#include "AdastreaDirectorModule.h"
#if WITH_EDITOR
#include "LevelEditor.h"
#include "Editor.h"
#endif
#include "Engine/World.h"
#include "Engine/Level.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Misc/Base64.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "EngineUtils.h"
#if WITH_EDITOR
#include "Engine/Selection.h"
#endif
#include "RenderingThread.h"
#include "RenderCommandFence.h"

#if WITH_EDITOR
FString USceneContextCapture::CaptureViewportScreenshot()
{
	TArray<uint8> ImageData;
	int32 Width = 0;
	int32 Height = 0;

	if (!CaptureViewportToImage(ImageData, Width, Height))
	{
		UE_LOG(LogAdastreaDirector, Warning, TEXT("Failed to capture viewport screenshot"));
		return TEXT("");
	}

	// Encode to base64
	FString Base64String = FBase64::Encode(ImageData);
	
	UE_LOG(LogAdastreaDirector, Log, TEXT("Captured screenshot (%dx%d, %d bytes)"), 
		Width, Height, ImageData.Num());
	
	return Base64String;
}

bool USceneContextCapture::CaptureViewportToImage(TArray<uint8>& OutImageData, int32& OutWidth, int32& OutHeight)
{
	// Verify we're on game thread
	if (!IsInGameThread())
	{
		UE_LOG(LogAdastreaDirector, Error, TEXT("CaptureViewportToImage must be called from game thread"));
		return false;
	}

	// Get editor viewport
	if (!GEditor)
	{
		UE_LOG(LogAdastreaDirector, Error, TEXT("GEditor is null"));
		return false;
	}

	FViewport* Viewport = GEditor->GetActiveViewport();
	if (!Viewport)
	{
		UE_LOG(LogAdastreaDirector, Error, TEXT("No active viewport"));
		return false;
	}

	// Get viewport size
	FIntPoint ViewportSize = Viewport->GetSizeXY();
	if (ViewportSize.X <= 0 || ViewportSize.Y <= 0)
	{
		UE_LOG(LogAdastreaDirector, Error, TEXT("Invalid viewport size: %dx%d"), 
			ViewportSize.X, ViewportSize.Y);
		return false;
	}

	OutWidth = ViewportSize.X;
	OutHeight = ViewportSize.Y;

	// Flush rendering commands to ensure stable state
	FRenderCommandFence Fence;
	Fence.BeginFence();
	Fence.Wait();

	// Re-validate viewport after flush
	Viewport = GEditor->GetActiveViewport();
	if (!Viewport)
	{
		UE_LOG(LogAdastreaDirector, Warning, TEXT("Viewport became invalid after flush"));
		return false;
	}

	// Read pixels from viewport
	TArray<FColor> Bitmap;
	FIntRect Rect(0, 0, OutWidth, OutHeight);
	FReadSurfaceDataFlags ReadFlags(RCM_UNorm, CubeFace_MAX);
	ReadFlags.SetLinearToGamma(false);

	if (!Viewport->ReadPixels(Bitmap, ReadFlags, Rect))
	{
		UE_LOG(LogAdastreaDirector, Warning, TEXT("ReadPixels failed"));
		return false;
	}

	// Validate bitmap
	const int32 ExpectedPixelCount = OutWidth * OutHeight;
	if (Bitmap.Num() != ExpectedPixelCount)
	{
		UE_LOG(LogAdastreaDirector, Warning, TEXT("Invalid bitmap size: %d (expected %d)"), 
			Bitmap.Num(), ExpectedPixelCount);
		return false;
	}

	// Convert to PNG
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

	if (!ImageWrapper.IsValid())
	{
		UE_LOG(LogAdastreaDirector, Error, TEXT("Failed to create image wrapper"));
		return false;
	}

	// Set raw image data
	const int32 ImageDataSize = Bitmap.Num() * sizeof(FColor);
	if (!ImageWrapper->SetRaw(Bitmap.GetData(), ImageDataSize, OutWidth, OutHeight, ERGBFormat::BGRA, 8))
	{
		UE_LOG(LogAdastreaDirector, Error, TEXT("Failed to set raw image data"));
		return false;
	}

	// Get compressed PNG data
	OutImageData = ImageWrapper->GetCompressed();
	if (OutImageData.Num() <= 0)
	{
		UE_LOG(LogAdastreaDirector, Error, TEXT("Image compression failed"));
		return false;
	}

	return true;
}

FString USceneContextCapture::GetSceneSummary(int32 PageSize)
{
	if (!GEditor)
	{
		return TEXT("{}");
	}

	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		return TEXT("{}");
	}

	TArray<TSharedPtr<FJsonValue>> ActorArray;
	int32 ActorCount = 0;

	for (TActorIterator<AActor> It(World); It && ActorCount < PageSize; ++It)
	{
		AActor* Actor = *It;
		if (Actor && !Actor->IsA<AWorldSettings>())
		{
			TSharedPtr<FJsonObject> ActorObj = SerializeActor(Actor);
			if (ActorObj.IsValid())
			{
				ActorArray.Add(MakeShared<FJsonValueObject>(ActorObj));
				ActorCount++;
			}
		}
	}

	// Build response JSON
	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();
	Response->SetArrayField(TEXT("actors"), ActorArray);
	Response->SetNumberField(TEXT("count"), ActorCount);
	Response->SetNumberField(TEXT("page_size"), PageSize);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(Response.ToSharedRef(), Writer);

	return OutputString;
}

FString USceneContextCapture::QueryScene(const FString& FiltersJson)
{
	// Default max results for scene queries
	static constexpr int32 DefaultMaxResults = 20;
	
	TSharedPtr<FJsonObject> Filters;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(FiltersJson);
	
	if (!FJsonSerializer::Deserialize(Reader, Filters) || !Filters.IsValid())
	{
		UE_LOG(LogAdastreaDirector, Warning, TEXT("Invalid filter JSON"));
		return TEXT("[]");
	}

	if (!GEditor)
	{
		return TEXT("[]");
	}

	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		return TEXT("[]");
	}

	// Parse filters using safe TryGet methods to avoid exceptions
	FString ClassContains;
	FString NameContains;
	FString LabelContains;
	int32 MaxResults = DefaultMaxResults;
	
	Filters->TryGetStringField(TEXT("class_contains"), ClassContains);
	Filters->TryGetStringField(TEXT("name_contains"), NameContains);
	Filters->TryGetStringField(TEXT("label_contains"), LabelContains);
	Filters->TryGetNumberField(TEXT("max_results"), MaxResults);
	
	if (MaxResults <= 0) MaxResults = DefaultMaxResults;

	TArray<TSharedPtr<FJsonValue>> Results;
	int32 ResultCount = 0;

	for (TActorIterator<AActor> It(World); It && ResultCount < MaxResults; ++It)
	{
		AActor* Actor = *It;
		if (!Actor || Actor->IsA<AWorldSettings>())
		{
			continue;
		}

		bool bMatches = true;

		// Apply filters
		if (!ClassContains.IsEmpty())
		{
			FString ClassName = Actor->GetClass()->GetName();
			if (!ClassName.Contains(ClassContains))
			{
				bMatches = false;
			}
		}

		if (bMatches && !NameContains.IsEmpty())
		{
			if (!Actor->GetName().Contains(NameContains))
			{
				bMatches = false;
			}
		}

		if (bMatches && !LabelContains.IsEmpty())
		{
			if (!Actor->GetName().Contains(LabelContains))
			{
				bMatches = false;
			}
		}

		if (bMatches)
		{
			TSharedPtr<FJsonObject> ActorObj = SerializeActor(Actor);
			if (ActorObj.IsValid())
			{
				Results.Add(MakeShared<FJsonValueObject>(ActorObj));
				ResultCount++;
			}
		}
	}

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(Results, Writer);

	return OutputString;
}

FString USceneContextCapture::GetSelectedActorsSummary()
{
	TArray<TSharedPtr<FJsonValue>> Results;

	if (GEditor)
	{
		USelection* Selection = GEditor->GetSelectedActors();
		if (Selection)
		{
			for (FSelectionIterator It(*Selection); It; ++It)
			{
				AActor* Actor = Cast<AActor>(*It);
				if (Actor)
				{
					TSharedPtr<FJsonObject> ActorObj = SerializeActor(Actor);
					if (ActorObj.IsValid())
					{
						Results.Add(MakeShared<FJsonValueObject>(ActorObj));
					}
				}
			}
		}
	}

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(Results, Writer);

	return OutputString;
}
#else
FString USceneContextCapture::CaptureViewportScreenshot()
{
	return TEXT("");
}

bool USceneContextCapture::CaptureViewportToImage(TArray<uint8>& OutImageData, int32& OutWidth, int32& OutHeight)
{
	return false;
}

FString USceneContextCapture::GetSceneSummary(int32 PageSize)
{
	return TEXT("{}");
}

FString USceneContextCapture::QueryScene(const FString& FiltersJson)
{
	return TEXT("[]");
}

FString USceneContextCapture::GetSelectedActorsSummary()
{
	return TEXT("[]");
}
#endif

TSharedPtr<FJsonObject> USceneContextCapture::SerializeActor(AActor* Actor)
{
	if (!Actor)
	{
		return nullptr;
	}

	TSharedPtr<FJsonObject> ActorObj = MakeShared<FJsonObject>();
	
	ActorObj->SetStringField(TEXT("name"), Actor->GetName());
	ActorObj->SetStringField(TEXT("label"), Actor->GetName());
	ActorObj->SetStringField(TEXT("class"), Actor->GetClass()->GetName());

	// Location
	FVector Location = Actor->GetActorLocation();
	TSharedPtr<FJsonObject> LocationObj = MakeShared<FJsonObject>();
	LocationObj->SetNumberField(TEXT("x"), Location.X);
	LocationObj->SetNumberField(TEXT("y"), Location.Y);
	LocationObj->SetNumberField(TEXT("z"), Location.Z);
	ActorObj->SetObjectField(TEXT("location"), LocationObj);

	// Rotation
	FRotator Rotation = Actor->GetActorRotation();
	TSharedPtr<FJsonObject> RotationObj = MakeShared<FJsonObject>();
	RotationObj->SetNumberField(TEXT("pitch"), Rotation.Pitch);
	RotationObj->SetNumberField(TEXT("yaw"), Rotation.Yaw);
	RotationObj->SetNumberField(TEXT("roll"), Rotation.Roll);
	ActorObj->SetObjectField(TEXT("rotation"), RotationObj);

	// Components (limited to avoid large JSON payloads)
	static constexpr int32 MaxComponentsToSerialize = 5;
	
	TArray<TSharedPtr<FJsonValue>> Components;
	int32 ComponentCount = 0;
	for (UActorComponent* Comp : Actor->GetComponents())
	{
		if (Comp && ComponentCount < MaxComponentsToSerialize)
		{
			TSharedPtr<FJsonObject> CompObj = SerializeComponent(Comp);
			if (CompObj.IsValid())
			{
				Components.Add(MakeShared<FJsonValueObject>(CompObj));
				ComponentCount++;
			}
		}
	}
	ActorObj->SetArrayField(TEXT("components"), Components);

	return ActorObj;
}

TSharedPtr<FJsonObject> USceneContextCapture::SerializeComponent(UActorComponent* Component)
{
	if (!Component)
	{
		return nullptr;
	}

	TSharedPtr<FJsonObject> CompObj = MakeShared<FJsonObject>();
	CompObj->SetStringField(TEXT("name"), Component->GetName());
	CompObj->SetStringField(TEXT("class"), Component->GetClass()->GetName());

	return CompObj;
}
