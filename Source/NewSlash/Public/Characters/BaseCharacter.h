#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/HitInterface.h"
#include "CharacterTypes.h"
#include "BaseCharacter.generated.h"

/* Forward declerations */
class AWeapon;
class UAttributeComponent;
class UAnimMontage;

UCLASS()
class NEWSLASH_API ABaseCharacter : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:
	ABaseCharacter();
	virtual void Tick(float DeltaTime) override;

protected:
	/** <AActor> */
	virtual void BeginPlay() override;
	/** </AActor> */

	/** <IHitInterface> */
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;
	/** </IHitInterface> */
	
	/** Combat */
	virtual void Attack();	

	UFUNCTION(BlueprintNativeEvent)
	void Die();

	void DirectionalHitReact(const FVector& ImpactPoint); // Find the Direction of the attack
	virtual void HandleDamage(float DamageAmount);
	void PlayHitSound(const FVector& ImpactPoint);
	void SpawnHitParticles(const FVector& ImpactPoint);
	void DisableCapsule();
	virtual bool CanAttack(); // Check if the player is available to attack
	bool IsAlive();
	void DisableMeshCollision();

	UFUNCTION(BlueprintCallable)
	virtual void AttackEnd(); // Set the Character State after Attacking to Unoccupied

	UFUNCTION(BlueprintCallable)
	virtual void DodgeEnd(); // Set the character Action State after Dodging to Unoccupied
	
	UFUNCTION(BlueprintCallable)
	void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled); 

	/* Combat Montages */
	virtual int32 PlayLightAttackMontage();
	virtual int32 PlayHeavyAttackMontage();
	virtual int32 PlayUnequipAttackMontage();
	virtual int32 PlayJumpAttackMontage();
	virtual void PlayDodgeMontage();
	void StopAttackMontage();
	
	UFUNCTION(BlueprintCallable)
	FVector GetTranslationWarpTarget();
	
	UFUNCTION(BlueprintCallable)
	FVector GetRotationWarpTarget();

	/* Hit & Death Montage*/
	void PlayHitReactMontage(const FName& SectionName);
	virtual int32 PlayDeathMontage();

	UPROPERTY(BlueprintReadOnly, Category = Combat)
	AActor* CombatTarget;

	UPROPERTY(EditDefaultsOnly, Category = Weapon)
	AWeapon* EquippedWeapon;
	
	UPROPERTY(EditDefaultsOnly, Category = Weapon)
	AWeapon* EquippedSecondWeapon;

	UPROPERTY(EditDefaultsOnly, Category = Weapon)
	AWeapon* EquippedFlamethrowerWeapon;

	/* Combat Properties */
	UPROPERTY(VisibleAnywhere)
	UAttributeComponent* Attributes;
	
	ECharacterState CharacterState = ECharacterState::ECS_Unequipped; // The weapon type of the character

	UPROPERTY(EditAnywhere, Category = Combat)
	double WarpTargetDistance = 75.f;

	UPROPERTY(BlueprintReadOnly)
	EDeathPose DeathPose; // Initialize Death Enum value 

private:
	void PlayMontageSection(UAnimMontage* Montage, const FName& SectionName);
	int32 PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames);
	
	/* Sound and Particles */
	UPROPERTY(EditAnywhere, Category = Combat)
	USoundBase* HitSound;

	UPROPERTY(EditAnywhere, Category = Combat)
	UParticleSystem* HitParticles;

	/* Animation Montage */
	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* LightAttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* HeavyAttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* UnequipAttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* JumpAttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* DeathMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* DodgeMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	TArray<FName> LightAttackMontageSections;

	UPROPERTY(EditAnywhere, Category = Combat)
	TArray<FName> HeavyAttackMontageSections;

	UPROPERTY(EditAnywhere, Category = Combat)
	TArray<FName> UnequippedAttackMontageSections;

	UPROPERTY(EditAnywhere, Category = Combat)
	TArray<FName> JumpAttackMontageSections;

	UPROPERTY(EditAnywhere, Category = Combat)
	TArray<FName> DeathMontageSections;

public:
	FORCEINLINE TEnumAsByte<EDeathPose> GetDeathPose() const { return DeathPose; }
};
