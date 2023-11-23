// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/HitInterface.h"
#include "CharacterStates.h"
#include "GameFramework/Character.h"
#include "BaseCharacter.generated.h"

class AWeapon;
class UAnimMontage;
class UAttributeComponent;


UCLASS()
class RASHEPUR_API ABaseCharacter : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:
	ABaseCharacter();
	virtual void Tick(float DeltaTime) override;
	UFUNCTION(BlueprintCallable)
	void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled);

protected:
	virtual void BeginPlay() override;
	virtual void Attack();
	virtual void Die();
	/**
	* Animation Montages
	*/
	virtual void PlayAttackMontage();
	virtual UAnimMontage* GetAttackMontageByWeaponType();
	FOnMontageEnded EndMontageDelegate;
	void OnActionEnded(UAnimMontage* Montage, bool bInterrupted);

	void PlayHitReactMontage(const FName& SectionName);
	void DirectionalHitReact(const FVector& ImpactPoint);
	void SelectDeathMontage();

	virtual bool CanAttack();
	UFUNCTION(BlueprintCallable)
	virtual void AttackEnd();

	UPROPERTY(BlueprintReadWrite)
	EActionState ActionState = EActionState::EAS_Unoccupied;

	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"), Category = "Animation")
	float AttackAnimationSpeed = 1.0f;

	UPROPERTY(EditDefaultsOnly)
	UAttributeComponent* CharAttributes;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Weapon")
	AWeapon* EquippedWeapon;

	/** Animation Montages */
	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	UAnimMontage* AttackMontage1H;

	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	UAnimMontage* AttackMontage2H;

	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	UAnimMontage* DeathMontage;

	UPROPERTY(BlueprintReadOnly)
	EDeathPose DeathPose = EDeathPose::EDP_Alive;

	/*
	* VFX
	*/

	UPROPERTY(EditAnywhere, Category = "Sounds")
	USoundBase* HitSound;

	UPROPERTY(EditAnywhere, Category = "VFX")
	UParticleSystem* HitParticles;
};
