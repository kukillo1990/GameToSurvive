// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Spawner.generated.h"

USTRUCT()
struct GAMETOSURVIVE_API FSpawnObjectInfo
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	TSubclassOf<class AGameToSurviveCharacter> Character;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	TSubclassOf<class AAIController> AIController;

};

UCLASS()
class GAMETOSURVIVE_API ASpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpawner();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	bool ShouldSpawn();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	TArray<class ATargetPoint*> SpawnPositions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	TArray<FSpawnObjectInfo> SpawnObjectList;

	UPROPERTY(VisibleDefaultsOnly, Category = Gameplay)
	USkeletalMeshComponent* SkeletalMesh = NULL;

	UPROPERTY(EditAnywhere, Category = Gameplay)
		unsigned int NumberToSpawn = 5;

	UPROPERTY(EditAnywhere, Category = Gameplay)
		unsigned int NumberAliveAtOnce = 5;

	UPROPERTY(EditAnywhere, Category = Gameplay)
		float SpawnRate = 1.0f;

private:
	unsigned int NumberSpawned = 0;
	unsigned int NumberAlive = 0;
	unsigned int IndexLocationToSpawn = 0;
	float NextTimeToSpawn = 0;
	
	void Spawn();
	void GetSpawnLocationAndRotation(FVector& SpawnLocation, FRotator& SpawnRotator);
	FSpawnObjectInfo& GetNextInfoToSpawn();
public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

};
