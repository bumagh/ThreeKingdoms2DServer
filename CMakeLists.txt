# 自动搜索当前目录及其子目录中的所有 C 文件  
file(GLOB_RECURSE SOURCES "src/*.c")  

add_executable(tk
	${SOURCES}
)

target_link_libraries(tk
	# /root/work/gitrep/lwan/src/samples/ThreeKingdoms2DServer/src/Model/model.a
	sqlite3
	m
	${LWAN_COMMON_LIBS}
	${ADDITIONAL_LIBRARIES}
)
