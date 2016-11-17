// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "CoverComponent.generated.h"


/*
 TODO: Check collision high/low, change channel cover meshes, Clamp camera
*/

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMETOSURVIVE_API UCoverComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Cover, meta = (AllowPrivateAccess = "true"))
	class USphereComponent* CoverCollision;

public:	
	// Sets default values for this component's properties
	UCoverComponent();

	// Called when the game starts
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

	UFUNCTION()
		void FoundCover(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& OverlapInfo);

	void ToggleCover();

	bool FindCoverPositionAndNormal(const AActor& CoverActor, FVector& Position, FVector& Normal);

	UFUNCTION(BlueprintCallable, Category = "Cover")
	bool IsInCover() const { return bInCover;  }

	UFUNCTION(BlueprintCallable, Category = "Cover")
	bool IsMaxRightCover() const { return bCoverReachedMax; }

	UFUNCTION(BlueprintCallable, Category = "Cover")
	bool IsMinRightCover() const { return bCoverReachedMin; }

	bool CanMoveWithValue(float Value);

	FVector& GetCoverMoveDir() { return CoverMoveDir;  }
protected:
	ACharacter* CharacterOwner = NULL;
	bool bInCover = false;
	AActor* CoverActor = NULL;
	FVector CoverMoveDir = FVector::ZeroVector;
	FVector LocalMoveDir = FVector::ZeroVector;

	float CoverMaxMove = 0.0f;
	bool bCoverReachedMax = false;
	bool bCoverReachedMin = false;


};
