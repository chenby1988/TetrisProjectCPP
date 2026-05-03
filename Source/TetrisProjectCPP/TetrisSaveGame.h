#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "TetrisSaveGame.generated.h"

UCLASS()
class TETRISPROJECTCPP_API UTetrisSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	UTetrisSaveGame() { HighScore = 0; }
	UPROPERTY() int32 HighScore;
};
