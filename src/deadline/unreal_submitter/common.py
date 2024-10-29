#  Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

import os
import unreal
from pathlib import Path


content_dir = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_content_dir())


def get_project_file_path() -> str:
    """
    Returns the Unreal project OS path

    :return: the Unreal project OS path
    :rtype: str
    """

    if unreal.Paths.is_project_file_path_set():
        project_file_path = unreal.Paths.convert_relative_path_to_full(
            unreal.Paths.get_project_file_path()
        )
        return project_file_path
    else:
        raise RuntimeError("Failed to get a project name. Please set a project!")


def get_project_directory() -> str:
    """
    Returns the Unreal project directory OS path

    :return: the Unreal project directory OS path
    :rtype: str
    """

    project_file_path = get_project_file_path()
    project_directory = str(Path(project_file_path).parent).replace("\\", "/")
    return project_directory


def get_project_name() -> str:
    """
    Returns the Unreal project pure name without extension

    :return: the Unreal project name
    :rtype: str
    """

    return Path(get_project_file_path()).stem


def soft_obj_path_to_str(soft_obj_path: unreal.SoftObjectPath) -> str:
    """
    Converts the given unreal.SoftObjectPath to the Unreal path

    :param soft_obj_path: unreal.SoftObjectPath instance
    :type soft_obj_path: unreal.SoftObjectPath
    :return: the Unreal path, e.g. /Game/Path/To/Asset
    """
    obj_ref = unreal.SystemLibrary.conv_soft_obj_path_to_soft_obj_ref(soft_obj_path)
    return unreal.SystemLibrary.conv_soft_object_reference_to_string(obj_ref)


def os_path_from_unreal_path(unreal_path, with_ext: bool = False):
    """
    Convert Unreal path to OS path, e.g. /Game/Assets/MyAsset to C:/UE_project/Content/Assets/MyAsset.uasset.

    if parameter with_ext is set to True, tries to get type of the asset by unreal.AssetData and set appropriate extension:

    - type World - .umap
    - other types - .uasset

    If for some reason it can't find asset data (e.g. temporary level's actors don't have asset data), it will set ".uasset"

    :param unreal_path: Unreal Path of the asset, e.g. /Game/Assets/MyAsset
    :param with_ext: if True, build the path with extension (.uasset or .umap), set asterisk "*" otherwise.

    :return: the OS path of the asset
    :rtype: str
    """

    os_path = str(unreal_path).replace('/Game/', content_dir)

    if with_ext:
        asset_data = unreal.EditorAssetLibrary.find_asset_data(unreal_path)
        asset_class_name = asset_data.asset_class_path.asset_name \
            if hasattr(asset_data, 'asset_class_path') \
            else asset_data.asset_class  # support older version of UE python API

        if not asset_class_name.is_none():  # AssetData not found - asset not in the project / on disk
            os_path += '.umap' if asset_class_name == 'World' else '.uasset'
        else:
            os_path += '.uasset'
    else:
        os_path += '.*'

    return os_path


def os_abs_from_relative(os_path):
    if os.path.isabs(os_path):
        return str(os_path)
    return get_project_directory() + '/' + os_path


class PathContext(dict):
    """
    Helper object for keeping any context related to render job paths.
    Inherited from Python dict (see https://docs.python.org/3/tutorial/datastructures.html#dictionaries).

    Overrides dunder method `__missing__` to avoid KeyError exception when not existed key requested.
    Instead of that create pair `key - "{key}"`
    """

    def __missing__(self, key):
        # if requested key is missed return string "{key}"
        return key.join('{}')


# TODO typehint
def get_path_context_from_mrq_job(mrq_job) -> PathContext:
    """
    Get build context from the given unreal.ConductorMoviePipelineExecutorJob

    :param mrq_job: unreal.ConductorMoviePipelineExecutorJob
    :return: :class:`deadline.unreal_submitter.common.PathContext` object
    :rtype: :class:`deadline.unreal_submitter.common.PathContext`
    """

    level_sequence_path = os.path.splitext(soft_obj_path_to_str(mrq_job.sequence))[0]
    level_sequence_name = level_sequence_path.split("/")[-1]

    map_path = os.path.splitext(soft_obj_path_to_str(mrq_job.map))[0]
    map_name = level_sequence_path.split("/")[-1]

    output_settings = mrq_job.get_configuration().find_setting_by_class(unreal.MoviePipelineOutputSetting)

    path_context = PathContext(
        {
            'project_path': get_project_file_path(),
            'project_dir': get_project_directory(),
            'job_name': mrq_job.job_name,
            'level_sequence': level_sequence_path,
            'level_sequence_name': level_sequence_name,
            'sequence_name': level_sequence_name,
            'map_path': map_path,
            'map_name': map_name,
            'level_name': map_name,
            'resolution': f'{output_settings.output_resolution.x}x{output_settings.output_resolution.y}',
        }
    )
    path_context.update(
        {
            'output_dir': output_settings.output_directory.path.format_map(path_context).replace(
                "\\", "/"
            ).rstrip("/"),
            'filename_format': output_settings.file_name_format.format_map(path_context)
        }
    )

    return path_context
