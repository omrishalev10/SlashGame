// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Item.h"
#include "Characters/CharacterTypes.h"
#include "Potions.generated.h"

/**
 * 
 */
UCLASS()
class NEWSLASH_API APotions : public AItem
{
	GENERATED_BODY()

protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:
	UPROPERTY(EditAnywhere, Category = "Potions Properties")
	float HealthPotion;

	UPROPERTY(EditAnywhere, Category = "Potions Properties")
	float StaminaPotion;

	UPROPERTY(EditAnywhere, Category = "Potions Properties")
	EPotionType PotionType;

public:
	FORCEINLINE float GetHealthPotion() const { return HealthPotion; }
	FORCEINLINE float GetStaminaPotion() { return StaminaPotion; }
	FORCEINLINE EPotionType GetPotionType() { return PotionType; }
	
};
