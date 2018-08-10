#pragma once

namespace M2
{
    namespace Wrappers
    {
        static inline void SwitchGenerators(bool bSwitch)
        {
            DWORD dwFunc = 0xAC6320;
            _asm push bSwitch;
            _asm call dwFunc;
        }

        static inline void SwitchFarAmbiants(bool bSwitch)
        {
            DWORD dwFunc = 0xAD2DA0;
            _asm push bSwitch;
            _asm call dwFunc;
        }
    };
};
