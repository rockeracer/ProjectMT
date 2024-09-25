#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Engine/DataAsset.h"
#include "NFAIMoveConfig.generated.h"

UENUM(BlueprintType)
enum class ENFAIMoveCurveType : uint8
{
	Speed,
	Location
};

USTRUCT(BlueprintType)
struct FNFAIMoveAnimPoint
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float StartTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float EndTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* AnimMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bLoop = false;
};

UCLASS (BlueprintType, BlueprintCallable, meta=(aaa, bbb, ccc))
class UNFAIMoveConfig : public UDataAsset
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UCurveFloat* MoveCurve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ENFAIMoveCurveType CurveType = ENFAIMoveCurveType::Speed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FNFAIMoveAnimPoint> AnimPoints;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<UCurveFloat*> Curves;

	UFUNCTION(BlueprintCallable)
	static void SetMoveFocus(AAIController* AIController, AActor* Focus);

	UFUNCTION(BlueprintCallable)
	static void ClearMoveFocus(AAIController* AIController);

	UFUNCTION(BlueprintCallable)
	void ReturnVoidFunction(AAIController* AIController);

	UFUNCTION(BlueprintCallable)
	FNFAIMoveAnimPoint ReturnValueFunction();

	UFUNCTION(BlueprintCallable)
	FNFAIMoveAnimPoint* ReturnPointerFunction(AAIController* AIController);

	UFUNCTION(BlueprintCallable)
	const FNFAIMoveAnimPoint ReturnConstValueFunction(AAIController* AIController);
public:
	UFUNCTION(BlueprintCallable)
	virtual void PureVirtualFunction(AAIController* AIController) = 0;

	UFUNCTION(BlueprintCallable)
	virtual int OverrideVirtualFunction(AAIController* AIController) const override;
};
