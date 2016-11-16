
// Fill out your copyright notice in the Description page of Project Settings.

#include "GameToSurvive.h"
#include "Spawner.h"
#include "Engine/TargetPoint.h"
#include "AIController.h"
#include "GameToSurviveCharacter.h"

// Sets default values
ASpawner::ASpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(RootComponent);

	SetActorHiddenInGame(true);
}

// Called when the game starts or when spawned
void ASpawner::BeginPlay()
{
	Super::BeginPlay();
	NextTimeToSpawn = GetWorld()->GetTimeSeconds() + SpawnRate;
}

// Called every frame
void ASpawner::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	if (ShouldSpawn())
		Spawn();
}

bool ASpawner::ShouldSpawn()
{
	return (SpawnObjectList.Num() > 0  && SpawnObjectList[0].Character && SpawnObjectList[0].Character->GetDefaultObject() && NumberAlive < NumberAliveAtOnce && NumberSpawned < NumberToSpawn && GetWorld()->GetTimeSeconds() > NextTimeToSpawn);
}

void ASpawner::Spawn()
{
	FVector SpawnLocation = GetActorLocation();
	FRotator SpawnRotation = GetActorRotation();
	GetSpawnLocationAndRotation(SpawnLocation, SpawnRotation);
	FSpawnObjectInfo& SpawnObjectInfo = GetNextInfoToSpawn();
	if (AGameToSurviveCharacter* Character = GetWorld()->SpawnActor<AGameToSurviveCharacter>(SpawnObjectInfo.Character, SpawnLocation, SpawnRotation))
	{
		AAIController* NewController = GetWorld()->SpawnActor<AAIController>(SpawnObjectInfo.AIController, GetActorLocation(), GetActorRotation());
		if (NewController != NULL)
		{
			NewController->Possess(Character);
		}
		NumberAlive++;
		NumberSpawned++;
	}
	NextTimeToSpawn = GetWorld()->GetTimeSeconds() + SpawnRate;
}

void ASpawner::GetSpawnLocationAndRotation(FVector& SpawnLocation, FRotator& SpawnRotator)
{
	int NumberSpawnPositions = SpawnPositions.Num();
	if (NumberSpawnPositions > 0)
	{
		ATargetPoint* SpawnTargetPoint = SpawnPositions[IndexLocationToSpawn++ % NumberSpawnPositions];
		SpawnLocation = SpawnTargetPoint->GetActorLocation();
		SpawnRotator = SpawnTargetPoint->GetActorRotation();
	}
}

FSpawnObjectInfo& ASpawner::GetNextInfoToSpawn()
{
	unsigned int Index = NumberSpawned % SpawnObjectList.Num();
	return SpawnObjectList[Index];
}

#if WITH_EDITOR
void ASpawner::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	static const FName CharacterToSpawnProperty("CharactersToSpawn");

	const FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : FName();
	if (PropertyName == CharacterToSpawnProperty )
	{
		if (SpawnObjectList.Num() > 0 )
		{			
			AGameToSurviveCharacter* FirstCharacter = Cast<AGameToSurviveCharacter>(SpawnObjectList[0].Character->GetDefaultObject());
			if (FirstCharacter)
			{
				SkeletalMesh->SetSkeletalMesh(FirstCharacter->GetMesh()->SkeletalMesh);
			}
		}
	}
}
#endif

