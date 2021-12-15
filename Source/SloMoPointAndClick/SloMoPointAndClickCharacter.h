// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SloMoPointAndClickCharacter.generated.h"

class UCheatManager;

UCLASS(Blueprintable)
class ASloMoPointAndClickCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASloMoPointAndClickCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** Returns TopDownCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns CursorToWorld subobject **/
	FORCEINLINE class UDecalComponent* GetCursorToWorld() { return CursorToWorld; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slomo")
	float CurrentSloMo;

private:
	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** A decal that projects to the cursor location. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UDecalComponent* CursorToWorld;

	bool bIsSloMo;
	float OldSloMo = 1;
	float DesiredSloMo = 1;
	float DeltaSecondsWithMargin = 1;
	UCheatManager* CheatManager;

	void SetSlowMo(float SloMoAmmount);

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Character")
	float TimeMargin = 1.0f;
};

