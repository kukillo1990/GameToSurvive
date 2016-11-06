// Fill out your copyright notice in the Description page of Project Settings.

#include "GameToSurvive.h"
#include "ChooseWaypointTask.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "PatrolComponent.h"
#include "Runtime/AIModule/Classes/AIController.h"


EBTNodeResult::Type UChooseWaypointTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	int Index = BlackboardComp->GetValueAsInt(IndexKey.SelectedKeyName);

	APawn* ControlledPawn = OwnerComp.GetAIOwner()->GetPawn();
	UPatrolComponent* PatrolComp = ControlledPawn->FindComponentByClass<UPatrolComponent>();

	if (!PatrolComp)
		return EBTNodeResult::Failed;
	
	TArray<AActor*> PatrolPoints = PatrolComp->GetPatrolPoints();
	if (PatrolPoints.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Patrol Points Added"));
		return EBTNodeResult::Failed;
	}

	BlackboardComp->SetValueAsObject(WaypointKey.SelectedKeyName, PatrolPoints[Index]);

	int NextIndex = (Index + 1) % PatrolPoints.Num();
	BlackboardComp->SetValueAsInt(IndexKey.SelectedKeyName, NextIndex);
	
	
	return EBTNodeResult::Succeeded;
}
