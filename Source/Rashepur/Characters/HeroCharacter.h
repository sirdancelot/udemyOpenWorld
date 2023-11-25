// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CharacterStates.h"
#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "Characters/BaseCharacter.h"
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
class RASHEPUR_API AHeroCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:

	AHeroCharacter();
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;

protected:
	/** <AActor> */
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	/** </AActor> */

	/** 
	 * Callback to inputs
	*/
	void Move(const FInputActionValue& Value);
	void LookAround(const FInputActionValue& Value);
	void EKeyPressed(const FInputActionValue& Value);


	/**
	 * Weapon Handling
	*/
	// gets called in abp_hero via notify in animation.
	UFUNCTION(BlueprintCallable)
	void Disarm();

	UFUNCTION(BlueprintCallable)
	void EquipWeapon();
	
	/** <ABaseCharacter> */
	virtual void Attack() override;
	virtual bool CanAttack() override;
	/** </ABaseCharacter> */

    bool CanEquip() const;
    bool CanUnequip() const;

    /**
	 * Animation Montages
	*/
	void PlayEActionMontage(const FName& SectionName);
	void OnActionEnded(UAnimMontage* Montage, bool bInterrupted);
	
private: 
	void AttachWeaponToSocket(FName Socket);

	/** 
	 *	INPUT 
	 */

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
		
	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	UAnimMontage* EActionMontage;

public:	
	FORCEINLINE void SetOverlappingItem(AItem* Item) { OverlappingItem = Item; }
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
	UFUNCTION(BlueprintPure)
	FORCEINLINE AWeapon* GetEquippedWeapon() const { return EquippedWeapon; }
};
