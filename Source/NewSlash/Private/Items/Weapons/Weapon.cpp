#include "Items/Weapons/Weapon.h"
#include "Characters//SlashCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Interfaces/HitInterface.h"
#include "NiagaraComponent.h"

// Constructor
AWeapon::AWeapon()
{
	// Make a Box collision for the weapons
	WeaponBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Weapon Box"));
	WeaponBox->SetupAttachment(GetRootComponent());

	// Set the Collision Presets
	WeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Set no Collision
	WeaponBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap); // Collision Responses : Overlap for all chanells	
	WeaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore); // Ignore just the PAWN Response	

	BoxTraceStart = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace Start"));
	BoxTraceStart->SetupAttachment(GetRootComponent());

	BoxTraceEnd = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace End"));
	BoxTraceEnd->SetupAttachment(GetRootComponent());
}

void AWeapon::Equip(USceneComponent* InParent, FName InSocketName, AActor* NewOwner, APawn* NewInstigator)
{
	ItemState = EItemState::EIS_Equipped;
	SetOwner(NewOwner);
	SetInstigator(NewInstigator);
	AttachMeshToSocket(InParent, InSocketName);
	DisableSphereCollision();
	PlayEquipSound();
	DeactivateEmbers();
}

void AWeapon::DeactivateEmbers()
{
	if (ItemEffect) ItemEffect->Deactivate();
}

void AWeapon::DisableSphereCollision()
{
	if (Sphere)
		Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWeapon::PlayEquipSound()	
{
	if (EquipSound)
		UGameplayStatics::PlaySoundAtLocation(this, EquipSound, GetActorLocation());
}

void AWeapon::AttachMeshToSocket(USceneComponent* InParent, const FName& InSocketName)
{
	FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, true);
	ItemMesh->AttachToComponent(InParent, TransformRules, InSocketName);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	// Add box overlap to the weapon
	WeaponBox->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnBoxOverlap);
}

void AWeapon::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ActorIsSameType(OtherActor)) return;
	FHitResult BoxHit; 
	BoxTrace(BoxHit);
	if (BoxHit.GetActor()) // If an actor got a hit
	{
		if (ActorIsSameType(BoxHit.GetActor())) return;
		/* ApplyDamage is a function that makes the damage from the weapon */
		UGameplayStatics::ApplyDamage(BoxHit.GetActor(), Damage, GetInstigator()->GetController(), this, UDamageType::StaticClass());
		ExecuteGetHit(BoxHit); // Implement the Interface
		CreateFields(BoxHit.ImpactPoint);
	}
}

bool AWeapon::ActorIsSameType(AActor* OtherActor)
{
	return GetOwner()->ActorHasTag(TEXT("Enemy")) && OtherActor->ActorHasTag(TEXT("Enemy"));
}

void AWeapon::ExecuteGetHit(FHitResult& BoxHit)
{
	IHitInterface* HitInterface = Cast<IHitInterface>(BoxHit.GetActor()); // Cast the BoxHit.GetActor() to IHitInterface to use GetHit() function
	if (HitInterface)
		HitInterface->Execute_GetHit(BoxHit.GetActor(), BoxHit.ImpactPoint, GetOwner());
}

void AWeapon::BoxTrace(FHitResult& BoxHit)
{
	const FVector Start = BoxTraceStart->GetComponentLocation();
	const FVector End = BoxTraceEnd->GetComponentLocation();
	
	TArray<AActor*> ActorsToIgnore; // array of actors to ignore - for the BoxTraceSingle function
	ActorsToIgnore.Add(this); // dont get hit from the weapon itself
	ActorsToIgnore.Add(GetOwner()); // dont get hit by my self 

	for (AActor* Actor : IgnoreActors) 
		ActorsToIgnore.AddUnique(Actor);

	UKismetSystemLibrary::BoxTraceSingle(
		this,
		Start,
		End,
		BoxTraceExtent,
		BoxTraceStart->GetComponentRotation(),
		ETraceTypeQuery::TraceTypeQuery1,
		false, ActorsToIgnore,
		bShowBoxDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
		BoxHit,
		true
	);
	IgnoreActors.AddUnique(BoxHit.GetActor()); // After hit once, ignore the actor to avoid multiple hits in one attack
}		