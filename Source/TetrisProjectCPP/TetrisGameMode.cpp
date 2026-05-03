#include "TetrisGameMode.h"
#include "TetrisBoard.h"
#include "TetrisHUD.h"

ATetrisGameMode::ATetrisGameMode()
{
	DefaultPawnClass = ATetrisBoard::StaticClass();
	HUDClass = ATetrisHUD::StaticClass();
}

void ATetrisGameMode::BeginPlay()
{
	Super::BeginPlay();
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		APawn* ExistingPawn = PC->GetPawn();
		ATetrisBoard* ExistingBoard = Cast<ATetrisBoard>(ExistingPawn);
		if (!ExistingBoard)
		{
			if (ExistingPawn)
			{
				PC->UnPossess();
				ExistingPawn->Destroy();
			}
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			ATetrisBoard* Board = GetWorld()->SpawnActor<ATetrisBoard>(ATetrisBoard::StaticClass(), FVector(0, 0, 1000), FRotator::ZeroRotator, Params);
			if (Board) PC->Possess(Board);
		}
	}
}
