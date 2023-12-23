#include "Enemy/Enemy.h"
#include "AIController.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/AttributeComponent.h"
#include "HUD/HealthBarComponent.h"
#include "Items/Weapons/Weapon.h"
#include "Perception/PawnSensingComponent.h"
#include "Items/Soul.h"

AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	// Set the Mesh collision states
	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block); // Block the visiblity	
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
 	//GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap); // Overlap the pawn when there is a contact
	GetMesh()->SetGenerateOverlapEvents(true);

	/* Attributes and Health Bar */
	HealthBarWidget = CreateDefaultSubobject<UHealthBarComponent>(TEXT("Health Bar"));
	HealthBarWidget->SetupAttachment(GetRootComponent());
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	/* Pawn Sensor */
	PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensing"));
	PawnSensing->SightRadius = 4000.f;
	PawnSensing->SetPeripheralVisionAngle(45.f);
	
}
 
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsDead()) return;
	if (EnemyState > EEnemyState::EES_Patrolling)
	{
		CheckCombatTarget(); // Show/Hide the HealthBar
	}
	else
	{
		CheckPatrolTarget(); // Move the Enemy to Patrol
	}
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	/* Take damage from attack and set the health of this enemy */
	HandleDamage(DamageAmount);
	CombatTarget = EventInstigator->GetPawn(); 	// Store the attacked enemy 
	
	if (IsInsideAttackRadius())
		EnemyState = EEnemyState::EES_Attcking;
	else if(IsOutsideAttackRadius())
		ChaseTarget();
	return DamageAmount;
}

void AEnemy::Destroyed()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Destroy();
	}
}

void AEnemy::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	Super::GetHit_Implementation(ImpactPoint, Hitter);

	if(!IsDead()) ShowHealthBar();
	ClearPatrolTimer();
	ClearAttackTimer();
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);

	StopAttackMontage();
	if (IsInsideAttackRadius())
		if(!IsDead()) StartAttackTimer();
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	if (PawnSensing) 
		PawnSensing->OnSeePawn.AddDynamic(this, &AEnemy::PawnSeen);
	InitializeEnemy();
	Tags.Add(FName("Enemy"));
}

void AEnemy::Die_Implementation()
{
	Super::Die_Implementation();

	EnemyState = EEnemyState::EES_Dead;
	ClearAttackTimer();
	HideHealthBar();
	DisableCapsule();
	SetLifeSpan(DeathLifeSpan);
	GetCharacterMovement()->bOrientRotationToMovement = false;
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
	SpawnSoul();
	SpawnPotion();
}

void AEnemy::SpawnSoul()
{
	UWorld* World = GetWorld();
	if (World && SoulClass && Attributes)
	{
		FVector Location = GetActorLocation();
		Location.X += 20.f;
		ASoul* SpawnedSoul = World->SpawnActor<ASoul>(SoulClass, GetActorLocation(), GetActorRotation());
		if(SpawnedSoul)
			SpawnedSoul->SetSouls(Attributes->GetSouls());
	}
}

void AEnemy::SpawnPotion()
{
	UWorld* World = GetWorld();
	if (World && SoulClass && Attributes)
	{
		const int32 PotionIndex = FMath::RandRange(0, PotionClasses.Num() - 1); 
		APotions* SpawnedPotion = World->SpawnActor<APotions>(PotionClasses[PotionIndex], GetActorLocation(), GetActorRotation());
	}
}

void AEnemy::Attack()
{
	Super::Attack();
	if (CombatTarget == nullptr) return;
	EnemyState = EEnemyState::EES_Engaged;
	PlayLightAttackMontage();
}

bool AEnemy::CanAttack()
{
	bool bCanAttack =
		IsInsideAttackRadius() &&
		!IsAttacking() &&
		!IsEngaged() &&
		!IsDead();
	return bCanAttack;
}

void AEnemy::AttackEnd()
{
	EnemyState = EEnemyState::EES_NoState;
	CheckCombatTarget();
}

void AEnemy::HandleDamage(float DamageAmount)
{
	Super::HandleDamage(DamageAmount);
	if (Attributes && HealthBarWidget)
		HealthBarWidget->SetHealthPercent(Attributes->GetHealthPercent()); // Update the Health Bar
}

void AEnemy::InitializeEnemy()
{
	EnemyController = Cast<AAIController>(GetController());
	MoveToTarget(PatrolTarget);
	HideHealthBar();
	SpawnDefaultWeapon();
}

void AEnemy::CheckPatrolTarget()
{
	/* Check if the patrol target is in the range, and move to it with a delay between each target */
	if (InTargetRange(PatrolTarget, PatrolRadius))
	{
		PatrolTarget = ChoosePatrolTarget();
		const int32 DelayTime = FMath::RandRange(PatrolWaitMin, PatrolWaitMax);
		GetWorldTimerManager().SetTimer(PatrolTimer, this, &AEnemy::PatrolTimerFinished, DelayTime);
	}
}

void AEnemy::CheckCombatTarget()
{
	/* Check if the Enemy is out of range to Hide the Health Bar */
	if (IsOutsideCombatRadius()) // If out of range, end the chasing and return to PATROL 
	{
		ClearPatrolTimer();
		LoseInterest();
		if (!IsEngaged()) StartPatrolling();
	}
	else if (IsOutsideAttackRadius() && !IsChasing())
	{
		ClearPatrolTimer();
		if (!IsEngaged()) ChaseTarget();
	}
	else if (CanAttack())
	{
		StartAttackTimer(); // start attacking
	}
}

void AEnemy::PatrolTimerFinished()
{
	// Used after the delay timer is over
	MoveToTarget(PatrolTarget);
}

void AEnemy::HideHealthBar()
{
	if (HealthBarWidget) HealthBarWidget->SetVisibility(false);
	
}

void AEnemy::ShowHealthBar()
{
	if (HealthBarWidget) HealthBarWidget->SetVisibility(true);
}

void AEnemy::LoseInterest()
{
	CombatTarget = nullptr;
	HideHealthBar();
}

void AEnemy::StartPatrolling()
{
	EnemyState = EEnemyState::EES_Patrolling;
	GetCharacterMovement()->MaxWalkSpeed = PartollingSpeed;
	MoveToTarget(PatrolTarget);
}

void AEnemy::ChaseTarget()
{
	EnemyState = EEnemyState::EES_Chasing;
	GetCharacterMovement()->MaxWalkSpeed = ChasingSpeed;
	MoveToTarget(CombatTarget);
}

bool AEnemy::InTargetRange(AActor* Target, double Radius)
{
	/* Check if the Target is in the range of the radius */
	if (Target == nullptr) return false;
	const double DistanceToTarget = (Target->GetActorLocation() - GetActorLocation()).Size();
	return DistanceToTarget <= Radius;
}

bool AEnemy::IsOutsideCombatRadius()
{
	return !InTargetRange(CombatTarget, CombatRadius);
}

bool AEnemy::IsOutsideAttackRadius()
{
	return !InTargetRange(CombatTarget, AttackRadius);
}

bool AEnemy::IsInsideAttackRadius()
{
	return InTargetRange(CombatTarget, AttackRadius);
}

bool AEnemy::IsChasing()
{
	return EnemyState == EEnemyState::EES_Chasing;
}

bool AEnemy::IsAttacking()
{
	return EnemyState == EEnemyState::EES_Attcking;
}

bool AEnemy::IsDead()
{
	return EnemyState == EEnemyState::EES_Dead;
}

bool AEnemy::IsEngaged()
{
	return EnemyState == EEnemyState::EES_Engaged;
}

void AEnemy::ClearPatrolTimer()
{
	GetWorldTimerManager().ClearTimer(PatrolTimer);
}

void AEnemy::StartAttackTimer()
{
	EnemyState = EEnemyState::EES_Attcking;
	const float AttackTime = FMath::RandRange(AttackMin, AttackMax);
	GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime); // Setup timer fot Attack function
}

void AEnemy::ClearAttackTimer()
{
	GetWorldTimerManager().ClearTimer(AttackTimer);
}

void AEnemy::MoveToTarget(AActor* Target)
{
	/* Move the Enemy to the Traget Point */
	if (EnemyController == nullptr || Target == nullptr) return;
	/* Create a request to move to the patrol target object in a specific radius */
	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(Target);
	MoveRequest.SetAcceptanceRadius(AcceptanceRadius);
	EnemyController->MoveTo(MoveRequest);
}

AActor* AEnemy::ChoosePatrolTarget()
{
	/* Return a random chosen patrol target */
	TArray<AActor*> ValidTargets;
	for (AActor* Target : PatrolTargets)
	{
		if (Target != PatrolTarget)
		{
			ValidTargets.AddUnique(Target);
		}
	}

	const int32 NumPatrolTargets = ValidTargets.Num();
	if (NumPatrolTargets > 0)
	{
		const int32 Selection = FMath::RandRange(0, NumPatrolTargets - 1);
		return ValidTargets[Selection];	
	}
	return nullptr;
}

void AEnemy::SpawnDefaultWeapon()
{
	UWorld* World = GetWorld();
	if (World && WeaponClass)
	{
		AWeapon* DefaultWeapon = World->SpawnActor<AWeapon>(WeaponClass); // SpawnActor is function of the World
		DefaultWeapon->Equip(GetMesh(), FName("WeaponSocket"), this, this); // Spawn a weapon in the Right Hand of the Enemy
		EquippedWeapon = DefaultWeapon;
		CharacterState = EquippedWeapon->WeaponType; // Set the weapon type the enemy holding
		if (SecondWeaponClass)
		{
			AWeapon* SecondDefaultWeapon = World->SpawnActor<AWeapon>(SecondWeaponClass);
			SecondDefaultWeapon->Equip(GetMesh(), FName("SecondWeaponSocket"), this, this);
			EquippedSecondWeapon = SecondDefaultWeapon;
		}
		if (FlamethrowerWeaponClass)
		{
			AWeapon* FlamethrowerWeapon = World->SpawnActor<AWeapon>(FlamethrowerWeaponClass);
			FlamethrowerWeapon->Equip(GetMesh(), FName("FlamethrowerSocket"), this, this);
			EquippedFlamethrowerWeapon = FlamethrowerWeapon;
		}
	}
}

void AEnemy::PawnSeen(APawn* SeenPawn)
{
	// Check all conditions at once
	bool bShouldChaseTarget =
		EnemyState != EEnemyState::EES_Dead &&
		EnemyState != EEnemyState::EES_Chasing &&
		EnemyState < EEnemyState::EES_Attcking &&
		SeenPawn->ActorHasTag(FName("EngageableTarget"));

	if (bShouldChaseTarget && !SeenPawn->ActorHasTag("Dead"))
	{
		CombatTarget = SeenPawn; // Change the enemy's target to the Slash Character 
		ClearPatrolTimer();
		ChaseTarget();
	}	
}	