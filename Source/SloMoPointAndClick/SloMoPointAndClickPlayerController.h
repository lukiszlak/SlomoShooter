// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SloMoPointAndClickPlayerController.generated.h"

UCLASS()
class ASloMoPointAndClickPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ASloMoPointAndClickPlayerController();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SloMo")
		float CurrentSloMo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SloMo")
		float TimeMargin = 1.0f;

protected:
	/** True if the controlled character should navigate to the mouse cursor. */
	uint32 bMoveToMouseCursor : 1;

	/** Stores player destination **/
	FVector fMoveDestination;

	// Begin PlayerController interface
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;
	// End PlayerController interface

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Navigate player to the current mouse cursor location. */
	void MoveToMouseCursor();

	/** Navigate player to the current touch location. */
	void MoveToTouchLocation(const ETouchIndex::Type FingerIndex, const FVector Location);
	
	/** Navigate player to the given world location. */
	void SetNewMoveDestination(const FVector DestLocation);

	/** Input handlers for SetDestination action. */
	void OnSetDestinationPressed();
	void OnSetDestinationReleased();

private:
	bool bIsSloMo;
	bool bIsDestinationReached;
	float OldSloMo = 1;
	float DesiredSloMo = 0.05f; // TODO maybe change it? Current logic need it to not be 1
	float DeltaSecondsWithMargin = 1;
	float DistanceToDestination;
	float RealTimeSeconds;
	float MarginHolder;
	float LerpAlpha;

	AWorldSettings* WorldSettings;
	FVector Destination;

	void SetSloMo(float SloMoAmmount);

};


