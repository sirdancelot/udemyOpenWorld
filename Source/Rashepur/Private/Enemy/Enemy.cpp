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
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (IsDead()) return;
	if (EnemyState > EEnemyState::EES_Patrolling)
	{
		CheckCombatTarget();
	}

	CheckPatrolTarget();
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
	Super::GetHit_Implementation(ImpactPoint, Hitter);
	if (IsAlive())
	{
		ShowHealthBar();
		if (!IsStaggered())
			Stagger();
	}
	ClearPatrolTimer();
}

void AEnemy::Stagger()
{
	if (IsAttacking())
	{
		StopAnimMontage(GetAttackMontageByWeaponType());
	}
	EnemyState = EEnemyState::EES_Staggered;
	
	if (bDebugStates)
		UE_LOG(LogTemp, Warning, TEXT("EnemyState set to EES_Stagger Enemy (Stagger)"));
	StartStaggerRecoverTimer();
}

void AEnemy::OnActionEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (IsAttacking()) // se foi chamado durante um ataque
	{
		EnemyState = EEnemyState::EES_Chasing;
		if (bDebugStates)
			UE_LOG(LogTemp, Warning, TEXT("EnemyState set to EES_Chasing Enemy (OnActionEnded)"));
	}
	else if (!IsStaggered()) // a hit montage nao pode recuperar do stagger, tem que ser o clearstagger
	{
		EnemyState = EEnemyState::EES_NoState;
		if (bDebugStates)
			UE_LOG(LogTemp, Warning, TEXT("EnemyState set to EES_NoState Enemy (OnActionEnded)"));
	}
	ActionState = EActionState::EAS_Unoccupied;
	if (bDebugStates)
		UE_LOG(LogTemp, Warning, TEXT("ActionState set to EAS_Unoccupied Enemy (OnActionEnded)"));

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
	Super::Die();
	HideHealthBar();
	if (ActionState == EActionState::EAS_Attacking)
	{
		StopAnimMontage(GetAttackMontageByWeaponType());
	}
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
	EnemyState = EEnemyState::EES_Dead;

	if (bDebugStates)
		UE_LOG(LogTemp, Warning, TEXT("EnemyState set to EES_Dead Enemy (die)"));
}

void AEnemy::Attack()
{
	if (CanAttack())
	{
		Super::Attack();
		EnemyState = EEnemyState::EES_Engaged;
		if (bDebugStates)
			UE_LOG(LogTemp, Warning, TEXT("Set EnemyState para EES_Engaged (attack)"));
	}		
}

bool AEnemy::CanAttack()
{
	bool bCanAttack =
		ActionState == EActionState::EAS_Unoccupied &&
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
		EnemyState != EEnemyState::EES_Staggered &&
		EnemyState != EEnemyState::EES_Dead &&
		EnemyState != EEnemyState::EES_Engaged &&
		ActionState != EActionState::EAS_Occupied &&
		SeenPawn->ActorHasTag(FName("Hero"));

	if (bShouldChaseTarget)
	{
		CombatTarget = SeenPawn;
		ClearPatrolTimer();
		ChaseTarget();
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
	CheckCombatTarget();
}

void AEnemy::CheckCombatTarget()
{
	if (IsOutsideCombatRadius() && !IsStaggered())
	{
		ClearAttackTimer();
		LoseInterest();
		if (!IsEngaged()) 
			StartPatrolling();
	}
	else if (!IsOutsideCombatRadius() && IsOutsideAttackRadius() && !IsStaggered() && !IsChasing())
	{
		ClearAttackTimer();
		if (!IsEngaged())
			ChaseTarget(); // seta pra chasing
	}
	else if (IsInsideAttackRadius() && !IsEngaged() && !IsStaggered() && !IsAttacking())
	{
		EnemyState = EEnemyState::EES_Engaged;
		StartAttackTimer();
		//Attack();
	}
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
	return EnemyState == EEnemyState::EES_Attacking;
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
	EnemyState = EEnemyState::EES_Attacking;
	if (bDebugStates)
		UE_LOG(LogTemp, Warning, TEXT("EnemyState set to EES_Attacking Enemy (Start Attack Timer"));
	
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