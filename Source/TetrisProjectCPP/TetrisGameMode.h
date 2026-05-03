#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TetrisGameMode.generated.h"

UCLASS()
class TETRISPROJECTCPP_API ATetrisGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	ATetrisGameMode();
	virtual void BeginPlay() override;
};
