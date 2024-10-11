#pragma once

#include "CoreMinimal.h"
#include "PythonAPILibrary.h"
#include "UObject/Object.h"
#include "PythonYamlLibrary.generated.h"

/*
 Intended to be implemented in Python: Content/Python/unreal_yaml_api.py
 */


 /*
  Job .yaml struct
  */
UENUM(BlueprintType)
enum class EValueType : uint8
{
	INT UMETA(DisplayName = "Integer"),
	FLOAT   UMETA(DisplayName = "Float"),
	STRING UMETA(DisplayName = "String"),
	PATH    UMETA(DisplayName = "Path")
};

USTRUCT(BlueprintType)
struct FParameterDefinition
{
	GENERATED_BODY()

	// Name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Job")
	FString Name;
	// Type	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Job")
	EValueType Type;

	// String Value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Job")
	FString StringValue;
	// Path value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Job")
	FString PathValue;
	// Path value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Job")
	int32 IntValue;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Job")
	float FloatValue;

	FParameterDefinition()
		: Name("DefaultName"),
		 Type(EValueType::STRING),
		 IntValue(0),
		 FloatValue(0.f),
		 StringValue(""),
		 PathValue("")
	{}

//	void ChangeParameterStringValue( FString string)
//	{
//		StringValue = string;
//	};
};
/*
Step .yaml struct
 */

USTRUCT(BlueprintType)
struct FStepTaskParameterDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Step")
	FString Name;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Step")
	EValueType Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Step")
	TArray <FString> Range;

};

USTRUCT(BlueprintType)
struct FEnvVariable
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	FString Name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	FString Value;

};

/*
Env .yaml struct
 */



USTRUCT(BlueprintType)
struct FEnvironmentStruct
{
	GENERATED_BODY()

	// Name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	FString Name;

	// Value	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Step")
	TArray <FEnvVariable> Variables;
};





UCLASS()
class UNREALDEADLINECLOUDSERVICE_API UPythonYamlLibrary: public UObject, public TPythonAPILibraryBase<UPythonYamlLibrary>
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintImplementableEvent)
	FString ReadName(const FString& Path);

	// job
	UFUNCTION(BlueprintImplementableEvent)
	TArray <FParameterDefinition> OpenJobFile(const FString& Path);

	// steps 
	UFUNCTION(BlueprintImplementableEvent)
	TArray<FStepTaskParameterDefinition> OpenStepFile(const FString& Path);

	// env
	UFUNCTION(BlueprintImplementableEvent)
	TArray <FEnvironmentStruct> OpenEnvFile(const FString& Path);
};
	
