#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CharacterTypes.h"
#include "SlashAnimInstance.generated.h"
/**
 * 
 */
UCLASS()
class NEWSLASH_API USlashAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	// These 2 functions are events 
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;

	// Create forward declerations pointers
	UPROPERTY(BlueprintReadOnly) // show SlashCharacter variable in event graph
	class ASlashCharacter* SlashCharacter;

	UPROPERTY(BlueprintReadOnly, Category = Movement) // show SlashCharacterMovement variable in event graph and details panel
	class UCharacterMovementComponent* SlashCharacterMovement;

	UPROPERTY(BlueprintReadOnly, Category = Movement) // show GroundSpeed variable in event graph and details panel
	float GroundSpeed; // Used for speed running

	UPROPERTY(BlueprintReadOnly, Category = Movement) // show IsFalling variable in event graph and details panel
	bool IsFalling; // Used for jumping 

	UPROPERTY(BlueprintReadOnly, Category = Movement) // show CharacterState variable in event graph and details panel
	ECharacterState CharacterState;

	UPROPERTY(BlueprintReadOnly, Category = Movement) // show ActionState variable in event graph and details panel
	EActionState ActionState;

	UPROPERTY(BlueprintReadOnly, Category = Movement) // show DeathPose variable in event graph and details panel
	TEnumAsByte<EDeathPose> DeathPose;

};
