#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputActionValue.h"
#include "SpaceshipControlsComponent.generated.h"

class UInputAction;
class UInputMappingContext;
class UEnhancedInputLocalPlayerSubsystem;
class UEnhancedInputComponent;
class UWeaponComponent;

/**
 * Spaceship Controls Component
 * 
 * Implements Enhanced Input spaceship controls for player-controlled ships.
 * Creates and manages its own Input Mapping Context with the following controls:
 * 
 * Movement:
 * - W/S: Vertical strafe (up/down)
 * - A/D: Horizontal strafe (left/right)
 * 
 * Look:
 * - Mouse: Rotation (pitch for up/down, yaw for left/right)
 * 
 * Combat:
 * - Left Mouse Button: Fire primary weapon
 * 
 * Speed:
 * - Mouse Wheel Up: Increase speed
 * - Mouse Wheel Down: Decrease speed
 * 
 * Usage:
 * - Add component to spaceship pawn Blueprint
 * - Configure MovementSpeed and LookSensitivity as needed
 * - Component will automatically create and register input bindings in BeginPlay
 * - For weapon firing, add a UWeaponComponent to the same actor
 * 
 * Integration:
 * - Works with ASpaceship pawn class
 * - Calls UWeaponComponent::Fire() for weapon functionality
 * - Uses Unreal Engine 5 Enhanced Input System
 */
UCLASS(ClassGroup=(Ships), meta=(BlueprintSpawnableComponent))
class ADASTREA_API USpaceshipControlsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USpaceshipControlsComponent();

	// ====================
	// CONFIGURATION
	// ====================

	/** Movement speed multiplier applied to strafe input */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Controls|Movement", meta=(ClampMin="0.1", ClampMax="10.0"))
	float MovementSpeed;

	/** Movement smoothing - controls how quickly ship responds to WASD input (0 = instant, higher = smoother) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Controls|Movement", meta=(ClampMin="0.0", ClampMax="20.0"))
	float MovementSmoothingSpeed;

	/** When true, movement input is smoothed over time. When false, movement is instant. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Controls|Movement")
	bool bEnableMovementSmoothing;

	/** Look sensitivity multiplier applied to mouse input */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Controls|Look", meta=(ClampMin="0.1", ClampMax="10.0"))
	float LookSensitivity;

	/** When true, inverts the Y axis for look input */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Controls|Look")
	bool bInvertLookY;

	/** Rotation smoothing - controls how quickly ship rotation follows camera (0 = instant, higher = more delay) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Controls|Look", meta=(ClampMin="0.0", ClampMax="20.0"))
	float RotationSmoothingSpeed;

	/** When true, camera rotates freely and ship follows with delay. When false, ship rotates instantly with camera. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Controls|Look")
	bool bEnableRotationSmoothing;

	/** Priority for the spaceship input mapping context */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Controls|Input", meta=(ClampMin="0"))
	int32 InputMappingPriority;

	// ====================
	// SPEED CONFIGURATION
	// ====================

	/** Current speed multiplier (adjusted by mouse wheel) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Controls|Speed", meta=(ClampMin="0.1", ClampMax="10.0"))
	float CurrentSpeed;

	/** Amount to change speed per mouse wheel tick */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Controls|Speed", meta=(ClampMin="0.1", ClampMax="2.0"))
	float SpeedStep;

	/** Minimum speed multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Controls|Speed", meta=(ClampMin="0.1", ClampMax="5.0"))
	float MinSpeed;

	/** Maximum speed multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Controls|Speed", meta=(ClampMin="1.0", ClampMax="10.0"))
	float MaxSpeed;

	// ====================
	// INPUT ACTIONS
	// ====================

	/** Input action for movement (WASD strafe) - 2D axis */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Controls|Input")
	UInputAction* MoveAction;

	/** Input action for look (mouse) - 2D axis */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Controls|Input")
	UInputAction* LookAction;

	/** Input action for primary weapon fire (LMB) - digital */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Controls|Input")
	UInputAction* FireAction;

	/** Input action for speed adjustment (mouse wheel) - 1D axis */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Controls|Input")
	UInputAction* SpeedAction;

	/** Input mapping context for spaceship controls */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Controls|Input")
	UInputMappingContext* SpaceshipMappingContext;

	// ====================
	// BLUEPRINT FUNCTIONS
	// ====================

	/**
	 * Enable spaceship controls input
	 * Adds the input mapping context to the local player
	 */
	UFUNCTION(BlueprintCallable, Category="Controls")
	void EnableControls();

	/**
	 * Disable spaceship controls input
	 * Removes the input mapping context from the local player
	 */
	UFUNCTION(BlueprintCallable, Category="Controls")
	void DisableControls();

	/**
	 * Check if controls are currently enabled
	 * @return True if input mapping context is active
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Controls")
	bool AreControlsEnabled() const;

	/**
	 * Set movement speed multiplier
	 * @param NewSpeed New movement speed value (clamped to valid range)
	 */
	UFUNCTION(BlueprintCallable, Category="Controls|Movement")
	void SetMovementSpeed(float NewSpeed);

	/**
	 * Set look sensitivity multiplier
	 * @param NewSensitivity New sensitivity value (clamped to valid range)
	 */
	UFUNCTION(BlueprintCallable, Category="Controls|Look")
	void SetLookSensitivity(float NewSensitivity);

	/**
	 * Toggle Y axis inversion for look input
	 */
	UFUNCTION(BlueprintCallable, Category="Controls|Look")
	void ToggleInvertLookY();

	/**
	 * Get the current speed multiplier
	 * @return Current speed value
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Controls|Speed")
	float GetCurrentSpeed() const;

	/**
	 * Set the current speed multiplier
	 * @param NewSpeed New speed value (clamped to min/max range)
	 */
	UFUNCTION(BlueprintCallable, Category="Controls|Speed")
	void SetCurrentSpeed(float NewSpeed);

	/**
	 * Increase speed by SpeedStep amount
	 */
	UFUNCTION(BlueprintCallable, Category="Controls|Speed")
	void IncreaseSpeed();

	/**
	 * Decrease speed by SpeedStep amount
	 */
	UFUNCTION(BlueprintCallable, Category="Controls|Speed")
	void DecreaseSpeed();

	// ====================
	// BLUEPRINT NATIVE EVENTS
	// ====================

	/**
	 * Called when movement input is received
	 * Override in Blueprint to customize movement behavior
	 * @param MoveValue 2D movement input (X = horizontal strafe, Y = vertical strafe)
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Controls")
	void OnMoveInput(FVector2D MoveValue);

	/**
	 * Called when look input is received
	 * Override in Blueprint to customize look behavior
	 * @param LookValue 2D look input (X = yaw, Y = pitch)
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Controls")
	void OnLookInput(FVector2D LookValue);

	/**
	 * Called when fire input is pressed
	 * Override in Blueprint to customize fire behavior
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Controls")
	void OnFirePressed();

	/**
	 * Called when fire input is released
	 * Override in Blueprint to customize fire release behavior
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Controls")
	void OnFireReleased();

	/**
	 * Called when speed is changed via mouse wheel
	 * Override in Blueprint to customize speed change behavior
	 * @param NewSpeed The new speed value after adjustment
	 * @param Delta The amount of speed change (positive = increase, negative = decrease)
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Controls")
	void OnSpeedChanged(float NewSpeed, float Delta);

	// ====================
	// EVENTS
	// ====================

	/** Event fired when controls are enabled */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnControlsEnabled);
	UPROPERTY(BlueprintAssignable, Category="Controls|Events")
	FOnControlsEnabled OnControlsEnabled;

	/** Event fired when controls are disabled */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnControlsDisabled);
	UPROPERTY(BlueprintAssignable, Category="Controls|Events")
	FOnControlsDisabled OnControlsDisabled;

	/** Event fired when speed changes */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSpeedChangedEvent, float, NewSpeed, float, Delta);
	UPROPERTY(BlueprintAssignable, Category="Controls|Events")
	FOnSpeedChangedEvent OnSpeedChangedEvent;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * Setup input bindings on the Enhanced Input Component
	 * @param PlayerInputComponent The Enhanced Input Component to bind to
	 */
	void SetupInputBindings(UEnhancedInputComponent* PlayerInputComponent);

	/**
	 * Create the input actions used by this component
	 */
	void CreateInputActions();

	/**
	 * Create the input mapping context and configure key bindings
	 */
	void CreateInputMappingContext();

	/**
	 * Get the Enhanced Input Local Player Subsystem
	 * @return The subsystem or nullptr if not available
	 */
	UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputSubsystem() const;

	/**
	 * Get the weapon component on the owning actor
	 * @return Weapon component or nullptr if not found
	 */
	UWeaponComponent* GetWeaponComponent() const;

	// Input handlers
	void HandleMove(const FInputActionValue& Value);
	void HandleLook(const FInputActionValue& Value);
	void HandleFirePressed(const FInputActionValue& Value);
	void HandleFireReleased(const FInputActionValue& Value);
	void HandleSpeed(const FInputActionValue& Value);

private:
	/** Whether controls are currently enabled */
	bool bControlsEnabled;

	/** Cached reference to weapon component */
	UPROPERTY()
	UWeaponComponent* CachedWeaponComponent;

	/** Current smoothed movement input values */
	FVector2D SmoothedMoveInput;
};
