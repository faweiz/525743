# Usage with Vitis IDE:
# In Vitis IDE create a Single Application Debug launch configuration,
# change the debug type to 'Attach to running target' and provide this 
# tcl script in 'Execute Script' option.
# Path of this script: C:\Users\fawei\Downloads\z20\525743\FPGA\Zynq_musicPlayer_FPGA\vitis\Zynq_musicPlayer_vitis_system\_ide\scripts\systemdebugger_zynq_musicplayer_vitis_system_standalone.tcl
# 
# 
# Usage with xsct:
# To debug using xsct, launch xsct and run below command
# source C:\Users\fawei\Downloads\z20\525743\FPGA\Zynq_musicPlayer_FPGA\vitis\Zynq_musicPlayer_vitis_system\_ide\scripts\systemdebugger_zynq_musicplayer_vitis_system_standalone.tcl
# 
connect -url tcp:127.0.0.1:3121
targets -set -nocase -filter {name =~"APU*"}
rst -system
after 3000
targets -set -filter {jtag_cable_name =~ "Digilent JTAG-HS1 210512180081" && level==0 && jtag_device_ctx=="jsn-JTAG-HS1-210512180081-23727093-0"}
fpga -file C:/Users/fawei/Downloads/z20/525743/FPGA/Zynq_musicPlayer_FPGA/vitis/Zynq_musicPlayer_vitis/_ide/bitstream/system_wrapper.bit
targets -set -nocase -filter {name =~"APU*"}
loadhw -hw C:/Users/fawei/Downloads/z20/525743/FPGA/Zynq_musicPlayer_FPGA/vitis/system_wrapper/export/system_wrapper/hw/system_wrapper.xsa -mem-ranges [list {0x40000000 0xbfffffff}] -regs
configparams force-mem-access 1
targets -set -nocase -filter {name =~"APU*"}
source C:/Users/fawei/Downloads/z20/525743/FPGA/Zynq_musicPlayer_FPGA/vitis/Zynq_musicPlayer_vitis/_ide/psinit/ps7_init.tcl
ps7_init
ps7_post_config
targets -set -nocase -filter {name =~ "*A9*#0"}
dow C:/Users/fawei/Downloads/z20/525743/FPGA/Zynq_musicPlayer_FPGA/vitis/Zynq_musicPlayer_vitis/Debug/Zynq_musicPlayer_vitis.elf
configparams force-mem-access 0
targets -set -nocase -filter {name =~ "*A9*#0"}
con
