// Copyright Epic Games, Inc. All Rights Reserved.

#include "SloMoPointAndClickPlayerController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Runtime/Engine/Classes/Components/DecalComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "SloMoPointAndClickCharacter.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/World.h"

ASloMoPointAndClickPlayerController::ASloMoPointAndClickPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
}

void ASloMoPointAndClickPlayerController::BeginPlay()
{
	SetSloMo(CurrentSloMo);
}

void ASloMoPointAndClickPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	// keep updating the destination every tick while desired
	if (bMoveToMouseCursor)
	{
		MoveToMouseCursor();
	}

	// Szlachu Test of getting player movement TODO refactor it / break to smaller functions, It's not readable as is
	if (bIsDestinationReached == false && !Destination.IsZero())
	{
		DistanceToDestination = FVector::Dist(this->GetPawn()->GetActorLocation(), Destination);
		if (DistanceToDestination >= 300)
		{
			if (!FMath::IsNearlyEqual(DesiredSloMo, 1.0f, 0.0001f))
			{
				DeltaSecondsWithMargin = GetWorld()->GetRealTimeSeconds() + TimeMargin;
				OldSloMo = CurrentSloMo;
			}

			DesiredSloMo = 1.0f; // TODO change it to variable
		}
		else
		{
			if (!FMath::IsNearlyEqual(DesiredSloMo, 0.05f, 0.0001f)) // TODO Add tolerance as a variable
			{
				DeltaSecondsWithMargin = GetWorld()->GetRealTimeSeconds() + TimeMargin;
				OldSloMo = CurrentSloMo;
			}

			DesiredSloMo = 0.05f; // TODO change it to variable
		}

		if (!FMath::IsNearlyEqual(CurrentSloMo, DesiredSloMo, 0.0001f))
		{
			RealTimeSeconds = GetWorld()->GetRealTimeSeconds(); // Can be a pain when we add pause 
			MarginHolder = DeltaSecondsWithMargin - RealTimeSeconds;
			LerpAlpha = FMath::Clamp(MarginHolder / TimeMargin, 0.0f, 1.0f);
			CurrentSloMo = FMath::Lerp(DesiredSloMo, OldSloMo, LerpAlpha);

			FString DebugMessageHolder = FString::Format(TEXT("Old Slomo: {0}\nCurrent Slomo: {1}\nDesired Slomo: {2}\nLerp Alpha: {3}\n"), { OldSloMo, CurrentSloMo, DesiredSloMo, LerpAlpha });
			GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Red, DebugMessageHolder);

			SetSloMo(CurrentSloMo);
		}
		else if (this->GetPawn()->GetActorLocation().Equals(Destination, 100.0f))
		{
			bIsDestinationReached = true;
		}
	}
	//
}

void ASloMoPointAndClickPlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	InputComponent->bBlockInput = false;

	InputComponent->BindAction("SetDestination", IE_Pressed, this, &ASloMoPointAndClickPlayerController::OnSetDestinationPressed);
	InputComponent->BindAction("SetDestination", IE_Released, this, &ASloMoPointAndClickPlayerController::OnSetDestinationReleased);

	// support touch devices 
	InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &ASloMoPointAndClickPlayerController::MoveToTouchLocation);
	InputComponent->BindTouch(EInputEvent::IE_Repeat, this, &ASloMoPointAndClickPlayerController::MoveToTouchLocation);

	InputComponent->BindAction("ResetVR", IE_Pressed, this, &ASloMoPointAndClickPlayerController::OnResetVR);

	// LSzlachciak Edit
	WorldSettings = CastToPlayerController()->GetWorldSettings();

	//

}

void ASloMoPointAndClickPlayerController::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ASloMoPointAndClickPlayerController::MoveToMouseCursor()
{
	if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled())
	{
		if (ASloMoPointAndClickCharacter* MyPawn = Cast<ASloMoPointAndClickCharacter>(GetPawn()))
		{
			if (MyPawn->GetCursorToWorld())
			{
				UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, MyPawn->GetCursorToWorld()->GetComponentLocation());
			}
		}
	}
	else
	{
		// Trace to see what is under the mouse cursor
		FHitResult Hit;
		GetHitResultUnderCursor(ECC_Visibility, false, Hit);

		if (Hit.bBlockingHit)
		{
			// We hit something, move there
			SetNewMoveDestination(Hit.ImpactPoint);
		}
	}
}

void ASloMoPointAndClickPlayerController::MoveToTouchLocation(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	FVector2D ScreenSpaceLocation(Location);

	// Trace to see what is under the touch location
	FHitResult HitResult;
	GetHitResultAtScreenPosition(ScreenSpaceLocation, CurrentClickTraceChannel, true, HitResult);
	if (HitResult.bBlockingHit)
	{
		// We hit something, move there
		SetNewMoveDestination(HitResult.ImpactPoint);
	}
}

void ASloMoPointAndClickPlayerController::SetNewMoveDestination(const FVector DestLocation)
{
	APawn* const MyPawn = GetPawn();
	if (MyPawn)
	{
		float const Distance = FVector::Dist(DestLocation, MyPawn->GetActorLocation());

		// We need to issue move command only if far enough in order for walk animation to play correctly
		if ((Distance > 120.0f))
		{
			UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, DestLocation);
		}

		// Lszlachciak edit
		Destination = DestLocation;
		bIsDestinationReached = false;
		//
	}
}

void ASloMoPointAndClickPlayerController::OnSetDestinationPressed()
{
	// set flag to keep updating destination until released
	bMoveToMouseCursor = true;
}

void ASloMoPointAndClickPlayerController::OnSetDestinationReleased()
{
	// clear flag to indicate we should stop updating the destination
	bMoveToMouseCursor = false;
}


void ASloMoPointAndClickPlayerController::SetSloMo(float SloMoAmmount)
{
	if (WorldSettings != nullptr)
	{
		WorldSettings->SetTimeDilation(SloMoAmmount);
	}
}
