#pragma once

UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	ECS_Unequipped UMETA(DisplayName= "Unequipped"),
	ECS_EquippedOneHandedWeapon UMETA(DisplayName= "Equipped One-Handed Weapon"),
	ECS_EquippedTwoHandedWeapon UMETA(DisplayName= "Equipped Two-Handed Weapon"),
	ECS_EquippedThrowingWeapon UMETA(DisplayName= "Equipped Throwing Weapon"),
	ECS_EquippedDualHands UMETA(DisplayName = "Equipped Dual Hands"),
	ECS_Dead UMETA(DisplayName = "Dead"),
	ECS_MAX UMETA(DisplayName = "DefaultMax")
};

UENUM(BlueprintType)
enum class EActionState : uint8
{
	EAS_Unoccupied UMETA (DisplayName = "Unoccupied"),
	EAS_HitReaction UMETA(DisplayName = "Hit Reaction"),
	EAS_Occupied UMETA (DisplayName = "Occupied"),
	EAS_PerformingAction UMETA (DisplayName = "Performing Action"),
	EAS_Attacking UMETA(DisplayName = "Attacking")
};

UENUM(BlueprintType)
enum class EDeathPose : uint8
{
	EDP_Death1 UMETA (DisplayName = "Death1"),
	EDP_Death2 UMETA (DisplayName = "Death2"),
	EDP_Death3 UMETA (DisplayName = "Death3"),
	EDP_Death4 UMETA (DisplayName = "Death4"),

	EDP_MAX UMETA(DisplayName = "DefaultMax")
};

UENUM(BlueprintType)
enum class EEnemyState: uint8
{
	EES_Staggered UMETA(DisplayName = "Staggered"),
	EES_Dead UMETA(DisplayName = "Dead"),
	EES_Patrolling UMETA(DisplayName = "Patrolling"),
	EES_Chasing UMETA(DisplayName = "Chasing"),
	EES_Searching UMETA(DisplayName = "Searching"),
	EES_Engaged UMETA(DisplayName = "Engaged"),
	EES_NoState UMETA(DisplayName = "No State")

};