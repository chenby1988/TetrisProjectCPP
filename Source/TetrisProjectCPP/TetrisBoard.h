#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Sound/SoundBase.h"
#include "TetrisBoard.generated.h"

UENUM(BlueprintType)
enum class ETetrisGameState : uint8
{
	Menu,
	Playing,
	GameOver
};

UCLASS()
class TETRISPROJECTCPP_API ATetrisBoard : public APawn
{
	GENERATED_BODY()
public:
	ATetrisBoard();
protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
public:
	virtual void Tick(float DeltaTime) override;
	void GetDisplayState(TArray<int32>& OutGrid, int32& OutWidth, int32& OutHeight, ETetrisGameState& OutState, int32& OutCurrentType, int32& OutNextType, int32& OutHeldType, int32& OutScore, int32& OutLevel, int32& OutLines, int32& OutCombo, int32& OutHighScore, const TArray<int32>*& OutFlashingLines, float& OutFlashTimer) const;
	FVector2D GetShakeOffset() const;

private:
	static constexpr int32 BOARD_WIDTH = 10;
	static constexpr int32 BOARD_HEIGHT = 20;
	static constexpr float CELL_SIZE = 100.0f;
	static constexpr float DROP_INTERVAL_INITIAL = 0.8f;
	static constexpr float MOVE_REPEAT_DELAY = 0.1f;
	static constexpr float FLASH_DURATION = 0.2f;
	static constexpr float SHAKE_DURATION = 0.12f;
	static constexpr float SHAKE_INTENSITY = 6.0f;

	static const int32 Tetrominoes[7][4][4][4];
	static const FLinearColor Colors[8];

	int32 Board[BOARD_HEIGHT][BOARD_WIDTH];
	int32 CurrentType;
	int32 CurrentRotation;
	int32 CurrentX;
	int32 CurrentY;

	ETetrisGameState GameState;
	bool bPaused;
	bool bIsFlashing;

	float DropTimer;
	float DropInterval;
	float MoveRepeatTimer;
	float FlashTimer;
	float ShakeTimer;

	int32 Score;
	int32 LinesCleared;
	int32 Level;
	int32 Combo;
	int32 HighScore;
	int32 HardDropStartY;

	int32 NextType;
	int32 HeldType;
	bool bCanHold;

	TArray<int32> Bag;
	void RefillBag();
	int32 DrawFromBag();

	TArray<int32> FlashingLines;

	UPROPERTY() USoundBase* SoundRotate;
	UPROPERTY() USoundBase* SoundMove;
	UPROPERTY() USoundBase* SoundDrop;
	UPROPERTY() USoundBase* SoundClear;
	UPROPERTY() USoundBase* SoundGameOver;
	UPROPERTY() USoundBase* SoundLevelUp;
	UPROPERTY() USoundBase* SoundHold;

	UPROPERTY() TArray<UInstancedStaticMeshComponent*> FixedBlockMeshes;
	UPROPERTY() TArray<UInstancedStaticMeshComponent*> CurrentBlockMeshes;
	UPROPERTY() UCameraComponent* CameraComp;
	UPROPERTY() TArray<int32> DisplayGrid;

	void InitializeGame();
	void SpawnNewPiece();
	bool IsValidPosition(int32 Type, int32 Rotation, int32 X, int32 Y) const;
	void LockPiece();
	int32 ClearLines(TArray<int32>& OutClearedLines);
	void UpdateVisuals();
	void HandleAutoDrop(float DeltaTime);
	void HandleInput(float DeltaTime);
	void AddScore(int32 ClearedLines);
	void UpdateLevelAndSpeed();
	void HoldPiece();
	void SaveHighScore();
	void LoadHighScore();
	void PlaySoundEffect(USoundBase* Sound);
	void MovePiece(int32 DeltaX);
	void RotatePiece();
	void HardDrop();
	void TogglePause();
	void ResetGame();
	FVector GetWorldLocation(int32 X, int32 Y) const;
};
