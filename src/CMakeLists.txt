# Adding shader sources as well, for better integration with IDEs
add_executable(plot main.cpp plot.vert plot.frag)

install(TARGETS plot DESTINATION ".")
