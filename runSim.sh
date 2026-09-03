unset GTK_PATH
unset LD_LIBRARY_PATH

make
gnome-terminal -- bash -c "./build/main; exec bash"
gnome-terminal -- bash -c "./build/imuProgram; exec bash"
gnome-terminal -- bash -c "./build/navProgram; exec bash"
gnome-terminal -- bash -c "./build/guidProgram; exec bash"
gnome-terminal -- bash -c "./build/initFileLoader; exec bash"




