// Fill out your copyright notice in the Description page of Project Settings.

#include "MyBulletProjectile.h"
#include "Runtime/Engine/Classes/Materials/Material.h"

void AMyBulletProjectile::SetMatField(UMaterial** PMat, const TCHAR* MatRef) {
	ConstructorHelpers::FObjectFinder<UMaterial> Material(MatRef);
	if (Material.Object) *PMat = Material.Object;
}

// Sets default values
AMyBulletProjectile::AMyBulletProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5);
	CollisionComp->OnComponentHit.AddDynamic(this, &AMyBulletProjectile::OnHit);

	BulletMeshCmp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Bullet"));
	BulletMeshCmp->SetSimulatePhysics(false);
	ConstructorHelpers::FObjectFinder<UStaticMesh> MeshSphere(TEXT("StaticMesh'/Game/Weapon/Shape_Sphere.Shape_Sphere'"));
	if (MeshSphere.Object) {
		BulletMeshCmp->SetStaticMesh(MeshSphere.Object);
	}

	SetMatField(&MLight, TEXT("Material'/Game/StarterContent/Materials/M_Basic_Wall.M_Basic_Wall'"));
	SetMatField(&MStandard, TEXT("Material'/Game/StarterContent/Materials/M_AssetPlatform.M_AssetPlatform'"));
	SetMatField(&MHard, TEXT("Material'/Game/StarterContent/Materials/M_Water_Ocean.M_Water_Ocean'"));
	SetMeshScale(MeshScale);

	(Cast<USceneComponent>(BulletMeshCmp))->SetupAttachment(CollisionComp);
	RootComponent = CollisionComp;
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 5000 / 2;
	ProjectileMovement->MaxSpeed = 5000;
	ProjectileMovement->ProjectileGravityScale = .1f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	SetBulletType(EBulletStandard);
	InitialLifeSpan = 3;
}

void AMyBulletProjectile::SetGravity(float gravity) {
	ProjectileMovement->ProjectileGravityScale = gravity;
}

void AMyBulletProjectile::SetMeshScale(float Scale) {
	(Cast<USceneComponent>(BulletMeshCmp))->SetWorldScale3D(FVector::OneVector * Scale);
}

void AMyBulletProjectile::SetBulletType(TBulletTypes bulletType) {
	switch (bulletType) {
	case EBulletLight:
		SetMatAndSpeed(MLight, 250); break;
	case EBulletStandard:
		SetMatAndSpeed(MStandard, 500); break;
	case EBulletHard:
		SetMatAndSpeed(MHard, 10000); break;
	}
	return;
}

void AMyBulletProjectile::SetMatAndSpeed(UMaterial* &Mat, float Speed) {
	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed * 1.5;
	BulletMeshCmp->SetMaterial(0, Mat);
}

// Called when the game starts or when spawned
void AMyBulletProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMyBulletProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMyBulletProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Only add impulse and destroy projectile if we hit a physics
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL) && OtherComp->IsSimulatingPhysics())
	{
		OtherComp->AddImpulseAtLocation(GetVelocity() * 100.0f, GetActorLocation());

		Destroy();
	}
}
