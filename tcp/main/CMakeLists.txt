idf_component_register(SRCS "app_main.c""gpio_setup.c""adc_module.c""mqtt.c""wifi.c"
                    INCLUDE_DIRS ".")
target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
