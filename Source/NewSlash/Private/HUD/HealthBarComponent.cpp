#include "HUD/HealthBarComponent.h"
#include "HUD/HealthBar.h"
#include "Components/ProgressBar.h"

void UHealthBarComponent::SetHealthPercent(float Percent)
{
	if (HealthBarWidget == nullptr)
	{
		/* GetUserWidgetObject is called from UWidgetComponent class and
		returns the user widget object displayed by this component */
		HealthBarWidget = Cast<UHealthBar>(GetUserWidgetObject()); // Cast from HealthBar class
	}

	if (HealthBarWidget && HealthBarWidget->HealthBar)
	{
		HealthBarWidget->HealthBar->SetPercent(Percent);
	}
}
