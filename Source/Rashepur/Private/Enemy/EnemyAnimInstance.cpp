// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/EnemyAnimInstance.h"
#include "Enemy/Enemy.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"


void UEnemyAnimInstance::NativeInitializeAnimation()
{
    Enemy = Cast<AEnemy>(TryGetPawnOwner());
    if (Enemy)
    {
        CharacterMovement = Enemy->GetCharacterMovement();
    }
}

void UEnemyAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
    Super::NativeUpdateAnimation(DeltaTime);
    if (CharacterMovement)
    {
        GroundSpeed = UKismetMathLibrary::VSizeXY(CharacterMovement->Velocity);
        IsFalling = CharacterMovement->IsFalling();
        EnemyState = Enemy->GetEnemyState();
    }
}



