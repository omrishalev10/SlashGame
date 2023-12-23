#include "Items/Potions.h"
#include "Interfaces/PickupInterface.h"



void APotions::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Overlap when touch the sphere
	IPickupInterface* PickupInterface = Cast<IPickupInterface>(OtherActor);
	if (PickupInterface)
	{
		PickupInterface->UsePotion(this);
		SpawnPickupSystem();
		SpawnPickupSound();
		Destroy();
	}
}
