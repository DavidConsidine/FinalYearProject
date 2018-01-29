#include "VRCharacter.h"


AVRCharacter::AVRCharacter()
{

}

void AVRCharacter::BeginPlay()
{

}


void AVRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void AVRCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{

}


void AVRCharacter::OnResetVR()
{

}


void AVRCharacter::StartTeleport()
{

}


void AVRCharacter::StopTeleport()
{

}

void AVRCharacter::CancelTeleport()
{

}


void AVRCharacter::SetTelState(ETeleportState NewState)
{

}


void AVRCharacter::OnTeleport(FVector TeleportPosition)
{

}


void AVRCharacter::UpdateTeleportCursor()
{

}

bool AVRCharacter::CheckValidTeleportLocation(FHitResult& HitResult)
{
	return true;
}


bool AVRCharacter::DoScreenFade(bool FadeOut)
{
	return true;
}


void AVRCharacter::MoveForward(float Val)
{

}


void AVRCharacter::MoveRight(float Val)
{

}


void AVRCharacter::TurnAtRate(float Rate)
{

}


void AVRCharacter::LookUpAtRate(float Rate)
{
	
}