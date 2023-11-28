// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/RashepurHUD.h"
#include "HUD/HUDOverlay.h"

void ARashepurHUD::BeginPlay()
{
	Super::BeginPlay();
	UWorld* World = GetWorld();
	if (World)
	{
		// Create the HUD overlay widget and add it to the viewport
		if (HUDOverlayClass)
		{
			APlayerController* PlayerController = World->GetFirstPlayerController();
			if (PlayerController)
			{
				HUDOverlay = CreateWidget<UHUDOverlay>(PlayerController, HUDOverlayClass);
				HUDOverlay->AddToViewport();
			}
		}
	}
	
}
