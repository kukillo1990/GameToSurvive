// Fill out your copyright notice in the Description page of Project Settings.

#include "GameToSurvive.h"
#include "CoverComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/EngineTypes.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/BodySetup.h"

// Sets default values for this component's properties
UCoverComponent::UCoverComponent() :
	UActorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = true;
	// ...
	AActor* ActorOwner = GetOwner();
	if (ActorOwner)
	{
		CharacterOwner = Cast<ACharacter>(ActorOwner);
		CoverCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
		CoverCollision->SetupAttachment(ActorOwner->GetRootComponent());
		CoverCollision->SetRelativeLocation(FVector::ZeroVector);
		CoverCollision->InitSphereRadius(CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius() * 1.2f);
		CoverCollision->BodyInstance.SetCollisionProfileName("CoverCollision");
	}
}


// Called when the game starts
void UCoverComponent::BeginPlay()
{
	Super::BeginPlay();
	if (CoverCollision)
		CoverCollision->OnComponentBeginOverlap.AddDynamic(this, &UCoverComponent::FoundCover);		// set up a notification for when this component hits something blocking

	if (CharacterOwner && CharacterOwner->InputComponent)
	{
		CharacterOwner->InputComponent->BindAction("ToggleCover", IE_Pressed, this, &UCoverComponent::ToggleCover);
	}
}


// Called every frame
void UCoverComponent::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
	if (bInCover && CoverActor)
	{
		FVector RelativePos;
		GetRelativePosToOrigin(*CoverActor, RelativePos);
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
	}
}

void UCoverComponent::FoundCover(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& OverlapInfo)
{
	if (!bInCover)
	{
		CoverActor = Other;
	}
}

void UCoverComponent::ToggleCover()
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
				FVector LocalScale = CoverActor->GetTransform().InverseTransformVector(FVector(CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius(), 0.0f, 0.0f));
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

bool UCoverComponent::FindCoverPositionAndNormal(const AActor& Actor, FVector& Position, FVector& Normal)
{
	FVector StartLocation = GetOwner()->GetActorLocation();
	FVector EndLocation;
	GetActorOrigin(Actor, EndLocation);
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

bool UCoverComponent::CanMoveWithValue(float Value)
{
	if ((Value > 0 && bCoverReachedMax) || (Value < 0 && bCoverReachedMin))
		return false;
	return true;
}

void UCoverComponent::GetActorOrigin(const AActor& Actor, FVector& Origin)
{
	FVector Extents;
	Actor.GetActorBounds(false, Origin, Extents);
}

void UCoverComponent::GetRelativePosToOrigin(const AActor& Actor, FVector& RelativePos)
{
	FVector OwnerLocation = GetOwner()->GetActorLocation();
	FTransform ActorTransform = Actor.GetActorTransform();
	FVector Origin;
	GetActorOrigin(Actor, Origin);
	ActorTransform.SetLocation(Origin);
	RelativePos = ActorTransform.InverseTransformPosition(OwnerLocation);
}

