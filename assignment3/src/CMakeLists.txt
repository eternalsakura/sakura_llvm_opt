add_library(Assignment3 MODULE
        loop_invariant_code_motion.cpp)

target_compile_features(Assignment3 PRIVATE cxx_range_for cxx_auto_type)

set_target_properties(Assignment3 PROPERTIES
        # LLVM is (typically) built with no C++ RTTI. We need to match that;
        # otherwise, we'll get linker errors about missing RTTI data.
        COMPILE_FLAGS "-fno-rtti"
        )
if(APPLE)
    set_target_properties(Assignment3 PROPERTIES
            LINK_FLAGS "-undefined dynamic_lookup"
            )
endif(APPLE)