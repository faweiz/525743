# 
# Usage: To re-create this platform project launch xsct with below options.
# xsct C:\Users\fawei\Downloads\z20\525743\FPGA\Zynq_musicPlayer_FPGA\vitis\system_wrapper\platform.tcl
# 
# OR launch xsct and run below command.
# source C:\Users\fawei\Downloads\z20\525743\FPGA\Zynq_musicPlayer_FPGA\vitis\system_wrapper\platform.tcl
# 
# To create the platform in a different location, modify the -out option of "platform create" command.
# -out option specifies the output directory of the platform project.

platform create -name {system_wrapper}\
-hw {C:\Users\fawei\Downloads\z20\525743\FPGA\Zynq_musicPlayer_FPGA\system_wrapper.xsa}\
-out {C:/Users/fawei/Downloads/z20/525743/FPGA/Zynq_musicPlayer_FPGA/vitis}

platform write
domain create -name {standalone_ps7_cortexa9_0} -display-name {standalone_ps7_cortexa9_0} -os {standalone} -proc {ps7_cortexa9_0} -runtime {cpp} -arch {32-bit} -support-app {empty_application}
platform generate -domains 
platform active {system_wrapper}
domain active {zynq_fsbl}
domain active {standalone_ps7_cortexa9_0}
platform generate -quick
platform generate
domain active {zynq_fsbl}
bsp reload
domain active {standalone_ps7_cortexa9_0}
bsp reload
bsp setlib -name lwip213 -ver 1.0
bsp removelib -name lwip213
bsp setlib -name lwip213 -ver 1.0
bsp setlib -name xilffs -ver 5.0
bsp reload
domain active {zynq_fsbl}
bsp write
domain active {standalone_ps7_cortexa9_0}
bsp setlib -name lwip213 -ver 1.0
bsp removelib -name lwip213
bsp setlib -name lwip213 -ver 1.0
bsp removelib -name lwip213
bsp setlib -name lwip213 -ver 1.0
bsp setlib -name xilffs -ver 5.0
bsp config dhcp_does_arp_check "true"
bsp config lwip_dhcp "false"
bsp config lwip_dhcp "true"
bsp config mem_size "524288"
bsp config memp_n_tcp_pcb "32"
bsp config memp_n_sys_timeout "8"
bsp config memp_n_pbuf "1024"
bsp config memp_n_tcp_pcb "32"
bsp config memp_n_tcp_seg "1024"
bsp config pbuf_pool_size "32768"
bsp config tcp_snd_buf "65535"
bsp config tcp_wnd "65535"
bsp config n_rx_descriptors "512"
bsp config n_tx_descriptors "512"
bsp config temac_use_jumbo_frames "false"
bsp write
bsp reload
catch {bsp regenerate}
bsp reload
platform clean
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform clean
platform generate
platform clean
platform clean
platform generate
platform clean
platform clean
platform config -updatehw {C:/Users/fawei/Downloads/z20/525743/FPGA/Zynq_musicPlayer_FPGA/system_wrapper.xsa}
platform clean
platform clean
platform config -updatehw {C:/Users/fawei/Downloads/z20/525743/FPGA/Zynq_musicPlayer_FPGA/system_wrapper.xsa}
platform generate
bsp reload
bsp reload
platform generate -domains 
platform active {system_wrapper}
platform config -updatehw {C:/Users/fawei/Downloads/z20/525743/FPGA/Zynq_musicPlayer_FPGA/system_wrapper.xsa}
platform clean
platform clean
platform generate
platform clean
platform config -updatehw {C:/Users/fawei/Downloads/z20/525743/FPGA/Zynq_musicPlayer_FPGA/system_wrapper.xsa}
platform generate
