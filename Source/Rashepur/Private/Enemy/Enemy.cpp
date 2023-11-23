// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "HUD/HealthBarComponent.h"
#include "Components/AttributeComponent.h"
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

	// Class default components setup
	CharAttributes = CreateDefaultSubobject<UAttributeComponent>(TEXT("CharAttributes"));

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

}

void AEnemy::MoveTo(AActor* Target, bool DrawDebugSpheresOnPath)
{
	EnemyController = Cast<AAIController>(GetController());
	if (EnemyController && Target) 
	{ 
		FAIMoveRequest MoveRequest;
		MoveRequest.SetGoalActor(Target);
		MoveRequest.SetAcceptanceRadius(15.f);
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
	GetCharacterMovement()->StopActiveMovement();
	SelectDeathMontage();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HealthBarWidget->SetVisibility(false);
	SetLifeSpan(10.0f);
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
	if (EnemyState == EEnemyState::EES_Chasing) 
		return;
	if (SeenPawn->ActorHasTag(FName("Hero")))
	{
		GetWorldTimerManager().ClearTimer(PatrolTimer);
		GetCharacterMovement()->MaxWalkSpeed = 300.f;
		CombatTarget = SeenPawn;
		if (EnemyState != EEnemyState::EES_Attacking)
		{
			EnemyState = EEnemyState::EES_Chasing;
			MoveTo(CombatTarget);
			UE_LOG(LogTemp, Warning, TEXT("Pawn Seen, Chase Player"));
		}
	}
}

void AEnemy::PatrolTimerFinished()
{
	MoveTo(PatrolTarget);
}

void AEnemy::PlayHitReactMontage(const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage, 1.0f);
		AnimInstance->Montage_JumpToSection(SectionName, HitReactMontage);
	}
}

void AEnemy::SelectDeathMontage()
{
	if (DeathMontage)
	{
		int32 Selection = FMath::RandRange(0,3);
		FName SectionName = FName();
		switch (Selection)
		{
			case 0:
				DeathPose = EDeathPose::EDP_Death1;
				break;
			case 1:
				DeathPose = EDeathPose::EDP_Death2;
				break;
			case 2:
				DeathPose = EDeathPose::EDP_Death3;
				break;
			case 3:
				DeathPose = EDeathPose::EDP_Death4;
				break;
			default:
				break;
		}
	}
}

void AEnemy::CheckCombatTarget()
{
	if (!InTargetRange(CombatTarget, CombatRadius))
	{
		// outsite combat radius, lose interest
		CombatTarget = nullptr;
		if (HealthBarWidget)
			HealthBarWidget->SetVisibility(false);
		EnemyState = EEnemyState::EES_Patrolling;
		GetCharacterMovement()->MaxWalkSpeed = 125.f;
		MoveTo(PatrolTarget);
		UE_LOG(LogTemp, Warning, TEXT("Lose Interest"));
	}
	else if (!InTargetRange(CombatTarget, AttackRadius) && EnemyState != EEnemyState::EES_Chasing)
	{
		// outside attack range, chase character
		EnemyState = EEnemyState::EES_Chasing;
		GetCharacterMovement()->MaxWalkSpeed = 300.f;
		MoveTo(CombatTarget);
		UE_LOG(LogTemp, Warning, TEXT("Chase Player"));
	}
	else if (InTargetRange(CombatTarget, AttackRadius) && EnemyState != EEnemyState::EES_Attacking)
	{
		// inside attack range, attack character
		EnemyState = EEnemyState::EES_Attacking;
		UE_LOG(LogTemp, Warning, TEXT("Attack"));
		// todo attack montage
	}
}

void AEnemy::CheckPatrolTarget()
{
	// isso aqui vai ser executado a cada frame, ou seja, sempre que chegar no alvo, vai resetar o array de alvos validos
	if (InTargetRange(PatrolTarget, PatrolRadius)) // chegou no proximo alvo
	{
		PatrolTarget = ChoosePatrolTarget();
		// vai executar a funcao depois de 5 segundos
		int32 WaitTime = FMath::RandRange(WaitTimeMin, WaitTimeMax);
		GetWorldTimerManager().SetTimer(PatrolTimer, this, &AEnemy::PatrolTimerFinished, WaitTime);
	}
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (EnemyState > EEnemyState::EES_Patrolling)
	{
		CheckCombatTarget();
	}

	CheckPatrolTarget();
}

void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemy::GetHit_Implementation(const FVector& ImpactPoint)
{
	//DRAW_SPHERE_COLOR(ImpactPoint, FColor::Blue);
	if (HealthBarWidget)
		HealthBarWidget->SetVisibility(true);
	if (CharAttributes && CharAttributes->isAlive())
		DirectionalHitReact(ImpactPoint);
	else 
		Die();

	if (HitSound)
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, ImpactPoint);
	if (HitParticles)
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticles, ImpactPoint);
}

void AEnemy::DirectionalHitReact(const FVector& ImpactPoint)
{
	// calcula onde foi o ponto de impacto no inimigo pra saber qual montagem dar play
	const FVector Forward = GetActorForwardVector();
	// abaixa o ponto de impacto para a localizacao z do inimigo 
	const FVector ImpactLowered(ImpactPoint.X, ImpactPoint.Y, GetActorLocation().Z);

	const FVector ToHit = (ImpactLowered - GetActorLocation()).GetSafeNormal();

	// Forward * ToHit = |Forward||ToHit| * cos(theta)
	// Forward e ToHit foram normalizados (viraram 1)
	// logo Forward * ToHit = cos(theta) (theta o angulo entre dois vetores, o vetor da frente do inimigo e do ponto de impacto
	const double CosTheta = FVector::DotProduct(Forward, ToHit);
	// acos cossenso ao reverso. (arc cosine) . voce passa o cosseno e recebe o angulo em radians
	double Theta = FMath::Acos(CosTheta);
	// converte de radians para graus
	Theta = FMath::RadiansToDegrees(Theta);

	// se crossproduct apontar para baixo, theta deveria ser negativo
	const FVector CrossProduct = FVector::CrossProduct(Forward, ToHit);
	if (CrossProduct.Z < 0)
	{
		Theta *= -1.f;
	}
	FName Section("FromBack");
	if (Theta > -45.f && Theta < 45.f)
		Section = FName("FromFront");
	else if (Theta >= -135.f && Theta < -45.f)
		Section = FName("FromLeft");
	else if (Theta >= 45.f && Theta < 135.f)
		Section = FName("FromRight");
	PlayHitReactMontage(Section);

}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const &DamageEvent, AController *EventInstigator, AActor *DamageCauser)
{
	if (CharAttributes && HealthBarWidget)
	{
		CharAttributes->ReceiveDamage(DamageAmount);
		HealthBarWidget->SetHealthPercent(CharAttributes->GetHealthPercent());
	}
	CombatTarget = EventInstigator->GetPawn();
	EnemyState = EEnemyState::EES_Chasing;
	GetCharacterMovement()->MaxWalkSpeed = 300.f;
	MoveTo(CombatTarget);
    return DamageAmount;
}
