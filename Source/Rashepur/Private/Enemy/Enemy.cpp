// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AIController.h"
#include "Components/AttributeComponent.h"
#include "Rashepur/Weapons/Weapon.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "HUD/HealthBarComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Perception/PawnSensingComponent.h"

#include "Rashepur/DebugMacros.h"


AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	// Class default collision setup
	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetGenerateOverlapEvents(true);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	HealthBarWidget = CreateDefaultSubobject<UHealthBarComponent>(TEXT("HealthBarDisplay"));
	HealthBarWidget->SetupAttachment(GetRootComponent());

	PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensing"));
	PawnSensing->SightRadius = 1800;
	PawnSensing->SetPeripheralVisionAngle(45.f);
 
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	if (HealthBarWidget)
	{
		HealthBarWidget->SetHealthPercent(CharAttributes->GetHealthPercent());
		HealthBarWidget->SetVisibility(false);
	}
	GetCharacterMovement()->MaxWalkSpeed = 125.f;
	MoveTo(PatrolTarget);
	if (PawnSensing)
	{
		PawnSensing->OnSeePawn.AddDynamic(this, &AEnemy::PawnSeen);
	}
	
	EquipDefaultWeapon();
}

void AEnemy::EquipDefaultWeapon()
{
	UWorld* World = GetWorld();
	if (World && DefaultWeaponClass)
	{
		AWeapon* DefaultWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);
		DefaultWeapon->Equip(GetMesh(), FName("OneHandedSocket"), this, this);
		EquippedWeapon = DefaultWeapon;
		if (DefaultWeapon->GetWeaponType() == EWeaponType::EWT_OneHand)
			CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
		else if (DefaultWeapon->GetWeaponType() == EWeaponType::EWT_TwoHand)
			CharacterState = ECharacterState::ECS_EquippedTwoHandedWeapon;
	}
}

void AEnemy::MoveTo(AActor* Target, bool DrawDebugSpheresOnPath)
{
	EnemyController = Cast<AAIController>(GetController());
	if (EnemyController && Target) 
	{ 
		FAIMoveRequest MoveRequest;
		MoveRequest.SetGoalActor(Target);
		MoveRequest.SetAcceptanceRadius(35.f);
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

void AEnemy::Die()
{
	Super::Die();
	EnemyState = EEnemyState::EES_Dead;
	ClearAttackTimer();
	HideHealthBar();
}

void AEnemy::Attack()
{
	if (CanAttack())
	{
		Super::Attack();
		EnemyState = EEnemyState::EES_Engaged;
	}
}

bool AEnemy::CanAttack()
{
	bool bCanAttack =
		IsInsideAttackRadius() &&
		ActionState != EActionState::EAS_Occupied &&
		!IsDead();
	return bCanAttack;
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

bool AEnemy::InTargetRange(AActor *Target, double Radius)
{
	if (Target == nullptr) return false;
	double DistanceToTarget = (Target->GetActorLocation() - GetActorLocation()).Size();
	//DRAW_SPHERE_SingleFrame(GetActorLocation());
	//DRAW_SPHERE_SingleFrame(Target->GetActorLocation());
    return DistanceToTarget <= Radius;
}

void AEnemy::PawnSeen(APawn* SeenPawn)
{
	const bool bShouldChaseTarget =
		EnemyState != EEnemyState::EES_Dead &&
		EnemyState != EEnemyState::EES_Attacking &&
		ActionState != EActionState::EAS_Occupied &&
		SeenPawn->ActorHasTag(FName("Hero"));

	if (bShouldChaseTarget)
	{
		CombatTarget = SeenPawn;
		ClearPatrolTimer();
		ChaseTarget();
	}
}

void AEnemy::ClearPatrolTimer()
{
	GetWorldTimerManager().ClearTimer(PatrolTimer);
}

void AEnemy::PatrolTimerFinished()
{
	MoveTo(PatrolTarget);
}


void AEnemy::CheckCombatTarget()
{
	if (IsOutsideCombatRadius())
	{
		ClearAttackTimer();
		LoseInterest();
		if (!IsEngaged()) 
			StartPatrolling();
		UE_LOG(LogTemp, Warning, TEXT("Lost interest"));
	}
	else if (IsOutsideAttackRadius() && !IsChasing())
	{
		ClearAttackTimer();
		if (!IsEngaged())
			ChaseTarget();
		UE_LOG(LogTemp, Warning, TEXT("Is chasing"));
	}
	else if (CanAttack())
	{
		UE_LOG(LogTemp, Warning, TEXT("Attack!"));
		Attack();
		StartAttackTimer();
	}
}

bool AEnemy::IsAttacking()
{
	return EnemyState == EEnemyState::EES_Attacking;
}

bool AEnemy::IsChasing()
{
	return EnemyState == EEnemyState::EES_Chasing;
}

bool AEnemy::IsOutsideAttackRadius()
{
	return !InTargetRange(CombatTarget, AttackRadius);
}
bool AEnemy::IsInsideAttackRadius()
{
	return InTargetRange(CombatTarget, AttackRadius);
}

void AEnemy::ChaseTarget()
{
	EnemyState = EEnemyState::EES_Chasing;
	GetCharacterMovement()->MaxWalkSpeed = ChasingSpeed;
	MoveTo(CombatTarget);
}

bool AEnemy::IsOutsideCombatRadius()
{
	return !InTargetRange(CombatTarget, CombatRadius);
}

void AEnemy::StartPatrolling()
{
	EnemyState = EEnemyState::EES_Patrolling;
	GetCharacterMovement()->MaxWalkSpeed = PatrollingSpeed;
	MoveTo(PatrolTarget);
}

void AEnemy::LoseInterest()
{
	CombatTarget = nullptr;
	HideHealthBar();
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
	const float AttackTime = FMath::RandRange(MinWaitBeforeAttack, MaxWaitBeforeAttack);
	GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, 1.f);
}

void AEnemy::ClearAttackTimer()
{
	UE_LOG(LogTemp, Warning, TEXT("Clear attack timer!"));

	GetWorldTimerManager().ClearTimer(AttackTimer);
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

bool AEnemy::IsDead()
{
	return EnemyState == EEnemyState::EES_Dead;
}

bool AEnemy::IsEngaged()
{
	return EnemyState == EEnemyState::EES_Engaged;
}

void AEnemy::GetHit_Implementation(const FVector& ImpactPoint)
{
	ShowHealthBar();
	if (IsAlive())
		DirectionalHitReact(ImpactPoint);
	else 
		Die();

	PlayHitSound(ImpactPoint);
	SpawnHitParticles(ImpactPoint);
}


float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const &DamageEvent, AController *EventInstigator, AActor *DamageCauser)
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
