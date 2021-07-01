add_llvm_component_library(LLVMScarr
        ScarrCpMarker.cpp
        ScarrLoaCollector.cpp
        TopoSorter.cpp

        DEPENDS
        intrinsics_gen

        LINK_COMPONENTS
        Core
        Support
        )
