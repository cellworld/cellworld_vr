// (c) Copyright 2021 HP Development Company, L.P.

#include "Runtime/Launch/Resources/Version.h"

#define HP_INCLUDE_IF_CORRESPONDING_ENGINE_VERSION() \
    ENGINE_MAJOR_VERSION == 4 && \
    ENGINE_MINOR_VERSION == 25