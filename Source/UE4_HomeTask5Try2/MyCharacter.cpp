// Fill out your copyright notice in the Description page of Project Settings.

#include "MyCharacter.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Pawn.h"
//#include <InputComponent.h>


// Sets default values
AMyCharacter::AMyCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	USkeletalMeshComponent *SkMesh = GetMesh();

	AutoPossessPlayer = EAutoReceiveInput::Player0;
	GetCapsuleComponent()->InitCapsuleSize(42, 96);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->JumpZVelocity = 400;
	GetCharacterMovement()->AirControl = 0;
	GetCharacterMovement()->RotationRate = FRotator(0, 700, 0);
	
	// SkeletalMesh'/Game/MyCharacter/SK_MyMannequin.SK_MyMannequin'
		ConstructorHelpers::FObjectFinder<USkeletalMesh> mesh(TEXT("SkeletalMesh'/Game/AnimStarterPack/UE4_Mannequin/Mesh/SK_Mannequin.SK_Mannequin'"));
	if (mesh.Object) {
		SkMesh->SetSkeletalMesh(mesh.Object);
		SkMesh->SetRelativeLocation(FVector(0, 0, -98));
		SkMesh->SetRelativeRotation(FRotator(0, 270, 0));
	}

	SpringArm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->TargetArmLength = 500;
	SpringArm->bDoCollisionTest = false;
	SpringArm->SetupAttachment(RootComponent);

	CameraFollow = CreateDefaultSubobject<UCameraComponent>("CameraFollow");
	CameraFollow->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon");
	ConstructorHelpers::FObjectFinder<USkeletalMesh> gunMesh(TEXT("SkeletalMesh'/Game/Weapon/SK_MyFPGun.SK_MyFPGun'"));
	if (gunMesh.Object) {
		(Cast<USkeletalMeshComponent>(Weapon))->SetSkeletalMesh(gunMesh.Object);
		(Cast<USceneComponent>(Weapon))->SetRelativeRotation(FRotator(158, 36, 345)); // x,y,z: 345, 158, 36
		(Cast<USceneComponent>(Weapon))->SetupAttachment(Cast<USceneComponent>(SkMesh), ForGunSocket);
		(Cast<USceneComponent>(Weapon))->SetVisibility(bWeaponVisible);
	}

	GunOffset = FVector(100.0f, 0.0f, 10.0f);
	ProjectileClass = AMyBulletProjectile::StaticClass();
	
	ConstructorHelpers::FObjectFinder<UAnimBlueprint> MyCharacterBPAnim(TEXT("AnimBlueprint'/Game/MyCharacter/Animations/BP_MyCharacter.BP_MyCharacter'"));
	if (MyCharacterBPAnim.Object) {
		SkMesh->SetAnimInstanceClass(MyCharacterBPAnim.Object->GetAnimBlueprintGeneratedClass());
	}
}
	

// Called when the game starts or when spawned
void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();
	Cast<USceneComponent>(Weapon)->SetVisibility(bWeaponVisible);
}

// Called every frame
void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!Velocity.IsZero())
		SetActorLocation(GetActorLocation() + Velocity * DeltaTime);
}

// Called to bind functionality to input
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis(TEXT("MoveForward"),this, &AMyCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AMyCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AMyCharacter::Turn);
	PlayerInputComponent->BindAxis(TEXT("Lookup"), this, &AMyCharacter::Lookup);
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &AMyCharacter::Jump);
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Released, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction(TEXT("Fire"), IE_Pressed, this, &AMyCharacter::StartFire);
	PlayerInputComponent->BindAction(TEXT("Fire"), IE_Released, this, &AMyCharacter::EndFire);
	PlayerInputComponent->BindAction(TEXT("SwitchWeapon"), IE_Pressed, this, &AMyCharacter::SwitchWeapon);
	PlayerInputComponent->BindAction(TEXT("SwitchAmmo"), IE_Pressed, this, &AMyCharacter::SwitchAmmo);
}

void AMyCharacter::MoveForward(float AxisValue) {
	if (Controller != nullptr) {
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, AxisValue);

	}
}

void AMyCharacter::MoveRight(float AxisValue) {
	if (Controller != nullptr && AxisValue) {
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, AxisValue);
	}
}

void AMyCharacter::Turn(float AxisValue) {
	/*GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Cyan, FString("Turn: ") + FString::SanitizeFloat(AxisValue));*/
	AddControllerYawInput(TurnRate * AxisValue * GetWorld()->GetDeltaSeconds());
}

void AMyCharacter::Lookup(float AxisValue) {
	/*GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Cyan, FString("Lookup: ") + FString::SanitizeFloat(AxisValue));*/
	AddControllerPitchInput(LookUpRate * AxisValue * GetWorld()->GetDeltaSeconds());
}

void AMyCharacter::ShootBullet() {
	UWorld* World = GetWorld();
	if (isFire && ProjectileClass && World) {
		const FRotator SpawnRotation = GetControlRotation();
		const FVector SpawnLocation = GetActorLocation() + SpawnRotation.RotateVector(GunOffset);
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		auto Bullet = World->SpawnActor<AMyBulletProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
		Bullet->SetMeshScale(BulletScale);
		Bullet->SetGravity(BulletGravity);
		Bullet->SetBulletType(static_cast<TBulletTypes>(CurAmmo));

		FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, true);
		Bullet->AttachToComponent(Weapon, TransformRules, GunAmmoSocket);

		FRotator Rotation = GetActorRotation();
		Bullet->ProjectileMovement->Velocity =
			1000 * FVector(FMath::Cos(FMath::DegreesToRadians(Rotation.Yaw)),
				FMath::Sin(FMath::DegreesToRadians(Rotation.Yaw)), 0);
		FDetachmentTransformRules DetachTransformRules(EDetachmentRule::KeepWorld, true);
		Bullet->DetachFromActor(DetachTransformRules);

		FTimerHandle Timer;
		GetWorldTimerManager().SetTimer(Timer, this, &AMyCharacter::ShootBullet, 0.1, false);
	}
}

void AMyCharacter::StartFire() {
	if (bWeaponVisible) {
		isFire = true;
		ShootBullet();
	}
}

void AMyCharacter::EndFire() {
	isFire = false;
}

void AMyCharacter::SwitchWeapon() {
	bWeaponVisible = !bWeaponVisible;
	Weapon->SetVisibility(bWeaponVisible);
}

void AMyCharacter::SwitchAmmo() {
	static TBulletTypes BulletTypes[] = { EBulletLight, EBulletStandard, EBulletHard };
	SwitchAmmoCurIdx = (++SwitchAmmoCurIdx) % 3;
	CurAmmo = BulletTypes[SwitchAmmoCurIdx];
	return;
}