#ifndef CODAL_MPU_H
#define CODAL_MPU_H

#include "stdint.h"
#include "cmsis.h" // or your MCU's CMSIS core header

namespace codal
{

    enum class MPUAccessPermission : uint8_t
    {
        NO_ACCESS = 0,
        PRIV_RW = 1,
        PRIV_RW_UNPRIV_RO = 2,
        FULL_ACCESS = 3,
        RESERVED = 4,
        PRIV_RO = 5,
        RO = 6
    };

    enum class MPURegionSize : uint8_t
    {
        SIZE_32B = 4,
        SIZE_64B,
        SIZE_128B,
        SIZE_256B,
        SIZE_512B,
        SIZE_1KB,
        SIZE_2KB,
        SIZE_4KB,
        SIZE_8KB,
        SIZE_16KB,
        SIZE_32KB,
        SIZE_64KB,
        SIZE_128KB,
        SIZE_256KB,
        SIZE_512KB,
        SIZE_1MB,
        SIZE_2MB,
        SIZE_4MB,
        SIZE_8MB,
        SIZE_16MB,
        SIZE_32MB,
        SIZE_64MB,
        SIZE_128MB,
        SIZE_256MB,
        SIZE_512MB,
        SIZE_1GB,
        SIZE_2GB,
        SIZE_4GB
    };

    enum class MPU_STATE : uint8_t {
        // the MPU is okay.
        MPU_OK,
        // the wanted operation is not allowed by the MPU
        MPU_OPERATION_NOT_ALLOWED,
        MPU_UNKOWN_PERMISSON_ACCESS
    };

    class CodalMPU
    {
    public:
        static inline int enable(bool privilegedDefault = true)
        {
            if (this->isPrivileged() == false) {return MPU_STATE::MPU_OPERATION_NOT_ALLOWED}
            __DMB();
            MPU->CTRL = (privilegedDefault ? 0 : MPU_CTRL_PRIVDEFENA_Msk) | MPU_CTRL_ENABLE_Msk;
            __DSB();
            __ISB();
            return MPU_STATE::MPU_OK;
        }

        static inline int disable()
        {
            if (this->isPrivileged() == false) {return MPU_STATE::MPU_OPERATION_NOT_ALLOWED} 
            MPU->CTRL = 0;
            __DSB();
            __ISB();
            return MPU_STATE::MPU_OK;
        }

        static inline bool isEnabled()
        {
            return (MPU->CTRL & MPU_CTRL_ENABLE_Msk) != 0;
        }

        static inline int configureRegion(uint8_t regionNumber, uint32_t baseAddress, MPURegionSize size,
                                           MPUAccessPermission access, bool executable = true,
                                           bool shareable = false, bool cacheable = false, bool bufferable = false)
        {
            if (access == MPUAccessPermission::RESERVED) {
                return MPU_STATE::MPU_UNKOWN_PERMISSON_ACCESS;
            }
            if (this->isPrivileged() == false) {return MPU_STATE::MPU_OPERATION_NOT_ALLOWED}
            MPU->RNR = regionNumber;
            MPU->RBAR = baseAddress & MPU_RBAR_ADDR_Msk;

            uint32_t rasr = (static_cast<uint32_t>(access) << MPU_RASR_AP_Pos) |
                            (static_cast<uint32_t>(size) << MPU_RASR_SIZE_Pos) |
                            (executable ? 0 : MPU_RASR_XN_Msk) |
                            (shareable ? MPU_RASR_S_Msk : 0) |
                            (cacheable ? MPU_RASR_C_Msk : 0) |
                            (bufferable ? MPU_RASR_B_Msk : 0) |
                            MPU_RASR_ENABLE_Msk;

            MPU->RASR = rasr;
            __DSB();
            __ISB();
            return MPU_STATE::MPU_OK;
        }

        static inline int setSVCHandler(void (*handler)(void))
        {
            if (this->isPrivileged() == false) {return MPU_STATE::MPU_OPERATION_NOT_ALLOWED}
            constexpr uint32_t SVC_VECTOR_INDEX = 11; // SVC is exception #11
            uint32_t *vectorTable = reinterpret_cast<uint32_t *>(SCB->VTOR);
            vectorTable[SVC_VECTOR_INDEX] = reinterpret_cast<uint32_t>(handler);
            __DSB();
            __ISB();
            return MPU_STATE::MPU_OK;
        }

        static bool isPrivileged()
        {
            return (__get_CONTROL() & 0x1) == 0;
        }
    };

} // namespace codal

#endif
