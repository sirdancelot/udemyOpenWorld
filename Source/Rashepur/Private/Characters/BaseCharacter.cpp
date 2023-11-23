#pragma once


#include "Characters/BaseCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/AttributeComponent.h"

#include "Rashepur/Weapons/Weapon.h"

ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	// Class default components setup
	CharAttributes = CreateDefaultSubobject<UAttributeComponent>(TEXT("CharAttributes"));
	
	//torna o status do personagem para desocupado ao final das montagens
	EndMontageDelegate.BindUObject(this, &ABaseCharacter::OnActionEnded);
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABaseCharacter::Attack()
{
}

void ABaseCharacter::Die()
{
}

void ABaseCharacter::PlayHitReactMontage(const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage, 1.0f);
		AnimInstance->Montage_JumpToSection(SectionName, HitReactMontage);
	}
}

void ABaseCharacter::DirectionalHitReact(const FVector& ImpactPoint)
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

void ABaseCharacter::SelectDeathMontage()
{
	if (DeathMontage)
	{
		int32 Selection = FMath::RandRange(0, DeathMontage->GetNumSections() - 1);
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

void ABaseCharacter::PlayAttackMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	UAnimMontage* EquippedWeaponMontage = GetAttackMontageByWeaponType();

	if (AnimInstance && EquippedWeaponMontage)
	{
		AnimInstance->Montage_Play(EquippedWeaponMontage, AttackAnimationSpeed);

		int32 Selection = FMath::RandRange(1, EquippedWeaponMontage->GetNumSections());
		FName SectionName = FName(FString("Attack" + FString::FromInt(Selection)));

		AnimInstance->Montage_JumpToSection(SectionName, EquippedWeaponMontage);
		AnimInstance->Montage_SetEndDelegate(EndMontageDelegate);
	}
}

UAnimMontage* ABaseCharacter::GetAttackMontageByWeaponType()
{
	if (EquippedWeapon)
	{
		if (EquippedWeapon->GetWeaponType() == EWeaponType::EWT_OneHand)
			return AttackMontage1H;
		else if (EquippedWeapon->GetWeaponType() == EWeaponType::EWT_TwoHand)
			return AttackMontage2H;
	}
	return nullptr;
}

void ABaseCharacter::OnActionEnded(UAnimMontage* Montage, bool bInterrupted)
{
	ActionState = EActionState::EAS_Unoccupied;
}

bool ABaseCharacter::CanAttack()
{
	return false;
}

void ABaseCharacter::AttackEnd()
{
}

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseCharacter::SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled)
{
	if (EquippedWeapon && EquippedWeapon->GetWeaponBox())
	{
		EquippedWeapon->GetWeaponBox()->SetCollisionEnabled(CollisionEnabled);
		if (CollisionEnabled == ECollisionResponse::ECR_Ignore)
		{
			EquippedWeapon->GetWeaponBox()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			
		}
		else 
		{
			EquippedWeapon->GetWeaponBox()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
		}
		EquippedWeapon->GetWeaponBox()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		EquippedWeapon->IgnoreActors.Empty();
	}
}



