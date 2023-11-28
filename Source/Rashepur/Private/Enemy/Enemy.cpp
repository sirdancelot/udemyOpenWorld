// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"
#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "Components/AttributeComponent.h"
#include "Rashepur/Weapons/Weapon.h"
#include "HUD/HealthBarComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/UnrealMathUtility.h"

AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	// Class default collision setup
	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetGenerateOverlapEvents(true);

	HealthBarWidget = CreateDefaultSubobject<UHealthBarComponent>(TEXT("HealthBarDisplay"));
	HealthBarWidget->SetupAttachment(GetRootComponent());
 
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	EndMontageDelegate.BindUObject(this, &AEnemy::OnActionEnded);
	HitReactEndedDelegate.BindUObject(this, &AEnemy::TurnToPlayer);
}

void AEnemy::Tick(float DeltaTime)
{
	if (IsDead() || IsStaggered()) return;

	Super::Tick(DeltaTime);
	if (EnemyState > EEnemyState::EES_Patrolling)
	{
		CheckCombatTarget();
		if (IsSearching() && !IsOutsideCombatRadius())
			ExpandSight(DeltaTime);
	} 
	else
	{
		CheckPatrolTarget();
	}
}
void AEnemy::TurnToPlayer(UAnimMontage* Montage, bool bInterrupted)
{
	if (CombatTarget && !CanSeeTarget(CombatTarget))
	{
		FRotator LookAround = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), CombatTarget->GetActorLocation());
		SetActorRotation(LookAround);
	}
}

void AEnemy::ExpandSight(float DeltaTime)
{
	if (PawnSensing && PawnSensing->GetPeripheralVisionAngle() <= (DefaultPeripheralVision*2))
	{
		float SightExpansionSpeed = 7.f;
		PawnSensing->SetPeripheralVisionAngle(PawnSensing->GetPeripheralVisionAngle() + DeltaTime * SightExpansionSpeed);
	}
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	HandleDamage(DamageAmount);
	CombatTarget = EventInstigator->GetPawn();
	ChaseTarget();
	return DamageAmount;
}

void AEnemy::Destroyed()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Destroy();
	}
}

void AEnemy::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	UAnimInstance* HitAnimInstance;
	if (IsAlive() && Hitter)
	{
		ShowHealthBar();
		if (!IsStaggered())
			Stagger();
		HitAnimInstance = DirectionalHitReact(Hitter->GetActorLocation());
		HitAnimInstance->Montage_SetEndDelegate(HitReactEndedDelegate);
	}
	else
		Die();

	PlayHitSound(ImpactPoint);
	SpawnHitParticles(ImpactPoint);

	ClearPatrolTimer();
	ClearAttackTimer();
	ClearSearchTimer();
}

void AEnemy::Stagger()
{
	EnemyState = EEnemyState::EES_Staggered;
	StopAllActions();

	if (bDebugStates)
		UE_LOG(LogTemp, Warning, TEXT("EnemyState set to EES_Stagger Enemy (Stagger)"));
	StartStaggerRecoverTimer();
}

void AEnemy::OnActionEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!IsStaggered() && !IsSearching() && !IsDead())
	{
		ClearStates();
	}
}

void AEnemy::ClearStates()
{
	EnemyState = EEnemyState::EES_NoState;
	ActionState = EActionState::EAS_Unoccupied;
	if (bDebugStates)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnemyState set to EES_NoState Enemy (OnActionEnded)"));
		UE_LOG(LogTemp, Warning, TEXT("ActionState set to EAS_Unoccupied Enemy (OnActionEnded)"));
	}
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	if (HealthBarWidget)	
		HealthBarWidget->SetHealthPercent(CharAttributes->GetHealthPercent());
	if (PawnSensing)
		PawnSensing->OnSeePawn.AddDynamic(this, &AEnemy::PawnSeen);
	InitializeEnemy();
	MoveTo(PatrolTarget);
}

void AEnemy::InitializeEnemy()
{
	EnemyController = Cast<AAIController>(GetController());
	HideHealthBar();
	EquipDefaultWeapon();
	Tags.Add("Enemy");
}

void AEnemy::Die()
{
	StopAllActions();
	Super::Die();
	HideHealthBar();
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
	EnemyState = EEnemyState::EES_Dead;

	if (bDebugStates)
		UE_LOG(LogTemp, Warning, TEXT("EnemyState set to EES_Dead Enemy (die)"));
}

void AEnemy::Attack()
{
	if (!IsCombatTargetDead())
	{
		Super::Attack();
	}
}

bool AEnemy::CanAttack()
{
	bool bCanAttack =
		ActionState == EActionState::EAS_Unoccupied &&
		!IsAttacking() &&
		!IsStaggered() &&
		IsAlive();
	return bCanAttack;
}

bool AEnemy::IsStaggered() const
{
	return EnemyState == EEnemyState::EES_Staggered;
}

void AEnemy::PlayAttackMontage()
{
	Super::PlayAttackMontage();

}

void AEnemy::HandleDamage(float DamageAmount)
{
	Super::HandleDamage(DamageAmount);
	if (CharAttributes && HealthBarWidget)
	{
		HealthBarWidget->SetHealthPercent(CharAttributes->GetHealthPercent());
	}
}

void AEnemy::MoveTo(AActor* Target, bool DrawDebugSpheresOnPath)
{
	if (EnemyController && Target)
	{
		FAIMoveRequest MoveRequest;
		MoveRequest.SetGoalActor(Target);
		MoveRequest.SetAcceptanceRadius(MoveAcceptanceRadius);
		FNavPathSharedPtr NavPath;
		EnemyController->MoveTo(MoveRequest, &NavPath); // navpath outparameter, a gente passa o parametro e a funcao muda o valor dele
		if (DrawDebugSpheresOnPath)
		{
			TArray<FNavPathPoint> PathPoints = NavPath->GetPathPoints();
			for (auto& Point : PathPoints)
			{
				const FVector& Location = Point.Location;
				DrawDebugSphere(GetWorld(), Location, 12.f, 12, FColor::Green, false, 10.f);
			}
		}
	}
}

void AEnemy::EquipDefaultWeapon()
{
	UWorld* World = GetWorld();
	if (World && DefaultWeaponClass)
	{
		AWeapon* DefaultWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);
		DefaultWeapon->Equip(GetMesh(), GetWeaponSocket(DefaultWeapon), this, this);
		if (DefaultWeapon->GetWeaponType() == EWeaponType::EWT_BothHands) 
		{
			AWeapon* SecondWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);
			SecondWeapon->Equip(GetMesh(), FName("OneHandedSocket"), this, this);
		}

		EquippedWeapon = DefaultWeapon;
		SetCharacterStateByWeaponType();
		
	}
}

void AEnemy::PawnSeen(APawn* SeenPawn)
{
	const bool bShouldChaseTarget =
		!IsStaggered() &&
		IsAlive() &&
		!IsEngaged() &&		
		!IsAttacking() &&
		SeenPawn->ActorHasTag(FName("Hero"));

	if (IsSearching())
	{
		StopSearchingForTarget();
	}

	if (bShouldChaseTarget)
	{
		CombatTarget = SeenPawn;
		ClearPatrolTimer();
		if (!IsCombatTargetDead()) {
			ChaseTarget();
		}
	}
}

void AEnemy::StaggerRecover()
{
	ActionState = EActionState::EAS_Unoccupied;
	EnemyState = EEnemyState::EES_NoState;
	if (bDebugStates)
		UE_LOG(LogTemp, Warning, TEXT("EnemyState set to EES_NoState Enemy (StaggerRecover)"));
	if (bDebugStates)
		UE_LOG(LogTemp, Warning, TEXT("ActionState set to EAS_Unoccupied Enemy (StaggerRecover)"));
}

void AEnemy::CheckCombatTarget()
{
	if (IsOutsideCombatRadius())
	{
		ClearAttackTimer();
		LoseInterest();
		ResetPeripheralVision();
		if (!IsEngaged()) 
			StartPatrolling();
	}
	else if (CanChase())
	{
		ClearAttackTimer();
		if (!IsEngaged())
			ChaseTarget(); // seta pra chasing
	}
	else if (CanSearch())
	{
		SearchForTarget();
	}
	else if (CanEngage())
	{
		EngageTarget();
	} 
	else if (IsCombatTargetDead() && !IsPatrolling())
	{
		ClearAttackTimer();
		LoseInterest();
		ResetPeripheralVision();
		StartPatrolling();
	}
}

bool AEnemy::CanEngage()
{
	return IsInsideAttackRadius() && 
		!IsEngaged() && 
		!IsAttacking() && 
		CanSeeTarget(CombatTarget) && 
		!IsSearching();
}

bool AEnemy::CanSearch()
{
	return !IsOutsideCombatRadius() && 
		!IsAttacking() && 
		!IsSearching() && !
		CanSeeTarget(CombatTarget);
}

bool AEnemy::CanChase()
{
	return !IsOutsideCombatRadius() && 
		IsOutsideAttackRadius() && 
		!IsChasing() && 
		!IsSearching() && 
		CanSeeTarget(CombatTarget);
}


bool AEnemy::IsOutsideCombatRadius() const
{
	return !InTargetRange(CombatTarget, CombatRadius);
}

bool AEnemy::IsOutsideAttackRadius() const
{
	return !InTargetRange(CombatTarget, AttackRadius);
}

bool AEnemy::IsInsideAttackRadius() const
{
	return InTargetRange(CombatTarget, AttackRadius);
}

bool AEnemy::IsAttacking() const
{
	return ActionState == EActionState::EAS_Attacking;
}

bool AEnemy::IsPatrolling() const
{
	return EnemyState == EEnemyState::EES_Patrolling;
}

bool AEnemy::IsChasing() const
{
	return EnemyState == EEnemyState::EES_Chasing;
}

void AEnemy::ChaseTarget()
{
	if (IsAlive() && !IsEngaged() && !IsAttacking())
	{
		EnemyState = EEnemyState::EES_Chasing;
		if (bDebugStates)
			UE_LOG(LogTemp, Warning, TEXT("EnemyState set to EES_Chasing Enemy (ChaseTarget)"));
		GetCharacterMovement()->MaxWalkSpeed = ChasingSpeed;
		MoveTo(CombatTarget);
	}
}

void AEnemy::LoseInterest()
{
	CombatTarget = nullptr;
	EnemyState = EEnemyState::EES_NoState;
	ActionState = EActionState::EAS_Unoccupied;
	if (bDebugStates)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnemyState set to EES_NoState Enemy (Start Patrolling)"));
		UE_LOG(LogTemp, Warning, TEXT("ActionState set to EAS_Unoccupied Enemy (Start Patrolling)"));
	}

	HideHealthBar();
}

void AEnemy::StartPatrolling()
{
	EnemyState = EEnemyState::EES_Patrolling;
	if (bDebugStates)
		UE_LOG(LogTemp, Warning, TEXT("EnemyState set to EES_Patrolling Enemy (Start Patrolling)"));
	GetCharacterMovement()->MaxWalkSpeed = PatrollingSpeed;
	MoveTo(PatrolTarget);
}

void AEnemy::ClearPatrolTimer() 
{
	GetWorldTimerManager().ClearTimer(PatrolTimer);
}

void AEnemy::HideHealthBar()
{
	if (HealthBarWidget)
		HealthBarWidget->SetVisibility(false);
}

void AEnemy::ShowHealthBar()
{
	if (HealthBarWidget)
		HealthBarWidget->SetVisibility(true);
}

void AEnemy::StartAttackTimer()
{
	const float AttackTime = FMath::RandRange(MinWaitBeforeAttack, MaxWaitBeforeAttack);
	GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime);
}

void AEnemy::ClearAttackTimer()
{
	GetWorldTimerManager().ClearTimer(AttackTimer);
}

void AEnemy::PatrolTimerFinished()
{
	MoveTo(PatrolTarget);
}

bool AEnemy::IsDead() const
{
	return EnemyState == EEnemyState::EES_Dead;
}

bool AEnemy::IsEngaged() const
{
	return EnemyState == EEnemyState::EES_Engaged;
}

bool AEnemy::IsSearching() const
{
	return EnemyState == EEnemyState::EES_Searching;
}

bool AEnemy::CanSeeTarget(APawn* Target) const
{
	if (Target == nullptr) return false;
	if (PawnSensing)
		return PawnSensing->CouldSeePawn(Target);
	else
		return false;
}

void AEnemy::SearchForTarget()
{
	ActionState = EActionState::EAS_Occupied;
	EnemyState = EEnemyState::EES_Searching;

	ClearAttackTimer();
	if (bDebugStates)
		UE_LOG(LogTemp, Display, TEXT("EnemyState set to EES_Searching Enemy (CheckCombatTarget)"));
	GetCharacterMovement()->StopMovementImmediately();
	PlaySearchMontage();
	StartSearchTimer(GetSearchMontageLength()* SearchAnimationLoopNTimes);
}

void AEnemy::EngageTarget()
{
	EnemyState = EEnemyState::EES_Engaged;
	if (bDebugStates)
		UE_LOG(LogTemp, Warning, TEXT("EnemyState set to EES_Engaged Enemy (CheckCombatTarget)"));
	if (CanAttack())
	{
		StartAttackTimer();
	}
}

void AEnemy::StartSearchTimer(float Duration)
{
	GetWorldTimerManager().SetTimer(SearchTimer, this, &AEnemy::SearchTimerFinished, Duration);
}

void AEnemy::ClearSearchTimer()
{
	GetWorldTimerManager().ClearTimer(SearchTimer);
}

void AEnemy::SearchTimerFinished()
{
	StopSearchingForTarget();
	StartPatrolling();
}

void AEnemy::StopSearchingForTarget()
{
	Super::StopSearchingForTarget();
	ClearSearchTimer();
}

void AEnemy::StopAllActions()
{
	if (IsAttacking())
		StopAnimMontage(GetAttackMontageByWeaponType());
	if (IsSearching())
		StopSearchingForTarget();
}

void AEnemy::StartStaggerRecoverTimer()
{
	const float StaggerTime = FMath::RandRange(MinWaitBeforeStaggerRecover, MaxWaitBeforeStaggerRecover);
	GetWorldTimerManager().SetTimer(StaggerTimer, this, &AEnemy::StaggerRecover, StaggerTime);
}

void AEnemy::ClearStaggerRecoverTimer()
{
	GetWorldTimerManager().ClearTimer(StaggerTimer);
}

AActor* AEnemy::ChoosePatrolTarget()
{
	// remove do array de patroltargets o atual pra nao repetir
	TArray<AActor*> ValidTargets;
	for (AActor* Target : PatrolTargets)
		if (Target != PatrolTarget)
			ValidTargets.AddUnique(Target);

	if (ValidTargets.Num() > 0)
	{
		const int32 NextPatrolTarget = FMath::RandRange(0,ValidTargets.Num()-1);
		return ValidTargets[NextPatrolTarget];
	}
    return nullptr;
}

bool AEnemy::InTargetRange(AActor *Target, double Radius) const
{
	if (Target == nullptr) return false;
	double DistanceToTarget = (Target->GetActorLocation() - GetActorLocation()).Size();
	//DRAW_SPHERE_SingleFrame(GetActorLocation());
	//DRAW_SPHERE_SingleFrame(Target->GetActorLocation());
	if (DistanceToTarget < 0)
		DistanceToTarget *= -1;
    return DistanceToTarget <= Radius;
}

void AEnemy::CheckPatrolTarget()
{
	// isso aqui vai ser executado a cada frame, ou seja, sempre que chegar no alvo, vai resetar o array de alvos validos
	if (InTargetRange(PatrolTarget, PatrolRadius)) // chegou no proximo alvo
	{
		PatrolTarget = ChoosePatrolTarget();
		// vai executar a funcao depois de 5 segundos
		int32 WaitTime = FMath::RandRange(MinWaitBeforePatrol, MaxWaitBeforePatrol);
		GetWorldTimerManager().SetTimer(PatrolTimer, this, &AEnemy::PatrolTimerFinished, WaitTime);
	}
}