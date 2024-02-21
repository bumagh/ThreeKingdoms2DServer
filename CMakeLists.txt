add_executable(tk
	main.c
)

target_link_libraries(tk
	${LWAN_COMMON_LIBS}
	${ADDITIONAL_LIBRARIES}
)
