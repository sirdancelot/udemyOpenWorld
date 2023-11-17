#pragma once

UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	ECS_Unequipped UMETA(DisplayName= "Unequipped"),
	ECS_EquippedOneHandedWeapon UMETA(DisplayName= "Equipped One-Handed Weapon"),
	ECS_EquippedTwoHandedWeapon UMETA(DisplayName= "Equipped Two-Handed Weapon")
};

UENUM(BlueprintType)
enum class EActionState : uint8
{
	EAS_Unoccupied UMETA (DisplayName = "Unoccupied"),
	EAS_Occupied UMETA (DisplayName = "Occupied"),
	EAS_PerformingAction UMETA (DisplayName = "Performing Action")
};