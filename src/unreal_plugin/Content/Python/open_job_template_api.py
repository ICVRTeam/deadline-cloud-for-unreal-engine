import yaml
import unreal
from typing import Any

from deadline.unreal_submitter.unreal_open_job import (
    UnrealOpenJobParameterDefinition,
    UnrealOpenJobStepParameterDefinition,
    ParametersConsistencyChecker,
    ParametersConsistencyCheckResult,
)


@unreal.uclass()
class PythonYamlLibraryImplementation(unreal.PythonYamlLibrary):

    @staticmethod
    def job_parameter_to_u_parameter_definition(
        job_parameter: dict[str, Any]
    ) -> unreal.ParameterDefinition:
        u_parameter_definition = unreal.ParameterDefinition()
        u_parameter_definition.name = job_parameter["name"]
        u_parameter_definition.type = getattr(unreal.ValueType, job_parameter["type"])

        if job_parameter.get("default") is not None:
            u_parameter_definition.value = str(job_parameter["default"])

        return u_parameter_definition

    @staticmethod
    def step_parameter_to_u_step_task_parameter(
        step_parameter: dict[str, str]
    ) -> unreal.StepTaskParameterDefinition:
        u_step_task_parameter_definition = unreal.StepTaskParameterDefinition()
        u_step_task_parameter_definition.name = step_parameter["name"]
        u_step_task_parameter_definition.type = getattr(unreal.ValueType, step_parameter["type"])
        u_step_task_parameter_definition.range = [str(v) for v in step_parameter["range"]]

        return u_step_task_parameter_definition

    @staticmethod
    def environment_to_u_environment(environment: dict[str, Any]) -> unreal.EnvironmentStruct:
        u_environment = unreal.EnvironmentStruct()
        u_environment.name = environment["name"]
        u_environment.description = environment.get("description", "")

        u_variables: list[unreal.EnvVariable] = []
        for k, v in environment.get("variables", {}).items():
            u_variable = unreal.EnvVariable()
            u_variable.name = k
            u_variable.value = v

            u_variables.append(u_variable.copy())

        u_environment.variables = u_variables

        return u_environment

    @unreal.ufunction(override=True)
    def open_job_file(self, path: str) -> list[unreal.ParameterDefinition]:
        with open(path, "r") as f:
            job_template = yaml.safe_load(f)

        u_parameter_definitions: list[unreal.ParameterDefinition] = []

        for parameter_definition in job_template["parameterDefinitions"]:
            u_param = PythonYamlLibraryImplementation.job_parameter_to_u_parameter_definition(
                parameter_definition
            )
            u_parameter_definitions.append(u_param.copy())

        return u_parameter_definitions

    @unreal.ufunction(override=True)
    def open_step_file(self, path: str) -> unreal.StepStruct:
        with open(path, "r") as f:
            step_template = yaml.safe_load(f)

        u_step_task_parameter_definitions: list[unreal.StepTaskParameterDefinition] = []

        for param_definition in step_template["parameterSpace"]["taskParameterDefinitions"]:
            u_param = PythonYamlLibraryImplementation.step_parameter_to_u_step_task_parameter(
                param_definition
            )
            u_step_task_parameter_definitions.append(u_param.copy())

        u_step_struct = unreal.StepStruct()
        u_step_struct.name = step_template["name"]
        u_step_struct.parameters = u_step_task_parameter_definitions

        return u_step_struct

    @unreal.ufunction(override=True)
    def open_env_file(self, path: str) -> unreal.EnvironmentStruct:
        with open(path, "r") as f:
            environment_template = yaml.safe_load(f)

        u_environment = PythonYamlLibraryImplementation.environment_to_u_environment(
            environment_template
        )

        return u_environment


@unreal.uclass()
class ParametersConsistencyCheckerImplementation(unreal.PythonParametersConsistencyChecker):

    @staticmethod
    def check_result_to_u_check_result(
        consistency_check_result: ParametersConsistencyCheckResult,
    ) -> unreal.ParametersConsistencyCheckResult:

        result = unreal.ParametersConsistencyCheckResult()
        result.passed = consistency_check_result.passed
        result.reason = consistency_check_result.reason
        return result

    @unreal.ufunction(override=True)
    def check_job_parameters_consistency(
        self, open_job: unreal.DeadlineCloudJob
    ) -> unreal.ParametersConsistencyCheckResult:

        result = ParametersConsistencyChecker.check_job_parameters_consistency(
            job_template_path=open_job.path_to_template.file_path,
            job_parameters=[
                UnrealOpenJobParameterDefinition.from_unreal_param_definition(param).to_dict()
                for param in open_job.get_job_parameters()
            ],
        )
        return ParametersConsistencyCheckerImplementation.check_result_to_u_check_result(result)

    @unreal.ufunction(override=True)
    def fix_job_parameters_consistency(self, open_job: unreal.DeadlineCloudJob):

        fixed_parameters = ParametersConsistencyChecker.fix_job_parameters_consistency(
            job_template_path=open_job.path_to_template.file_path,
            job_parameters=[
                UnrealOpenJobParameterDefinition.from_unreal_param_definition(param).to_dict()
                for param in open_job.get_job_parameters()
            ],
        )

        if fixed_parameters:
            open_job.set_job_parameters(
                [
                    PythonYamlLibraryImplementation.job_parameter_to_u_parameter_definition(fixed)
                    for fixed in fixed_parameters
                ]
            )

    @unreal.ufunction(override=True)
    def check_step_parameters_consistency(
        self, open_job_step: unreal.DeadlineCloudStep
    ) -> unreal.ParametersConsistencyCheckResult:

        result = ParametersConsistencyChecker.check_step_parameters_consistency(
            step_template_path=open_job_step.path_to_template.file_path,
            step_parameters=[
                UnrealOpenJobStepParameterDefinition.from_unreal_param_definition(param).to_dict()
                for param in open_job_step.get_step_parameters()
            ],
        )
        return ParametersConsistencyCheckerImplementation.check_result_to_u_check_result(result)

    @unreal.ufunction(override=True)
    def fix_step_parameters_consistency(self, open_job_step: unreal.DeadlineCloudStep):

        unreal.log("Fixing OpenJobStep parameters consistency ...")

        fixed_parameters = ParametersConsistencyChecker.fix_job_parameters_consistency(
            job_template_path=open_job_step.path_to_template.file_path,
            job_parameters=[
                UnrealOpenJobStepParameterDefinition.from_unreal_param_definition(param).to_dict()
                for param in open_job_step.get_step_parameters()
            ],
        )

        if fixed_parameters:
            open_job_step.set_step_parameters(
                [
                    PythonYamlLibraryImplementation.step_parameter_to_u_step_task_parameter(fixed)
                    for fixed in fixed_parameters
                ]
            )

    @unreal.ufunction(override=True)
    def check_environment_variables_consistency(
        self, open_job_environment: unreal.DeadlineCloudEnvironment
    ) -> unreal.ParametersConsistencyCheckResult:

        result = ParametersConsistencyChecker.check_environment_variables_consistency(
            environment_template_path=open_job_environment.path_to_template.file_path,
            environment_variables=open_job_environment.variables.get_editor_property("variables"),
        )
        return ParametersConsistencyCheckerImplementation.check_result_to_u_check_result(result)

    @unreal.ufunction(override=True)
    def fix_environment_variables_consistency(
        self, open_job_environment: unreal.DeadlineCloudEnvironment
    ):

        fixed_variables = ParametersConsistencyChecker.fix_environment_variables_consistency(
            environment_template_path=open_job_environment.path_to_template.file_path,
            environment_variables=open_job_environment.variables.get_editor_property("variables"),
        )
        if fixed_variables:
            open_job_environment.variables.set_editor_property("variables", fixed_variables)
