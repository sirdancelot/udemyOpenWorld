// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CharacterStates.h"

#include "HeroAnimInstance.generated.h"

/**
 * 
 */
class AHeroCharacter;
class UCharacterMovementComponent;

UCLASS()
class RASHEPUR_API UHeroAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime);

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	AHeroCharacter* HeroCharacter;

	UPROPERTY(BlueprintReadOnly, Category = "Movement" )
	UCharacterMovementComponent* HeroCharacterMovement;

	UPROPERTY(BlueprintReadOnly, Category = "Movement" )
	float GroundSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "Movement" )
	bool IsFalling = false;

	ECharacterState CharacterState;

};
