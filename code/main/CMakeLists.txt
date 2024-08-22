set(srcs
	"ethernet.c"
	"http.c"
	"ota.c"
	"main.c"
)

idf_build_get_property(project_dir PROJECT_DIR)
idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS ".")
