// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Rashepur/Characters/HeroCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Interfaces/HitInterface.h"
#include "NiagaraComponent.h"


AWeapon::AWeapon()
{
    WeaponBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Weapon Box"));
    WeaponBox->SetupAttachment(GetRootComponent());

    DisableWeaponCollision();
    WeaponBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
    WeaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

    BoxTraceStart  = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace Start"));
    BoxTraceStart->SetupAttachment(WeaponBox);

    BoxTraceEnd  = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace End"));
    BoxTraceEnd->SetupAttachment(WeaponBox);

}

void AWeapon::DisableWeaponCollision()
{
    if (WeaponBox)
    {
        WeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        WeaponBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    }
}
void AWeapon::EnableWeaponCollision()
{
    if (WeaponBox)
    {
        WeaponBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        WeaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
        WeaponBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
    }
}

void AWeapon::Equip(USceneComponent* InParent, FName InSocketName, AActor* NewOwner, APawn* NewInstigator)
{    
    ItemState = EItemState::EIS_Equipped;
    SetOwner(NewOwner);
    SetInstigator(NewInstigator);
    AttachMeshSocket(InParent, InSocketName);
    PlayEquipSound();
    DisableSphereCollision();
    DeactivateEmbers();
    
}

void AWeapon::DeactivateEmbers()
{
    if (EmbersEffect)
    {
        EmbersEffect->Deactivate();
    }
}

void AWeapon::DisableSphereCollision()
{
    if (Sphere)
    {
        Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}

void AWeapon::PlayEquipSound()
{
    if (EquipSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, EquipSound, GetActorLocation());
    }
}

void AWeapon::AttachMeshSocket(USceneComponent *InParent, const FName &InSocketName)
{
    FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, true);
    ItemMesh->AttachToComponent(InParent, TransformRules, InSocketName);
}

void AWeapon::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AWeapon::BeginPlay()
{
    Super::BeginPlay();
    WeaponBox->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnBoxOverlap);
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
    Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep,SweepResult );
}

void AWeapon::OnSphereOverlapEnd(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex)
{
    Super::OnSphereOverlapEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
}


void AWeapon::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (ActorIsSameType(OtherActor)) return;

    FHitResult BoxHit;
    BoxTrace(BoxHit);
    IgnoreActors.AddUnique(GetOwner());

    if (BoxHit.GetActor())
    {
        if (ActorIsSameType(BoxHit.GetActor())) return;

        UGameplayStatics::ApplyDamage(BoxHit.GetActor(), Damage, GetInstigator()->GetController(), this, UDamageType::StaticClass());
        ExecuteGetHit(BoxHit);
        CreateFields(BoxHit.ImpactPoint);
    }
}

bool AWeapon::ActorIsSameType(AActor* OtherActor)
{
    return GetOwner()->ActorHasTag(TEXT("Enemy")) && OtherActor->ActorHasTag(TEXT("Enemy"));
}

void AWeapon::ExecuteGetHit(FHitResult& BoxHit)
{
    IHitInterface* HitInterface = Cast<IHitInterface>(BoxHit.GetActor());
    if (HitInterface)
    {
        AActor* Hitter = GetOwner();
        HitInterface->Execute_GetHit(BoxHit.GetActor(), BoxHit.ImpactPoint, Hitter);
    }
}

void AWeapon::BoxTrace(FHitResult& BoxHit)
{
    const FVector Start = BoxTraceStart->GetComponentLocation();
    const FVector End = BoxTraceEnd->GetComponentLocation();

    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(GetOwner());

    for (AActor* Actor : IgnoreActors)
    {
        ActorsToIgnore.AddUnique(Actor);
    }

    UKismetSystemLibrary::BoxTraceSingle(
        this,
        Start,
        End,
        BoxTraceExtent,
        BoxTraceStart->GetComponentRotation(),
        ETraceTypeQuery::TraceTypeQuery1,
        false,
        ActorsToIgnore,
        bShowBoxDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
        BoxHit,
        true
    );
    IgnoreActors.AddUnique(BoxHit.GetActor());

}


