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
	void DisableCapsule();

	void PlayMontageSection(UAnimMontage* Montage, const FName& SectionName, float AnimationSpeed = 1.0f);
	FName RandomMontageSection(UAnimMontage* Montage, FString MontagePrefix);

	virtual bool CanAttack();
	bool IsAlive();
	UFUNCTION(BlueprintCallable)
	virtual void AttackEnd();

	void PlayHitSound(const FVector& ImpactPoint);
	void SpawnHitParticles(const FVector& ImpactPoint);
	virtual void HandleDamage(float DamageAmount);

	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float DeathLifeSpan = 10.0f;

	UPROPERTY(BlueprintReadWrite)
	EActionState ActionState = EActionState::EAS_Unoccupied;

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
	EDeathPose DeathPose = EDeathPose::EDP_Death1;

	/*
	* VFX
	*/

	UPROPERTY(EditAnywhere, Category = "Sounds")
	USoundBase* HitSound;

	UPROPERTY(EditAnywhere, Category = "VFX")
	UParticleSystem* HitParticles;
};
