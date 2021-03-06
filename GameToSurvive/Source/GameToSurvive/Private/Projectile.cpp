// Fill out your copyright notice in the Description page of Project Settings.

#include "GameToSurvive.h"
#include "Projectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameToSurviveCharacter.h"
#include "Gun.h"


AProjectile::AProjectile()
{
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);		// set up a notification for when this component hits something blocking

																						// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->ProjectileGravityScale = 0.0f;

	// Die after 3 seconds by default
	InitialLifeSpan = 3.0f;
}

void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Only add impulse and destroy projectile if we hit a physics
	if ((OtherActor != NULL) && (OtherActor != this))
	{
		FDamageEvent DamageEvent;
		AController* OwnerController = OwnerGun ? (OwnerGun->GetCharacterOwner() ? OwnerGun->GetCharacterOwner()->GetController() : NULL) : NULL;
		OtherActor->TakeDamage(Damage, DamageEvent, OwnerController, this);
		Destroy();
	}
	else
	{
		Destroy();
	}
}

void AProjectile::SetVelocity(const FVector& ForcedVelocity)
{
	ProjectileMovement->Velocity = ForcedVelocity * ProjectileMovement->InitialSpeed;
}

float AProjectile::GetDamage()
{
	return Damage;
}

void AProjectile::SetGunOwner(AGun* NewOwnerGun)
{
	OwnerGun = NewOwnerGun;
}

