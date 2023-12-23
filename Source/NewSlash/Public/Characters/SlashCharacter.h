#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "CharacterTypes.h"
#include "BaseCharacter.h"
#include "Components/BoxComponent.h"
#include "Interfaces/PickupInterface.h"
#include "Enemy/Enemy.h"
#include "SlashCharacter.generated.h"

// Forward declerations
class UInputMappingContext;
class UInputAction;
class USpringArmComponent;
class UCameraComponent;
class UGroomComponent;
class AItem;
class UAnimMontage;
class UBoxComponent;
class USlashOverlay;
class ASoul;
class ATreasure;

UCLASS()
class NEWSLASH_API ASlashCharacter : public ABaseCharacter, public IPickupInterface
{
	GENERATED_BODY()

public:
	ASlashCharacter();
	/** <AActor> */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	/** </AActor> */

	/** <Interfaces> */
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override; // Get Hit function - inherited from IHitInterface
	virtual void SetOverlappingItem(AItem* Item) override;
	virtual void AddSouls(ASoul* Soul) override;
	virtual void AddGold(ATreasure* Treasure) override;
	virtual void UsePotion(APotions* Potion) override;
	/** </Interfaces> */

	virtual void Jump() override; 	
protected:
	virtual void BeginPlay() override; 
	void Tick(float DeltaTime);

	/* Callbacks for Input Action functions */
	void Move(const FInputActionValue& Value); // Create a Move function. FInputActionValue is a STRUCT so must be included!!--Can not be forward declared!
	void Look(const FInputActionValue& Value); // Create a Look function (Moving the mouse to change the look view)
	void EKeyPressed();	// Using E key to few actions
	virtual void Attack() override;
	void StartCrouching();
	void StopCrouching();
	void StartSprinting();
	void StopSprinting();
	void StartWalking();
	void StopWalking();
	void TargetLockOn();
	void ChangeTargetLockedOn();
	void Dodge();
	
	/** Combat */
	void EquipWeapon(AWeapon* Weapon, ECharacterState WeaponType);
	virtual void AttackEnd() override; // Set the character state after attacking to Unoccupied
	virtual void DodgeEnd() override; // Set the character state after attacking to Unoccupied
	virtual bool CanAttack() override; // Check if the player is available to attack
	bool CanDisarm(); // Check if character can Disarm the weapon
	bool CanArm(); // Check if character can Arm the weapon
	void Disarm();
	void Arm();
	void PlayEquipMontage(FName SectionName); // Play the animations of Equip / Unequip
	virtual void Die_Implementation() override;
	bool IsOccupied();
	bool HasEnoughStaminaToDodge();
	bool HasEnoughStaminaToSprint();

	/* Anim help functions */
	UFUNCTION(BlueprintCallable)
	void SetCanJump(bool bCan) { bCanJump = bCan; }

	UFUNCTION(BlueprintCallable)
	void FinishJumping();  // Set that the player is not in the air anymore

	UFUNCTION(BlueprintCallable)
	void AttachWeaponToBack(); // Attach the weapon to the character's BACK by pressing 'E'

	UFUNCTION(BlueprintCallable)
	void AttachWeaponToHand();

	UFUNCTION(BlueprintCallable)
	void FinishEquipping(); // After Equipped the weapon, set the ActionState to Unoccupied. Used for ENABLE Moving after equipped

	UFUNCTION(BlueprintCallable)
	void HitReactEnd();

	UPROPERTY(EditAnywhere, Category = Input)
	UInputMappingContext* SlashContext; // Create Mapping Context variable

	/** Input Action variables */	
	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* MovementAction; // Create InputAction variable for MOVING
	
	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* LookAction; 	// Create InputAction variable for LOOKING	
	
	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* JumpAction;	// Create InputAction variable for JUMP	
	
	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* EKeyAction;	// Create InputAction variable for E key	
	
	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* AttackAction;	// Create InputAction variable for Attack	

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* DodgeAction;	// Create InputAction variable for Dodge
	
	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* StartCrouchAction;	// Create InputAction variable for Start Crouch

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* StopCrouchAction;	// Create InputAction variable for Stop Crouch

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* StartSprintAction;	// Create InputAction variable for Start Sprinting
	
	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* StopSprintAction;	// Create InputAction variable for Stop Sprinting

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* StartWalkAction;	// Create InputAction variable for Start Walking

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* StopWalkAction;	// Create InputAction variable for Stop Walking
	/** /Input Action variables */

	/* Lock-On Enemy */
	UPROPERTY(EditAnywhere, Category = "Lock-On")
	UInputAction* TargetLockOnAction;	// Create InputAction variable for Target Lock

	UPROPERTY(EditAnywhere, Category = "Lock-On")
	UInputAction* ChangeTargetLockedOnAction;	// Create InputAction variable for Target Lock

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lock-On")
	bool IsLockedOn;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lock-On")
	AEnemy* EnemyLockedOn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lock-On")
	TArray<AEnemy*> LockedOnEnemiesOptions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lock-On")
	float MaxDistanceFromTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lock-On")
	float TargetCameraHeight; // The Camera height during the LockOn 
	/* /Lock-On Enemy */

	UPROPERTY(BlueprintReadOnly)
	bool bCanJump = true; // Check availablity for jumping
	
	UPROPERTY(BlueprintReadWrite)
	bool OnGround = true; // Check if the character is on ground

	UPROPERTY(BlueprintReadWrite, Category = "Movement")
	bool IsCrouching;

	UPROPERTY(BlueprintReadWrite, Category = "Movement")
	bool IsWalking;

	UPROPERTY(BlueprintReadWrite, Category = "Movement")
	bool IsSprinting;
	
	/** Adding Hair and Eyebrows variables to the Character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Hair)
	UGroomComponent* Hair;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Hair)
	UGroomComponent* Eyebrows;
	
private:
	void InitializeSlashOverlay();
	void SetHUDHealth();
	void AddEnhacedInputSystem();
	void LockOnTargetSystem();
	void StaminaManagementSystem(float DeltaTime);
	bool EnemiesOnSight();
	void CancelLockOn();
	void LockOnClosestEnemy();
	bool IsUnocuppied();
	//void CancelLockOnDeadEnemy();

	UPROPERTY(EditAnywhere)
	double LockEnemyRadius = 500.f; // The radius that the enemy can be locked-on

	/** Character Components */
	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* CameraBoom; // CameraBoom = SpringArm

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* ViewCamera;

	UPROPERTY(VisibleInstanceOnly)
	AItem* OverlappingItem;

	UPROPERTY(VisibleInstanceOnly, meta = (AllowPrivateAccess = "true"))
	ECharacterState WeaponOnBack;

	UPROPERTY(EditAnywhere, Category = Combat)
	TSubclassOf<class AWeapon> SecondWeaponClass;

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* EquipMontage;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	EActionState ActionState = EActionState::EAS_Unoccupied; // Initialize the character action state to be able to attack
	 
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	EMovementState MovementState = EMovementState::EMS_Idle;

	UPROPERTY(EditAnywhere)
	UBoxComponent* TargetBoxCollision;

	UPROPERTY()
	USlashOverlay* SlashOverlay;

public:
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
	FORCEINLINE EActionState GetActionState() const { return ActionState; }
};
