#include "DeadlineCloudJobSettings/DeadlineCloudStepDetails.h"
#include "DeadlineCloudJobSettings/DeadlineCloudJobDetails.h"
#include "DeadlineCloudJobSettings/DeadlineCloudStep.h"
#include "PropertyEditorModule.h"
#include "Modules/ModuleManager.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "DesktopPlatformModule.h"
#include "Widgets/Input/SFilePathPicker.h"
#include "IDetailChildrenBuilder.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "PythonAPILibraries/PythonParametersConsistencyChecker.h"
#include "DeadlineCloudJobSettings/DeadlineCloudDetailsWidgetsHelper.h"

#define LOCTEXT_NAMESPACE "StepDetails"



bool FDeadlineCloudStepDetails::CheckConsistency(UDeadlineCloudStep* Step)
{
	FParametersConsistencyCheckResult result;
	result = Step->CheckStepParametersConsistency(Step);

	UE_LOG(LogTemp, Warning, TEXT("Check consistency result: %s"), *result.Reason);
	return result.Passed;
}

void FDeadlineCloudStepDetails::OnButtonClicked()
{
	Settings->FixStepParametersConsistency(Settings.Get());
	UE_LOG(LogTemp, Warning, TEXT("FixStepParametersConsistency"));
	ForceRefreshDetails();
}

void FDeadlineCloudStepDetails::ForceRefreshDetails()
{
    MyDetailLayout->ForceRefreshDetails();
}

/*Details*/
TSharedRef<IDetailCustomization> FDeadlineCloudStepDetails::MakeInstance()
{
    return MakeShareable(new FDeadlineCloudStepDetails);
}

void FDeadlineCloudStepDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	MyDetailLayout = &DetailBuilder;
    TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
    DetailBuilder.GetObjectsBeingCustomized(ObjectsBeingCustomized);
    Settings = Cast<UDeadlineCloudStep>(ObjectsBeingCustomized[0].Get());

	TSharedPtr<FDeadlineCloudDetailsWidgetsHelper::SConsistencyWidget> ConsistencyUpdateWidget;
	FParametersConsistencyCheckResult result;

	/* Consistency check */
	if (Settings.IsValid() && Settings->GetStepParameters().Num() > 0)
	{
		UDeadlineCloudStep* MyObject = Settings.Get();
		bCheckConsistensyPassed = CheckConsistency(MyObject);
	}

	/* If passed - Open job file*/
	if (bCheckConsistensyPassed || Settings->GetStepParameters().Num() == 0)
	{
		Settings->OpenStepFile(Settings->PathToTemplate.FilePath);
	}

	IDetailCategoryBuilder& PropertiesCategory = MyDetailLayout->EditCategory("Parameters");

	PropertiesCategory.AddCustomRow(FText::FromString("Consistency"))
		.Visibility(TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateSP(this, &FDeadlineCloudStepDetails::GetWidgetVisibility)))
		.WholeRowContent()
		[
			SAssignNew(ConsistencyUpdateWidget, FDeadlineCloudDetailsWidgetsHelper::SConsistencyWidget)
				.OnFixButtonClicked(FSimpleDelegate::CreateSP(this, &FDeadlineCloudStepDetails::OnButtonClicked))
		];

    if (Settings.IsValid() && (MyDetailLayout != nullptr))
    {
        Settings->OnSomethingChanged = FSimpleDelegate::CreateSP(this, &FDeadlineCloudStepDetails::ForceRefreshDetails);
    };
}

bool FDeadlineCloudStepDetails::IsEnvironmentContainsErrors() const
{
	TArray<UObject*> ExistingEnvironment;
	for (auto Environment : Settings->Environments)
	{
		if (!IsValid(Environment) || ExistingEnvironment.Contains(Environment))
		{
			return true;
		}

		ExistingEnvironment.Add(Environment);
	}

	return false;
}

EVisibility FDeadlineCloudStepDetails::GetEnvironmentErrorWidgetVisibility() const
{
	return IsEnvironmentContainsErrors() ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility FDeadlineCloudStepDetails::GetEnvironmentDefaultWidgetVisibility() const
{
	return IsEnvironmentContainsErrors() ? EVisibility::Collapsed : EVisibility::Visible;
}

void FDeadlineCloudStepParametersArrayCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> InPropertyHandle, FDetailWidgetRow& InHeaderRow, IPropertyTypeCustomizationUtils& InCustomizationUtils)
{
	const TSharedPtr<IPropertyHandle> ArrayHandle = InPropertyHandle->GetChildHandle("Parameters", false);
	ArrayBuilder = FDeadlineCloudStepParametersArrayBuilder::MakeInstance(ArrayHandle.ToSharedRef());

	auto OuterStep = FDeadlineCloudStepParametersArrayBuilder::GetOuterStep(InPropertyHandle);
	if (IsValid(OuterStep))
	{
		ArrayBuilder->OnIsEnabled.BindSPLambda(this, [OuterStep]()->bool
			{
				return !OuterStep->TaskParameterDefinitions.Parameters.IsEmpty();
			});
	}

    ArrayBuilder->GenerateWrapperStructHeaderRowContent(InHeaderRow, InPropertyHandle->CreatePropertyNameWidget());
}

void FDeadlineCloudStepParametersArrayCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> InPropertyHandle, IDetailChildrenBuilder& InChildBuilder, IPropertyTypeCustomizationUtils& InCustomizationUtils)
{
    InChildBuilder.AddCustomBuilder(ArrayBuilder.ToSharedRef());
}

UDeadlineCloudStep* FDeadlineCloudStepParametersArrayBuilder::GetOuterStep(TSharedRef<IPropertyHandle> Handle)
{
	TArray<UObject*> OuterObjects;
	Handle->GetOuterObjects(OuterObjects);

	if (OuterObjects.Num() == 0)
	{
		return nullptr;
	}

	const TWeakObjectPtr<UObject> OuterObject = OuterObjects[0];
	if (!OuterObject.IsValid())
	{
		return nullptr;
	}
	UDeadlineCloudStep* OuterStep = Cast<UDeadlineCloudStep>(OuterObject);
	return OuterStep;
}

TSharedRef<FDeadlineCloudStepParametersArrayBuilder> FDeadlineCloudStepParametersArrayBuilder::MakeInstance(TSharedRef<IPropertyHandle> InPropertyHandle)
{
	TSharedRef<FDeadlineCloudStepParametersArrayBuilder> Builder =
		MakeShared<FDeadlineCloudStepParametersArrayBuilder>(InPropertyHandle);
	
	Builder->OnGenerateArrayElementWidget(
		FOnGenerateArrayElementWidget::CreateSP(Builder, &FDeadlineCloudStepParametersArrayBuilder::OnGenerateEntry));
	return Builder;
}

FDeadlineCloudStepParametersArrayBuilder::FDeadlineCloudStepParametersArrayBuilder(TSharedRef<IPropertyHandle> InPropertyHandle)
    : FDetailArrayBuilder(InPropertyHandle, false, false, true),
		ArrayProperty(InPropertyHandle->AsArray())
{

}

void FDeadlineCloudStepParametersArrayBuilder::GenerateHeaderRowContent(FDetailWidgetRow& NodeRow)
{
}

void FDeadlineCloudStepParametersArrayBuilder::GenerateWrapperStructHeaderRowContent(FDetailWidgetRow& NodeRow, TSharedRef<SWidget> NameContent)
{
	FDetailArrayBuilder::GenerateHeaderRowContent(NodeRow);

	EmptyCopyPasteAction = FUIAction(
		FExecuteAction::CreateLambda([]() {}),
		FCanExecuteAction::CreateLambda([]() { return false; })
	);

	NodeRow.CopyAction(EmptyCopyPasteAction);
	NodeRow.PasteAction(EmptyCopyPasteAction);

	NodeRow.OverrideResetToDefault(FResetToDefaultOverride::Create(TAttribute<bool>(false)));

	NodeRow.ValueContent()
	.HAlign( HAlign_Left )
	.VAlign( VAlign_Center )
	.MinDesiredWidth(170.f)
	.MaxDesiredWidth(170.f);

	NodeRow.NameContent()
	[
		NameContent
	];

	NodeRow.IsEnabled(TAttribute<bool>::CreateLambda([this]()
		{
			if (OnIsEnabled.IsBound())
				return OnIsEnabled.Execute();
			return true;
		})
	);
}

void FDeadlineCloudStepParametersArrayBuilder::OnGenerateEntry(TSharedRef<IPropertyHandle> ElementProperty, int32 ElementIndex, IDetailChildrenBuilder& ChildrenBuilder) const
{
	IDetailPropertyRow& PropertyRow = ChildrenBuilder.AddProperty(ElementProperty);

	const TSharedPtr<IPropertyHandle> NameHandle = ElementProperty->GetChildHandle("Name", false);
	if (!NameHandle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("FDeadlineCloudStepParametersArrayBuilder Name handle is not valid"));
		return;
	}

    FString ParameterName;
	NameHandle->GetValue(ParameterName);

	auto OuterStep = FDeadlineCloudStepParametersArrayBuilder::GetOuterStep(ElementProperty);
	if (IsValid(OuterStep))
	{
        const FResetToDefaultOverride ResetDefaultOverride = FResetToDefaultOverride::Create(
			FIsResetToDefaultVisible::CreateSPLambda(this, [this, OuterStep, ParameterName](TSharedPtr<IPropertyHandle> PropertyHandle)->bool 
                { 
                    if (!PropertyHandle.IsValid())
                    {
                        return false;
                    }

					if (!IsValid(OuterStep))
					{
                        return false;
					}
                          
                    return !OuterStep->IsParameterArrayDefault(ParameterName); 
                }),
            FResetToDefaultHandler::CreateSPLambda(this, [this, ParameterName, OuterStep](TSharedPtr<IPropertyHandle> PropertyHandle) 
                {
                    if (!PropertyHandle.IsValid())
                    {
                        return;
                    }

					if (!IsValid(OuterStep))
					{
                        return;
					}

					OuterStep->ResetParameterArray(ParameterName);
                })
        );
		PropertyRow.OverrideResetToDefault(ResetDefaultOverride);
	}
	else
	{
		// Hide the reset to default button since it provides little value
		const FResetToDefaultOverride ResetDefaultOverride = FResetToDefaultOverride::Create(TAttribute<bool>(false));
		PropertyRow.OverrideResetToDefault(ResetDefaultOverride);
	}

	PropertyRow.ShowPropertyButtons(false);

	TSharedPtr<SWidget> NameWidget;
	TSharedPtr<SWidget> ValueWidget;

	PropertyRow.GetDefaultWidgets( NameWidget, ValueWidget);

	PropertyRow.CustomWidget(true)
	.CopyAction(EmptyCopyPasteAction)
	.PasteAction(EmptyCopyPasteAction)
	.NameContent()
	.HAlign(HAlign_Fill)
	[
        SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
		    .Padding( FMargin( 0.0f, 1.0f, 0.0f, 1.0f) )
		    .FillWidth(1)
            [
                SNew(STextBlock)
                    .Text(FText::FromString(ParameterName))
				    .Font(IDetailLayoutBuilder::GetDetailFont())
                    .ColorAndOpacity(FSlateColor::UseForeground())
            ]	
	]
	.ValueContent()
	.HAlign(HAlign_Fill)
	[
		ValueWidget.ToSharedRef()
	];
	ValueWidget.ToSharedRef()->SetEnabled(
		TAttribute<bool>::CreateLambda([this]()
		{
			if (OnIsEnabled.IsBound())
				return OnIsEnabled.Execute();
			return true;
		})
	);
}

TSharedRef<FDeadlineCloudStepParameterListBuilder> FDeadlineCloudStepParameterListBuilder::MakeInstance(TSharedRef<IPropertyHandle> InPropertyHandle, EValueType Type)
{
	TSharedRef<FDeadlineCloudStepParameterListBuilder> Builder =
		MakeShared<FDeadlineCloudStepParameterListBuilder>(InPropertyHandle);

	Builder->Type = Type;

	Builder->OnGenerateArrayElementWidget(
		FOnGenerateArrayElementWidget::CreateSP(Builder, &FDeadlineCloudStepParameterListBuilder::OnGenerateEntry));
	return Builder;
}

FDeadlineCloudStepParameterListBuilder::FDeadlineCloudStepParameterListBuilder(TSharedRef<IPropertyHandle> InPropertyHandle)
    : FDetailArrayBuilder(InPropertyHandle, true, false, true),
		ArrayProperty(InPropertyHandle->AsArray())
{
}

void FDeadlineCloudStepParameterListBuilder::GenerateHeaderRowContent(FDetailWidgetRow& NodeRow)
{
}

void FDeadlineCloudStepParameterListBuilder::GenerateWrapperStructHeaderRowContent(FDetailWidgetRow& NodeRow, TSharedRef<SWidget> NameContent)
{
	FDetailArrayBuilder::GenerateHeaderRowContent(NodeRow);

	EmptyCopyPasteAction = FUIAction(
		FExecuteAction::CreateLambda([]() {}),
		FCanExecuteAction::CreateLambda([]() { return false; })
	);

	NodeRow.CopyAction(EmptyCopyPasteAction);
	NodeRow.PasteAction(EmptyCopyPasteAction);

	NodeRow.ValueContent()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.MinDesiredWidth(170.f)
		.MaxDesiredWidth(170.f);


	NodeRow.NameContent()
	[
		NameContent
	];

	NodeRow.IsEnabled(TAttribute<bool>::CreateLambda([this]()
		{
			if (OnIsEnabled.IsBound())
				return OnIsEnabled.Execute();
			return true;
		})
	);
}

void FDeadlineCloudStepParameterListBuilder::OnGenerateEntry(TSharedRef<IPropertyHandle> ElementProperty, int32 ElementIndex, IDetailChildrenBuilder& ChildrenBuilder) const
{
    IDetailPropertyRow& PropertyRow = ChildrenBuilder.AddProperty(ElementProperty);

	// Hide the reset to default button since it provides little value
	const FResetToDefaultOverride ResetDefaultOverride =
		FResetToDefaultOverride::Create(TAttribute<bool>(false));

	PropertyRow.OverrideResetToDefault(ResetDefaultOverride);
	PropertyRow.ShowPropertyButtons(true);

	TSharedPtr<SWidget> NameWidget;
	TSharedPtr<SWidget> ValueWidget;

	PropertyRow.GetDefaultWidgets( NameWidget, ValueWidget);

	PropertyRow.CustomWidget(true)
		.CopyAction(EmptyCopyPasteAction)
		.PasteAction(EmptyCopyPasteAction)
		.NameContent()
		.HAlign(HAlign_Fill)
		[
			NameWidget.ToSharedRef()
		]
		.ValueContent()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		[	
			FDeadlineCloudDetailsWidgetsHelper::CreatePropertyWidgetByType(ElementProperty, Type)
		];

	ValueWidget.ToSharedRef()->SetEnabled(
		TAttribute<bool>::CreateLambda([this]()
		{
			if (OnIsEnabled.IsBound())
				return OnIsEnabled.Execute();
			return true;
		})
	);
}

void FDeadlineCloudStepParameterListCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> InPropertyHandle, FDetailWidgetRow& InHeaderRow, IPropertyTypeCustomizationUtils& InCustomizationUtils)
{
    TSharedPtr<IPropertyHandle> ArrayHandle = InPropertyHandle->GetChildHandle("Range", false);

	const TSharedPtr<IPropertyHandle> TypeHandle = InPropertyHandle->GetChildHandle("Type", false);

	if (!TypeHandle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("FDeadlineCloudStepParameterListBuilder Type handle is not valid"));
		return;
	}

	uint8 TypeValue;
	TypeHandle->GetValue(TypeValue);

	EValueType Type = (EValueType)TypeValue;

	ArrayBuilder = FDeadlineCloudStepParameterListBuilder::MakeInstance(ArrayHandle.ToSharedRef(), Type);

    ArrayBuilder->GenerateWrapperStructHeaderRowContent(InHeaderRow, InPropertyHandle->CreatePropertyNameWidget());
}

void FDeadlineCloudStepParameterListCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> InPropertyHandle, IDetailChildrenBuilder& InChildBuilder, IPropertyTypeCustomizationUtils& InCustomizationUtils)
{
    InChildBuilder.AddCustomBuilder(ArrayBuilder.ToSharedRef());
}

#undef LOCTEXT_NAMESPACE
