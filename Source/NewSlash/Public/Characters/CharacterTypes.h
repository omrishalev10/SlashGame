#pragma once

// UE has different enum then regular c++
UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	// UMETA is the name we will see on the blue prints
	ECS_Unequipped UMETA(DisplayName = "Unequiped"),
	ECS_LightWeapon UMETA(DisplayName = "LightWeapon"),
	ECS_HeavyWeapon UMETA(DisplayName = "HeavyWeapon")
};

UENUM(BlueprintType)
enum class EActionState : uint8
{
	EAS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	EAS_HitReaction UMETA(DisplayName = "HitReaction"),
	EAS_Attacking UMETA(DisplayName = "Attacking"),
	EAS_EquippingWeapon UMETA(DisplayName = "Equipping Weapon"),
	EAS_Dodge UMETA(DisplayName = "Dodge"),
	EAS_Dead UMETA(DisplayName = "Dead")	
};

UENUM(BlueprintType)
enum class EDeathPose : uint8
{
	EDP_Death1 UMETA(DisplayName = "Death1"),
	EDP_Death2 UMETA(DisplayName = "Death2"),
	EDP_Death3 UMETA(DisplayName = "Death3"),
	EDP_Death4 UMETA(DisplayName = "Death4"),
	EDP_Death5 UMETA(DisplayName = "Death5"),

	EDP_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	// UMETA is the name we will see on the blue prints
	EES_NoState UMETA(DisplayName = "NoState"),
	EES_Dead UMETA(DisplayName = "Dead"),
	EES_Patrolling UMETA(DisplayName = "Patrolling"),
	EES_Chasing UMETA(DisplayName = "Chasing"),
	EES_Attcking UMETA(DisplayName = "Attcking"),
	EES_Engaged UMETA(DisplayName = "Engaged")
};

UENUM(BlueprintType)
enum class EMovementState : uint8
{
	EMS_Idle UMETA(DisplayName = "Idle"),
	EMS_Walking UMETA(DisplayName = "Walking"),
	EMS_Crouching UMETA(DisplayName = "Crouching"),
	EMS_Running UMETA(DisplayName = "Running"),
	EMS_Sprinting UMETA(DisplayName = "Sprinting"),
	EMS_Other UMETA(DisplayName = "Other")
};

UENUM(BlueprintType)
enum class EPotionType : uint8
{
	EPT_HealthPotion UMETA(DisplayName = "Health Potion"),
	EPT_StaminaPotion UMETA(DisplayName = "Stamina Potion"),
	EPT_FullAttributesPotion UMETA(DisplayName = "Full Attributes Potion")
};