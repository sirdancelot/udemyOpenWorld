#pragma once


#include "Characters/BaseCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/AttributeComponent.h"
#include "Components/CapsuleComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Rashepur/Weapons/Weapon.h"
#include "Navigation/PathFollowingComponent.h"


ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	// Class default components setup
	CharAttributes = CreateDefaultSubobject<UAttributeComponent>(TEXT("CharAttributes"));
	
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensing"));
	PawnSensing->SightRadius = 2600;
	PawnSensing->SetPeripheralVisionAngle(DefaultPeripheralVision);

	//torna o status do personagem para desocupado ao final das montagens
	EndMontageDelegate.BindUObject(this, &ABaseCharacter::OnActionEnded);
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABaseCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	if (IsAlive() && Hitter)
		DirectionalHitReact(Hitter->GetActorLocation());
	else
		Die();

	PlayHitSound(ImpactPoint);
	SpawnHitParticles(ImpactPoint);
}

void ABaseCharacter::Attack()
{
	ActionState = EActionState::EAS_Attacking;
	if (bDebugStates)
		UE_LOG(LogTemp, Warning, TEXT("ActionState set to EAS_Attacking BaseCharacter (Attack)"));
	PlayAttackMontage();
}

bool ABaseCharacter::IsCombatTargetDead()
{
	if (CombatTarget)
		return CombatTarget->ActorHasTag(FName("Dead"));
	else
		return false;
}

void ABaseCharacter::Die()
{
	DisableCapsule();
	SelectDeathMontage();
	Tags.Add(FName("Dead"));
	SetLifeSpan(DeathLifeSpan);
}

void ABaseCharacter::MoveTo(AActor* Target, bool DrawDebugSpheresOnPath)
{
	//implement into child
}

FName ABaseCharacter::GetWeaponSocket(AWeapon* Weapon)
{
	FName WeaponSocket = FName("OneHandedSocket");

	EWeaponType WeaponType = Weapon->GetWeaponType();
	switch (WeaponType)
	{
	case EWeaponType::EWT_TwoHand:
		WeaponSocket = FName("TwoHandedSocket");
		break;
	case EWeaponType::EWT_Throw:
		WeaponSocket = FName("TwoHandedSocket");
		break;
	case EWeaponType::EWT_BothHands:
		WeaponSocket = FName("DualHandSocket");
		break;
	}

	return WeaponSocket;
}

FName ABaseCharacter::GetWeaponSpineSocket(AWeapon* OverlappingWeapon)
{
	return FName("SpineSocket");
}

void ABaseCharacter::SetCharacterStateByWeaponType()
{
	if (EquippedWeapon)
	{
		switch (EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_OneHand:
			CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
			break;
		case EWeaponType::EWT_TwoHand:
			CharacterState = ECharacterState::ECS_EquippedTwoHandedWeapon;
			break;
		case EWeaponType::EWT_Throw:
			CharacterState = ECharacterState::ECS_EquippedThrowingWeapon;
			break;
		case EWeaponType::EWT_BothHands:
			CharacterState = ECharacterState::ECS_EquippedDualHands;
			break;
		}
	}
	else
	{
		CharacterState = ECharacterState::ECS_Unequipped;
	}
}

void ABaseCharacter::StopSearchingForTarget()
{
	if (SearchMontage)
		StopAnimMontage(SearchMontage);
}

// Give a location near the combat target (distanciated by WarpTargetOffset from the combat target, towards the actor)
FVector ABaseCharacter::GetTranslationWarpTarget() 
{
	if (!CombatTarget) return FVector();

	const FVector CombatTargetLocation = CombatTarget->GetActorLocation();
	const FVector Location = GetActorLocation();
	
	FVector TargetToMe = (Location - CombatTargetLocation).GetSafeNormal();
	TargetToMe *= WarpTargetOffset;

	return CombatTargetLocation + TargetToMe;

}

FVector ABaseCharacter::GetRotationWarpTarget()
{
	if (!CombatTarget) return FVector();
	return CombatTarget->GetActorLocation();
}



void ABaseCharacter::PlaySearchMontage()
{
	PlayMontageSection(SearchMontage, FName("LookAround"), 1.f);
}

float ABaseCharacter::GetSearchMontageLength()
{
	if (SearchMontage)
		return SearchMontage->GetSectionLength(0);
	return 0.0f;
}

UAnimInstance* ABaseCharacter::PlayHitReactMontage(const FName& SectionName)
{
	return PlayMontageSection(HitReactMontage, SectionName, 1.f, false);
}

UAnimInstance* ABaseCharacter::DirectionalHitReact(const FVector& ImpactPoint)
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
	return PlayHitReactMontage(Section);
}

void ABaseCharacter::SelectDeathMontage()
{
	if (DeathMontage)
	{
		int32 Selection = FMath::RandRange(0, DeathMontage->GetNumSections() - 1);
		TEnumAsByte<EDeathPose> Pose(Selection);
		DeathPose = Pose;
	}
}

void ABaseCharacter::DisableCapsule()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABaseCharacter::ResetPeripheralVision()
{
	if (PawnSensing)
		PawnSensing->SetPeripheralVisionAngle(DefaultPeripheralVision);
}

UAnimInstance* ABaseCharacter::PlayMontageSection(UAnimMontage* Montage, const FName& SectionName, float AnimationSpeed, bool SetEndDelegate)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && Montage)
	{
		AnimInstance->Montage_Play(Montage, AnimationSpeed);
		AnimInstance->Montage_JumpToSection(SectionName, Montage);
		if (SetEndDelegate)
			AnimInstance->Montage_SetEndDelegate(EndMontageDelegate);
		return AnimInstance;
	}
	return nullptr;
}
void ABaseCharacter::StopAnimMontage(UAnimMontage* Montage)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && Montage)
	{
		AnimInstance->Montage_Stop(0.1f, Montage);
	}
}

FName ABaseCharacter::RandomMontageSection(UAnimMontage* Montage, FString MontagePrefix)
{
	FName SectionName;
	if (Montage)
	{
		int32 Selection = FMath::RandRange(1, Montage->GetNumSections());
		SectionName = FName(MontagePrefix + FString::FromInt(Selection));
	}
	return SectionName;
}

void ABaseCharacter::PlayAttackMontage()
{
	UAnimMontage* EquippedWeaponMontage = GetAttackMontageByWeaponType();
	if (EquippedWeaponMontage)
	{		
		PlayMontageSection(EquippedWeaponMontage, RandomMontageSection(EquippedWeaponMontage, FString("Attack")), AttackAnimationSpeed);
	}
}

UAnimMontage* ABaseCharacter::GetAttackMontageByWeaponType()
{
	if (EquippedWeapon)
	{
		EWeaponType WeaponType = EquippedWeapon->GetWeaponType();
		switch (WeaponType)
		{
			case EWeaponType::EWT_OneHand:
				return AttackMontage1H;
			case EWeaponType::EWT_TwoHand:
				return AttackMontage2H;
			case EWeaponType::EWT_Throw:
				return AttackMontage1H;
			case EWeaponType::EWT_BothHands:
				return AttackMontage1H;
		}
	}
	return nullptr;
}

void ABaseCharacter::OnActionEnded(UAnimMontage* Montage, bool bInterrupted)
{
	ActionState = EActionState::EAS_Unoccupied;
	if (bDebugStates)
		UE_LOG(LogTemp, Warning, TEXT("ActionState set to EAS_Unoccupied BaseCharacter (onactionended)"));
}

bool ABaseCharacter::CanAttack()
{
	return (ActionState == EActionState::EAS_Unoccupied);
}

bool ABaseCharacter::IsAlive()
{
	return CharAttributes && CharAttributes->isAlive();
}

bool ABaseCharacter::IsUnocuppied()
{
	return ActionState == EActionState::EAS_Unoccupied;
}

void ABaseCharacter::PlayHitSound(const FVector& ImpactPoint)
{
	if (HitSound)
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, ImpactPoint);
}

void ABaseCharacter::SpawnHitParticles(const FVector& ImpactPoint)
{
	if (HitParticles)
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticles, ImpactPoint);
}

void ABaseCharacter::HandleDamage(float DamageAmount)
{
	if (CharAttributes)
	{
		CharAttributes->ReceiveDamage(DamageAmount);
	}
}

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABaseCharacter::SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled)
{
	if (EquippedWeapon)
	{
		if (CollisionEnabled == ECollisionEnabled::NoCollision)
			EquippedWeapon->DisableWeaponCollision();
		else
			EquippedWeapon->EnableWeaponCollision();
		EquippedWeapon->IgnoreActors.Empty();

	}
}



