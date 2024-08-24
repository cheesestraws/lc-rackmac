set(srcs
	"buffer_pool.c"
	"ethernet.c"
	"http.c"
	"http_noncore.c"
	"gpio_interrupts.c"
	"temperature.c"
	"ota.c"
	"led.c"
	"reset.c"
	"stats.c"
	"main.c"
)

idf_build_get_property(project_dir PROJECT_DIR)
idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS ".")
