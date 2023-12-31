// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterStates.h"
#include "Characters/BaseCharacter.h"
#include "Enemy.generated.h"


class UHealthBarComponent;


UCLASS()
class RASHEPUR_API AEnemy : public ABaseCharacter
{
	GENERATED_BODY()

public:
	AEnemy();

	/** <AActor> */
	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Destroyed() override;
	/** </AActor> */

	/** <IHitInterface> */
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;

	/** </IHitInterface> */


protected:
	/** <ABaseCharacter> */
	virtual void OnActionEnded(UAnimMontage* Montage, bool bInterrupted) override;
	void ClearStates();
	virtual void BeginPlay() override;
	virtual void Die() override;
	virtual void Attack() override;
	virtual bool CanAttack() override;

	virtual void PlayAttackMontage() override;
	virtual void HandleDamage(float DamageAmount) override;
	virtual void MoveTo(AActor* Target, bool DrawDebugSpheresOnPath = false) override;

	/** </ABaseCharacter> */

	void EquipDefaultWeapon();
	
	UFUNCTION() // se for ser usado como delegate precisa de ufunction
	void PawnSeen(APawn* SeenPawn);

	

	UPROPERTY(BlueprintReadOnly)
	EEnemyState EnemyState = EEnemyState::EES_Patrolling;



	UPROPERTY(EditAnywhere, Category = "Combat")
	float MoveAcceptanceRadius = 20.f;

private:
	void InitializeEnemy();

	/** AI Navigation and control */
	void Stagger();
	void StaggerRecover();
	bool IsStaggered() const;
	void StartStaggerRecoverTimer();
	void ClearStaggerRecoverTimer();

	void TurnToPlayer(UAnimMontage* Montage, bool bInterrupted);

	void CheckCombatTarget();
	bool CanEngage();
	bool CanSearch();
	bool CanChase();
	void CheckPatrolTarget();
	
	bool IsOutsideCombatRadius() const;
	bool IsOutsideAttackRadius() const;
		
	bool IsInsideAttackRadius() const;

	bool IsAttacking() const;
	bool IsPatrolling() const;
	bool IsChasing() const;
	void ChaseTarget();

	void LoseInterest();
	void StartPatrolling();
	void ClearPatrolTimer();

	void HideHealthBar();
	void ShowHealthBar();

	void StartAttackTimer();
	void ClearAttackTimer();
	void PatrolTimerFinished();

	bool IsDead() const;
	bool IsEngaged() const;
	bool IsSearching() const;
	bool CanSeeTarget(APawn* Target) const;
	void SearchForTarget();
	void EngageTarget();

	void StartSearchTimer(float Duration);
	void ClearSearchTimer();
	void SearchTimerFinished();
	void ExpandSight(float DeltaTime);

	virtual void StopSearchingForTarget() override;
	void StopAllActions();

	AActor* ChoosePatrolTarget();

	bool InTargetRange(AActor* Target, double Radius) const;

	FOnMontageEnded HitReactEndedDelegate;

	/*
	* Components
	*/

	UPROPERTY(VisibleAnywhere)
	UHealthBarComponent* HealthBarWidget;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AWeapon> DefaultWeaponClass;

	/**
	 * Combat
	 */

	FTimerHandle SearchTimer;
	FTimerHandle AttackTimer;


	UPROPERTY()
	class AAIController* EnemyController;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float MinWaitBeforeStaggerRecover = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float MaxWaitBeforeStaggerRecover = 1.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float MinWaitBeforeAttack = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float MaxWaitBeforeAttack = 1.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	double CombatRadius = 1000.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	double AttackRadius = 190.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	double PatrolRadius = 200.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float SearchAnimationLoopNTimes = 5.f;



	/**
	 * AI Behaviour
	 */

	
	FTimerHandle StaggerTimer;
	FTimerHandle PatrolTimer;

	UPROPERTY(EditInstanceOnly, Category = "AI Navigation", BlueprintReadWrite, meta = (AllowPrivateAccess))
	AActor* PatrolTarget;

	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	TArray<AActor*> PatrolTargets;

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float MinWaitBeforePatrol = 5.f;

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float MaxWaitBeforePatrol = 10.f;

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float PatrollingSpeed = 125.f;

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float ChasingSpeed = 300.f;
public:
	FORCEINLINE EEnemyState GetEnemyState() const { return EnemyState; }

};
