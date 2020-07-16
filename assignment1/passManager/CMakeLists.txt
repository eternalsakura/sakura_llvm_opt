add_library(passManager MODULE
        analysis.cpp
        transform.cpp
        analysis.h
        another_transform.cpp
        )
target_compile_features(passManager PRIVATE cxx_range_for cxx_auto_type)

set_target_properties(passManager PROPERTIES
        # LLVM is (typically) built with no C++ RTTI. We need to match that;
        # otherwise, we'll get linker errors about missing RTTI data.
        COMPILE_FLAGS "-fno-rtti"
        )
if(APPLE)
    set_target_properties(passManager PROPERTIES
            LINK_FLAGS "-undefined dynamic_lookup"
            )
endif(APPLE)