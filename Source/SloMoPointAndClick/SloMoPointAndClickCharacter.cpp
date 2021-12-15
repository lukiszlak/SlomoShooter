// Copyright Epic Games, Inc. All Rights Reserved.

#include "SloMoPointAndClickCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/CheatManager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Materials/Material.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/World.h"
#include "Blueprint\AIBlueprintHelperLibrary.h"

ASloMoPointAndClickCharacter::ASloMoPointAndClickCharacter()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true); // Don't want arm to rotate when character does
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Create a decal in the world to show the cursor's location
	CursorToWorld = CreateDefaultSubobject<UDecalComponent>("CursorToWorld");
	CursorToWorld->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UMaterial> DecalMaterialAsset(TEXT("Material'/Game/TopDownCPP/Blueprints/M_Cursor_Decal.M_Cursor_Decal'"));
	if (DecalMaterialAsset.Succeeded())
	{
		CursorToWorld->SetDecalMaterial(DecalMaterialAsset.Object);
	}
	CursorToWorld->DecalSize = FVector(16.0f, 32.0f, 32.0f);
	CursorToWorld->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f).Quaternion());

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

}

void ASloMoPointAndClickCharacter::BeginPlay()
{
	Super::BeginPlay();

	// We need cheat manager for using SloMo
	CheatManager = this->GetController()->CastToPlayerController()->CheatManager;

	bIsSloMo = false;
}


void ASloMoPointAndClickCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
	if (CursorToWorld != nullptr)
	{
		if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled())
		{
			if (UWorld* World = GetWorld())
			{
				FHitResult HitResult;
				FCollisionQueryParams Params(NAME_None, FCollisionQueryParams::GetUnknownStatId());
				FVector StartLocation = TopDownCameraComponent->GetComponentLocation();
				FVector EndLocation = TopDownCameraComponent->GetComponentRotation().Vector() * 2000.0f;
				Params.AddIgnoredActor(this);
				World->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, Params);
				FQuat SurfaceRotation = HitResult.ImpactNormal.ToOrientationRotator().Quaternion();
				CursorToWorld->SetWorldLocationAndRotation(HitResult.Location, SurfaceRotation);
			}
		}
		else if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			FHitResult TraceHitResult;
			PC->GetHitResultUnderCursor(ECC_Visibility, true, TraceHitResult);
			FVector CursorFV = TraceHitResult.ImpactNormal;
			FRotator CursorR = CursorFV.Rotation();
			CursorToWorld->SetWorldLocation(TraceHitResult.Location);
			CursorToWorld->SetWorldRotation(CursorR);
		}
	}

	// Szlachu Test of getting player movement
	if (!GetVelocity().Equals(FVector(0, 0, 0), 100) && bIsSloMo == false)
	{
		bIsSloMo = true;
		DeltaSecondsWithMargin = GetWorld()->GetRealTimeSeconds() + TimeMargin;
		OldSloMo = CurrentSloMo;
		DesiredSloMo = 1.0f;
		// TODO add some sort of bool/enum/check if we have already started changing SloMo ammount
		// Also think of something else better sounding then SloMoAmmount
	}
	else if (GetVelocity().Equals(FVector(0, 0, 0), 100) && bIsSloMo == true)
	{
		bIsSloMo = false;
		DeltaSecondsWithMargin = GetWorld()->GetRealTimeSeconds() + TimeMargin;
		OldSloMo = CurrentSloMo;
		DesiredSloMo = 0.05f;
	}

	if (CurrentSloMo != DesiredSloMo)
	{
		float MarginHolder = DeltaSecondsWithMargin - GetWorld()->GetRealTimeSeconds();
		float LerpAlpha = FMath::Clamp(MarginHolder / TimeMargin, 0.0f, 1.0f);
		CurrentSloMo = FMath::Lerp(DesiredSloMo, OldSloMo, LerpAlpha);

		FString DebugMessageHolder = FString::Format(TEXT("Old Slomo: {0}\nCurrent Slomo: {1}\nDesired Slomo: {2}\nLerp Alpha: {3}\n"), { OldSloMo, CurrentSloMo, DesiredSloMo, LerpAlpha});
		GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Red, DebugMessageHolder);

		SetSlowMo(CurrentSloMo);
	}
	//
}

void ASloMoPointAndClickCharacter::SetSlowMo(float SloMoAmmount)
{
	CheatManager->Slomo(SloMoAmmount);
}
