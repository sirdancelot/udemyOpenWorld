// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroCharacter.h"
#include "Rashepur/Weapons/WeaponTypes.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GroomComponent.h"
#include "Rashepur/Item.h"
#include "Rashepur/Weapons/Weapon.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AttributeComponent.h"
#include "Animation/AnimMontage.h"
#include "Perception/PawnSensingComponent.h"
#include "HUD/RashepurHUD.h"
#include "HUD/HUDOverlay.h"


// Sets default values
AHeroCharacter::AHeroCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
	
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->TargetArmLength = 400;

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	ViewCamera->SetupAttachment(SpringArm);

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 410, 0.f);

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);

	Hair = CreateDefaultSubobject<UGroomComponent>(TEXT("Hero Hair"));
	Hair->SetupAttachment(GetMesh());
	Hair->AttachmentName = FString("head");

	EyeBrows = CreateDefaultSubobject<UGroomComponent>(TEXT("Hero EyeBrows"));
	EyeBrows->SetupAttachment(GetMesh());
	EyeBrows->AttachmentName = FString("head");

	AttackAnimationSpeed = 1.5f;
}

// Called when the game starts or when spawned
void AHeroCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		InitializeOverlay(PlayerController);
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(HeroMappingContext, 0);
		}
	}
	Tags.Add(FName("Hero")); 
}

void AHeroCharacter::InitializeOverlay(APlayerController* PlayerController)
{
	ARashepurHUD* HUD = Cast<ARashepurHUD>(PlayerController->GetHUD());
	if (HUD)
	{
		HUDOverlay = HUD->GetHUDOverlay();
		if (HUDOverlay && CharAttributes)
		{
			HUDOverlay->SetHealthBarPercent(CharAttributes->GetHealthPercent());
			HUDOverlay->SetStaminaBarPercent(1.0f);
			HUDOverlay->SetGold(0);
			HUDOverlay->SetSouls(0);
		}
	}
}



// Called every frame
void AHeroCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AHeroCharacter::Move(const FInputActionValue &Value)
{
	if (ActionState != EActionState::EAS_Unoccupied) return;
	const FVector2D MovementVector = Value.Get<FVector2D>();
	const FRotator ControlRotation = GetControlRotation();
	const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);

	// yaw é esquerda direita, pitch é cima baixo
	const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(Forward, MovementVector.Y);
	AddMovementInput(Right, MovementVector.X); 
	
}

void AHeroCharacter::LookAround(const FInputActionValue &Value)
{
	const FVector2D LookAxisValue = Value.Get<FVector2D>();
	if(GetController())
	{
		AddControllerYawInput(LookAxisValue.X);
		AddControllerPitchInput(LookAxisValue.Y * -1);
	}
}

void AHeroCharacter::EKeyPressed(const FInputActionValue &Value)
{
	AWeapon* OverlappingWeapon = Cast<AWeapon>(OverlappingItem);
	if (OverlappingWeapon)
	{
		// se ja possuir uma arma, destruir a arma atual primeiro
		if (EquippedWeapon)
		{
			EquippedWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);	
			EquippedWeapon->SetItemState(EItemState::EIS_Hovering);
			EquippedWeapon->SetActorRotation(FRotator(0.f, 0.f, 0.f));
			EquippedWeapon=nullptr;
		}
        FName WeaponSocket = GetWeaponSocket(OverlappingWeapon);
		OverlappingWeapon->Equip(GetMesh(), WeaponSocket, this, this);
		OverlappingItem = nullptr; // reseta o ponteiro para o item que foi pego
		EquippedWeapon = OverlappingWeapon;
		SetCharacterStateByWeaponType();		
	}
	else
	{
		if (CanUnequip()) 
		{ 
			PlayEActionMontage("Unequip");
			CharacterState = ECharacterState::ECS_Unequipped;			
		} 
		else if (CanEquip()) 
		{
			PlayEActionMontage("Equip");
			SetCharacterStateByWeaponType();
		}
	}
}

bool AHeroCharacter::CanAttack()
{
	return (ActionState == EActionState::EAS_Unoccupied) && (CharacterState != ECharacterState::ECS_Unequipped);
}

void AHeroCharacter::Attack()
{
	if (CanAttack())
	{
		Super::Attack();
	}
}

// chamados a partir do blueprint pra tirar da mão ou colocar na mão a arma do personagem
void AHeroCharacter::Disarm()
{
	AttachWeaponToSocket(GetWeaponSpineSocket(EquippedWeapon));
}

// chamados a partir do blueprint pra tirar da mão ou colocar na mão a arma do personagem
void AHeroCharacter::EquipWeapon()
{
	AttachWeaponToSocket(GetWeaponSocket(EquippedWeapon));
}

void AHeroCharacter::AttachWeaponToSocket(FName Socket)
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshSocket(GetMesh(), Socket);
	} 
}


bool AHeroCharacter::CanEquip() const
{
    return ActionState == EActionState::EAS_Unoccupied 
			&& EquippedWeapon;
}

bool AHeroCharacter::CanUnequip() const
{
    return ActionState == EActionState::EAS_Unoccupied 
			&& CharacterState != ECharacterState::ECS_Unequipped;
}

void AHeroCharacter::PlayEActionMontage(const FName& SectionName)
{
	if (EActionMontage) 
	{
		ActionState = EActionState::EAS_Occupied;
		if (bDebugStates)
			UE_LOG(LogTemp, Warning, TEXT("ActionState set to EAS_Occupied HeroCharacter (PlayEActionMontage)"));
		PlayMontageSection(EActionMontage, SectionName);
	}
}

void AHeroCharacter::OnActionEnded(UAnimMontage *Montage, bool bInterrupted)
{
	ActionState = EActionState::EAS_Unoccupied;
	if (bDebugStates)
		UE_LOG(LogTemp, Warning, TEXT("ActionState set to EAS_Unoccupied HeroCharacter (OnActionEnded)"));
}

// Called to bind functionality to input
void AHeroCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// adiciona ao componente enhanced input a acao e a função que responde por essa ação
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &AHeroCharacter::Move);
		EnhancedInputComponent->BindAction(LookAroundAction, ETriggerEvent::Triggered, this, &AHeroCharacter::LookAround);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(EKeyPressAction, ETriggerEvent::Triggered, this, &AHeroCharacter::EKeyPressed);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &AHeroCharacter::Attack);

	}
}

void AHeroCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	Super::GetHit_Implementation(ImpactPoint, Hitter);
}

float AHeroCharacter::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	HandleDamage(DamageAmount);
	SetHUDHealth();

	return DamageAmount;
}

void AHeroCharacter::Die()
{
	Super::Die();
	CharacterState = ECharacterState::ECS_Dead;
}

void AHeroCharacter::Jump()
{
	if (IsUnocuppied() && IsAlive())
		Super::Jump();
}

void AHeroCharacter::SetOverlappingItem(AItem* Item)
{
	OverlappingItem = Item;
}

void AHeroCharacter::AddSouls(ASoul* Soul)
{
	UE_LOG(LogTemp, Warning, TEXT("Add Souls: "));
}

void AHeroCharacter::SetHUDHealth()
{
	if (HUDOverlay && CharAttributes)
	{
		HUDOverlay->SetHealthBarPercent(CharAttributes->GetHealthPercent());
	}
}


