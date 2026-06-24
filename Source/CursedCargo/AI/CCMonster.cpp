#include "AI/CCMonster.h"

#include "Characters/CCursedCargoCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

ACCMonster::ACCMonster()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    SetReplicateMovement(true);

    GetCapsuleComponent()->InitCapsuleSize(55.0f, 110.0f);
    GetCharacterMovement()->bRunPhysicsWithNoController = true;
    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;

    PrototypeBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PrototypeBody"));
    PrototypeBody->SetupAttachment(RootComponent);
    PrototypeBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    PrototypeBody->SetRelativeLocation(FVector(0.0f, 0.0f, -10.0f));
    PrototypeBody->SetRelativeScale3D(FVector(0.8f, 0.8f, 1.8f));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeMesh(
        TEXT("/Engine/BasicShapes/Cone.Cone"));
    if (ConeMesh.Succeeded())
    {
        PrototypeBody->SetStaticMesh(ConeMesh.Object);
    }
}

void ACCMonster::BeginPlay()
{
    Super::BeginPlay();

    SpawnOrigin = GetActorLocation();
    if (HasAuthority())
    {
        SelectPatrolTarget();
    }
}

void ACCMonster::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!HasAuthority())
    {
        return;
    }

    if (MonsterState == ECCMonsterState::Chasing)
    {
        ChaseTimeRemaining -= DeltaSeconds;
        if (!IsValid(ChaseTarget) || ChaseTimeRemaining <= 0.0f)
        {
            StopChasing();
            return;
        }

        MoveToward(ChaseTarget->GetActorLocation(), ChaseSpeed, DeltaSeconds);

        if (FVector::DistSquared2D(GetActorLocation(), ChaseTarget->GetActorLocation()) <= FMath::Square(135.0f))
        {
            if (ACCursedCargoCharacter* Character = Cast<ACCursedCargoCharacter>(ChaseTarget))
            {
                Character->ReceiveMonsterHit();
            }
        }
        return;
    }

    const FVector ToTarget = PatrolTarget - GetActorLocation();
    if (ToTarget.SizeSquared2D() <= FMath::Square(80.0f))
    {
        PatrolPauseRemaining -= DeltaSeconds;
        if (PatrolPauseRemaining <= 0.0f)
        {
            SelectPatrolTarget();
        }
        return;
    }

    MoveToward(PatrolTarget, PatrolSpeed, DeltaSeconds);
}

void ACCMonster::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ACCMonster, MonsterState);
    DOREPLIFETIME(ACCMonster, ChaseTarget);
}

void ACCMonster::ReactToNoise(const FVector& NoiseLocation, float Loudness, AActor* NoiseInstigator)
{
    if (!HasAuthority())
    {
        return;
    }

    const float EffectiveRadius = HearingRadius * FMath::Max(0.1f, Loudness);
    if (FVector::DistSquared(GetActorLocation(), NoiseLocation) > FMath::Square(EffectiveRadius))
    {
        return;
    }

    const bool bWasAlreadyChasing = MonsterState == ECCMonsterState::Chasing;
    ChaseTarget = NoiseInstigator;
    if (!IsValid(ChaseTarget))
    {
        float BestDistanceSquared = TNumericLimits<float>::Max();
        for (TActorIterator<ACCursedCargoCharacter> It(GetWorld()); It; ++It)
        {
            const float DistanceSquared = FVector::DistSquared(It->GetActorLocation(), NoiseLocation);
            if (DistanceSquared < BestDistanceSquared)
            {
                BestDistanceSquared = DistanceSquared;
                ChaseTarget = *It;
            }
        }
    }

    if (!IsValid(ChaseTarget))
    {
        return;
    }

    MonsterState = ECCMonsterState::Chasing;
    ChaseTimeRemaining = ChaseDuration;
    GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed;
    ForceNetUpdate();
    OnMonsterStateChanged(MonsterState);

    if (GEngine && !bWasAlreadyChasing)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            3.0f,
            FColor::Red,
            TEXT("THE MONSTER HEARD THE CARGO!"));
    }
}

void ACCMonster::OnRep_MonsterState()
{
    OnMonsterStateChanged(MonsterState);
}

void ACCMonster::SelectPatrolTarget()
{
    const FVector2D RandomDirection = FMath::RandPointInCircle(PatrolRadius);
    PatrolTarget = SpawnOrigin + FVector(RandomDirection.X, RandomDirection.Y, 0.0f);
    PatrolPauseRemaining = FMath::FRandRange(0.5f, 1.5f);
}

void ACCMonster::StopChasing()
{
    ChaseTarget = nullptr;
    MonsterState = ECCMonsterState::Patrolling;
    GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
    SelectPatrolTarget();
    ForceNetUpdate();
    OnMonsterStateChanged(MonsterState);
}

void ACCMonster::MoveToward(const FVector& Destination, float Speed, float DeltaSeconds)
{
    FVector Direction = Destination - GetActorLocation();
    Direction.Z = 0.0f;
    if (!Direction.Normalize())
    {
        return;
    }

    GetCharacterMovement()->MaxWalkSpeed = Speed;
    AddMovementInput(Direction, 1.0f, true);

    const FRotator DesiredRotation = Direction.Rotation();
    SetActorRotation(FMath::RInterpTo(GetActorRotation(), DesiredRotation, DeltaSeconds, 6.0f));
}
