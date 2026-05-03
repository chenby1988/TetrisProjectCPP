#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "TetrisHUD.generated.h"

class ATetrisBoard;

UCLASS()
class TETRISPROJECTCPP_API ATetrisHUD : public AHUD
{
	GENERATED_BODY()
public:
	virtual void DrawHUD() override;
private:
	UPROPERTY() ATetrisBoard* TetrisBoard;
	void DrawTextItem(const FString& Text, float X, float Y, UFont* Font, const FLinearColor& Color, float Scale = 1.0f);
};
