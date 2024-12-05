#  Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

import os
import json
import unreal
from typing import Any
from pathlib import Path

from deadline.unreal_logger import get_logger


logger = get_logger()


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


def soft_obj_path_to_str(soft_obj_path: unreal.SoftObjectPath) -> str:
    """
    Converts the given unreal.SoftObjectPath to the Unreal path

    :param soft_obj_path: unreal.SoftObjectPath instance
    :type soft_obj_path: unreal.SoftObjectPath
    :return: the Unreal path, e.g. /Game/Path/To/Asset
    """
    obj_ref = unreal.SystemLibrary.conv_soft_obj_path_to_soft_obj_ref(soft_obj_path)
    return unreal.SystemLibrary.conv_soft_object_reference_to_string(obj_ref)


def create_deadline_cloud_temp_file(file_prefix: str, file_data: Any, file_ext: str) -> str:
    destination_dir = os.path.join(
        unreal.Paths.project_saved_dir(),
        "UnrealDeadlineCloudService",
        file_prefix,
    )
    os.makedirs(destination_dir, exist_ok=True)

    temp_file = unreal.Paths.create_temp_filename(
        destination_dir, prefix=file_prefix, extension=file_ext
    )

    with open(temp_file, "w") as f:
        logger.info(f"Saving {file_prefix} file '{temp_file}'")
        if file_ext == ".json":
            json.dump(file_data, f)
        else:
            f.write(file_data)

    temp_file = unreal.Paths.convert_relative_path_to_full(temp_file).replace("\\", "/")

    return temp_file
