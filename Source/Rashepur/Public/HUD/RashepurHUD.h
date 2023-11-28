// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "RashepurHUD.generated.h"

/**
 * 
 */
UCLASS()
class RASHEPUR_API ARashepurHUD : public AHUD
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
private:
	UPROPERTY(EditDefaultsOnly, Category = "Rashepur")
	TSubclassOf<class UHUDOverlay> HUDOverlayClass;

	UPROPERTY()
	UHUDOverlay* HUDOverlay;
public:
	FORCEINLINE UHUDOverlay* GetHUDOverlay() const { return HUDOverlay; }
};
