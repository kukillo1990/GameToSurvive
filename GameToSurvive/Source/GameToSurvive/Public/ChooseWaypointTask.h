// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BehaviorTree/BTTaskNode.h"
#include "ChooseWaypointTask.generated.h"

/**
 * 
 */
UCLASS()
class GAMETOSURVIVE_API UChooseWaypointTask : public UBTTaskNode
{
	GENERATED_BODY()
	
	
	
	
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector IndexKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector WaypointKey;

};
