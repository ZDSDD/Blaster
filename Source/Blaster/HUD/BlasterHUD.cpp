// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterHUD.h"

void ABlasterHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2d ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
		float SpreadScaled = CrosshairSpreadMax * HudPackage.CrosshairSpread;
		//CENTER
		if (HudPackage.Crosshairscenter)
		{
			FVector2D Spread(0.f,0.f);
			DrawCrosshair(HudPackage.Crosshairscenter, ViewportCenter,Spread);
		}
		//RIGHT
		if (HudPackage.CrosshairsRight)
		{
			FVector2D Spread(SpreadScaled,0.f);
			DrawCrosshair(HudPackage.CrosshairsRight, ViewportCenter,Spread);
		}
		//LEFT
		if (HudPackage.CrosshairsLeft)
		{
			FVector2D Spread(-SpreadScaled,0.f);
			DrawCrosshair(HudPackage.CrosshairsLeft, ViewportCenter,Spread);
		}
		//BOTTOM
		if (HudPackage.CrosshairsBottom)
		{
			FVector2D Spread(0,SpreadScaled);
			DrawCrosshair(HudPackage.CrosshairsBottom, ViewportCenter,Spread);
		}
		//TOP
		if (HudPackage.CrosshairsTop)
		{
			FVector2D Spread(0.f,-SpreadScaled);
			DrawCrosshair(HudPackage.CrosshairsTop, ViewportCenter,Spread);
		}
	}
}

void ABlasterHUD::DrawCrosshair(UTexture2D* Texture, FVector2d ViewportCenter, FVector2D Spread)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2d TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / 2.f )+ Spread.X,
		ViewportCenter.Y - (TextureHeight / 2.f ) + Spread.Y
	);
	DrawTexture(
		Texture,
		TextureDrawPoint.X,
		TextureDrawPoint.Y,
		TextureWidth,
		TextureHeight,
		0.f,
		0.f,
		1.f,
		1.f,
		FLinearColor::White
		);
}
