// (c) Copyright 2019-2020 HP Development Company, L.P.

#pragma once

#include <list>

#include "Modules/ModuleManager.h"

struct Dll
{
    FString name;
    void* handle;
};

class FHPGliaModule : public IModuleInterface
    {
    public:

        /** IModuleInterface implementation */
        virtual HPGLIA_API void StartupModule() override;
        virtual HPGLIA_API void ShutdownModule() override;

    private:
        void HPGLIA_API RegisterSettings();
        FString HPGLIA_API SearchForDllPath(FString _searchBase, FString _dllName);

    protected:
        std::list<Dll> dllsToLoad;

};
