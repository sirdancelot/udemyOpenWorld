// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "Enemy.generated.h"


class UHealthBarComponent;
class UPawnSensingComponent;

UCLASS()
class RASHEPUR_API AEnemy : public ABaseCharacter
{
	GENERATED_BODY()

public:
	AEnemy();
	virtual void Tick(float DeltaTime) override;
	/**
	*	Taking Damage
	*/
	virtual void GetHit_Implementation(const FVector& ImpactPoint) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Destroyed() override;
	
protected:
	virtual void BeginPlay() override;
	void EquipDefaultWeapon();
	virtual void Die() override;

	virtual void Attack() override;
	virtual bool CanAttack() override;
	virtual void PlayAttackMontage() override;
	virtual void HandleDamage(float DamageAmount) override;

	/**
	 *  AI Navigation and control
	 */
	void CheckCombatTarget();

    void MoveTo(AActor *Target, bool DrawDebugSpheresOnPath = false);
	AActor* ChoosePatrolTarget();
    bool InTargetRange(AActor* Target, double Radius);
	
	UFUNCTION() // se for ser usado como delegate precisa de ufunction
	void PawnSeen(APawn* SeenPawn);
	void ClearPatrolTimer();

	UPROPERTY(BlueprintReadOnly)
	EEnemyState EnemyState = EEnemyState::EES_Patrolling;


private:

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

	UPROPERTY(EditAnywhere, Category = "Combat")
	double CombatRadius = 900.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	double AttackRadius = 160.f;

	UPROPERTY(EditInstanceOnly, Category = "AI Navigation", BlueprintReadWrite, meta=(AllowPrivateAccess))
	AActor* PatrolTarget;

	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	TArray<AActor*> PatrolTargets;

	UPROPERTY(EditAnywhere, Category = "Combat")
	double PatrolRadius = 200.f;

	FTimerHandle PatrolTimer;
	void PatrolTimerFinished();

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float MinWaitBeforePatrol = 5.f;

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float MaxWaitBeforePatrol = 10.f;

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float PatrollingSpeed = 125.f;
	
	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float ChasingSpeed = 300.f;

	/** 
	 *  AI CONTROL 
	 */
	bool IsOutsideCombatRadius();
	bool IsOutsideAttackRadius();

	bool IsInsideAttackRadius();

	bool IsAttacking();
	bool IsChasing();
	void ChaseTarget();

	void LoseInterest();
	void CheckPatrolTarget();
	void StartPatrolling();

	void HideHealthBar();
	void ShowHealthBar();

	void StartAttackTimer();
	void ClearAttackTimer();
	bool IsDead();
	bool IsEngaged();


	FTimerHandle AttackTimer;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float MinWaitBeforeAttack = 0.5f;
	
	UPROPERTY(EditAnywhere, Category = "Combat")
	float MaxWaitBeforeAttack = 1.f;
};
