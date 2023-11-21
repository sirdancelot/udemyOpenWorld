// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Rashepur/DebugMacros.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AttributeComponent.h"
#include "HUD/HealthBarComponent.h"
#include "GameFramework/CharacterMovementComponent.h"



AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;
	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetGenerateOverlapEvents(true);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	// componente dos atributos
	CharAttributes = CreateDefaultSubobject<UAttributeComponent>(TEXT("CharAttributes"));

	HealthBarWidget = CreateDefaultSubobject<UHealthBarComponent>(TEXT("HealthBarDisplay"));
	HealthBarWidget->SetupAttachment(GetRootComponent());

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
}

void AEnemy::Die()
{
	SelectDeathMontage();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HealthBarWidget->SetVisibility(false);
	SetLifeSpan(10.0f);
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

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (CombatTarget)
	{
		double DistanceToTarget = (CombatTarget->GetActorLocation() - GetActorLocation()).Size();
		if (DistanceToTarget > CombatRadius)
		{
			CombatTarget = nullptr;
			if (HealthBarWidget)
				HealthBarWidget->SetVisibility(false);
		}
	}

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
    return DamageAmount;
}
