// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CharacterStates.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "HeroCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UGroomComponent;
class AItem;
class UAnimMontage;
class AWeapon;

UCLASS()
class RASHEPUR_API AHeroCharacter : public ACharacter
{
	GENERATED_BODY()

public:

	AHeroCharacter();
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable)
    void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/** 
	 * Callback to inputs
	*/
	void Move(const FInputActionValue& Value);
	void LookAround(const FInputActionValue& Value);
	void EKeyPressed(const FInputActionValue& Value);
    void SetCharacterStateByWeaponType();
    FName GetWeaponSocket(AWeapon *OverlappingWeapon);
    FName GetWeaponSpineSocket(AWeapon *OverlappingWeapon);
    void Attack(const FInputActionValue &Value);

    bool CanEquip();
    bool CanUnequip();

    // gets called in abp_hero via notify in animation.
    UFUNCTION(BlueprintCallable)	
	void Disarm();

	UFUNCTION(BlueprintCallable)	
	void EquipWeapon();

    /**
	 * Animation Montages
	*/
	void PlayAttackMontage();
	void PlayEActionMontage(const FName& SectionName);
	void OnActionEnded(UAnimMontage* Montage, bool bInterrupted);
	
private: 
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputMappingContext* HeroMappingContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MovementAction;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* LookAroundAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* EKeyPressAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* AttackAction;

	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* ViewCamera;

	UPROPERTY(VisibleAnywhere)
	UGroomComponent* Hair;

	UPROPERTY(VisibleAnywhere)
	UGroomComponent* EyeBrows;

	UPROPERTY(VisibleInstanceOnly)
	AItem* OverlappingItem;

	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	EActionState ActionState = EActionState::EAS_Unoccupied;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"), Category = "Animation")
	float AttackAnimationSpeed = 1.5f;

	/** Animation Montages */
	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	UAnimMontage* AttackMontage1H;

	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	UAnimMontage* AttackMontage2H;
	
	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	UAnimMontage* EActionMontage;

	FOnMontageEnded EndMontageDelegate;
	
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Weapon")
	AWeapon* EquippedWeapon;

	UAnimMontage* GetAttackAnimationByWeaponType();

    void AttachWeaponToSocket(FName Socket);

public:	
	FORCEINLINE void SetOverlappingItem(AItem* Item) { OverlappingItem = Item; }
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
	UFUNCTION(BlueprintPure)
	FORCEINLINE AWeapon* GetEquippedWeapon() const { return EquippedWeapon; }
};
