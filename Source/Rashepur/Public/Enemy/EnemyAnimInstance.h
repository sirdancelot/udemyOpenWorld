// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CharacterStates.h"

#include "EnemyAnimInstance.generated.h"

class UCharacterMovementComponent;
class AEnemy;
/**
 * 
 */
UCLASS()
class RASHEPUR_API UEnemyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime);

	UPROPERTY(BlueprintReadOnly, Category = "AnimInstance")
	AEnemy* Enemy;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	UCharacterMovementComponent* CharacterMovement;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float GroundSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool IsFalling = false;

	UPROPERTY(BlueprintReadOnly, Category = "Movement | Character State")
	EEnemyState EnemyState;

};
