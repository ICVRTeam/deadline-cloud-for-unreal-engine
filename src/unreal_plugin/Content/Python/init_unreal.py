# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

import os
from pathlib import Path
import sys

remote_execution = os.getenv("REMOTE_EXECUTION", "False")
if remote_execution != "True":

    # Add predefined OpenJD templates directory to sys path
    # to get available to submit jobs without providing YAMLs for default entities
    if "OPENJD_TEMPLATES_DIRECTORY" not in os.environ:
        os.environ["OPENJD_TEMPLATES_DIRECTORY"] = (
            f"{Path(__file__).parent.as_posix()}/openjd_templates"
        )

    # Add the custom submit actions path to sys path
    actions_path = Path(__file__).parent.joinpath("submit_actions").as_posix()

    if actions_path not in sys.path:
        sys.path.append(actions_path)

    libraries_path = f"{os.path.dirname(__file__)}/libraries".replace("\\", "/")
    if not os.getenv("DEADLINE_CLOUD") and os.path.exists(libraries_path):
        os.environ["DEADLINE_CLOUD"] = libraries_path

    if os.getenv("DEADLINE_CLOUD") and os.environ["DEADLINE_CLOUD"] not in sys.path:
        sys.path.append(os.environ["DEADLINE_CLOUD"])

    from deadline.unreal_logger import get_logger

    logger = get_logger()

    logger.info("INIT DEADLINE CLOUD")

    logger.info(f'DEADLINE CLOUD PATH: {os.getenv("DEADLINE_CLOUD")}')

    # These unused imports are REQUIRED!!!
    # Unreal Engine loads any init_unreal.py it finds in its search paths.
    # These imports finish the setup for the plugin.
    from settings import DeadlineCloudDeveloperSettingsImplementation  # noqa: F401
    from job_library import DeadlineCloudJobBundleLibraryImplementation  # noqa: F401
    from open_job_template_api import (  # noqa: F401
        PythonYamlLibraryImplementation,
        ParametersConsistencyCheckerImplementation,
    )
    import remote_executor  # noqa: F401

    logger.info("DEADLINE CLOUD INITIALIZED")
