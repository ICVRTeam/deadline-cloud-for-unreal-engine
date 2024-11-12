#  Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

import logging

from deadline.unreal_logger.handlers import UnrealLogHandler

try:
    import unreal  # noqa: F401

    UNREAL_INITIALIZED = True
except ModuleNotFoundError:
    unreal = None
    UNREAL_INITIALIZED = False


UNREAL_HANDLER_ADDED = False


def get_logger() -> logging.Logger:
    """
    Returns an instance of logging.Handler.
    Attach handler :class:`deadline.unreal_logger.handlers.UnrealLogHandler` if unreal module is
    available
    """

    unreal_logger = logging.getLogger("unreal_logger")
    unreal_logger.setLevel(logging.DEBUG)

    global UNREAL_HANDLER_ADDED

    # can be called outside UE so need to check before adding UE specific handler
    if not UNREAL_HANDLER_ADDED and UNREAL_INITIALIZED:
        unreal_log_handler = UnrealLogHandler(unreal)
        unreal_log_handler.setLevel(logging.DEBUG)
        unreal_logger.addHandler(unreal_log_handler)
        UNREAL_HANDLER_ADDED = True

    return unreal_logger
