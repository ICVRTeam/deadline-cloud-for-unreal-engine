#pragma once

#include "PythonAPILibraries/PythonYamlLibrary.h"
#include "DeadlineCloudJobSettings/DeadlineCloudStep.h"
#include "DetailLayoutBuilder.h"
#include "IDetailCustomization.h"
#include "PropertyCustomizationHelpers.h"
#include "IPropertyTypeCustomization.h"

class UDeadlineCloudStep;

class FDeadlineCloudStepParametersArrayBuilder
	: public FDetailArrayBuilder
	, public TSharedFromThis<FDeadlineCloudStepParametersArrayBuilder>
{
public:

	static TSharedRef<FDeadlineCloudStepParametersArrayBuilder> MakeInstance(
		TSharedRef<IPropertyHandle> InPropertyHandle);

	FDeadlineCloudStepParametersArrayBuilder(
		TSharedRef<IPropertyHandle> InPropertyHandle);
	
	virtual void GenerateHeaderRowContent(FDetailWidgetRow& NodeRow) override;

	void GenerateWrapperStructHeaderRowContent(FDetailWidgetRow& NodeRow, TSharedRef<SWidget> NameContent);

	bool IsResetToDefaultVisible(TSharedPtr<IPropertyHandle> PropertyHandle, FString InParameterName) const;

	void ResetToDefaultHandler(TSharedPtr<IPropertyHandle> PropertyHandle, FString InParameterName) const;

	static UDeadlineCloudStep* GetOuterStep(TSharedRef<IPropertyHandle> Handle);

	FUIAction EmptyCopyPasteAction;
	FOnIsEnabled OnIsEnabled;

private:
	void OnGenerateEntry(TSharedRef<IPropertyHandle> ElementProperty, int32 ElementIndex, IDetailChildrenBuilder& ChildrenBuilder) const;


	TSharedPtr<IPropertyHandleArray> ArrayProperty;
};

class FDeadlineCloudStepParametersArrayCustomization : public IPropertyTypeCustomization
{
public:

	static TSharedRef<IPropertyTypeCustomization> MakeInstance()
	{
		return MakeShared<FDeadlineCloudStepParametersArrayCustomization>();
	}


	bool IsEnabled(TSharedRef<IPropertyHandle> InPropertyHandle) const;

	FDeadlineCloudStepParametersArrayCustomization() {}
	
	/** Begin IPropertyTypeCustomization interface */
	virtual void CustomizeHeader(
		TSharedRef<IPropertyHandle> InPropertyHandle,
		FDetailWidgetRow& InHeaderRow,
		IPropertyTypeCustomizationUtils& InCustomizationUtils) override;

	virtual void CustomizeChildren(
		TSharedRef<IPropertyHandle> InPropertyHandle,
		IDetailChildrenBuilder& InChildBuilder,
		IPropertyTypeCustomizationUtils& InCustomizationUtils) override;
	/** End IPropertyTypeCustomization interface */
	
private:

	TSharedPtr<FDeadlineCloudStepParametersArrayBuilder> ArrayBuilder;
};

class FDeadlineCloudStepParameterListBuilder
	: public FDetailArrayBuilder
	, public TSharedFromThis<FDeadlineCloudStepParameterListBuilder>
{
public:

	static TSharedRef<FDeadlineCloudStepParameterListBuilder> MakeInstance(
		TSharedRef<IPropertyHandle> InPropertyHandle, EValueType Type
	);

	FDeadlineCloudStepParameterListBuilder(
		TSharedRef<IPropertyHandle> InPropertyHandle);
	
	virtual void GenerateHeaderRowContent(FDetailWidgetRow& NodeRow) override;

	void GenerateWrapperStructHeaderRowContent(FDetailWidgetRow& NodeRow, TSharedRef<SWidget> NameContent);

	FUIAction EmptyCopyPasteAction;
	FOnIsEnabled OnIsEnabled;

private:
	void OnGenerateEntry(TSharedRef<IPropertyHandle> ElementProperty, int32 ElementIndex, IDetailChildrenBuilder& ChildrenBuilder) const;

	EValueType Type;
	TSharedPtr<IPropertyHandleArray> ArrayProperty;
};

class FDeadlineCloudStepParameterListCustomization : public IPropertyTypeCustomization
{
public:

	static TSharedRef<IPropertyTypeCustomization> MakeInstance()
	{
		return MakeShared<FDeadlineCloudStepParameterListCustomization>();
	}

	FDeadlineCloudStepParameterListCustomization() {}
	
	/** Begin IPropertyTypeCustomization interface */
	virtual void CustomizeHeader(
		TSharedRef<IPropertyHandle> InPropertyHandle,
		FDetailWidgetRow& InHeaderRow,
		IPropertyTypeCustomizationUtils& InCustomizationUtils) override;

	virtual void CustomizeChildren(
		TSharedRef<IPropertyHandle> InPropertyHandle,
		IDetailChildrenBuilder& InChildBuilder,
		IPropertyTypeCustomizationUtils& InCustomizationUtils) override;
	/** End IPropertyTypeCustomization interface */
	
private:

	TSharedPtr<FDeadlineCloudStepParameterListBuilder> ArrayBuilder;
};

class FDeadlineCloudStepDetails : public IDetailCustomization
{
private:
    TWeakObjectPtr<UDeadlineCloudStep> Settings;
    IDetailLayoutBuilder* MyDetailLayout;
public:
    static TSharedRef<IDetailCustomization> MakeInstance();
    virtual  void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	void OnButtonClicked();
	bool CheckConsistency(UDeadlineCloudStep* Step);
	bool bCheckConsistensyPassed = true;
	EVisibility GetWidgetVisibility() const	{ return (!bCheckConsistensyPassed) ? EVisibility::Visible : EVisibility::Collapsed;	}

	bool IsEnvironmentContainsErrors() const;
	EVisibility GetEnvironmentErrorWidgetVisibility() const;
	EVisibility GetEnvironmentDefaultWidgetVisibility() const;

private:
    TSharedRef<SWidget> GenerateStringsArrayContent(const TArray<FString>& StringArray);
    TSharedRef<SWidget> GenerateTasksContent(const TArray<FStepTaskParameterDefinition> tasks);

    void ForceRefreshDetails();
};