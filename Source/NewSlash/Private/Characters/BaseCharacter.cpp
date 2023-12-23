#include "Characters/BaseCharacter.h"
#include "Components/BoxComponent.h"
#include "Items/Weapons/Weapon.h"
#include "Components/AttributeComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	Attributes = CreateDefaultSubobject<UAttributeComponent>(TEXT("Attributes"));
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABaseCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	if (IsAlive() && Hitter) DirectionalHitReact(Hitter->GetActorLocation());
	else Die();

	PlayHitSound(ImpactPoint);
	SpawnHitParticles(ImpactPoint);	
}

void ABaseCharacter::Attack()
{
	/* Stop the enemies attacks if the main character is dead */
	if (CombatTarget && CombatTarget->ActorHasTag(FName("Dead")))
		CombatTarget = nullptr;
}

void ABaseCharacter::Die_Implementation()
{
	Tags.Add(FName("Dead")); 
	PlayDeathMontage();
}

void ABaseCharacter::DisableCapsule()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABaseCharacter::DirectionalHitReact(const FVector& ImpactPoint)
{
	// The enemy forward vector - to know his direction
	const FVector Forward = GetActorForwardVector();

	// Lower Impact Point to the Enemy's Actor Location Z 
	const FVector ImpactLowered(ImpactPoint.X, ImpactPoint.Y, GetActorLocation().Z);

	// Normalize the vector ToHit - from where got the hit
	const FVector ToHit = (ImpactLowered - GetActorLocation()).GetSafeNormal();

	/*  Forward* ToHit = | Forward || ToHit | *cos(theta)
	|Forward| = 1, |ToHit| = 1, so Forward * ToHit = cos(theta) */
	const double CosTheta = FVector::DotProduct(Forward, ToHit);

	// Take the inverse cos (arc-cos) of cos(theta) to get theta
	double Theta = FMath::Acos(CosTheta);

	// Convert from radians to degrees
	Theta = FMath::RadiansToDegrees(Theta);

	/* If CrossProduct points down, Theta should be negative */
	const FVector CrossProduct = FVector::CrossProduct(Forward, ToHit);
	if (CrossProduct.Z < 0)
	{
		Theta *= -1.f;
	}

	FName Section("FromBack");
	// Check from where the attack came from
	if (Theta >= -45.f && Theta < 45.f)
	{
		Section = FName("FromFront");
	}
	else if (Theta >= -135.f && Theta < 45.f)
	{
		Section = FName("FromLeft");
	}
	else if (Theta >= 45.f && Theta < 135.f)
	{
		Section = FName("FromRight");
	}
	PlayHitReactMontage(FName(Section));
}

void ABaseCharacter::PlayHitSound(const FVector& ImpactPoint)
{
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, ImpactPoint); // Hit sound  
	}
}

void ABaseCharacter::SpawnHitParticles(const FVector& ImpactPoint)
{
	if (HitParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticles, ImpactPoint);
	}
}

void ABaseCharacter::HandleDamage(float DamageAmount)
{
	if (Attributes)
	{
		Attributes->ReceiveDamage(DamageAmount);
	}
}

void ABaseCharacter::PlayMontageSection(UAnimMontage* Montage, const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && Montage)
	{
		AnimInstance->Montage_Play(Montage);
	}
	// Send the name of the chosen attack to the montage 
	AnimInstance->Montage_JumpToSection(SectionName, Montage);
}

int32 ABaseCharacter::PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames)
{
	if (SectionNames.Num() <= 0) return -1;
	const int32 MaxSectionIndex = SectionNames.Num() - 1;
	const int32 Selection = FMath::RandRange(0, MaxSectionIndex);
	PlayMontageSection(Montage, SectionNames[Selection]);
	return Selection;
}

int32 ABaseCharacter::PlayLightAttackMontage()
{
	return PlayRandomMontageSection(LightAttackMontage, LightAttackMontageSections);
}

int32 ABaseCharacter::PlayHeavyAttackMontage()
{
	return PlayRandomMontageSection(HeavyAttackMontage, HeavyAttackMontageSections);
}

int32 ABaseCharacter::PlayUnequipAttackMontage()
{
	return PlayRandomMontageSection(UnequipAttackMontage, UnequippedAttackMontageSections);
}

int32 ABaseCharacter::PlayJumpAttackMontage()
{
	return PlayRandomMontageSection(JumpAttackMontage, JumpAttackMontageSections);
}

void ABaseCharacter::PlayDodgeMontage()
{
	PlayMontageSection(DodgeMontage, FName("Default")); // Got just the default section in the montage
}

void ABaseCharacter::StopAttackMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		if (CharacterState == ECharacterState::ECS_LightWeapon) 
			AnimInstance->Montage_Stop(0.25, LightAttackMontage);
		if (CharacterState == ECharacterState::ECS_HeavyWeapon)
			AnimInstance->Montage_Stop(0.25, LightAttackMontage);
	}
}

FVector ABaseCharacter::GetTranslationWarpTarget()
{
	if(CombatTarget == nullptr) return FVector();
	
	const FVector CombatTargetLocation = CombatTarget->GetActorLocation();
	const FVector EnemyLocation = GetActorLocation();

	FVector TargetToMe = (EnemyLocation - CombatTargetLocation).GetSafeNormal();
	TargetToMe *= WarpTargetDistance;

	return CombatTargetLocation + TargetToMe;
}

FVector ABaseCharacter::GetRotationWarpTarget()
{
	if (CombatTarget)
	{
		return CombatTarget->GetActorLocation();
	}
	return FVector();
}

void ABaseCharacter::PlayHitReactMontage(const FName& SectionName)
{
	/* Play Get Hit Montage */
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		AnimInstance->Montage_JumpToSection(SectionName, HitReactMontage);
	}
}

int32 ABaseCharacter::PlayDeathMontage()
{
	const int32 Selection = PlayRandomMontageSection(DeathMontage, DeathMontageSections);
	TEnumAsByte<EDeathPose> Pose(Selection); // Initiallize the Enum costructor with the selected pose
	if (Pose < EDeathPose::EDP_MAX)
		DeathPose = Pose;
	return Selection;
}

bool ABaseCharacter::CanAttack()
{
	return false;
}

bool ABaseCharacter::IsAlive()
{
	return Attributes && Attributes->IsAlive();
}

void ABaseCharacter::DisableMeshCollision()
{
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABaseCharacter::AttackEnd()
{
}

void ABaseCharacter::DodgeEnd()
{
}

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABaseCharacter::SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled)
{
	if (EquippedWeapon && EquippedWeapon->GetWeaponBox())
	{
		// Get the WeaponBox and set its collision
		EquippedWeapon->GetWeaponBox()->SetCollisionEnabled(CollisionEnabled);
		EquippedWeapon->IgnoreActors.Empty(); // after attack finished we can hit again

		/* If got secondary weapon */
		if (EquippedSecondWeapon && EquippedSecondWeapon->GetWeaponBox())
		{
			EquippedSecondWeapon->GetWeaponBox()->SetCollisionEnabled(CollisionEnabled);
			EquippedSecondWeapon->IgnoreActors.Empty(); // after attack finished we can hit again
		}
	}
}