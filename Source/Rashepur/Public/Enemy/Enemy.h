// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "Enemy.generated.h"

class USoundBase;
class UParticleSystem;
class UHealthBarComponent;
class UPawnSensingComponent;

UCLASS()
class RASHEPUR_API AEnemy : public ABaseCharacter
{
	GENERATED_BODY()

public:
	AEnemy();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/**
	*	Taking Damage
	*/
	virtual void GetHit_Implementation(const FVector& ImpactPoint) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Destroyed() override;
	/**
	 *  AI Navigation
	 */
	void CheckCombatTarget();
	void CheckPatrolTarget();
	
protected:
	virtual void BeginPlay() override;
	void EquipDefaultWeapon();
	virtual void Die() override;

	virtual void Attack() override;
	virtual void PlayAttackMontage() override;

	/**
	 *  AI Navigation
	 */

    void MoveTo(AActor *Target, bool DrawDebugSpheresOnPath = false);
	AActor* ChoosePatrolTarget();
    bool InTargetRange(AActor* Target, double Radius);
	
	UFUNCTION() // se for ser usado como delegate precisa de ufunction
	void PawnSeen(APawn* SeenPawn);

private:
	EEnemyState EnemyState = EEnemyState::EES_Patrolling;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AWeapon> DefaultWeaponClass;
	/* 
	* Components
	*/

	UPROPERTY(VisibleAnywhere)
	UHealthBarComponent* HealthBarWidget;

	UPROPERTY(VisibleAnywhere)
	UPawnSensingComponent* PawnSensing;


	/**
	 * Navigation
	 */
	
	UPROPERTY()
	class AAIController* EnemyController;

	UPROPERTY()
	AActor* CombatTarget;

	UPROPERTY(EditAnywhere)
	double CombatRadius = 500.f;

	UPROPERTY(EditAnywhere)
	double AttackRadius = 150.f;

	UPROPERTY(EditInstanceOnly, Category = "AI Navigation", BlueprintReadWrite, meta=(AllowPrivateAccess))
	AActor* PatrolTarget;

	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	TArray<AActor*> PatrolTargets;

	UPROPERTY(EditAnywhere)
	double PatrolRadius = 200.f;

	FTimerHandle PatrolTimer;
	void PatrolTimerFinished();

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float WaitTimeMin = 5.f;

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float WaitTimeMax = 10.f;


};
