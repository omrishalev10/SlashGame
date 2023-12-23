#include "Characters/SlashAnimInstance.h"
#include "Characters/SlashCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

//Initialize
void USlashAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	// Try to cast the object to ASlashCharacter and use TryGetPawnOwner()
	SlashCharacter = Cast<ASlashCharacter>(TryGetPawnOwner());
	if (SlashCharacter)
	{
		SlashCharacterMovement = SlashCharacter->GetCharacterMovement();	
	}
}

// Updating every single frame
void USlashAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	// Get the SlashCharacter Component
	if (SlashCharacterMovement)
	{
		GroundSpeed = UKismetMathLibrary::VSizeXY(SlashCharacterMovement->Velocity); // Using UKismetMathLibrary library to get the length of the velocity vector (getting the speed of the character)
		IsFalling = SlashCharacterMovement->IsFalling(); // IsFalling() used for jumping in Blueprints code 
		CharacterState = SlashCharacter->GetCharacterState(); // Update the state of the character every frame (equipped or not)
		ActionState = SlashCharacter->GetActionState();
		DeathPose = SlashCharacter->GetDeathPose();
	}	
}
