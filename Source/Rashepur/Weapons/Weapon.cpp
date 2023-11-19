// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Rashepur/Characters/HeroCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Interfaces/HitInterface.h"


AWeapon::AWeapon()
{
    WeaponBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Weapon Box"));
    WeaponBox->SetupAttachment(GetRootComponent());
    WeaponBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    WeaponBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    WeaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

    BoxTraceStart  = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace Start"));
    BoxTraceStart->SetupAttachment(WeaponBox);

    BoxTraceEnd  = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace End"));
    BoxTraceEnd->SetupAttachment(WeaponBox);
}

void AWeapon::Equip(USceneComponent* InParent, FName InSocketName)
{
    AttachMeshSocket(InParent, InSocketName);
    ItemState = EItemState::EIS_Equipped;
    PlayEquipSound();
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

// funcao que faz um sweep de trace pra ver se acerta algo.
void AWeapon::OnBoxOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
    const FVector Start = BoxTraceStart->GetComponentLocation();
    const FVector End = BoxTraceEnd->GetComponentLocation();

    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(this);
    for (AActor* Actor : IgnoreActors)
    {
        ActorsToIgnore.AddUnique(Actor);
    }

    FHitResult BoxHit;

    UKismetSystemLibrary::BoxTraceSingle(this,
                                        Start, 
                                        End, 
                                        FVector(5.f, 5.f, 5.f), 
                                        BoxTraceStart->GetComponentRotation(), 
                                        ETraceTypeQuery::TraceTypeQuery1, 
                                        false,
                                        ActorsToIgnore,
                                        EDrawDebugTrace::None,
                                        BoxHit,
                                        true
    );
    if (BoxHit.GetActor()) // vai ser nulo se n�o acertar nada
    {
        IHitInterface* HitInterface = Cast<IHitInterface>(BoxHit.GetActor()); // se o ator implementa a interface de objetos "hitaveis" retorna algo, senao null
        if (HitInterface)
        {
            HitInterface->GetHit(BoxHit.ImpactPoint);
        }
        IgnoreActors.AddUnique(BoxHit.GetActor());
    }
}
