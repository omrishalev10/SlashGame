#include "Characters/SlashCharacter.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GroomComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/AttributeComponent.h"
#include "Items/Item.h"
#include "Items/Weapons/Weapon.h"
#include "Items/Soul.h"
#include "Items/Potions.h"
#include "Items/Treasure.h"
#include "Animation/AnimMontage.h"
#include "Kismet/KismetMathLibrary.h"
#include "HUD/SlashHUD.h"
#include "HUD/SlashOverlay.h"
#include "Enemy/Enemy.h"

ASlashCharacter::ASlashCharacter() 
{
	PrimaryActorTick.bCanEverTick = true;
	
	// Set the Controller Rotations
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Set the player Rotation movement (Change the front of the character) 
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f,400.f, 0.f); // the pace of this movement on YAW
	
	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannels(ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);	
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);

	// Create a SpringArm for the Camera that we will make later
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 300.f;

	// Create the Camera that follows the Pawn
	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(CameraBoom);

	// Make this Pawn as the main player
	AutoPossessPlayer = EAutoReceiveInput::Player0; // Determind the player controller 

	// Create a subobject of the hair and attach it to the mesh(player)
	Hair = CreateDefaultSubobject<UGroomComponent>(TEXT("Hair"));
	Hair->SetupAttachment(GetMesh());	
	Hair->AttachmentName = FString("head"); 
	
	// Create a subobject of the Eyebrows and attach it to the mesh(player)
	Eyebrows = CreateDefaultSubobject<UGroomComponent>(TEXT("Eyebrows"));
	Eyebrows->SetupAttachment(GetMesh());
	Eyebrows->AttachmentName = FString("head");

	CharacterState = ECharacterState::ECS_Unequipped;

	// Movement states parameters
	IsCrouching = false;
	IsWalking = false;
	IsSprinting = false;

	// Lock-On initialize properties
	IsLockedOn = false;
	TargetCameraHeight = 30.f;
	EnemyLockedOn = nullptr;
	MaxDistanceFromTarget = 2000.f;
	TargetBoxCollision = CreateDefaultSubobject<UBoxComponent>("TargetBoxCollision");
	TargetBoxCollision->SetupAttachment(GetRootComponent());

}

void ASlashCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Bind the Enhanced functions to the EnhancedInputComponent with a trigger
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Jump);
		EnhancedInputComponent->BindAction(EKeyAction, ETriggerEvent::Triggered, this, &ASlashCharacter::EKeyPressed);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Attack);
		EnhancedInputComponent->BindAction(TargetLockOnAction, ETriggerEvent::Triggered, this, &ASlashCharacter::TargetLockOn);
		EnhancedInputComponent->BindAction(ChangeTargetLockedOnAction, ETriggerEvent::Triggered, this, &ASlashCharacter::ChangeTargetLockedOn);
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Dodge);
		EnhancedInputComponent->BindAction(StartCrouchAction, ETriggerEvent::Triggered, this, &ASlashCharacter::StartCrouching);
		EnhancedInputComponent->BindAction(StopCrouchAction, ETriggerEvent::Triggered, this, &ASlashCharacter::StopCrouching);
		EnhancedInputComponent->BindAction(StartSprintAction, ETriggerEvent::Triggered, this, &ASlashCharacter::StartSprinting);
		EnhancedInputComponent->BindAction(StopSprintAction, ETriggerEvent::Triggered, this, &ASlashCharacter::StopSprinting);
		EnhancedInputComponent->BindAction(StartWalkAction, ETriggerEvent::Triggered, this, &ASlashCharacter::StartWalking);
		EnhancedInputComponent->BindAction(StopWalkAction, ETriggerEvent::Triggered, this, &ASlashCharacter::StopWalking);
	}
}

float ASlashCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	HandleDamage(DamageAmount);
	SetHUDHealth();
	return DamageAmount;
}

void ASlashCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	Super::GetHit_Implementation(ImpactPoint, Hitter);
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
	if (Attributes && Attributes->GetHealthPercent() > 0.f)
		ActionState = EActionState::EAS_HitReaction;
}

void ASlashCharacter::SetOverlappingItem(AItem* Item)
{
	OverlappingItem = Item;
}

void ASlashCharacter::AddSouls(ASoul* Soul)
{
	// Add picked-up Souls to the attributes and update the HUD
	if (Attributes && SlashOverlay)
	{
		Attributes->AddSouls(Soul->GetSouls());
		SlashOverlay->SetSouls(Attributes->GetSouls());
	}
}

void ASlashCharacter::AddGold(ATreasure* Treasure)
{
	// Add picked-up Gold to the attributes and update the HUD
	if (Treasure && SlashOverlay)
	{
		Attributes->AddGold(Treasure->GetGold());
		SlashOverlay->SetGold(Attributes->GetGold());
	}
}

void ASlashCharacter::UsePotion(APotions* Potion)
{
	if (Potion && Attributes && SlashOverlay)
	{
		if (Potion->GetPotionType() == EPotionType::EPT_HealthPotion)
		{
			Attributes->AddHealth(Potion->GetHealthPotion());
			SlashOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
		}
		else if (Potion->GetPotionType() == EPotionType::EPT_StaminaPotion)
		{
			Attributes->AddStamina(Potion->GetStaminaPotion());
			SlashOverlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());
		}
		else if (Potion->GetPotionType() == EPotionType::EPT_FullAttributesPotion)
		{
			Attributes->AddHealth(Potion->GetHealthPotion());
			Attributes->AddStamina(Potion->GetStaminaPotion());
			SlashOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
			SlashOverlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());
		}
	}
}

void ASlashCharacter::Jump()
{ 
	//Jump function
	if (IsUnocuppied() && bCanJump)
	{
		Super::Jump();
		bCanJump = false;
		OnGround = false;
	}
}

void ASlashCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	Tags.Add(FName("EngageableTarget")); // Add a TAG name for this class
	Tags.Add(FName("Collider")); // Add a TAG name to be able get hit by Flamethrower
	AddEnhacedInputSystem();
	InitializeSlashOverlay();	
}

void ASlashCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	LockOnTargetSystem();
	//CancelLockOnDeadEnemy();
	StaminaManagementSystem(DeltaTime);
}

void ASlashCharacter::Move(const FInputActionValue& Value)
{	
	// Move the character function (WSDA buttons)
	if (ActionState != EActionState::EAS_Unoccupied) return;
	const FVector2D MovementVecor = Value.Get<FVector2D>();
	// Find out which direction to move
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	// Moving to that direction
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(ForwardDirection, MovementVecor.Y);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(RightDirection, MovementVecor.X);
	bCanJump = true;

	if(!IsSprinting && !IsCrouching && !IsWalking)
		MovementState = EMovementState::EMS_Running;
}
	
void ASlashCharacter::Look(const FInputActionValue& Value)
{
	// Look function - looking by moving the mouse 
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (GetController())
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ASlashCharacter::EKeyPressed()
{
	/* 
	* Actions that made by pressing E key
	* Pickup weapon
	* Equip the weapon
	* Unequip the weapon
	*/
	AWeapon* OverlappingWeapon = Cast<AWeapon>(OverlappingItem);
	if (OverlappingWeapon) // if the overlapping item is a weapon so by pressing E we attach the weapon to the character
	{	
		if (OverlappingWeapon->WeaponType == ECharacterState::ECS_LightWeapon)
			EquipWeapon(OverlappingWeapon, ECharacterState::ECS_LightWeapon);
		else if (OverlappingWeapon->WeaponType == ECharacterState::ECS_HeavyWeapon)
			EquipWeapon(OverlappingWeapon, ECharacterState::ECS_HeavyWeapon);	
	}
	// If the character isn't attacking and is equipped with a weapon
	else 
	{
		if (CanDisarm())
			Disarm();
		else if(CanArm())
			Arm();
	}
}

void ASlashCharacter::Attack()
{
	// Attack function
	Super::Attack();
	if (CanAttack())
	{
		if (CharacterState == ECharacterState::ECS_LightWeapon) 
		{
			if (!OnGround)
			{
				PlayJumpAttackMontage();
				OnGround = true;
			}
			else PlayLightAttackMontage();	
		}
		else if (CharacterState == ECharacterState::ECS_HeavyWeapon)
		{
			if (!OnGround)
			{
				PlayJumpAttackMontage();
				OnGround = true;
			}
			else PlayHeavyAttackMontage();
		}
		else if (CharacterState == ECharacterState::ECS_Unequipped) {
			PlayUnequipAttackMontage();
		}
		ActionState = EActionState::EAS_Attacking; // change the state to be attacking
	}
}

void ASlashCharacter::StartCrouching()
{
	if (IsSprinting) StopSprinting();
	IsCrouching = true;
	GetCharacterMovement()->MaxWalkSpeed = 300.f;
	MovementState = EMovementState::EMS_Crouching;
}

void ASlashCharacter::StopCrouching()
{
	IsCrouching = false;
	GetCharacterMovement()->MaxWalkSpeed = 600.f;
}

void ASlashCharacter::StartSprinting()
{
	if(!HasEnoughStaminaToSprint()) return;
	if (IsCrouching) IsCrouching = false; // If crouching so stop crouch and sprint
	IsSprinting = true;
	GetCharacterMovement()->MaxWalkSpeed = 900;
	MovementState = EMovementState::EMS_Sprinting;
	if (Attributes && SlashOverlay)
	{
		Attributes->UseStamina(Attributes->GetSprintCost());
		SlashOverlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());
	}
}

void ASlashCharacter::StopSprinting()
{
	MovementState = EMovementState::EMS_Running;
	IsSprinting = false;
	GetCharacterMovement()->MaxWalkSpeed = 600;
}

void ASlashCharacter::StartWalking()
{
	MovementState = EMovementState::EMS_Walking;
	IsWalking = true;
	GetCharacterMovement()->MaxWalkSpeed = 200;
}

void ASlashCharacter::StopWalking()
{
	MovementState = EMovementState::EMS_Running;
	IsWalking = false;
	GetCharacterMovement()->MaxWalkSpeed = 600;
}

void ASlashCharacter::TargetLockOn()
{
	if (IsLockedOn)	CancelLockOn();
	else	
		if (EnemiesOnSight()) LockOnClosestEnemy();	
}

void ASlashCharacter::ChangeTargetLockedOn()
{
	for (int EnemyIndex = 0; EnemyIndex < LockedOnEnemiesOptions.Num(); EnemyIndex++)
	{
		if(EnemyLockedOn == LockedOnEnemiesOptions[EnemyIndex])
		{
			if (EnemyIndex >= LockedOnEnemiesOptions.Num() - 1)
			{
				EnemyLockedOn = LockedOnEnemiesOptions[0];
				break;
			}
			else
			{
				EnemyLockedOn = LockedOnEnemiesOptions[EnemyIndex + 1];
				break;
			}
		}
	}
}

void ASlashCharacter::Dodge()
{
	if (IsOccupied() || !HasEnoughStaminaToDodge()) return;
	PlayDodgeMontage();
	ActionState = EActionState::EAS_Dodge;
	if (Attributes && SlashOverlay)
	{
		Attributes->UseStamina(Attributes->GetDodgeCost());	
		SlashOverlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());
	}
}

void ASlashCharacter::EquipWeapon(AWeapon* Weapon, ECharacterState WeaponType)
{
	Weapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
	CharacterState = WeaponType;
	OverlappingItem = nullptr;
	EquippedWeapon = Weapon;

	UWorld* World = GetWorld();
	if (World && SecondWeaponClass)
	{
		AWeapon* DefaultWeapon = World->SpawnActor<AWeapon>(SecondWeaponClass);
		DefaultWeapon->Equip(GetMesh(), FName("SecondWeaponSocket"), this, this);
		EquippedSecondWeapon = DefaultWeapon;
	}
}

void ASlashCharacter::AttackEnd()
{
	ActionState = EActionState::EAS_Unoccupied; // Change the state of the character
}

void ASlashCharacter::DodgeEnd()
{
	Super::DodgeEnd();

	ActionState = EActionState::EAS_Unoccupied;
}

bool ASlashCharacter::CanAttack()
{
	return ActionState == EActionState::EAS_Unoccupied; // Check if the character can attack now
}

bool ASlashCharacter::CanDisarm()
{
	// Check if the character can disarm the weapon now
	return ActionState ==  EActionState::EAS_Unoccupied && CharacterState != ECharacterState::ECS_Unequipped;
}

bool ASlashCharacter::CanArm()
{
	return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState == ECharacterState::ECS_Unequipped &&
		EquippedWeapon;
}

void ASlashCharacter::Disarm()
{
	PlayEquipMontage(FName("Unequip"));
	WeaponOnBack = CharacterState;
	CharacterState = ECharacterState::ECS_Unequipped;
	ActionState = EActionState::EAS_EquippingWeapon;
}

void ASlashCharacter::Arm()
{
	PlayEquipMontage(FName("Equip"));
	CharacterState = WeaponOnBack;
	ActionState = EActionState::EAS_EquippingWeapon;
}

void ASlashCharacter::PlayEquipMontage(FName SectionName)
{
	// Play the Equip weapon montage
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && EquipMontage)
	{
		AnimInstance->Montage_Play(EquipMontage); 
		AnimInstance->Montage_JumpToSection(SectionName, EquipMontage);
	}
}

void ASlashCharacter::FinishJumping()
{
	OnGround = true; // Set that the player is on the ground after jumping
}

void ASlashCharacter::Die_Implementation()
{
	Super::Die_Implementation();
	ActionState = EActionState::EAS_Dead;
	DisableMeshCollision();
	CancelLockOn();
}

bool ASlashCharacter::IsOccupied()
{
	return ActionState != EActionState::EAS_Unoccupied;
}

bool ASlashCharacter::HasEnoughStaminaToDodge()
{
	return Attributes && Attributes->GetStamina() > Attributes->GetDodgeCost();
}

bool ASlashCharacter::HasEnoughStaminaToSprint()
{
	return Attributes && Attributes->GetStamina() > Attributes->GetSprintCost();;
}

void ASlashCharacter::AttachWeaponToBack()
{
	// Attach the Weapon Mesh to the charcter's back
	if (EquippedWeapon)
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("SpineSocket")); 
}

void ASlashCharacter::AttachWeaponToHand()
{
	// Attach the Weapon from the BACK to the HAND
	if (EquippedWeapon)
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("RightHandSocket"));
}

void ASlashCharacter::FinishEquipping()
{
	ActionState = EActionState::EAS_Unoccupied; // Set the Action State to Unoccupied after equipped the weapon
}

void ASlashCharacter::HitReactEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void ASlashCharacter::InitializeSlashOverlay()
{
	APlayerController* PlayerConroller = Cast<APlayerController>(GetController());
	if (PlayerConroller)
	{
		ASlashHUD* SlashHUD = Cast<ASlashHUD>(PlayerConroller->GetHUD());
		if (SlashHUD)
		{
			SlashOverlay = SlashHUD->GetSlashOverlay();
			if (SlashOverlay && Attributes)
			{
				SlashOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
				SlashOverlay->SetStaminaBarPercent(1.f);
				SlashOverlay->SetGold(0);
				SlashOverlay->SetSouls(0);
			}
		}
	}
}

void ASlashCharacter::SetHUDHealth()
{
	if (SlashOverlay && Attributes)
		SlashOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
}

void ASlashCharacter::AddEnhacedInputSystem()
{
	// Get the player controller in a pointer and cast it to the correct object.
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		// Create a Subsysyem
		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem< UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		if (Subsystem)
		{
			Subsystem->AddMappingContext(SlashContext, 0); // Add the mapping context to the subsystem
		}
	}
}

void ASlashCharacter::LockOnTargetSystem()
{
	if (IsLockedOn)
	{
		if (GetDistanceTo(EnemyLockedOn) <= MaxDistanceFromTarget)
		{
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), EnemyLockedOn->GetActorLocation());
			LookAtRotation.Pitch -= TargetCameraHeight;
			GetController()->SetControlRotation(LookAtRotation);
		}
		else TargetLockOn(); // Disable the Lock-On function if run away
	}
}

void ASlashCharacter::StaminaManagementSystem(float DeltaTime)
{
	if (Attributes && SlashOverlay && MovementState != EMovementState::EMS_Sprinting)
	{
		Attributes->RegeneStamina(DeltaTime);
		SlashOverlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());
	}
	if (!HasEnoughStaminaToSprint() && MovementState == EMovementState::EMS_Sprinting)
	{
		StopSprinting();
	}
}

bool ASlashCharacter::EnemiesOnSight()
{
	return LockedOnEnemiesOptions.Num() > 0;
}

void ASlashCharacter::CancelLockOn()
{
	IsLockedOn = false;
	EnemyLockedOn = nullptr;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
}

void ASlashCharacter::LockOnClosestEnemy()
{
	AEnemy* ClosestEnemy = nullptr;
	ClosestEnemy = LockedOnEnemiesOptions[0];
	for (AEnemy* Enemy : LockedOnEnemiesOptions)
		if (GetDistanceTo(Enemy) < GetDistanceTo(ClosestEnemy))
			ClosestEnemy = Enemy;
	EnemyLockedOn = ClosestEnemy;
	if (EnemyLockedOn) 
	{
		IsLockedOn = true;
		bUseControllerRotationYaw = true;
		GetCharacterMovement()->bOrientRotationToMovement = false;
	}
		
	/*const int32 EnemyIndex = FMath::RandRange(0, LockedOnEnemiesOptions.Num() - 1);
	EnemyLockedOn = LockedOnEnemiesOptions[EnemyIndex];
	if (EnemyLockedOn) IsLockedOn = true;*/
}

bool ASlashCharacter::IsUnocuppied()
{
	return ActionState == EActionState::EAS_Unoccupied;
}

//void ASlashCharacter::CancelLockOnDeadEnemy(){
//	if (EnemyLockedOn->GetEnemyState() == EEnemyState::EES_Dead)
//	{
//		if (LockedOnEnemiesOptions.Num() > 0)
//			ChangeTargetLockedOn();
//		else
//			CancelLockOn();
//	}
//}
