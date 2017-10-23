// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Runtime/Engine/Classes/GameFramework/ProjectileMovementComponent.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"
#include "MyBulletProjectile.generated.h"

UENUM()
enum TBulletTypes {
	EBulletLight, EBulletStandard, EBulletHard
};

//typedef _TBulletTypes TBulletTypes;


UCLASS()
class UE4_HOMETASK5TRY2_API AMyBulletProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMyBulletProjectile();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USphereComponent* CollisionComp;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UStaticMeshComponent* BulletMeshCmp;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float MeshScale = 0.01;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void SetMatField(UMaterial** PMat, const TCHAR* MatRef);
	void SetMatAndSpeed(UMaterial* &Mat, float Speed);

	UMaterial* MLight;
	UMaterial* MStandard;
	UMaterial* MHard;



public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
		void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	UFUNCTION()
		void SetMeshScale(float Scale);
	UFUNCTION()
		void SetGravity(float gravity);
	UFUNCTION()
		void SetBulletType(TBulletTypes bulletType);
};