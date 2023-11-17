// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Rashepur/Item.h"
#include "Weapon.generated.h"

/**
 * 
 */
class USoundBase;
UCLASS()
class RASHEPUR_API AWeapon : public AItem
{
	GENERATED_BODY()
	
public:
	void Equip(USceneComponent* InParent, FName InSocketName);
    void PlayEquipSound();
    void AttachMeshSocket(USceneComponent *InParent, const FName &InSocketName);

protected:
    virtual void OnSphereOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult) override;
    virtual void OnSphereOverlapEnd( UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;

private:
    UPROPERTY(EditAnywhere, Category = "Weapon Properties")
    USoundBase* EquipSound;

    UPROPERTY(EditAnywhere, Category = "Weapon Properties")
    USoundBase* UnequipSound;

public:
    UFUNCTION(BlueprintPure)
	FORCEINLINE USoundBase* GetEquipWeaponSound() const { return EquipSound; }

    UFUNCTION(BlueprintPure)
	FORCEINLINE USoundBase* GetUnequipWeaponSound() const { return UnequipSound; }
};
