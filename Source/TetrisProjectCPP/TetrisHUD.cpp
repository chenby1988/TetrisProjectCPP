#include "TetrisHUD.h"
#include "Kismet/GameplayStatics.h"
#include "TetrisBoard.h"
#include "Engine/Canvas.h"
#include "CanvasItem.h"

static const FLinearColor HUDColors[] = {
	FLinearColor(0.05f, 0.05f, 0.05f),
	FLinearColor(0.0f, 0.9f, 0.9f), FLinearColor(0.9f, 0.9f, 0.0f), FLinearColor(0.7f, 0.0f, 0.9f),
	FLinearColor(0.0f, 0.9f, 0.0f), FLinearColor(0.9f, 0.0f, 0.0f), FLinearColor(0.0f, 0.0f, 0.9f), FLinearColor(0.9f, 0.6f, 0.0f)
};

static void DrawBlockPreview(UCanvas* Canvas, float X, float Y, float CellSize, int32 Type, const FLinearColor* Colors)
{
	static const int32 PreviewShape[7][4][4] = {
		{{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}}, {{0,1,1,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
		{{0,1,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}}, {{0,1,1,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}},
		{{1,1,0,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}}, {{1,0,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},
		{{0,0,1,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}}
	};
	if (Type < 0 || Type > 6) return;
	for (int32 r = 0; r < 4; r++)
		for (int32 c = 0; c < 4; c++)
			if (PreviewShape[Type][r][c])
			{
				FCanvasTileItem Item(FVector2D(X + c * CellSize + 2, Y + r * CellSize + 2), FVector2D(CellSize - 2, CellSize - 2), Colors[Type + 1]);
				Item.BlendMode = SE_BLEND_Opaque;
				Canvas->DrawItem(Item);
			}
}

void ATetrisHUD::DrawHUD()
{
	Super::DrawHUD();
	if (!Canvas) return;
	if (!TetrisBoard)
	{
		TArray<AActor*> Boards;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATetrisBoard::StaticClass(), Boards);
		if (Boards.Num() > 0) TetrisBoard = Cast<ATetrisBoard>(Boards[0]);
	}
	if (!TetrisBoard)
	{
		UFont* Font = GEngine->GetMediumFont();
		float CX = (float)Canvas->SizeX / 2.0f;
		float CY = (float)Canvas->SizeY / 2.0f;
		FCanvasTextItem Msg(FVector2D(CX - 200, CY - 30), FText::FromString(TEXT("NO BOARD FOUND")), Font, FLinearColor::Yellow);
		Msg.Scale = FVector2D(2.0f, 2.0f);
		Canvas->DrawItem(Msg);
		return;
	}
	TArray<int32> Grid;
	int32 Width, Height, CurrentType, NextType, HeldType, Score, Level, Lines, Combo, HighScore;
	ETetrisGameState State;
	const TArray<int32>* FlashingLines;
	float FlashTimer;
	TetrisBoard->GetDisplayState(Grid, Width, Height, State, CurrentType, NextType, HeldType, Score, Level, Lines, Combo, HighScore, FlashingLines, FlashTimer);
	if (Grid.Num() == 0) return;
	FVector2D ShakeOffset = TetrisBoard->GetShakeOffset();
	if (State == ETetrisGameState::Menu)
	{
		FCanvasTileItem Bg(FVector2D(0, 0), FVector2D((float)Canvas->SizeX, (float)Canvas->SizeY), FLinearColor(0.06f, 0.06f, 0.12f, 1.0f));
		Bg.BlendMode = SE_BLEND_Opaque; Canvas->DrawItem(Bg);
		UFont* LargeFont = GEngine->GetLargeFont();
		UFont* MedFont = GEngine->GetMediumFont();
		float CX = (float)Canvas->SizeX / 2.0f;
		float CY = (float)Canvas->SizeY / 2.0f;
		FCanvasTextItem Title(FVector2D(CX - 180, CY - 150), FText::FromString(TEXT("TETRIS")), LargeFont, FLinearColor::White);
		Title.Scale = FVector2D(4.0f, 4.0f);
		Title.EnableShadow(FLinearColor(0.0f, 0.5f, 1.0f, 1.0f));
		Canvas->DrawItem(Title);
		FCanvasTextItem Sub(FVector2D(CX - 120, CY - 20), FText::FromString(TEXT("UE5 C++ Edition")), MedFont, FLinearColor(0.6f, 0.8f, 1.0f));
		Sub.Scale = FVector2D(1.2f, 1.2f);
		Canvas->DrawItem(Sub);
		FCanvasTextItem HS(FVector2D(CX - 130, CY + 40), FText::FromString(FString::Printf(TEXT("High Score: %d"), HighScore)), MedFont, FLinearColor(1.0f, 0.85f, 0.0f));
		HS.Scale = FVector2D(1.3f, 1.3f);
		Canvas->DrawItem(HS);
		float Pulse = FMath::Sin(GetWorld()->GetTimeSeconds() * 4.0f) * 0.3f + 0.7f;
		FCanvasTextItem Hint(FVector2D(CX - 150, CY + 120), FText::FromString(TEXT("Press ENTER to Start")), MedFont, FLinearColor(1.0f, 1.0f, 1.0f, Pulse));
		Hint.Scale = FVector2D(1.5f, 1.5f);
		Canvas->DrawItem(Hint);
		FCanvasTextItem Controls(FVector2D(CX - 100, CY + 200), FText::FromString(TEXT("Arrow Keys: Move / Rotate | Space: Hard Drop | C: Hold")), GEngine->GetSmallFont(), FLinearColor(0.5f, 0.5f, 0.5f));
		Canvas->DrawItem(Controls);
		return;
	}
	FCanvasTileItem BgItem(FVector2D(0, 0), FVector2D((float)Canvas->SizeX, (float)Canvas->SizeY), FLinearColor(0.06f, 0.06f, 0.12f, 1.0f));
	BgItem.BlendMode = SE_BLEND_Opaque; Canvas->DrawItem(BgItem);
	float CellSize = FMath::Min((float)Canvas->SizeX / 26.0f, (float)Canvas->SizeY / 24.0f);
	float BoardWidth = CellSize * Width;
	float BoardHeight = CellSize * Height;
	float PanelW = CellSize * 6;
	float TotalW = PanelW + BoardWidth + PanelW + CellSize * 2;
	float StartX = ((float)Canvas->SizeX - TotalW) / 2.0f + PanelW + CellSize + ShakeOffset.X;
	float StartY = ((float)Canvas->SizeY - BoardHeight) / 2.0f + ShakeOffset.Y;
	float LeftX = StartX - PanelW - CellSize;
	float RightX = StartX + BoardWidth + CellSize;
	float FlashAlpha = 0.0f;
	if (FlashingLines && FlashingLines->Num() > 0 && FlashTimer > 0)
		FlashAlpha = FMath::Sin(FlashTimer * 30.0f) * 0.5f + 0.5f;
	FCanvasTileItem BoardBorder(FVector2D(StartX - 4, StartY - 4), FVector2D(BoardWidth + 8, BoardHeight + 8), FLinearColor(0.3f, 0.3f, 0.4f, 1.0f));
	BoardBorder.BlendMode = SE_BLEND_Opaque; Canvas->DrawItem(BoardBorder);
	for (int32 y = 0; y < Height; y++)
	{
		for (int32 x = 0; x < Width; x++)
		{
			int32 Value = Grid[y * Width + x];
			float ScreenX = StartX + x * CellSize;
			float ScreenY = StartY + y * CellSize;
			bool bIsFlashingLine = FlashingLines && FlashingLines->Contains(y);
			FLinearColor CellColor = HUDColors[0];
			bool bIsGhost = false;
			int32 BlockValue = 0;
			if (Value > 0 && Value <= 7) { CellColor = HUDColors[Value]; BlockValue = Value; }
			else if (Value < 0 && Value >= -7) { CellColor = HUDColors[-Value]; bIsGhost = true; BlockValue = -Value; }
			FCanvasTileItem CellItem(FVector2D(ScreenX, ScreenY), FVector2D(CellSize - 1, CellSize - 1), HUDColors[0]);
			CellItem.BlendMode = SE_BLEND_Opaque; Canvas->DrawItem(CellItem);
			if (BlockValue > 0)
			{
				if (bIsGhost)
				{
					FLinearColor GhostColor = CellColor; GhostColor.A = 0.3f;
					FCanvasTileItem GhostItem(FVector2D(ScreenX + 1, ScreenY + 1), FVector2D(CellSize - 3, CellSize - 3), GhostColor);
					GhostItem.BlendMode = SE_BLEND_Translucent; Canvas->DrawItem(GhostItem);
				}
				else if (bIsFlashingLine)
				{
					FLinearColor FlashColor = FMath::Lerp(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f), CellColor, FlashAlpha);
					FCanvasTileItem FlashItem(FVector2D(ScreenX + 1, ScreenY + 1), FVector2D(CellSize - 3, CellSize - 3), FlashColor);
					FlashItem.BlendMode = SE_BLEND_Opaque; Canvas->DrawItem(FlashItem);
				}
				else
				{
					FCanvasTileItem BlockItem(FVector2D(ScreenX + 1, ScreenY + 1), FVector2D(CellSize - 3, CellSize - 3), CellColor);
					BlockItem.BlendMode = SE_BLEND_Opaque; Canvas->DrawItem(BlockItem);
				}
			}
		}
	}
	UFont* TextFont = GEngine->GetMediumFont();
	UFont* SmallFont = GEngine->GetSmallFont();
	float LineH = CellSize * 1.3f;
	float LeftY = StartY;
	DrawTextItem(TEXT("HOLD (C)"), LeftX, LeftY, TextFont, FLinearColor::White);
	float HoldY = LeftY + CellSize * 1.0f;
	float PreviewCell = CellSize * 0.85f;
	float PBox = PreviewCell * 4 + 8;
	FCanvasTileItem HoldBorder(FVector2D(LeftX - 4, HoldY - 4), FVector2D(PBox + 8, PBox + 8), FLinearColor(0.2f, 0.2f, 0.25f, 1.0f));
	HoldBorder.BlendMode = SE_BLEND_Opaque; Canvas->DrawItem(HoldBorder);
	FCanvasTileItem HoldBg(FVector2D(LeftX, HoldY), FVector2D(PBox, PBox), FLinearColor(0.05f, 0.05f, 0.05f, 1.0f));
	HoldBg.BlendMode = SE_BLEND_Opaque; Canvas->DrawItem(HoldBg);
	if (HeldType >= 0 && HeldType <= 6)
	{
		DrawBlockPreview(Canvas, LeftX, HoldY, PreviewCell, HeldType, HUDColors);
	}
	float InfoY = StartY;
	DrawTextItem(TEXT("NEXT"), RightX, InfoY, TextFont, FLinearColor::White);
	float NextY = InfoY + CellSize * 1.0f;
	FCanvasTileItem NextBorder(FVector2D(RightX - 4, NextY - 4), FVector2D(PBox + 8, PBox + 8), FLinearColor(0.2f, 0.2f, 0.25f, 1.0f));
	NextBorder.BlendMode = SE_BLEND_Opaque; Canvas->DrawItem(NextBorder);
	FCanvasTileItem NextBg(FVector2D(RightX, NextY), FVector2D(PBox, PBox), FLinearColor(0.05f, 0.05f, 0.05f, 1.0f));
	NextBg.BlendMode = SE_BLEND_Opaque; Canvas->DrawItem(NextBg);
	if (NextType >= 0 && NextType <= 6)
	{
		DrawBlockPreview(Canvas, RightX, NextY, PreviewCell, NextType, HUDColors);
	}
	float InfoStartY = NextY + PBox + CellSize * 1.2f;
	DrawTextItem(TEXT("SCORE"), RightX, InfoStartY, TextFont, FLinearColor(0.7f, 0.7f, 0.7f));
	DrawTextItem(FString::Printf(TEXT("%d"), Score), RightX, InfoStartY + LineH * 0.6f, TextFont, FLinearColor::White, 1.5f);
	DrawTextItem(TEXT("HIGH SCORE"), RightX, InfoStartY + LineH * 2.0f, TextFont, FLinearColor(0.7f, 0.7f, 0.7f));
	DrawTextItem(FString::Printf(TEXT("%d"), HighScore), RightX, InfoStartY + LineH * 2.6f, TextFont, FLinearColor(1.0f, 0.85f, 0.0f), 1.3f);
	DrawTextItem(TEXT("LEVEL"), RightX, InfoStartY + LineH * 4.0f, TextFont, FLinearColor(0.7f, 0.7f, 0.7f));
	DrawTextItem(FString::Printf(TEXT("%d"), Level), RightX, InfoStartY + LineH * 4.6f, TextFont, FLinearColor::Yellow, 1.5f);
	DrawTextItem(TEXT("LINES"), RightX, InfoStartY + LineH * 6.0f, TextFont, FLinearColor(0.7f, 0.7f, 0.7f));
	DrawTextItem(FString::Printf(TEXT("%d"), Lines), RightX, InfoStartY + LineH * 6.6f, TextFont, FLinearColor::Green, 1.5f);
	if (Combo > 1)
	{
		DrawTextItem(FString::Printf(TEXT("COMBO x%d!"), Combo), RightX, InfoStartY + LineH * 8.0f, TextFont, FLinearColor(1.0f, 0.5f, 0.0f), 1.8f);
	}
	float HelpY = InfoStartY + LineH * 9.5f;
	DrawTextItem(TEXT("CONTROLS"), RightX, HelpY, SmallFont, FLinearColor(0.4f, 0.4f, 0.4f));
	DrawTextItem(TEXT("Left/Right : Move"), RightX, HelpY + CellSize * 0.8f, SmallFont, FLinearColor(0.4f, 0.4f, 0.4f));
	DrawTextItem(TEXT("Up : Rotate"), RightX, HelpY + CellSize * 1.3f, SmallFont, FLinearColor(0.4f, 0.4f, 0.4f));
	DrawTextItem(TEXT("Down : Soft Drop"), RightX, HelpY + CellSize * 1.8f, SmallFont, FLinearColor(0.4f, 0.4f, 0.4f));
	DrawTextItem(TEXT("Space : Hard Drop"), RightX, HelpY + CellSize * 2.3f, SmallFont, FLinearColor(0.4f, 0.4f, 0.4f));
	DrawTextItem(TEXT("C : Hold"), RightX, HelpY + CellSize * 2.8f, SmallFont, FLinearColor(0.4f, 0.4f, 0.4f));
	DrawTextItem(TEXT("P : Pause"), RightX, HelpY + CellSize * 3.3f, SmallFont, FLinearColor(0.4f, 0.4f, 0.4f));
	DrawTextItem(TEXT("R : Reset"), RightX, HelpY + CellSize * 3.8f, SmallFont, FLinearColor(0.4f, 0.4f, 0.4f));
	if (State == ETetrisGameState::GameOver)
	{
		FCanvasTextItem GOItem(FVector2D(StartX + BoardWidth / 2.0f - 120.0f, StartY + BoardHeight / 2.0f - 50.0f), FText::FromString(TEXT("GAME OVER!")), GEngine->GetLargeFont(), FLinearColor::Red);
		GOItem.Scale = FVector2D(3.0f, 3.0f);
		GOItem.EnableShadow(FLinearColor::Black);
		Canvas->DrawItem(GOItem);
		if (Score >= HighScore && Score > 0)
		{
			FCanvasTextItem NRItem(FVector2D(StartX + BoardWidth / 2.0f - 130.0f, StartY + BoardHeight / 2.0f + 20.0f), FText::FromString(TEXT("NEW RECORD!")), TextFont, FLinearColor::Yellow);
			NRItem.Scale = FVector2D(1.8f, 1.8f);
			NRItem.EnableShadow(FLinearColor::Black);
			Canvas->DrawItem(NRItem);
		}
		FCanvasTextItem Retry(FVector2D(StartX + BoardWidth / 2.0f - 100.0f, StartY + BoardHeight / 2.0f + 70.0f), FText::FromString(TEXT("Press ENTER or R")), TextFont, FLinearColor::White);
		Retry.Scale = FVector2D(1.2f, 1.2f);
		Canvas->DrawItem(Retry);
	}
}

void ATetrisHUD::DrawTextItem(const FString& Text, float X, float Y, UFont* Font, const FLinearColor& Color, float Scale)
{
	FCanvasTextItem TextItem(FVector2D(X, Y), FText::FromString(Text), Font, Color);
	TextItem.Scale = FVector2D(Scale, Scale);
	TextItem.EnableShadow(FLinearColor::Black);
	Canvas->DrawItem(TextItem);
}
