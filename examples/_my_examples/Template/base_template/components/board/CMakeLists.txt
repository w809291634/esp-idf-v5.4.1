idf_component_register(SRCS "board.c"
                    INCLUDE_DIRS .
                    REQUIRES spi_flash apl_utility apl_console board_config)
