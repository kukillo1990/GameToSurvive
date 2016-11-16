// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "GameToSurvive.h"
#include "Kismet/HeadMountedDisplayFunctionLibrary.h"
#include "GameToSurviveCharacter.h"
#include "Gun.h"
#include "Projectile.h"
#include "Animation/AnimInstance.h"
#include "DrawDebugHelpers.h"

//////////////////////////////////////////////////////////////////////////
// AGameToSurviveCharacter

AGameToSurviveCharacter::AGameToSurviveCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	
	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	CoverCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CoverCollision->SetupAttachment(RootComponent);
	CoverCollision->SetRelativeLocation(FVector::ZeroVector);
	CoverCollision->InitSphereRadius(GetCapsuleComponent()->GetScaledCapsuleRadius() * 1.3f);
	CoverCollision->BodyInstance.SetCollisionProfileName("CoverCollision");


	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

void AGameToSurviveCharacter::BeginPlay()
{
	ACharacter::BeginPlay();

	CoverCollision->OnComponentBeginOverlap.AddDynamic(this, &AGameToSurviveCharacter::FoundCover);		// set up a notification for when this component hits something blocking

	if (GunToCarryClass && GetMesh())
	{
		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			// spawn the projectile at the muzzle
			GunCarried = World->SpawnActor<AGun>(GunToCarryClass);
			GunCarried->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
			EquipGun(GunCarried);
		}
	}
}

float AGameToSurviveCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = APawn::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	if ((Health -= ActualDamage) <= 0)
		Destroy();
	else
		PlayHitAnim();
	return ActualDamage;
}

void AGameToSurviveCharacter::PlayHitAnim()
{
	if (HitReactionMontages.Num() <= 0)
		return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && !AnimInstance->Montage_IsPlaying(HitReactionMontages[LastHitAnimIndex]))
	{
		LastHitAnimIndex = (++LastHitAnimIndex) % HitReactionMontages.Num();
		AnimInstance->Montage_Play(HitReactionMontages[LastHitAnimIndex]);
	}
}

void AGameToSurviveCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (bInCover && CoverActor)
	{
		FVector Origin;
		FVector Extent;
		CoverActor->GetActorBounds(false, Origin, Extent);
		FVector ActorLocation = GetActorLocation();
		FVector RelativePos  = CoverActor->GetActorTransform().InverseTransformPosition(ActorLocation);
		float RelativePosDist = FVector::DotProduct(RelativePos, LocalMoveDir);
		bCoverReachedMax = false;
		bCoverReachedMin = false;
		if (RelativePosDist >= CoverMaxMove)
		{
			bCoverReachedMax = true;
		}
		else if (RelativePosDist <= -CoverMaxMove)
		{
			bCoverReachedMin = true;
		}
		UE_LOG(LogTemp, Warning, TEXT("%s!"), *RelativePos.ToString());
		int i = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AGameToSurviveCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &AGameToSurviveCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGameToSurviveCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AGameToSurviveCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AGameToSurviveCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AGameToSurviveCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AGameToSurviveCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AGameToSurviveCharacter::OnResetVR);

	PlayerInputComponent->BindAction("Shoot", IE_Pressed, this, &AGameToSurviveCharacter::OnStartFire);
	PlayerInputComponent->BindAction("Shoot", IE_Released, this, &AGameToSurviveCharacter::OnStopFire);

	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AGameToSurviveCharacter::OnStartAim);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &AGameToSurviveCharacter::OnStopAim);

	PlayerInputComponent->BindAction("ToggleCover", IE_Pressed, this, &AGameToSurviveCharacter::ToggleCover);
}


void AGameToSurviveCharacter::OnStartFire()
{
	if (GunCarried)
		GunCarried->StartFiring();
}

void AGameToSurviveCharacter::OnStopFire()
{
	if (GunCarried)
		GunCarried->StopFiring();
}

void AGameToSurviveCharacter::OnStartAim()
{
	if (GunCarried)
		GunCarried->StartAiming();
}

void AGameToSurviveCharacter::OnStopAim()
{
	if (GunCarried)
		GunCarried->StopAiming();
}

void AGameToSurviveCharacter::EquipGun(AGun* Gun)
{
	if (Gun)
	{
		Gun->SetCharacterOwner(this);
	}
}

void AGameToSurviveCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AGameToSurviveCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	// jump, but only on the first touch
	if (FingerIndex == ETouchIndex::Touch1)
	{
		Jump();
	}
}

void AGameToSurviveCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	if (FingerIndex == ETouchIndex::Touch1)
	{
		StopJumping();
	}
}

void AGameToSurviveCharacter::FoundCover(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& OverlapInfo)
{
	if (!bInCover)
	{
		CoverActor = Other;
	}	
}

void AGameToSurviveCharacter::ToggleCover()
{
	if (!bInCover)
	{
		if (CoverActor)
		{
			FVector CoverPosition;
			FVector CoverNormal;
			if (FindCoverPositionAndNormal(*CoverActor, CoverPosition, CoverNormal))
			{
				CoverMoveDir = FVector::CrossProduct(CoverNormal, FVector::UpVector);
				
				FBox AABB = CoverActor->CalculateComponentsBoundingBoxInLocalSpace();
				FVector Extents = AABB.GetExtent();
				FVector LocalScale = CoverActor->GetTransform().InverseTransformVector(FVector(GetCapsuleComponent()->GetScaledCapsuleRadius(), 0.0f, 0.0f));
				LocalMoveDir = CoverActor->GetActorRotation().UnrotateVector(CoverMoveDir);
				CoverMaxMove = FMath::Abs(FVector::DotProduct(Extents, LocalMoveDir)) - LocalScale.X;
				bInCover = true;
			}
			
		}
	}
	else
	{
		bInCover = false;
	}
}

bool AGameToSurviveCharacter::FindCoverPositionAndNormal(const AActor& Actor, FVector& Position, FVector& Normal)
{
	FVector StartLocation = GetActorLocation();
	FVector EndLocation = Actor.GetActorLocation();
	FHitResult HitResult;
	if (GetWorld()->LineTraceSingleByObjectType(HitResult, StartLocation, EndLocation, FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllStaticObjects)))
	{
		if (HitResult.Actor == &Actor)
		{
			if (GetWorld()->LineTraceSingleByObjectType(HitResult, StartLocation, StartLocation - HitResult.ImpactNormal * 150.0f, FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllStaticObjects)))
			{
				if (HitResult.Actor == &Actor)
				{
					Position = HitResult.ImpactPoint;
					Normal = HitResult.ImpactNormal;
					return true;
				}
			}
		}
	}
	return false;
}

void AGameToSurviveCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AGameToSurviveCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AGameToSurviveCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f) && !bInCover)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AGameToSurviveCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		if (bInCover && ((Value > 0 && bCoverReachedMax) || (Value < 0 && bCoverReachedMin)))
			return;

		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = bInCover ? CoverMoveDir : FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}
