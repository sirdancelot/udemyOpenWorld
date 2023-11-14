// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroAnimInstance.h"
#include "HeroCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UHeroAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    HeroCharacter = Cast<AHeroCharacter>(TryGetPawnOwner());
    if (HeroCharacter) 
    {
        HeroCharacterMovement = HeroCharacter->GetCharacterMovement();
    }
}

void UHeroAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
    Super::NativeUpdateAnimation(DeltaTime);
    if (HeroCharacterMovement) 
    {
        GroundSpeed = UKismetMathLibrary::VSizeXY(HeroCharacterMovement->Velocity);
        IsFalling = HeroCharacterMovement->IsFalling();
    }

}
