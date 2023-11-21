// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroCharacter.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GroomComponent.h"
#include "Rashepur/Item.h"
#include "Rashepur/Weapons/Weapon.h"
#include "Animation/AnimMontage.h"
#include "Components/BoxComponent.h"

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

	Hair = CreateDefaultSubobject<UGroomComponent>(TEXT("Hero Hair"));
	Hair->SetupAttachment(GetMesh());
	Hair->AttachmentName = FString("head");

	EyeBrows = CreateDefaultSubobject<UGroomComponent>(TEXT("Hero EyeBrows"));
	EyeBrows->SetupAttachment(GetMesh());
	EyeBrows->AttachmentName = FString("head");

	//torna o status do personagem para desocupado ao final das montagens
	EndMontageDelegate.BindUObject(this, &AHeroCharacter::OnActionEnded);
}

// Called when the game starts or when spawned
void AHeroCharacter::BeginPlay()
{
	Super::BeginPlay();
	// aqui tá fzendo o cast pra pegar o controlador e saber se não tá nulo, pega o jogador a partir do controle embaixo
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			// o jogador tem subsistemas. pega o subsistema do enhancedinput e adiciona o contexto a ele
			Subsystem->AddMappingContext(HeroMappingContext, 0);
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
		OverlappingWeapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
		CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
		OverlappingItem = nullptr; // reseta o ponteiro para o item que foi pego
		EquippedWeapon = OverlappingWeapon;
	}
	else
	{
		if (CanUnequip()) 
		{ 
			CharacterState = ECharacterState::ECS_Unequipped;
			PlayEActionMontage("Unequip");
			ActionState = EActionState::EAS_PerformingAction;
		} 
		else if (CanEquip()) 
		{
			CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
			PlayEActionMontage("Equip");
			ActionState = EActionState::EAS_PerformingAction;
		}
	}
}

void AHeroCharacter::Attack(const FInputActionValue &Value)
{
	if ((ActionState == EActionState::EAS_Unoccupied) && (CharacterState != ECharacterState::ECS_Unequipped))
	{
		PlayAttackMontage();
	}
}

bool AHeroCharacter::CanUnequip()
{
    return ActionState == EActionState::EAS_Unoccupied 
			&& CharacterState != ECharacterState::ECS_Unequipped;
}

void AHeroCharacter::Disarm()
{
	if (EquippedWeapon) EquippedWeapon->AttachMeshSocket(GetMesh(), FName("SpineSocket"));
}

void AHeroCharacter::EquipWeapon()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshSocket(GetMesh(), FName("RightHandSocket"));
		//SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
	} 
	
}

bool AHeroCharacter::CanEquip()
{
    return ActionState == EActionState::EAS_Unoccupied 
			&& CharacterState == ECharacterState::ECS_Unequipped 
			&& EquippedWeapon;
}

void AHeroCharacter::PlayAttackMontage()
{
	ActionState = EActionState::EAS_Occupied;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AttackMontage)
	{
		AnimInstance->Montage_Play(AttackMontage, AttackAnimationSpeed);
		int32 Selection = FMath::RandRange(0,1);
		FName SectionName = FName();
		switch (Selection)
		{
			case 0:
				SectionName = FName ("Attack1");
				break;
			case 1:
				SectionName = FName ("Attack2");
				break;
			default:
				break;
		}
		AnimInstance->Montage_JumpToSection(SectionName, AttackMontage);
		AnimInstance->Montage_SetEndDelegate(EndMontageDelegate);
	}
}
void AHeroCharacter::PlayEActionMontage(const FName& SectionName)
{
	ActionState = EActionState::EAS_Occupied;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && EActionMontage)
	{
		AnimInstance->Montage_Play(EActionMontage, AttackAnimationSpeed);
		AnimInstance->Montage_JumpToSection(SectionName, EActionMontage);
		AnimInstance->Montage_SetEndDelegate(EndMontageDelegate);
	}
}

void AHeroCharacter::OnActionEnded(UAnimMontage *Montage, bool bInterrupted)
{
	ActionState = EActionState::EAS_Unoccupied;
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

void AHeroCharacter::SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled)
{
    if (EquippedWeapon && EquippedWeapon->GetWeaponBox())
	{
		EquippedWeapon->GetWeaponBox()->SetCollisionEnabled(CollisionEnabled);
		EquippedWeapon->IgnoreActors.Empty();
	}
}

