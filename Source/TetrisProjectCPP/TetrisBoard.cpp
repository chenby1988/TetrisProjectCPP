#include "TetrisBoard.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/Material.h"
#include "MaterialDomain.h"
#include "TetrisSaveGame.h"

const int32 ATetrisBoard::Tetrominoes[7][4][4][4] = {
	{{{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}}, {{0,0,1,0},{0,0,1,0},{0,0,1,0},{0,0,1,0}}, {{0,0,0,0},{0,0,0,0},{1,1,1,1},{0,0,0,0}}, {{0,1,0,0},{0,1,0,0},{0,1,0,0},{0,1,0,0}}},
	{{{0,1,1,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}}, {{0,1,1,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}}, {{0,1,1,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}}, {{0,1,1,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}}},
	{{{0,1,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}}, {{0,1,0,0},{0,1,1,0},{0,1,0,0},{0,0,0,0}}, {{0,0,0,0},{1,1,1,0},{0,1,0,0},{0,0,0,0}}, {{0,1,0,0},{1,1,0,0},{0,1,0,0},{0,0,0,0}}},
	{{{0,1,1,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}}, {{0,1,0,0},{0,1,1,0},{0,0,1,0},{0,0,0,0}}, {{0,0,0,0},{0,1,1,0},{1,1,0,0},{0,0,0,0}}, {{1,0,0,0},{1,1,0,0},{0,1,0,0},{0,0,0,0}}},
	{{{1,1,0,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}}, {{0,0,1,0},{0,1,1,0},{0,1,0,0},{0,0,0,0}}, {{0,0,0,0},{1,1,0,0},{0,1,1,0},{0,0,0,0}}, {{0,1,0,0},{1,1,0,0},{1,0,0,0},{0,0,0,0}}},
	{{{1,0,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}}, {{0,1,1,0},{0,1,0,0},{0,1,0,0},{0,0,0,0}}, {{0,0,0,0},{1,1,1,0},{0,0,1,0},{0,0,0,0}}, {{0,1,0,0},{0,1,0,0},{1,1,0,0},{0,0,0,0}}},
	{{{0,0,1,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}}, {{0,1,0,0},{0,1,0,0},{0,1,1,0},{0,0,0,0}}, {{0,0,0,0},{1,1,1,0},{1,0,0,0},{0,0,0,0}}, {{1,1,0,0},{0,1,0,0},{0,1,0,0},{0,0,0,0}}}
};

const FLinearColor ATetrisBoard::Colors[8] = {
	FLinearColor(0.05f, 0.05f, 0.05f), FLinearColor(0.0f, 1.0f, 1.0f), FLinearColor(1.0f, 1.0f, 0.0f),
	FLinearColor(0.6f, 0.0f, 0.8f), FLinearColor(0.0f, 1.0f, 0.0f), FLinearColor(1.0f, 0.0f, 0.0f),
	FLinearColor(0.0f, 0.0f, 1.0f), FLinearColor(1.0f, 0.5f, 0.0f)
};

ATetrisBoard::ATetrisBoard()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_LastDemotable;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	for (int32 i = 0; i < 7; i++)
	{
		FString FixedName = FString::Printf(TEXT("FixedBlocks_%d"), i);
		UInstancedStaticMeshComponent* FixedMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(*FixedName);
		FixedMesh->SetupAttachment(RootComponent);
		FixedMesh->SetCastShadow(false);
		FixedMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		FixedBlockMeshes.Add(FixedMesh);
		FString CurrentName = FString::Printf(TEXT("CurrentBlocks_%d"), i);
		UInstancedStaticMeshComponent* CurrentMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(*CurrentName);
		CurrentMesh->SetupAttachment(RootComponent);
		CurrentMesh->SetCastShadow(false);
		CurrentMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CurrentBlockMeshes.Add(CurrentMesh);
	}
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(RootComponent);
	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

void ATetrisBoard::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if (APlayerController* PC = Cast<APlayerController>(NewController))
	{
		PC->SetViewTargetWithBlend(this, 0.0f);
	}
}

void ATetrisBoard::BeginPlay()
{
	Super::BeginPlay();
	UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
	if (CubeMesh)
	{
		for (int32 i = 0; i < 7; i++)
		{
			FixedBlockMeshes[i]->SetStaticMesh(CubeMesh);
			CurrentBlockMeshes[i]->SetStaticMesh(CubeMesh);
		}
	}
	UMaterialInterface* DefaultMat = UMaterial::GetDefaultMaterial(MD_Surface);
	if (DefaultMat)
	{
		for (int32 i = 0; i < 7; i++)
		{
			UMaterialInstanceDynamic* FixedDynMat = UMaterialInstanceDynamic::Create(DefaultMat, this);
			if (FixedDynMat) { FixedDynMat->SetVectorParameterValue(TEXT("Color"), Colors[i + 1]); FixedDynMat->SetVectorParameterValue(TEXT("BaseColor"), Colors[i + 1]); FixedBlockMeshes[i]->SetMaterial(0, FixedDynMat); }
			UMaterialInstanceDynamic* CurrentDynMat = UMaterialInstanceDynamic::Create(DefaultMat, this);
			if (CurrentDynMat) { CurrentDynMat->SetVectorParameterValue(TEXT("Color"), Colors[i + 1]); CurrentDynMat->SetVectorParameterValue(TEXT("BaseColor"), Colors[i + 1]); CurrentBlockMeshes[i]->SetMaterial(0, CurrentDynMat); }
		}
	}
	CameraComp->SetRelativeLocation(FVector(0.0f, 0.0f, 2500.0f));
	CameraComp->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
	SoundRotate = LoadObject<USoundBase>(nullptr, TEXT("/Engine/EditorSounds/Notifications/CompileStart_Cue"));
	SoundMove = LoadObject<USoundBase>(nullptr, TEXT("/Engine/EditorSounds/Notifications/CompileStart_Cue"));
	SoundDrop = LoadObject<USoundBase>(nullptr, TEXT("/Engine/EditorSounds/Notifications/CompileSuccess_Cue"));
	SoundClear = LoadObject<USoundBase>(nullptr, TEXT("/Engine/EditorSounds/Notifications/CompileSuccess_Cue"));
	SoundGameOver = LoadObject<USoundBase>(nullptr, TEXT("/Engine/EditorSounds/Notifications/CompileFailed_Cue"));
	SoundLevelUp = LoadObject<USoundBase>(nullptr, TEXT("/Engine/EditorSounds/Notifications/CompileSuccess_Cue"));
	SoundHold = LoadObject<USoundBase>(nullptr, TEXT("/Engine/EditorSounds/Notifications/CompileStart_Cue"));
	LoadHighScore();
	InitializeGame();
	GameState = ETetrisGameState::Menu;
}

void ATetrisBoard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->SetControlRotation(FRotator(-90.0f, 0.0f, 0.0f));
		if (APlayerCameraManager* PCM = PC->PlayerCameraManager)
		{
			FVector BoardCenter = GetWorldLocation(BOARD_WIDTH / 2, BOARD_HEIGHT / 2);
			FVector CameraPos = BoardCenter + FVector(0.0f, 0.0f, 2500.0f);
			if (ShakeTimer > 0.0f)
			{
				ShakeTimer -= DeltaTime;
				CameraPos += FVector(FMath::RandRange(-SHAKE_INTENSITY, SHAKE_INTENSITY), FMath::RandRange(-SHAKE_INTENSITY, SHAKE_INTENSITY), 0.0f);
			}
			PCM->SetActorLocationAndRotation(CameraPos, FRotator(-90.0f, 0.0f, 0.0f));
		}
	}
	if (GameState == ETetrisGameState::Menu)
	{
		HandleInput(DeltaTime);
		return;
	}
	if (bIsFlashing)
	{
		FlashTimer -= DeltaTime;
		if (FlashTimer <= 0.0f)
		{
			bIsFlashing = false;
			FlashingLines.Empty();
		}
		return;
	}
	HandleInput(DeltaTime);
	HandleAutoDrop(DeltaTime);
}

void ATetrisBoard::InitializeGame()
{
	for (int32 y = 0; y < BOARD_HEIGHT; y++)
		for (int32 x = 0; x < BOARD_WIDTH; x++)
			Board[y][x] = 0;
	CurrentType = 0; CurrentRotation = 0; CurrentX = 0; CurrentY = 0;
	bPaused = false; bIsFlashing = false;
	DropTimer = 0.0f; DropInterval = DROP_INTERVAL_INITIAL; MoveRepeatTimer = 0.0f;
	FlashTimer = 0.0f; ShakeTimer = 0.0f;
	Score = 0; LinesCleared = 0; Level = 1; Combo = 0;
	HeldType = -1; bCanHold = true;
	FlashingLines.Empty();
	for (int32 i = 0; i < 7; i++) { FixedBlockMeshes[i]->ClearInstances(); CurrentBlockMeshes[i]->ClearInstances(); }
	DisplayGrid.Reset(); DisplayGrid.SetNum(BOARD_WIDTH * BOARD_HEIGHT);
	RefillBag();
	NextType = DrawFromBag();
	SpawnNewPiece();
	UpdateVisuals();
}

void ATetrisBoard::RefillBag()
{
	Bag.Empty();
	for (int32 i = 0; i < 7; i++) Bag.Add(i);
	for (int32 i = Bag.Num() - 1; i > 0; i--)
	{
		int32 j = FMath::RandRange(0, i);
		Bag.Swap(i, j);
	}
}

int32 ATetrisBoard::DrawFromBag()
{
	if (Bag.Num() == 0) RefillBag();
	return Bag.Pop();
}

void ATetrisBoard::SpawnNewPiece()
{
	CurrentType = NextType;
	NextType = DrawFromBag();
	CurrentRotation = 0;
	CurrentX = BOARD_WIDTH / 2 - 2;
	CurrentY = 0;
	if (!IsValidPosition(CurrentType, CurrentRotation, CurrentX, CurrentY))
	{
		GameState = ETetrisGameState::GameOver;
		PlaySoundEffect(SoundGameOver);
		SaveHighScore();
		return;
	}
	bCanHold = true;
	UpdateVisuals();
}

bool ATetrisBoard::IsValidPosition(int32 Type, int32 Rotation, int32 X, int32 Y) const
{
	for (int32 r = 0; r < 4; r++)
	{
		for (int32 c = 0; c < 4; c++)
		{
			if (Tetrominoes[Type][Rotation][r][c])
			{
				int32 BoardX = X + c;
				int32 BoardY = Y + r;
				if (BoardX < 0 || BoardX >= BOARD_WIDTH || BoardY >= BOARD_HEIGHT) return false;
				if (BoardY >= 0 && Board[BoardY][BoardX] != 0) return false;
			}
		}
	}
	return true;
}

void ATetrisBoard::LockPiece()
{
	for (int32 r = 0; r < 4; r++)
	{
		for (int32 c = 0; c < 4; c++)
		{
			if (Tetrominoes[CurrentType][CurrentRotation][r][c])
			{
				int32 BoardX = CurrentX + c;
				int32 BoardY = CurrentY + r;
				if (BoardY >= 0 && BoardY < BOARD_HEIGHT && BoardX >= 0 && BoardX < BOARD_WIDTH)
					Board[BoardY][BoardX] = CurrentType + 1;
			}
		}
	}
	TArray<int32> ClearedLines;
	int32 NumCleared = ClearLines(ClearedLines);
	if (NumCleared > 0)
	{
		bIsFlashing = true;
		FlashTimer = FLASH_DURATION;
		FlashingLines = ClearedLines;
		AddScore(NumCleared);
		PlaySoundEffect(SoundClear);
		UpdateLevelAndSpeed();
	}
	else
	{
		Combo = 0;
		PlaySoundEffect(SoundDrop);
	}
	SpawnNewPiece();
	UpdateVisuals();
}

int32 ATetrisBoard::ClearLines(TArray<int32>& OutClearedLines)
{
	OutClearedLines.Empty();
	for (int32 y = BOARD_HEIGHT - 1; y >= 0; y--)
	{
		bool bFull = true;
		for (int32 x = 0; x < BOARD_WIDTH; x++)
		{
			if (Board[y][x] == 0) { bFull = false; break; }
		}
		if (bFull)
		{
			OutClearedLines.Add(y);
			for (int32 yy = y; yy > 0; yy--)
				for (int32 x = 0; x < BOARD_WIDTH; x++)
					Board[yy][x] = Board[yy - 1][x];
			for (int32 x = 0; x < BOARD_WIDTH; x++) Board[0][x] = 0;
			y++;
		}
	}
	return OutClearedLines.Num();
}

void ATetrisBoard::UpdateVisuals()
{
	for (int32 i = 0; i < BOARD_WIDTH * BOARD_HEIGHT; i++) DisplayGrid[i] = 0;
	for (int32 y = 0; y < BOARD_HEIGHT; y++)
		for (int32 x = 0; x < BOARD_WIDTH; x++)
			if (Board[y][x] != 0) DisplayGrid[y * BOARD_WIDTH + x] = Board[y][x];
	for (int32 r = 0; r < 4; r++)
	{
		for (int32 c = 0; c < 4; c++)
		{
			if (Tetrominoes[CurrentType][CurrentRotation][r][c])
			{
				int32 BoardX = CurrentX + c;
				int32 BoardY = CurrentY + r;
				if (BoardY >= 0 && BoardY < BOARD_HEIGHT && BoardX >= 0 && BoardX < BOARD_WIDTH)
					DisplayGrid[BoardY * BOARD_WIDTH + BoardX] = CurrentType + 1;
			}
		}
	}
	int32 GhostY = CurrentY;
	while (IsValidPosition(CurrentType, CurrentRotation, CurrentX, GhostY + 1)) GhostY++;
	if (GhostY != CurrentY)
	{
		for (int32 r = 0; r < 4; r++)
		{
			for (int32 c = 0; c < 4; c++)
			{
				if (Tetrominoes[CurrentType][CurrentRotation][r][c])
				{
					int32 BoardX = CurrentX + c;
					int32 BoardY = GhostY + r;
					if (BoardY >= 0 && BoardY < BOARD_HEIGHT && BoardX >= 0 && BoardX < BOARD_WIDTH && DisplayGrid[BoardY * BOARD_WIDTH + BoardX] == 0)
						DisplayGrid[BoardY * BOARD_WIDTH + BoardX] = -(CurrentType + 1);
				}
			}
		}
	}
}

void ATetrisBoard::HandleAutoDrop(float DeltaTime)
{
	if (GameState != ETetrisGameState::Playing || bPaused) return;
	DropTimer += DeltaTime;
	if (DropTimer >= DropInterval)
	{
		DropTimer = 0.0f;
		if (IsValidPosition(CurrentType, CurrentRotation, CurrentX, CurrentY + 1))
		{
			CurrentY++;
			UpdateVisuals();
		}
		else
		{
			LockPiece();
		}
	}
}

void ATetrisBoard::HandleInput(float DeltaTime)
{
	if (bIsFlashing) return;
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;
	if (GameState == ETetrisGameState::Menu)
	{
		if (PC->WasInputKeyJustPressed(EKeys::Enter) || PC->WasInputKeyJustPressed(EKeys::SpaceBar) || PC->WasInputKeyJustPressed(EKeys::LeftMouseButton))
		{
			GameState = ETetrisGameState::Playing;
			InitializeGame();
		}
		return;
	}
	if (PC->WasInputKeyJustPressed(EKeys::P)) { TogglePause(); return; }
	if (bPaused) return;
	if (PC->WasInputKeyJustPressed(EKeys::R)) { ResetGame(); return; }
	if (GameState == ETetrisGameState::GameOver)
	{
		if (PC->WasInputKeyJustPressed(EKeys::Enter) || PC->WasInputKeyJustPressed(EKeys::SpaceBar)) { ResetGame(); }
		return;
	}
	if (GameState != ETetrisGameState::Playing) return;
	if (PC->WasInputKeyJustPressed(EKeys::SpaceBar)) { HardDrop(); return; }
	if (PC->WasInputKeyJustPressed(EKeys::C)) { HoldPiece(); return; }
	if (PC->WasInputKeyJustPressed(EKeys::Up) || PC->WasInputKeyJustPressed(EKeys::W)) { RotatePiece(); PlaySoundEffect(SoundRotate); }
	if (PC->IsInputKeyDown(EKeys::Down) || PC->IsInputKeyDown(EKeys::S)) { DropTimer += DeltaTime * 8.0f; }
	bool bLeftDown = PC->IsInputKeyDown(EKeys::Left) || PC->IsInputKeyDown(EKeys::A);
	bool bRightDown = PC->IsInputKeyDown(EKeys::Right) || PC->IsInputKeyDown(EKeys::D);
	if (bLeftDown && !bRightDown)
	{
		if (PC->WasInputKeyJustPressed(EKeys::Left) || PC->WasInputKeyJustPressed(EKeys::A))
		{
			MovePiece(-1); MoveRepeatTimer = MOVE_REPEAT_DELAY; PlaySoundEffect(SoundMove);
		}
		else
		{
			MoveRepeatTimer -= DeltaTime;
			if (MoveRepeatTimer <= 0.0f) { MovePiece(-1); MoveRepeatTimer = MOVE_REPEAT_DELAY; PlaySoundEffect(SoundMove); }
		}
	}
	else if (bRightDown && !bLeftDown)
	{
		if (PC->WasInputKeyJustPressed(EKeys::Right) || PC->WasInputKeyJustPressed(EKeys::D))
		{
			MovePiece(1); MoveRepeatTimer = MOVE_REPEAT_DELAY; PlaySoundEffect(SoundMove);
		}
		else
		{
			MoveRepeatTimer -= DeltaTime;
			if (MoveRepeatTimer <= 0.0f) { MovePiece(1); MoveRepeatTimer = MOVE_REPEAT_DELAY; PlaySoundEffect(SoundMove); }
		}
	}
	else { MoveRepeatTimer = 0.0f; }
}

void ATetrisBoard::MovePiece(int32 DeltaX)
{
	if (IsValidPosition(CurrentType, CurrentRotation, CurrentX + DeltaX, CurrentY))
	{
		CurrentX += DeltaX;
		UpdateVisuals();
	}
}

void ATetrisBoard::RotatePiece()
{
	int32 NewRotation = (CurrentRotation + 1) % 4;
	if (IsValidPosition(CurrentType, NewRotation, CurrentX, CurrentY))
	{
		CurrentRotation = NewRotation;
		UpdateVisuals();
		return;
	}
	static const int32 WallKickOffsets[5] = { -1, 1, -2, 2, 0 };
	for (int32 Offset : WallKickOffsets)
	{
		if (IsValidPosition(CurrentType, NewRotation, CurrentX + Offset, CurrentY))
		{
			CurrentRotation = NewRotation;
			CurrentX += Offset;
			UpdateVisuals();
			return;
		}
	}
}

void ATetrisBoard::HardDrop()
{
	if (GameState != ETetrisGameState::Playing || bPaused) return;
	HardDropStartY = CurrentY;
	while (IsValidPosition(CurrentType, CurrentRotation, CurrentX, CurrentY + 1)) CurrentY++;
	ShakeTimer = SHAKE_DURATION;
	LockPiece();
}

void ATetrisBoard::HoldPiece()
{
	if (GameState != ETetrisGameState::Playing || bPaused || !bCanHold) return;
	if (HeldType == -1)
	{
		HeldType = CurrentType;
		SpawnNewPiece();
	}
	else
	{
		int32 Temp = CurrentType;
		CurrentType = HeldType;
		HeldType = Temp;
		CurrentRotation = 0;
		CurrentX = BOARD_WIDTH / 2 - 2;
		CurrentY = 0;
		UpdateVisuals();
	}
	bCanHold = false;
	PlaySoundEffect(SoundHold);
}

void ATetrisBoard::TogglePause()
{
	if (GameState == ETetrisGameState::Playing)
	{
		bPaused = !bPaused;
	}
}

void ATetrisBoard::ResetGame()
{
	GameState = ETetrisGameState::Menu;
	InitializeGame();
}

void ATetrisBoard::AddScore(int32 ClearedLines)
{
	Combo++;
	int32 BaseScore = 0;
	switch (ClearedLines)
	{
	case 1: BaseScore = 100; break;
	case 2: BaseScore = 300; break;
	case 3: BaseScore = 600; break;
	case 4: BaseScore = 1000; break;
	}
	Score += BaseScore * Level * Combo;
	LinesCleared += ClearedLines;
}

void ATetrisBoard::UpdateLevelAndSpeed()
{
	int32 NewLevel = (LinesCleared / 10) + 1;
	if (NewLevel > Level)
	{
		Level = NewLevel;
		DropInterval = FMath::Max(0.05f, DROP_INTERVAL_INITIAL * FMath::Pow(0.85f, Level - 1));
		PlaySoundEffect(SoundLevelUp);
	}
}

void ATetrisBoard::SaveHighScore()
{
	if (Score > HighScore)
	{
		HighScore = Score;
		UTetrisSaveGame* SaveGame = Cast<UTetrisSaveGame>(UGameplayStatics::CreateSaveGameObject(UTetrisSaveGame::StaticClass()));
		if (SaveGame)
		{
			SaveGame->HighScore = HighScore;
			UGameplayStatics::SaveGameToSlot(SaveGame, TEXT("TetrisHighScore"), 0);
		}
	}
}

void ATetrisBoard::LoadHighScore()
{
	if (UGameplayStatics::DoesSaveGameExist(TEXT("TetrisHighScore"), 0))
	{
		UTetrisSaveGame* SaveGame = Cast<UTetrisSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("TetrisHighScore"), 0));
		if (SaveGame) HighScore = SaveGame->HighScore;
	}
	else
	{
		HighScore = 0;
	}
}

void ATetrisBoard::PlaySoundEffect(USoundBase* Sound)
{
	if (Sound) UGameplayStatics::PlaySound2D(GetWorld(), Sound);
}

FVector ATetrisBoard::GetWorldLocation(int32 X, int32 Y) const
{
	float OriginX = -(BOARD_WIDTH * CELL_SIZE) / 2.0f;
	float OriginY = (BOARD_HEIGHT * CELL_SIZE) / 2.0f;
	return FVector(OriginX + X * CELL_SIZE + CELL_SIZE * 0.5f, OriginY - Y * CELL_SIZE - CELL_SIZE * 0.5f, 0.0f);
}

void ATetrisBoard::GetDisplayState(TArray<int32>& OutGrid, int32& OutWidth, int32& OutHeight, ETetrisGameState& OutState, int32& OutCurrentType, int32& OutNextType, int32& OutHeldType, int32& OutScore, int32& OutLevel, int32& OutLines, int32& OutCombo, int32& OutHighScore, const TArray<int32>*& OutFlashingLines, float& OutFlashTimer) const
{
	OutGrid = DisplayGrid;
	OutWidth = BOARD_WIDTH;
	OutHeight = BOARD_HEIGHT;
	OutState = GameState;
	OutCurrentType = CurrentType;
	OutNextType = NextType;
	OutHeldType = HeldType;
	OutScore = Score;
	OutLevel = Level;
	OutLines = LinesCleared;
	OutCombo = Combo;
	OutHighScore = HighScore;
	OutFlashingLines = &FlashingLines;
	OutFlashTimer = FlashTimer;
}

FVector2D ATetrisBoard::GetShakeOffset() const
{
	if (ShakeTimer > 0.0f)
	{
		return FVector2D(FMath::RandRange(-SHAKE_INTENSITY, SHAKE_INTENSITY), FMath::RandRange(-SHAKE_INTENSITY, SHAKE_INTENSITY));
	}
	return FVector2D::ZeroVector;
}
