
add_executable(fenrir_tests
    test_main.cpp
    test_hash_engines.cpp
    test_hash_parser.cpp
    test_rule_parser.cpp
    test_rule_engine.cpp
    test_mask_generator.cpp
    test_checkpoint.cpp
    test_api_manager.cpp
    test_pipeline.cpp
)

target_include_directories(fenrir_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(fenrir_tests PRIVATE
    Catch2::Catch2WithMain
    fenrir_core
    fenrir_utils
    fenrir_attack
    fenrir_rules
    fenrir_cli
    $<$<BOOL:${FENRIR_USE_OPENCL}>:fenrir_gpu>
    $<$<BOOL:${FENRIR_USE_CURL}>:fenrir_api>
)

catch_discover_tests(fenrir_tests)
