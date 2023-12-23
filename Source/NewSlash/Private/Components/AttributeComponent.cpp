#include "Components/AttributeComponent.h"

UAttributeComponent::UAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAttributeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UAttributeComponent::RegeneStamina(float DeltaTime)
{
	Stamina = FMath::Clamp(Stamina + StaminaRegeneRate * DeltaTime, 0.f, MaxStamina);
}

void UAttributeComponent::BeginPlay()
{
	Super::BeginPlay();
}
	
void UAttributeComponent::ReceiveDamage(float Damage)
{
	/* The damage the character got */
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth); 
}

void UAttributeComponent::UseStamina(float StaminaCost)
{
	/* The Stamina Cost the character use */
	Stamina = FMath::Clamp(Stamina - StaminaCost, 0.f, MaxStamina);
}

float UAttributeComponent::GetHealthPercent()
{
	/* Get the percentage of the health */
	return Health / MaxHealth;
}

float UAttributeComponent::GetStaminaPercent()
{
	return Stamina / MaxStamina;
}

bool UAttributeComponent::IsAlive()
{
	return Health > 0.f;
}

void UAttributeComponent::AddSouls(int32 NumberOfSouls)
{
	Souls += NumberOfSouls;
}

void UAttributeComponent::AddGold(int32 AmountOfGold)
{
	Gold += AmountOfGold;
}

void UAttributeComponent::AddHealth(float AmountOfHealth)
{
	Health = FMath::Clamp(Health + AmountOfHealth, 0.f, MaxHealth);;
}

void UAttributeComponent::AddStamina(float AmountOfStamina)
{
	Stamina = FMath::Clamp(Stamina + AmountOfStamina, 0.f, MaxStamina);
}


