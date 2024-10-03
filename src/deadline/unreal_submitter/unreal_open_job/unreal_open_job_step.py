import math
import unreal
from typing import Any

from openjd.model.v2023_09 import *
from openjd.model.v2023_09._model import StepDependency

from deadline.unreal_submitter.unreal_open_job.unreal_open_job_entity import UnrealOpenJobEntity
from deadline.unreal_submitter.unreal_open_job.unreal_open_job_environment import UnrealOpenJobEnvironment


class HostRequirements:
    """OpenJob host requirements representation"""

    def __init__(self, host_requirements):
        self.source_host_requirements = host_requirements
        self.requirements: dict = {}

        os_requirements = self._get_os_requirements()
        if os_requirements:
            # OS requirements are currently all amount type capabilities
            self.requirements["attributes"] = os_requirements

        hardware_requirements = self._get_hardware_requirements()
        if hardware_requirements:
            # hardware requirements are currently all amount
            self.requirements["amounts"] = hardware_requirements

    def _get_os_requirements(self) -> list[dict]:
        """
        Get requirements for OS family and CPU architecture

        :return: list of the OS requirements
        :rtype: list[dict]
        """
        requirements: list[dict] = []
        if self.source_host_requirements.operating_system:
            requirements.append(
                {
                    "name": "attr.worker.os.family",
                    "anyOf": [self.source_host_requirements.operating_system],
                }
            )
        if self.source_host_requirements.cpu_architecture:
            requirements.append(
                {
                    "name": "attr.worker.cpu.arch",
                    "anyOf": [self.source_host_requirements.cpu_architecture],
                }
            )
        return requirements

    def _get_hardware_requirements(self) -> list[dict[str, Any]]:
        """
        Get requirements for cpu, gpu and memory limits

        :return: list of the OS requirements
        :rtype: list[dict]
        """
        cpus = self._get_amount_requirement(
            self.source_host_requirements.cp_us, "amount.worker.vcpu"
        )
        memory = self._get_amount_requirement(
            self.source_host_requirements.memory, "amount.worker.memory", 1024
        )
        # TODO gpu amount
        # gpus = self._get_amount_requirement(self.source_host_requirements.gpus, "amount.worker.gpu")
        gpu_memory = self._get_amount_requirement(
            self.source_host_requirements.gp_us, "amount.worker.gpu.memory", 1024
        )
        scratch_space = self._get_amount_requirement(
            self.source_host_requirements.scratch_space, "amount.worker.disk.scratch"
        )
        requirements: list[dict[str, Any]] = [
            item for item in [cpus, memory, gpu_memory, scratch_space] if item is not None
        ]
        return requirements

    @staticmethod
    def _get_amount_requirement(source_interval, name: str, scaling_factor: int = 1) -> dict:
        """
        Helper method to get the amount of Host Requirement setting interval

        :param source_interval: Interval unreal setting
        :param name: AWS HostRequirements setting name
        :param scaling_factor: Multiplier number by which to scale the source_interval values

        :return: Amount requirement as dictionary
        :rtype: dict
        """
        requirement = {}
        if source_interval.min > 0 or source_interval.max > 0:
            requirement = {"name": name}
            if source_interval.min > 0:
                requirement["min"] = source_interval.min * scaling_factor
            if source_interval.max > 0:
                requirement["max"] = source_interval.max * scaling_factor
        return requirement

    def as_dict(self) -> dict:
        """
        Returns the HostRequirements as dictionary

        :return: Host Requirements as dictionary
        :rtype: dict
        """
        return self.requirements


class UnrealOpenJobStep(UnrealOpenJobEntity):
    """
    Unreal Open Job Step entity
    """
    
    param_type_map = {
        'INT': IntTaskParameterDefinition,
        'FLOAT': FloatTaskParameterDefinition,
        'STRING': StringTaskParameterDefinition,
        'PATH': PathTaskParameterDefinition
    }

    def __init__(
            self,
            file_path: str,
            name: str = None,
            step_dependencies: list[str] = None,
            environments: list = None,
            extra_parameters=None,
            host_requirements=None
    ):
        """
        :param file_path: The file path of the step descriptor
        :type file_path: str
        
        :param name: The name of the step
        :type name: str
        
        :param step_dependencies: The list of step dependencies
        :type step_dependencies: list[str]
        
        :param environments: The list of environments
        :type environments: list
        """
        
        self._step_dependencies = step_dependencies or []

        self._environments = [
            UnrealOpenJobEnvironment(file_path=environment.file_path, name=environment.name)
            for environment in environments
        ]

        # TODO - Santi: Ask about this
        self._extra_parameters = extra_parameters or []

        self._host_requirements = host_requirements

        super().__init__(StepTemplate, file_path, name)

    @classmethod
    def from_data_asset(cls, data_asset: unreal.DeadlineCloudStep, host_requirements=None) -> "UnrealOpenJobStep":
        return cls(
            file_path=data_asset.path_to_template,
            name=data_asset.name,
            step_dependencies=[data_asset.depends_on],  # TODO depends_on should be list of steps
            environments=[UnrealOpenJobEnvironment.from_data_asset(env) for env in data_asset.environments],
            host_requirements=host_requirements
        )

    def _build_step_parameter_definition_list(self) -> list:
        """
        Build the job parameter definition list from the step template object.
        """
        
        step_template_object = self.get_template_object()

        step_parameter_definition_list = JobParameterDefinitionList()

        params = step_template_object['parameterSpace']['taskParameterDefinitions']
        for param in params:
            param['range'] = ['TestValue1', 'TestValue2', 'TestValue3']  # TODO Slate object bug: values not saved
            override_param = (
                next((p for p in self._extra_parameters if p.get('name', '') == param['name']), None)
                if self._extra_parameters else None
            )
            if override_param:
                param.update(override_param)

            param_definition_cls = self.param_type_map.get(param['type'])
            if param_definition_cls:
                step_parameter_definition_list.append(
                    param_definition_cls(**param)
                )

        return step_parameter_definition_list
        
    def build_template(self) -> StepTemplate:
        step_template_object = self.get_template_object()

        step_parameters = self._build_step_parameter_definition_list()

        host_requirements = HostRequirements(host_requirements=self._host_requirements)

        return self.template_class(
            name=self.name,
            script=StepScript(**step_template_object['script']),
            parameterSpace=StepParameterSpaceDefinition(
                taskParameterDefinitions=step_parameters
            ) if step_parameters else None,
            stepEnvironments=[env.build_template() for env in self._environments] if self._environments else None,
            dependencies=[
                StepDependency(dependsOn=step_dependency)
                for step_dependency in self._step_dependencies
            ] if all(self._step_dependencies) else None,
            # TODO check host requirements validation issue
            hostRequirements=None  # HostRequirementsTemplate(**(host_requirements.as_dict()))
        )


class RenderUnrealOpenJobStep(UnrealOpenJobStep):
    """
    Unreal Open Job Render Step entity
    """

    def __init__(
            self,
            file_path: str,
            name: str = None,
            step_dependencies: list[str] = None,
            environments: list = None,
            extra_parameters: list = None,
            host_requirements=None,
            task_chunk_size: int = None,
            shots_count: int = None
    ):
        """
        :param file_path: The file path of the step descriptor
        :type file_path: str
        
        :param name: The name of the step
        :type name: str
        
        :param step_dependencies: The list of step dependencies
        :type step_dependencies: list[str]
        
        :param environments: The list of environments
        :type environments: list
        
        :param extra_parameters: The list of extra parameters
        :type extra_parameters: list
        
        :param task_chunk_size: The task chunk size
        :type task_chunk_size: int
        
        :param shots_count: The shots count
        :type shots_count: int
        """
        
        self._task_chunk_size = task_chunk_size
        self._shots_count = shots_count

        super().__init__(file_path, name, step_dependencies, environments, extra_parameters, host_requirements)

    @staticmethod
    def validate_parameters(parameters: TaskParameterList) -> bool:
        """
        Validate the parameters
        
        :param parameters: The list of parameter entities
        :type parameters: list
        """
        
        param_names = [p.name for p in parameters]

        if 'QueueManifestPath' in param_names:
            return True
        elif 'MoviePipelineQueuePath' in param_names:
            return True
        elif {'LevelSequencePath', 'LevelPath', 'MoviePipelineConfigurationPath'}.issubset(set(param_names)):
            return True

        raise ValueError('RenderOpenJobStep parameters are not valid. Expect at least one of the following:\n'
                         '- QueueManifestPath\n'
                         '- MoviePipelineQueuePath\n'
                         '- LevelSequencePath, LevelPath, MoviePipelineConfigurationPath\n'
                         )
        
    def get_chunk_size_parameters(self) -> dict:
        """
        Get parameters
        """
        
        if not self._task_chunk_size or not self._shots_count:
            raise ValueError('Task chunk size and shots count must be provided')
        
        step_object = self.get_template_object()

        task_chunk_size = {'name': 'TaskChunkSize', 'type': 'INT', 'range': [self._task_chunk_size]}
        task_chunk_size_updated = False

        task_chunk_ids_count = math.ceil(self._shots_count / self._task_chunk_size)
        task_chunk_ids = [i for i in range(task_chunk_ids_count)]
        task_chunk_id = {'name': 'TaskChunkId', 'type': 'INT', 'range': task_chunk_ids}
        task_chunk_id_updated = False

        for param_definition in step_object['parameterSpace']['taskParameterDefinitions']:
                
            if param_definition['name'] == task_chunk_size['name']:
                param_definition.update(task_chunk_size)
                task_chunk_size_updated = True

            if param_definition['name'] == task_chunk_id['name']:
                param_definition.update(task_chunk_id)
                task_chunk_id_updated = True

        if not task_chunk_size_updated:
            step_object['parameterSpace']['taskParameterDefinitions'].append(task_chunk_size)
        if not task_chunk_id_updated:
            step_object['parameterSpace']['taskParameterDefinitions'].append(task_chunk_id)

        return step_object['parameterSpace']['taskParameterDefinitions']
    
    def build_template(self) -> StepTemplate:
        """
        Build the definition template entity
        """
        
        for param in self.get_chunk_size_parameters():
            self.extra_parameters.append(param)
            
        step_entity = super().build()
        
        RenderUnrealOpenJobStep.validate_parameters(
            step_entity.parameterSpace.taskParameterDefinitions
        )
        
        return step_entity
