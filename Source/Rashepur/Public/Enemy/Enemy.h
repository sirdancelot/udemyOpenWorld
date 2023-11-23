// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/HitInterface.h"
#include "GameFramework/Character.h"
#include "CharacterStates.h"
#include "Enemy.generated.h"

class UAnimMontage;
class USoundBase;
class UParticleSystem;
class UAttributeComponent;
class UHealthBarComponent;
class UPawnSensingComponent;

UCLASS()
class RASHEPUR_API AEnemy : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:
	AEnemy();
	void CheckCombatTarget();
	void CheckPatrolTarget();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetHit_Implementation(const FVector& ImpactPoint) override;
	void DirectionalHitReact(const FVector& ImpactPoint);
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	UPROPERTY(BlueprintReadOnly)
	EDeathPose DeathPose = EDeathPose::EDP_Alive;
	
protected:
	virtual void BeginPlay() override;
    void MoveTo(AActor *Target, bool DrawDebugSpheresOnPath = false);
	AActor* ChoosePatrolTarget();
    void Die();
    bool InTargetRange(AActor* Target, double Radius);
	
	UFUNCTION() // se for ser usado como delegate precisa de ufunction
	void PawnSeen(APawn* SeenPawn);
private:

	/* 
	* Components
	*/
	UPROPERTY(EditDefaultsOnly)
	UAttributeComponent* CharAttributes;

	UPROPERTY(VisibleAnywhere)
	UHealthBarComponent* HealthBarWidget;

	UPROPERTY(VisibleAnywhere)
	UPawnSensingComponent* PawnSensing;

	/* 
	* VFX
	*/

	UPROPERTY(EditAnywhere, Category = "Sounds")
	USoundBase* HitSound;

	UPROPERTY(EditAnywhere, Category = "VFX")
	UParticleSystem* HitParticles;

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

	/**
	 * Animation Montages
	 */
	void PlayHitReactMontage(const FName& SectionName);
	void SelectDeathMontage();

	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	UAnimMontage* DeathMontage;

	EEnemyState EnemyState = EEnemyState::EES_Patrolling;

};
