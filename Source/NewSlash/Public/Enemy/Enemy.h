#pragma once

#include "CoreMinimal.h"
#include "Characters/CharacterTypes.h"
#include "Characters/BaseCharacter.h"
#include "Items/Potions.h"
#include "Perception/PawnSensingComponent.h"
#include "Enemy.generated.h"

/* Forward declerations */
class UHealthBarComponent;
class UPawnSensingComponent;

UCLASS()
class NEWSLASH_API AEnemy : public ABaseCharacter
{
	GENERATED_BODY()

public:
	AEnemy();
	/** <AActor> */
	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Destroyed() override;
	/** </AActor> */
	
	/** <IHitInterface> */
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter); // Get Hit function - inherited from IHitInterface
	/** </IHitInterface> */
		
protected:
	/** <AActor> */
	virtual void BeginPlay() override;
	/** </AActor> */
	
	/** <ABaseCharacter> */
	virtual void Die_Implementation() override;
	void SpawnSoul();
	void SpawnPotion();
	virtual void Attack() override;
	virtual bool CanAttack() override;
	virtual void AttackEnd() override;
	virtual void HandleDamage(float DamageAmount) override;
	/** </ABaseCharacter> */

	UFUNCTION()
	void PawnSeen(APawn* SeenPawn); // Callback for OnPawnSeen in UPawnSensingComponent

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere) 
	EEnemyState EnemyState = EEnemyState::EES_Patrolling; // Initialize EnemyState Enum value 
		
private:

	/** AI Behavior */
	void InitializeEnemy();
	void CheckPatrolTarget();
	void CheckCombatTarget();
	void PatrolTimerFinished();
	void HideHealthBar();
	void ShowHealthBar();
	void LoseInterest();
	void StartPatrolling();
	void ChaseTarget();
	bool InTargetRange(AActor* Target, double Radius);
	bool IsOutsideCombatRadius();
	bool IsOutsideAttackRadius();
	bool IsInsideAttackRadius();
	bool IsChasing();
	bool IsAttacking();
	bool IsDead();
	bool IsEngaged();
	void ClearPatrolTimer();
	void StartAttackTimer();
	void ClearAttackTimer();
	void MoveToTarget(AActor* Target);
	AActor* ChoosePatrolTarget();
	void SpawnDefaultWeapon();
	
	UPROPERTY(VisibleAnywhere)
	UHealthBarComponent* HealthBarWidget;

	UPROPERTY(EditAnywhere, Category = Combat)
	TSubclassOf<class AWeapon> WeaponClass;

	UPROPERTY(EditAnywhere, Category = Combat)
	TSubclassOf<class AWeapon> SecondWeaponClass;
	
	UPROPERTY(EditAnywhere, Category = Combat)
	TSubclassOf<class AWeapon> FlamethrowerWeaponClass;

	UPROPERTY(VisibleAnywhere)
	UPawnSensingComponent* PawnSensing;

	UPROPERTY(EditAnywhere, Category = Combat)
	double AttackRadius = 150.f; // The radius that the enemy start to attack

	UPROPERTY(EditAnywhere, Category = Combat)
	double AcceptanceRadius = 50.f; // The radius that Acceptance enemy start to attack

	/* Navigation of the enemy's patrol */
	UPROPERTY()
	class AAIController* EnemyController;

	// Current patrol target
	UPROPERTY(EditInstanceOnly, Category = "AI Navigaion")
	AActor* PatrolTarget;

	// Array of targets
	UPROPERTY(EditInstanceOnly, Category = "AI Navigaion")
	TArray<AActor*> PatrolTargets;

	UPROPERTY(EditAnywhere, Category = Combat)
	double PatrolRadius = 200.f; // The radius of the Patrol Movement

	FTimerHandle PatrolTimer;

	UPROPERTY(EditInstanceOnly, Category = "AI Navigaion", BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float PatrolWaitMin = 3.f;

	UPROPERTY(EditInstanceOnly, Category = "AI Navigaion", BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float PatrolWaitMax = 7.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float PartollingSpeed = 125.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	double CombatRadius = 1000;

	FTimerHandle AttackTimer;

	UPROPERTY(EditAnywhere, Category = Combat)
	float AttackMin = 0.5;

	UPROPERTY(EditAnywhere, Category = Combat)
	float AttackMax = 1.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ChasingSpeed = 300.f;
	
	UPROPERTY(EditAnywhere, Category = Combat)
	float DeathLifeSpan = 6.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	TSubclassOf<class ASoul> SoulClass;

	UPROPERTY(EditAnywhere, Category = Combat)
	TArray<TSubclassOf<class APotions>> PotionClasses;

public:
	FORCEINLINE EEnemyState GetEnemyState() const { return EnemyState; }
};
