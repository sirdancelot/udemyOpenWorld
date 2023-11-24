// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponTypes.h"
#include "Rashepur/Item.h"
#include "Weapon.generated.h"

/**
 * 
 */
class USoundBase;
class UBoxComponent;
class USceneComponent;


UCLASS()
class RASHEPUR_API AWeapon : public AItem
{
	GENERATED_BODY()
	
public:
    AWeapon();
    void DisableWeaponCollision();
    void EnableWeaponCollision();
	void Equip(USceneComponent* InParent, FName InSocketName, AActor* NewOwner, APawn* NewInstigator);
    void DeactivateEmbers();
    void DisableSphereCollision();
    void PlayEquipSound();
    void AttachMeshSocket(USceneComponent *InParent, const FName &InSocketName);
    virtual void Tick(float DeltaTime) override;
    TArray<AActor*> IgnoreActors;

protected:
    virtual void BeginPlay() override;
    virtual void OnSphereOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult) override;
    virtual void OnSphereOverlapEnd( UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;
    
    UFUNCTION()
    virtual void OnBoxOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);

    bool ActorIsSameType(AActor* OtherActor);

    void ExecuteGetHit(FHitResult& BoxHit);

    UFUNCTION(BlueprintImplementableEvent)
    void CreateFields(const FVector& FieldLocation);

private:

    void BoxTrace(FHitResult& BoxHit);

    UPROPERTY(EditAnywhere, Category = "Weapon Properties")
    FVector BoxTraceExtent = FVector(8.f);

    UPROPERTY(EditAnywhere, Category = "Weapon Properties")
    bool bShowBoxDebug = false;

    UPROPERTY(EditAnywhere, Category = "Weapon Properties")
    USoundBase* EquipSound;

    UPROPERTY(EditAnywhere, Category = "Weapon Properties")
    USoundBase* UnequipSound;
    
    UPROPERTY(EditAnywhere, Category = "Weapon Properties")
    float Damage = 20;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon Properties")
    EWeaponType WeaponType = EWeaponType::EWT_OneHand;

    UPROPERTY(VisibleAnywhere)
    UBoxComponent* WeaponBox;

    UPROPERTY(VisibleAnywhere)
    USceneComponent* BoxTraceStart;

    UPROPERTY(VisibleAnywhere)
    USceneComponent* BoxTraceEnd;

 
public:
    UFUNCTION(BlueprintPure)
	FORCEINLINE USoundBase* GetEquipWeaponSound() const { return EquipSound; }

    UFUNCTION(BlueprintPure)
	FORCEINLINE USoundBase* GetUnequipWeaponSound() const { return UnequipSound; }

    FORCEINLINE UBoxComponent* GetWeaponBox() const { return WeaponBox; }
    FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
};
