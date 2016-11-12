// Fill out your copyright notice in the Description page of Project Settings.

#include "GameToSurvive.h"
#include "Gun.h"
#include "Animation/AnimInstance.h"
#include "Projectile.h"
#include "GameToSurviveCharacter.h"
#include "DrawDebugHelpers.h"

//Sets default values
AGun::AGun()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

}

//Called when the game starts or when spawned
void AGun::BeginPlay()
{
	Super::BeginPlay();
	
}

//Called every frame
void AGun::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	if (bIsAiming)
	{
		if (OwnersAnimInstance && !OwnersAnimInstance->Montage_IsPlaying(AimAnimation))
		{
			OwnersAnimInstance->Montage_Play(AimAnimation);
		}
	}

	if (bIsFiring && CanShoot())
	{
		Fire();
	}


}

void AGun::StartAiming()
{
	if (AimAnimation)
		bIsAiming = true;
}

void AGun::StopAiming()
{
	bIsAiming = false;
	if (OwnersAnimInstance)
	{
		OwnersAnimInstance->Montage_Stop(0.5f, AimAnimation);
	}
}

void AGun::Fire()
{
	UE_LOG(LogTemp, Warning, TEXT("SHOOTING!"));
	//try and fire a projectile
	if (ProjectileClass != NULL)
	{
		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			const FRotator SpawnRotation = FP_MuzzleLocation->GetComponentRotation();
			const FVector SpawnLocation = FP_MuzzleLocation->GetComponentLocation();

			// spawn the projectile at the muzzle
			AProjectile* NewProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SpawnLocation, SpawnRotation);
			NewProjectile->SetGunOwner(this);
			UCameraComponent* CameraComp = CharacterOwner->GetFollowCamera();

			FVector Diff = SpawnLocation - CameraComp->GetComponentLocation();
			float Dot = FVector::DotProduct(Diff, CameraComp->GetForwardVector());
			FVector StartLocation = CameraComp->GetComponentLocation() + CameraComp->GetForwardVector() * Dot;
			FVector EndLocation = StartLocation + CameraComp->GetForwardVector() * 100000.0f;
			struct FHitResult HitResult;
			if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Pawn))
			{
				FVector NewVelocity = HitResult.ImpactPoint - SpawnLocation;
				NewVelocity.Normalize();
				NewProjectile->SetVelocity(NewVelocity);
				DrawDebugLine(GetWorld(), StartLocation, HitResult.ImpactPoint, FColor::Red, false, 5.0f);
			}
		}
	}

	// try and play the sound if specified
	if (FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	if (OwnersAnimInstance)
	{
		UAnimMontage* TargetAnim = bIsAiming ? FireFromShoulderAnimation : FireFromHipAnimation;
		if (TargetAnim && !OwnersAnimInstance->Montage_IsPlaying(TargetAnim))
			OwnersAnimInstance->Montage_Play(TargetAnim);
	}

	NextTimeToShoot = GetWorld()->GetTimeSeconds() + FireRate;
}

void AGun::SetCharacterOwner(class AGameToSurviveCharacter* NewCharacterOwner)
{
	CharacterOwner = NewCharacterOwner;
	if (CharacterOwner && CharacterOwner->GetMesh())
	{
		OwnersAnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();
	}
}

bool AGun::CanShoot()
{
	return GetWorld()->GetTimeSeconds() > NextTimeToShoot;
}

