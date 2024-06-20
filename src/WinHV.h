/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

Module Name:

    WinHvApiDefs.h

Abstract:

    This module contains the constant, type and structure definitions for
    the Windows Hypervisor User-Mode APIs.

--*/

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#pragma warning(push)
#pragma warning(disable:4201) /* nonstandard extension used: nameless struct/union */
#pragma warning(disable:4214) /* nonstandard extension used: bit field types other than int */
#endif

//
// Platform capabilities
//
typedef enum WHV_CAPABILITY_CODE
{
    // Capabilities of the API implementation
    WHvCapabilityCodeHypervisorPresent      = 0x00000000,
    WHvCapabilityCodeFeatures               = 0x00000001,
    WHvCapabilityCodeExtendedVmExits        = 0x00000002,
    WHvCapabilityCodeExceptionExitBitmap    = 0x00000003,

    // Capabilities of the system's processor
    WHvCapabilityCodeProcessorVendor        = 0x00001000,
    WHvCapabilityCodeProcessorFeatures      = 0x00001001,
    WHvCapabilityCodeProcessorClFlushSize   = 0x00001002
} WHV_CAPABILITY_CODE;

//
// Return values for WhvCapabilityCodeFeatures
//
typedef union WHV_CAPABILITY_FEATURES
{
    struct
    {
        UINT64 Reserved : 64;
    };

    UINT64 AsUINT64;
} WHV_CAPABILITY_FEATURES;

//
// Return values for WhvCapabilityCodeExtendedVmExits
//
typedef union WHV_EXTENDED_VM_EXITS
{
    struct
    {
        UINT64 X64CpuidExit  : 1; // RunVpExitReasonX64CPUID supported
        UINT64 X64MsrExit    : 1; // RunVpExitX64ReasonMSRAccess supported
        UINT64 ExceptionExit : 1; // RunVpExitReasonException supported
        UINT64 Reserved      : 61;
    };

    UINT64 AsUINT64;
} WHV_EXTENDED_VM_EXITS;

//
// Return values for WhvCapabilityCodeProcessorVendor
//
typedef enum WHV_PROCESSOR_VENDOR
{
    WHvProcessorVendorAmd   = 0x0000,
    WHvProcessorVendorIntel = 0x0001

} WHV_PROCESSOR_VENDOR;

//
// Return values for WhvCapabilityCodeProcessorFeatures
//
typedef union WHV_PROCESSOR_FEATURES
{
    struct
    {
        UINT64 Sse3Support : 1;
        UINT64 LahfSahfSupport : 1;
        UINT64 Ssse3Support : 1;
        UINT64 Sse4_1Support : 1;
        UINT64 Sse4_2Support : 1;
        UINT64 Sse4aSupport : 1;
        UINT64 XopSupport : 1;
        UINT64 PopCntSupport : 1;
        UINT64 Cmpxchg16bSupport : 1;
        UINT64 Altmovcr8Support : 1;
        UINT64 LzcntSupport : 1;
        UINT64 MisAlignSseSupport : 1;
        UINT64 MmxExtSupport : 1;
        UINT64 Amd3DNowSupport : 1;
        UINT64 ExtendedAmd3DNowSupport : 1;
        UINT64 Page1GbSupport : 1;
        UINT64 AesSupport : 1;
        UINT64 PclmulqdqSupport : 1;
        UINT64 PcidSupport : 1;
        UINT64 Fma4Support : 1;
        UINT64 F16CSupport : 1;
        UINT64 RdRandSupport : 1;
        UINT64 RdWrFsGsSupport : 1;
        UINT64 SmepSupport : 1;
        UINT64 EnhancedFastStringSupport : 1;
        UINT64 Bmi1Support : 1;
        UINT64 Bmi2Support : 1;
        UINT64 Reserved1 : 2;
        UINT64 MovbeSupport : 1;
        UINT64 Npiep1Support : 1;
        UINT64 DepX87FPUSaveSupport : 1;
        UINT64 RdSeedSupport : 1;
        UINT64 AdxSupport : 1;
        UINT64 IntelPrefetchSupport : 1;
        UINT64 SmapSupport : 1;
        UINT64 HleSupport : 1;
        UINT64 RtmSupport : 1;
        UINT64 RdtscpSupport : 1;
        UINT64 ClflushoptSupport : 1;
        UINT64 ClwbSupport : 1;
        UINT64 ShaSupport : 1;
        UINT64 X87PointersSavedSupport : 1;
        UINT64 Reserved2 : 21;
    };

    UINT64 AsUINT64;
} WHV_PROCESSOR_FEATURES;

//
// WHvGetCapability output buffer
//
typedef union WHV_CAPABILITY
{
    BOOL HypervisorPresent;
    WHV_CAPABILITY_FEATURES Features;
    WHV_EXTENDED_VM_EXITS ExtendedVmExits;
    WHV_PROCESSOR_VENDOR ProcessorVendor;
    WHV_PROCESSOR_FEATURES ProcessorFeatures;
    UINT8 ProcessorClFlushSize;
    UINT64 ExceptionExitBitmap;
} WHV_CAPABILITY;

//
// Partitions
//

typedef VOID* WHV_PARTITION_HANDLE;

typedef enum WHV_PARTITION_PROPERTY_CODE
{
    WHvPartitionPropertyCodeExtendedVmExits         = 0x00000001,
    WHvPartitionPropertyCodeExceptionExitBitmap     = 0x00000002,

    WHvPartitionPropertyCodeProcessorFeatures       = 0x00001001,
    WHvPartitionPropertyCodeProcessorClFlushSize    = 0x00001002,
    WHvPartitionPropertyCodeCpuidExitList           = 0x00001003,
    WHvPartitionPropertyCodeCpuidResultList         = 0x00001004,

    WHvPartitionPropertyCodeProcessorCount          = 0x00001fff
} WHV_PARTITION_PROPERTY_CODE;

//
// WHvPartitionPropertyCodeCpuidResultList input buffer list element.
//
typedef struct WHV_X64_CPUID_RESULT
{
    UINT32 Function;
    UINT32 Reserved[3];
    UINT32 Eax;
    UINT32 Ebx;
    UINT32 Ecx;
    UINT32 Edx;
} WHV_X64_CPUID_RESULT;

//
// WHvPartitionPropertyCodeExceptionBitmap enumeration values.
//
typedef enum WHV_EXCEPTION_TYPE
{
    WHvX64ExceptionTypeDivideErrorFault = 0x0,
    WHvX64ExceptionTypeDebugTrapOrFault = 0x1,
    WHvX64ExceptionTypeBreakpointTrap = 0x3,
    WHvX64ExceptionTypeOverflowTrap = 0x4,
    WHvX64ExceptionTypeBoundRangeFault = 0x5,
    WHvX64ExceptionTypeInvalidOpcodeFault = 0x6,
    WHvX64ExceptionTypeDeviceNotAvailableFault = 0x7,
    WHvX64ExceptionTypeDoubleFaultAbort = 0x8,
    WHvX64ExceptionTypeInvalidTaskStateSegmentFault = 0x0A,
    WHvX64ExceptionTypeSegmentNotPresentFault = 0x0B,
    WHvX64ExceptionTypeStackFault = 0x0C,
    WHvX64ExceptionTypeGeneralProtectionFault = 0x0D,
    WHvX64ExceptionTypePageFault = 0x0E,
    WHvX64ExceptionTypeFloatingPointErrorFault = 0x10,
    WHvX64ExceptionTypeAlignmentCheckFault = 0x11,
    WHvX64ExceptionTypeMachineCheckAbort = 0x12,
    WHvX64ExceptionTypeSimdFloatingPointFault = 0x13,
} WHV_EXCEPTION_TYPE;

//
// WHvGetPartitionProperty output buffer / WHvSetPartitionProperty input buffer
//
typedef union WHV_PARTITION_PROPERTY
{
    WHV_EXTENDED_VM_EXITS ExtendedVmExits;
    WHV_PROCESSOR_FEATURES ProcessorFeatures;
    UINT8 ProcessorClFlushSize;
    UINT32 ProcessorCount;
    UINT32 CpuidExitList[1];
    WHV_X64_CPUID_RESULT CpuidResultList[1];
    UINT64 ExceptionExitBitmap;
} WHV_PARTITION_PROPERTY;

//
// Memory Management
//

//
// Guest physical or virtual address
//
typedef UINT64 WHV_GUEST_PHYSICAL_ADDRESS;
typedef UINT64 WHV_GUEST_VIRTUAL_ADDRESS;

//
// Flags used by WHvMapGpaRange
//
/*typedef enum WHV_MAP_GPA_RANGE_FLAGS
{
    WHvMapGpaRangeFlagNone    = 0x00000000,
    WHvMapGpaRangeFlagRead    = 0x00000001,
    WHvMapGpaRangeFlagWrite   = 0x00000002,
    WHvMapGpaRangeFlagExecute = 0x00000004,
} WHV_MAP_GPA_RANGE_FLAGS;*/

#define   WHvMapGpaRangeFlagNone     0x00000000
#define    WHvMapGpaRangeFlagRead     0x00000001
#define    WHvMapGpaRangeFlagWrite    0x00000002
#define    WHvMapGpaRangeFlagExecute  0x00000004
#define WHV_MAP_GPA_RANGE_FLAGS DWORD
//DEFINE_ENUM_FLAG_OPERATORS(WHV_MAP_GPA_RANGE_FLAGS);

//
// Flags used by WHvTranslateGva
//
typedef enum WHV_TRANSLATE_GVA_FLAGS
{
    WHvTranslateGvaFlagNone             = 0x00000000,
    WHvTranslateGvaFlagValidateRead     = 0x00000001,
    WHvTranslateGvaFlagValidateWrite    = 0x00000002,
    WHvTranslateGvaFlagValidateExecute  = 0x00000004,
    WHvTranslateGvaFlagPrivilegeExempt  = 0x00000008,
    WHvTranslateGvaFlagSetPageTableBits = 0x00000010
} WHV_TRANSLATE_GVA_FLAGS;

//DEFINE_ENUM_FLAG_OPERATORS(WHV_TRANSLATE_GVA_FLAGS);

//
// Result of an attempt to translate a guest virtual address
//
typedef enum WHV_TRANSLATE_GVA_RESULT_CODE
{
    WHvTranslateGvaResultSuccess                 = 0,

    // Translation failures
    WHvTranslateGvaResultPageNotPresent          = 1,
    WHvTranslateGvaResultPrivilegeViolation      = 2,
    WHvTranslateGvaResultInvalidPageTableFlags   = 3,

    // GPA access failures
    WHvTranslateGvaResultGpaUnmapped             = 4,
    WHvTranslateGvaResultGpaNoReadAccess         = 5,
    WHvTranslateGvaResultGpaNoWriteAccess        = 6,
    WHvTranslateGvaResultGpaIllegalOverlayAccess = 7,
    WHvTranslateGvaResultIntercept               = 8
} WHV_TRANSLATE_GVA_RESULT_CODE;

//
// Output buffer of WHvTranslateGva
//
typedef struct WHV_TRANSLATE_GVA_RESULT
{
    WHV_TRANSLATE_GVA_RESULT_CODE ResultCode;
    UINT32 Reserved;
} WHV_TRANSLATE_GVA_RESULT;

//
// Virtual Processor Register Definitions
//
typedef enum WHV_REGISTER_NAME
{
    // X64 General purpose registers
    WHvX64RegisterRax              = 0x00000000,
    WHvX64RegisterRcx              = 0x00000001,
    WHvX64RegisterRdx              = 0x00000002,
    WHvX64RegisterRbx              = 0x00000003,
    WHvX64RegisterRsp              = 0x00000004,
    WHvX64RegisterRbp              = 0x00000005,
    WHvX64RegisterRsi              = 0x00000006,
    WHvX64RegisterRdi              = 0x00000007,
    WHvX64RegisterR8               = 0x00000008,
    WHvX64RegisterR9               = 0x00000009,
    WHvX64RegisterR10              = 0x0000000A,
    WHvX64RegisterR11              = 0x0000000B,
    WHvX64RegisterR12              = 0x0000000C,
    WHvX64RegisterR13              = 0x0000000D,
    WHvX64RegisterR14              = 0x0000000E,
    WHvX64RegisterR15              = 0x0000000F,
    WHvX64RegisterRip              = 0x00000010,
    WHvX64RegisterRflags           = 0x00000011,

    // X64 Segment registers
    WHvX64RegisterEs               = 0x00000012,
    WHvX64RegisterCs               = 0x00000013,
    WHvX64RegisterSs               = 0x00000014,
    WHvX64RegisterDs               = 0x00000015,
    WHvX64RegisterFs               = 0x00000016,
    WHvX64RegisterGs               = 0x00000017,
    WHvX64RegisterLdtr             = 0x00000018,
    WHvX64RegisterTr               = 0x00000019,

    // X64 Table registers
    WHvX64RegisterIdtr             = 0x0000001A,
    WHvX64RegisterGdtr             = 0x0000001B,

    // X64 Control Registers
    WHvX64RegisterCr0              = 0x0000001C,
    WHvX64RegisterCr2              = 0x0000001D,
    WHvX64RegisterCr3              = 0x0000001E,
    WHvX64RegisterCr4              = 0x0000001F,
    WHvX64RegisterCr8              = 0x00000020,

    // X64 Debug Registers
    WHvX64RegisterDr0              = 0x00000021,
    WHvX64RegisterDr1              = 0x00000022,
    WHvX64RegisterDr2              = 0x00000023,
    WHvX64RegisterDr3              = 0x00000024,
    WHvX64RegisterDr6              = 0x00000025,
    WHvX64RegisterDr7              = 0x00000026,

    // X64 Floating Point and Vector Registers
    WHvX64RegisterXmm0             = 0x00001000,
    WHvX64RegisterXmm1             = 0x00001001,
    WHvX64RegisterXmm2             = 0x00001002,
    WHvX64RegisterXmm3             = 0x00001003,
    WHvX64RegisterXmm4             = 0x00001004,
    WHvX64RegisterXmm5             = 0x00001005,
    WHvX64RegisterXmm6             = 0x00001006,
    WHvX64RegisterXmm7             = 0x00001007,
    WHvX64RegisterXmm8             = 0x00001008,
    WHvX64RegisterXmm9             = 0x00001009,
    WHvX64RegisterXmm10            = 0x0000100A,
    WHvX64RegisterXmm11            = 0x0000100B,
    WHvX64RegisterXmm12            = 0x0000100C,
    WHvX64RegisterXmm13            = 0x0000100D,
    WHvX64RegisterXmm14            = 0x0000100E,
    WHvX64RegisterXmm15            = 0x0000100F,
    WHvX64RegisterFpMmx0           = 0x00001010,
    WHvX64RegisterFpMmx1           = 0x00001011,
    WHvX64RegisterFpMmx2           = 0x00001012,
    WHvX64RegisterFpMmx3           = 0x00001013,
    WHvX64RegisterFpMmx4           = 0x00001014,
    WHvX64RegisterFpMmx5           = 0x00001015,
    WHvX64RegisterFpMmx6           = 0x00001016,
    WHvX64RegisterFpMmx7           = 0x00001017,
    WHvX64RegisterFpControlStatus  = 0x00001018,
    WHvX64RegisterXmmControlStatus = 0x00001019,

    // X64 MSRs
    WHvX64RegisterTsc              = 0x00002000,
    WHvX64RegisterEfer             = 0x00002001,
    WHvX64RegisterKernelGsBase     = 0x00002002,
    WHvX64RegisterApicBase         = 0x00002003,
    WHvX64RegisterPat              = 0x00002004,
    WHvX64RegisterSysenterCs       = 0x00002005,
    WHvX64RegisterSysenterEip      = 0x00002006,
    WHvX64RegisterSysenterEsp      = 0x00002007,
    WHvX64RegisterStar             = 0x00002008,
    WHvX64RegisterLstar            = 0x00002009,
    WHvX64RegisterCstar            = 0x0000200A,
    WHvX64RegisterSfmask           = 0x0000200B,

    WHvX64RegisterMsrMtrrCap         = 0x0000200D,
    WHvX64RegisterMsrMtrrDefType     = 0x0000200E,

    WHvX64RegisterMsrMtrrPhysBase0   = 0x00002010,
    WHvX64RegisterMsrMtrrPhysBase1   = 0x00002011,
    WHvX64RegisterMsrMtrrPhysBase2   = 0x00002012,
    WHvX64RegisterMsrMtrrPhysBase3   = 0x00002013,
    WHvX64RegisterMsrMtrrPhysBase4   = 0x00002014,
    WHvX64RegisterMsrMtrrPhysBase5   = 0x00002015,
    WHvX64RegisterMsrMtrrPhysBase6   = 0x00002016,
    WHvX64RegisterMsrMtrrPhysBase7   = 0x00002017,
    WHvX64RegisterMsrMtrrPhysBase8   = 0x00002018,
    WHvX64RegisterMsrMtrrPhysBase9   = 0x00002019,
    WHvX64RegisterMsrMtrrPhysBaseA   = 0x0000201A,
    WHvX64RegisterMsrMtrrPhysBaseB   = 0x0000201B,
    WHvX64RegisterMsrMtrrPhysBaseC   = 0x0000201C,
    WHvX64RegisterMsrMtrrPhysBaseD   = 0x0000201D,
    WHvX64RegisterMsrMtrrPhysBaseE   = 0x0000201E,
    WHvX64RegisterMsrMtrrPhysBaseF   = 0x0000201F,

    WHvX64RegisterMsrMtrrPhysMask0   = 0x00002040,
    WHvX64RegisterMsrMtrrPhysMask1   = 0x00002041,
    WHvX64RegisterMsrMtrrPhysMask2   = 0x00002042,
    WHvX64RegisterMsrMtrrPhysMask3   = 0x00002043,
    WHvX64RegisterMsrMtrrPhysMask4   = 0x00002044,
    WHvX64RegisterMsrMtrrPhysMask5   = 0x00002045,
    WHvX64RegisterMsrMtrrPhysMask6   = 0x00002046,
    WHvX64RegisterMsrMtrrPhysMask7   = 0x00002047,
    WHvX64RegisterMsrMtrrPhysMask8   = 0x00002048,
    WHvX64RegisterMsrMtrrPhysMask9   = 0x00002049,
    WHvX64RegisterMsrMtrrPhysMaskA   = 0x0000204A,
    WHvX64RegisterMsrMtrrPhysMaskB   = 0x0000204B,
    WHvX64RegisterMsrMtrrPhysMaskC   = 0x0000204C,
    WHvX64RegisterMsrMtrrPhysMaskD   = 0x0000204D,
    WHvX64RegisterMsrMtrrPhysMaskE   = 0x0000204E,
    WHvX64RegisterMsrMtrrPhysMaskF   = 0x0000204F,

    WHvX64RegisterMsrMtrrFix64k00000 = 0x00002070,
    WHvX64RegisterMsrMtrrFix16k80000 = 0x00002071,
    WHvX64RegisterMsrMtrrFix16kA0000 = 0x00002072,
    WHvX64RegisterMsrMtrrFix4kC0000  = 0x00002073,
    WHvX64RegisterMsrMtrrFix4kC8000  = 0x00002074,
    WHvX64RegisterMsrMtrrFix4kD0000  = 0x00002075,
    WHvX64RegisterMsrMtrrFix4kD8000  = 0x00002076,
    WHvX64RegisterMsrMtrrFix4kE0000  = 0x00002077,
    WHvX64RegisterMsrMtrrFix4kE8000  = 0x00002078,
    WHvX64RegisterMsrMtrrFix4kF0000  = 0x00002079,
    WHvX64RegisterMsrMtrrFix4kF8000  = 0x0000207A,

    WHvX64RegisterTscAux           = 0x0000207B,

    // Interrupt / Event Registers
    WHvRegisterPendingInterruption = 0x80000000,
    WHvRegisterInterruptState      = 0x80000001,
    WHvRegisterPendingEvent0       = 0x80000002,
    WHvRegisterPendingEvent1       = 0x80000003,
    WHvX64RegisterDeliverabilityNotifications = 0x80000004,

} WHV_REGISTER_NAME;

typedef union DECLSPEC_ALIGN(16) WHV_UINT128
{
    struct
    {
        UINT64  Low64;
        UINT64  High64;
    };

    UINT32  Dword[4];
} WHV_UINT128;

typedef union WHV_X64_FP_REGISTER
{
    struct
    {
        UINT64 Mantissa;
        UINT64 BiasedExponent:15;
        UINT64 Sign:1;
        UINT64 Reserved:48;
    };

    WHV_UINT128 AsUINT128;
} WHV_X64_FP_REGISTER;

typedef union WHV_X64_FP_CONTROL_STATUS_REGISTER
{
    struct
    {
        UINT16 FpControl;
        UINT16 FpStatus;
        UINT8  FpTag;
        UINT8  Reserved;
        UINT16 LastFpOp;
        union
        {
            // Long Mode
            UINT64 LastFpRip;

            // 32 Bit Mode
            struct
            {
                UINT32 LastFpEip;
                UINT16 LastFpCs;
                UINT16 Reserved2;
            };
        };
    };

    WHV_UINT128 AsUINT128;
} WHV_X64_FP_CONTROL_STATUS_REGISTER;

typedef union WHV_X64_XMM_CONTROL_STATUS_REGISTER
{
    struct
    {
        union
        {
            // Long Mode
            UINT64 LastFpRdp;

            // 32 Bit Mode
            struct
            {
                UINT32 LastFpDp;
                UINT16 LastFpDs;
                UINT16 Reserved;
            };
        };
        UINT32 XmmStatusControl;
        UINT32 XmmStatusControlMask;
    };

    WHV_UINT128 AsUINT128;
} WHV_X64_XMM_CONTROL_STATUS_REGISTER;

typedef struct WHV_X64_SEGMENT_REGISTER
{
    UINT64 Base;
    UINT32 Limit;
    UINT16 Selector;

    union
    {
        struct
        {
            UINT16 SegmentType:4;
            UINT16 NonSystemSegment:1;
            UINT16 DescriptorPrivilegeLevel:2;
            UINT16 Present:1;
            UINT16 Reserved:4;
            UINT16 Available:1;
            UINT16 Long:1;
            UINT16 Default:1;
            UINT16 Granularity:1;
        };

        UINT16 Attributes;
    };
} WHV_X64_SEGMENT_REGISTER;

typedef struct WHV_X64_TABLE_REGISTER
{
    UINT16     Pad[3];
    UINT16     Limit;
    UINT64     Base;
} WHV_X64_TABLE_REGISTER;

typedef union WHV_X64_INTERRUPT_STATE_REGISTER
{
    struct
    {
        UINT64 InterruptShadow:1;
        UINT64 NmiMasked:1;
        UINT64 Reserved:62;
    };

    UINT64 AsUINT64;
} WHV_X64_INTERRUPT_STATE_REGISTER;

typedef union WHV_X64_PENDING_INTERRUPTION_REGISTER
{
    struct
    {
        UINT32 InterruptionPending:1;
        UINT32 InterruptionType:3;  // WHV_X64_PENDING_INTERRUPTION_TYPE
        UINT32 DeliverErrorCode:1;
        UINT32 InstructionLength:4;
        UINT32 NestedEvent:1;
        UINT32 Reserved:6;
        UINT32 InterruptionVector:16;
        UINT32 ErrorCode;
    };

    UINT64 AsUINT64;
} WHV_X64_PENDING_INTERRUPTION_REGISTER;

typedef union WHV_X64_DELIVERABILITY_NOTIFICATIONS_REGISTER
{
    struct
    {
        UINT64 NmiNotification:1;
        UINT64 InterruptNotification:1;
        UINT64 InterruptPriority:4;
        UINT64 Reserved:58;
    };

    UINT64 AsUINT64;
} WHV_X64_DELIVERABILITY_NOTIFICATIONS_REGISTER;

//
// Register values
//
typedef union WHV_REGISTER_VALUE
{
    WHV_UINT128 Reg128;
    UINT64 Reg64;
    UINT32 Reg32;
    UINT16 Reg16;
    UINT8 Reg8;
    WHV_X64_FP_REGISTER Fp;
    WHV_X64_FP_CONTROL_STATUS_REGISTER FpControlStatus;
    WHV_X64_XMM_CONTROL_STATUS_REGISTER XmmControlStatus;
    WHV_X64_SEGMENT_REGISTER Segment;
    WHV_X64_TABLE_REGISTER Table;
    WHV_X64_INTERRUPT_STATE_REGISTER InterruptState;
    WHV_X64_PENDING_INTERRUPTION_REGISTER PendingInterruption;
    WHV_X64_DELIVERABILITY_NOTIFICATIONS_REGISTER DeliverabilityNotifications;
} WHV_REGISTER_VALUE;

//
// Virtual Processor Execution
//

//
// Reason for a VM exit
//
typedef enum WHV_RUN_VP_EXIT_REASON
{
    WHvRunVpExitReasonNone                   = 0x00000000,

    // Standard exits caused by operations of the virtual processor
    WHvRunVpExitReasonMemoryAccess           = 0x00000001,
    WHvRunVpExitReasonX64IoPortAccess        = 0x00000002,
    WHvRunVpExitReasonUnrecoverableException = 0x00000004,
    WHvRunVpExitReasonInvalidVpRegisterValue = 0x00000005,
    WHvRunVpExitReasonUnsupportedFeature     = 0x00000006,
    WHvRunVpExitReasonX64InterruptWindow     = 0x00000007,
    WHvRunVpExitReasonX64Halt                = 0x00000008,

    // Additional exits that can be configured through partition properties
    WHvRunVpExitReasonX64MsrAccess           = 0x00001000,
    WHvRunVpExitReasonX64Cpuid               = 0x00001001,
    WHvRunVpExitReasonException              = 0x00001002,

    // Exits caused by the host
    WHvRunVpExitReasonCanceled               = 0x00002001,
} WHV_RUN_VP_EXIT_REASON;

//
// Execution state of the virtual processor
//
typedef union WHV_X64_VP_EXECUTION_STATE
{
    struct
    {
        UINT16 Cpl : 2;
        UINT16 Cr0Pe : 1;
        UINT16 Cr0Am : 1;
        UINT16 EferLma : 1;
        UINT16 DebugActive : 1;
        UINT16 InterruptionPending : 1;
        UINT16 Reserved0 : 5;
        UINT16 InterruptShadow : 1;
        UINT16 Reserved1 : 3;
    };

    UINT16 AsUINT16;
} WHV_X64_VP_EXECUTION_STATE;

//
// Execution context of a virtual processor at the time of an exit
//
typedef struct WHV_VP_EXIT_CONTEXT
{
    WHV_X64_VP_EXECUTION_STATE ExecutionState;
    UINT8 InstructionLength : 4;
    UINT8 Cr8 : 4;
    UINT8 Reserved;
    UINT32 Reserved2;
    WHV_X64_SEGMENT_REGISTER Cs;
    UINT64 Rip;
    UINT64 Rflags;
} WHV_VP_EXIT_CONTEXT;

//
// Context data for a VM exit caused by a memory access (WHvRunVpExitReasonMemoryAccess)
//
typedef enum WHV_MEMORY_ACCESS_TYPE
{
    WHvMemoryAccessRead    = 0,
    WHvMemoryAccessWrite   = 1,
    WHvMemoryAccessExecute = 2
} WHV_MEMORY_ACCESS_TYPE;

typedef union WHV_MEMORY_ACCESS_INFO
{
    struct {
        UINT32 AccessType  : 2;  // WHV_MEMORY_ACCESS_TYPE
        UINT32 GpaUnmapped : 1;
        UINT32 GvaValid    : 1;
        UINT32 Reserved    : 28;
    };

    UINT32 AsUINT32;
} WHV_MEMORY_ACCESS_INFO;

typedef struct WHV_MEMORY_ACCESS_CONTEXT
{
    // Context of the virtual processor
    UINT8 InstructionByteCount;
    UINT8 Reserved[3];
    UINT8 InstructionBytes[16];

    // Memory access info
    WHV_MEMORY_ACCESS_INFO AccessInfo;
    WHV_GUEST_PHYSICAL_ADDRESS Gpa;
    WHV_GUEST_VIRTUAL_ADDRESS Gva;
} WHV_MEMORY_ACCESS_CONTEXT;

//
// Context data for an exit caused by an I/O port access (WHvRunVpExitReasonX64IOPortAccess)
//
typedef union WHV_X64_IO_PORT_ACCESS_INFO
{
    struct
    {
        UINT32 IsWrite : 1;
        UINT32 AccessSize: 3;
        UINT32 StringOp : 1;
        UINT32 RepPrefix : 1;
        UINT32 Reserved : 26;
    };

    UINT32 AsUINT32;
} WHV_X64_IO_PORT_ACCESS_INFO;

typedef struct WHV_X64_IO_PORT_ACCESS_CONTEXT
{
    // Context of the virtual processor
    UINT8 InstructionByteCount;
    UINT8 Reserved[3];
    UINT8 InstructionBytes[16];

    // I/O port access info
    WHV_X64_IO_PORT_ACCESS_INFO AccessInfo;
    UINT16 PortNumber;
    UINT16 Reserved2[3];
    UINT64 Rax;
    UINT64 Rcx;
    UINT64 Rsi;
    UINT64 Rdi;
    WHV_X64_SEGMENT_REGISTER Ds;
    WHV_X64_SEGMENT_REGISTER Es;
} WHV_X64_IO_PORT_ACCESS_CONTEXT;

//
// Context data for an exit caused by an MSR access (WHvRunVpExitReasonX64MSRAccess)
//
typedef union WHV_X64_MSR_ACCESS_INFO
{
    struct
    {
        UINT32 IsWrite : 1;
        UINT32 Reserved : 31;
    };

    UINT32 AsUINT32;
} WHV_X64_MSR_ACCESS_INFO;

typedef struct WHV_X64_MSR_ACCESS_CONTEXT
{
    // MSR access info
    WHV_X64_MSR_ACCESS_INFO AccessInfo;
    UINT32 MsrNumber;
    UINT64 Rax;
    UINT64 Rdx;
} WHV_X64_MSR_ACCESS_CONTEXT;

//
// Context data for an exit caused by a CPUID call (WHvRunVpExitReasonX64CPUID)
//
typedef struct WHV_X64_CPUID_ACCESS_CONTEXT
{
    // CPUID access info
    UINT64 Rax;
    UINT64 Rcx;
    UINT64 Rdx;
    UINT64 Rbx;
    UINT64 DefaultResultRax;
    UINT64 DefaultResultRcx;
    UINT64 DefaultResultRdx;
    UINT64 DefaultResultRbx;
} WHV_X64_CPUID_ACCESS_CONTEXT;

//
// Context data for an exit caused by an exception generated by the virtual processor
// (WHvRunVpExitReasonException)
//
typedef union WHV_VP_EXCEPTION_INFO
{
    struct
    {
        UINT32 ErrorCodeValid : 1;
        UINT32 SoftwareException : 1;
        UINT32 Reserved : 30;
    };

    UINT32 AsUNIT32;
} WHV_VP_EXCEPTION_INFO;

typedef struct WHV_VP_EXCEPTION_CONTEXT
{
    UINT8 InstructionByteCount;
    UINT8 Reserved[3];
    UINT8 InstructionBytes[16];

    // Exception info
    WHV_VP_EXCEPTION_INFO ExceptionInfo;
    UINT8 ExceptionType; // WHV_EXCEPTION_TYPE
    UINT8 Reserved2[3];
    UINT32 ErrorCode;
    UINT64 ExceptionParameter;
} WHV_VP_EXCEPTION_CONTEXT;

//
// Context data for an exit caused by the use of an unsupported processor feature
// (WHvRunVpExitReasonUnsupportedFeature)
//
typedef enum WHV_X64_UNSUPPORTED_FEATURE_CODE
{
    WHvUnsupportedFeatureIntercept     = 1,
    WHvUnsupportedFeatureTaskSwitchTss = 2
} WHV_X64_UNSUPPORTED_FEATURE_CODE;

typedef struct WHV_X64_UNSUPPORTED_FEATURE_CONTEXT
{
    WHV_X64_UNSUPPORTED_FEATURE_CODE FeatureCode;
    UINT32 Reserved;
    UINT64 FeatureParameter;
} WHV_X64_UNSUPPORTED_FEATURE_CONTEXT;

//
// Context data for an exit caused by a cancellation from the host (WHvRunVpExitReasonCanceled)
//
typedef enum WHV_RUN_VP_CANCEL_REASON
{
    WhvRunVpCancelReasonUser = 0 // Execution canceled by HvCancelRunVirtualProcessor
} WHV_RUN_VP_CANCEL_REASON;

typedef struct WHV_RUN_VP_CANCELED_CONTEXT
{
    WHV_RUN_VP_CANCEL_REASON CancelReason;
} WHV_RUN_VP_CANCELED_CONTEXT;

//
// Context data for an exit caused by an interrupt delivery window cancellation from the host
// (WHvRunVpExitReasonX64InterruptWindow)
//
typedef enum WHV_X64_PENDING_INTERRUPTION_TYPE
{
    WHvX64PendingInterrupt           = 0,
    WHvX64PendingNmi                 = 2,
    WHvX64PendingException           = 3
} WHV_X64_PENDING_INTERRUPTION_TYPE, *PWHV_X64_PENDING_INTERRUPTION_TYPE;

typedef struct WHV_X64_INTERRUPTION_DELIVERABLE_CONTEXT
{
    WHV_X64_PENDING_INTERRUPTION_TYPE DeliverableType;
} WHV_X64_INTERRUPTION_DELIVERABLE_CONTEXT, *PWHV_X64_INTERRUPTION_DELIVERABLE_CONTEXT;


// WHvRunVirtualProcessor output buffer
typedef struct WHV_RUN_VP_EXIT_CONTEXT
{
    WHV_RUN_VP_EXIT_REASON ExitReason;
    UINT32 Reserved;
    WHV_VP_EXIT_CONTEXT VpContext;

    union
    {
        WHV_MEMORY_ACCESS_CONTEXT MemoryAccess;
        WHV_X64_IO_PORT_ACCESS_CONTEXT IoPortAccess;
        WHV_X64_MSR_ACCESS_CONTEXT MsrAccess;
        WHV_X64_CPUID_ACCESS_CONTEXT CpuidAccess;
        WHV_VP_EXCEPTION_CONTEXT VpException;
        WHV_X64_INTERRUPTION_DELIVERABLE_CONTEXT InterruptWindow;
        WHV_X64_UNSUPPORTED_FEATURE_CONTEXT UnsupportedFeature;
        WHV_RUN_VP_CANCELED_CONTEXT CancelReason;
    };
} WHV_RUN_VP_EXIT_CONTEXT;

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma warning(pop)
#endif

