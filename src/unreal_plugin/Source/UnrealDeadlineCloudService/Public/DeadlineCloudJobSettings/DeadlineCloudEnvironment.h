#pragma once

#include "PythonAPILibraries/PythonYamlLibrary.h"
#include "DeadlineCloudEnvironment.generated.h"



UCLASS(BlueprintType, Blueprintable)
class UNREALDEADLINECLOUDSERVICE_API UDeadlineCloudEnvironment : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:

	UDeadlineCloudEnvironment();
	//Config,
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	FFilePath PathToTemplate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	FEnvironmentStruct EnvironmentStructure;

	/** Read path */
	UFUNCTION()
	TArray <FEnvironmentStruct> OpenEnvFile(const FString& Path);
};