// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "PatrolComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMETOSURVIVE_API UPatrolComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPatrolComponent();

	TArray<AActor*> GetPatrolPoints() const;


private:
	UPROPERTY(Category = "Patrol Route", EditInstanceOnly)
		TArray<AActor*> PatrolPoints;
	
};
