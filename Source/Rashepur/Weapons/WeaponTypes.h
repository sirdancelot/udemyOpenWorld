#pragma once

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_OneHand UMETA(DisplayName= "One-Handed Weapon"),
	EWT_TwoHand UMETA(DisplayName= "Two-Handed Weapon"),
    EWT_Throw UMETA(DisplayName= "Throwing Weapon"),
	EWT_BothHands UMETA(DisplayName = "Both Hands Weapon"),

};
