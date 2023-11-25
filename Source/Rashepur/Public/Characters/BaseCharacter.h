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

protected:
	virtual void BeginPlay() override;
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;
	virtual void Attack();
	virtual void Die();

	UFUNCTION(BlueprintCallable)
	void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled);

	virtual	void MoveTo(AActor* Target, bool DrawDebugSpheresOnPath = false);

	FName GetWeaponSocket(AWeapon* Weapon);

	/**
	 *	Animation Montages
	 */
	virtual UAnimMontage* GetAttackMontageByWeaponType();
	void DirectionalHitReact(const FVector& ImpactPoint);
	void SelectDeathMontage();
	virtual void PlayAttackMontage();
	FName GetWeaponSpineSocket(AWeapon* OverlappingWeapon);
	void SetCharacterStateByWeaponType();

	void PlayHitReactMontage(const FName& SectionName);
	void PlayMontageSection(UAnimMontage* Montage, const FName& SectionName, float AnimationSpeed = 1.0f);
	void StopAnimMontage(UAnimMontage* Montage);
	FName RandomMontageSection(UAnimMontage* Montage, FString MontagePrefix);

	virtual void OnActionEnded(UAnimMontage* Montage, bool bInterrupted); // callback to end montage

	/** 
	 *	Combat
	 */
	void DisableCapsule();
	virtual bool CanAttack();
	bool IsAlive();
	virtual void HandleDamage(float DamageAmount);

	/**
	 *	Special Effects
	 */
	void PlayHitSound(const FVector& ImpactPoint);
	void SpawnHitParticles(const FVector& ImpactPoint);

	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;
	FOnMontageEnded EndMontageDelegate;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float DeathLifeSpan = 10.0f;

	UPROPERTY(BlueprintReadWrite)
	EActionState ActionState = EActionState::EAS_Unoccupied;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"), Category = "Combat")
	float AttackAnimationSpeed = 1.0f;

	UPROPERTY(EditDefaultsOnly)
	UAttributeComponent* CharAttributes;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Weapon")
	AWeapon* EquippedWeapon;

	UPROPERTY(BlueprintReadOnly)
	EDeathPose DeathPose = EDeathPose::EDP_Death1;

	UPROPERTY(EditAnywhere, Category = "Special Effects")
	USoundBase* HitSound;

	UPROPERTY(EditAnywhere, Category = "Special Effects")
	UParticleSystem* HitParticles;

	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bDebugStates = false;

private:
	/** Animation Montages */
	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	UAnimMontage* AttackMontage1H;

	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	UAnimMontage* AttackMontage2H;

	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	UAnimMontage* DeathMontage;
};
