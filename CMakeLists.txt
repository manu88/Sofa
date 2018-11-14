cmake_minimum_required(VERSION 3.7.2)

if (${PLATFORM} IN_LIST KernelX86Sel4Arch_all_strings)
        set(KernelArch x86 CACHE STRING "" FORCE)
        set(KernelX86Sel4Arch ${PLATFORM} CACHE STRING "" FORCE)
endif()


include(tools/cmake-tool/default-CMakeLists.txt)

#config_option(SUPPORT_SERIAL_TEXT_EGA LIB_PLAT_SUPPORT_SERIAL_TEXT_EGA "Makes library awesome" DEFAULT ON)
#LIB_PLAT_SUPPORT_SERIAL_TEXT_EGA


if(SIMULATION)
        ApplyCommonSimulationSettings("x86")
else()
        if(KernelArchX86)
                set(KernelIOMMU ON CACHE BOOL "" FORCE)
        endif()
endif()

# We must build the debug kernel because the tutorials rely on seL4_DebugPutChar
# and they don't initialize a platsupport driver.

ApplyCommonReleaseVerificationSettings(FALSE FALSE)
#ApplyCommonReleaseVerificationSettings(TRUE TRUE)

# Elfloader settings that correspond to how Data61 sets its boards up.
#ApplyData61ElfLoaderSettings()

SetSimulationScriptProperty(GRAPHIC TRUE)
GenerateSimulateScript()


