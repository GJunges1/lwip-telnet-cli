set(COMPONENT_SRCS "lwip_cli_main.c" "FreeRTOS_CLI.c" "FreeRTOS_CLI.h")
set(COMPONENT_ADD_INCLUDEDIRS "")

register_component()

target_compile_definitions(${COMPONENT_TARGET} PUBLIC "-DREGION_AU915" "-DACTIVE_REGION=LORAMAC_REGION_AU915")
