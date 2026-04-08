#----------------------LCD½Ó¿Ú---------------------------

#RGB LCD
set_property -dict {PACKAGE_PIN Y18 IOSTANDARD LVCMOS33} [get_ports {lcd_rgb_tri_io[0]}]
set_property -dict {PACKAGE_PIN Y19 IOSTANDARD LVCMOS33} [get_ports {lcd_rgb_tri_io[1]}]
set_property -dict {PACKAGE_PIN W20 IOSTANDARD LVCMOS33} [get_ports {lcd_rgb_tri_io[2]}]
set_property -dict {PACKAGE_PIN V20 IOSTANDARD LVCMOS33} [get_ports {lcd_rgb_tri_io[3]}]
set_property -dict {PACKAGE_PIN U14 IOSTANDARD LVCMOS33} [get_ports {lcd_rgb_tri_io[4]}]
set_property -dict {PACKAGE_PIN U15 IOSTANDARD LVCMOS33} [get_ports {lcd_rgb_tri_io[5]}]
set_property -dict {PACKAGE_PIN T20 IOSTANDARD LVCMOS33} [get_ports {lcd_rgb_tri_io[6]}]
set_property -dict {PACKAGE_PIN U20 IOSTANDARD LVCMOS33} [get_ports {lcd_rgb_tri_io[7]}]
set_property -dict {PACKAGE_PIN W14 IOSTANDARD LVCMOS33} [get_ports {lcd_rgb_tri_io[8]}]
set_property -dict {PACKAGE_PIN Y14 IOSTANDARD LVCMOS33} [get_ports {lcd_rgb_tri_io[9]}]
set_property -dict {PACKAGE_PIN N15 IOSTANDARD LVCMOS33} [get_ports {lcd_rgb_tri_io[10]}]
set_property -dict {PACKAGE_PIN N16 IOSTANDARD LVCMOS33} [get_ports {lcd_rgb_tri_io[11]}]
set_property -dict {PACKAGE_PIN V16 IOSTANDARD LVCMOS33} [get_ports {lcd_rgb_tri_io[12]}]
set_property -dict {PACKAGE_PIN W16 IOSTANDARD LVCMOS33} [get_ports {lcd_rgb_tri_io[13]}]
set_property -dict {PACKAGE_PIN W18 IOSTANDARD LVCMOS33} [get_ports {lcd_rgb_tri_io[14]}]
set_property -dict {PACKAGE_PIN W19 IOSTANDARD LVCMOS33} [get_ports {lcd_rgb_tri_io[15]}]
set_property -dict {PACKAGE_PIN T10 IOSTANDARD LVCMOS33} [get_ports {lcd_rgb_tri_io[16]}]
set_property -dict {PACKAGE_PIN T11 IOSTANDARD LVCMOS33} [get_ports {lcd_rgb_tri_io[17]}]
set_property -dict {PACKAGE_PIN P14 IOSTANDARD LVCMOS33} [get_ports {lcd_rgb_tri_io[18]}]
set_property -dict {PACKAGE_PIN R14 IOSTANDARD LVCMOS33} [get_ports {lcd_rgb_tri_io[19]}]
set_property -dict {PACKAGE_PIN V13 IOSTANDARD LVCMOS33} [get_ports {lcd_rgb_tri_io[20]}]
set_property -dict {PACKAGE_PIN U13 IOSTANDARD LVCMOS33} [get_ports {lcd_rgb_tri_io[21]}]
set_property -dict {PACKAGE_PIN G15 IOSTANDARD LVCMOS33} [get_ports {lcd_rgb_tri_io[22]}]
set_property -dict {PACKAGE_PIN H15 IOSTANDARD LVCMOS33} [get_ports {lcd_rgb_tri_io[23]}]
set_property -dict {PACKAGE_PIN U17 IOSTANDARD LVCMOS33} [get_ports lcd_hs]
set_property -dict {PACKAGE_PIN P20 IOSTANDARD LVCMOS33} [get_ports lcd_vs]
set_property -dict {PACKAGE_PIN N20 IOSTANDARD LVCMOS33} [get_ports lcd_de]
set_property -dict {PACKAGE_PIN Y16 IOSTANDARD LVCMOS33} [get_ports lcd_bl]
set_property -dict {PACKAGE_PIN T16 IOSTANDARD LVCMOS33} [get_ports lcd_clk]
set_property -dict {PACKAGE_PIN J15  IOSTANDARD LVCMOS33} [get_ports lcd_rst]

#lcd_scl:
set_property -dict {PACKAGE_PIN V17 IOSTANDARD LVCMOS33} [get_ports {GPIO_EMIO_tri_io[0]}]
#lcd_sda:
set_property -dict {PACKAGE_PIN M19 IOSTANDARD LVCMOS33} [get_ports {GPIO_EMIO_tri_io[1]}]
#CT_RST
set_property -dict {PACKAGE_PIN Y17 IOSTANDARD LVCMOS33} [get_ports {GPIO_EMIO_tri_io[2]}]
#CT_INT
set_property -dict {PACKAGE_PIN V18 IOSTANDARD LVCMOS33} [get_ports {GPIO_EMIO_tri_io[3]}]

set_property PULLUP true [get_ports {GPIO_EMIO_tri_io[1]}]


#audio WM8960
set_property -dict {PACKAGE_PIN M17 IOSTANDARD LVCMOS33} [get_ports aud_iic_scl_io]
set_property -dict {PACKAGE_PIN M18 IOSTANDARD LVCMOS33} [get_ports aud_iic_sda_io]
set_property -dict {PACKAGE_PIN C20 IOSTANDARD LVCMOS33} [get_ports aud_adc_lrc]
set_property -dict {PACKAGE_PIN A20 IOSTANDARD LVCMOS33} [get_ports aud_adc_data]
set_property -dict {PACKAGE_PIN B20 IOSTANDARD LVCMOS33} [get_ports aud_dac_lrc]
set_property -dict {PACKAGE_PIN K16 IOSTANDARD LVCMOS33} [get_ports aud_dac_data]
set_property -dict {PACKAGE_PIN B19 IOSTANDARD LVCMOS33} [get_ports aud_bclk]
set_property -dict {PACKAGE_PIN U19 IOSTANDARD LVCMOS33} [get_ports aud_mclk]
#set_property -dict {PACKAGE_PIN U18 IOSTANDARD LVCMOS33} [get_ports sys_clk]

#PL_KEY0
set_property PACKAGE_PIN L20 [get_ports {GPIO_EMIO_tri_io[4]}]
#PL_KEY1
set_property PACKAGE_PIN J20 [get_ports {GPIO_EMIO_tri_io[5]}]
#PL_LED0
set_property PACKAGE_PIN J18 [get_ports {GPIO_EMIO_tri_io[6]}]
#PL_LED1
set_property PACKAGE_PIN H18 [get_ports {GPIO_EMIO_tri_io[7]}]
set_property IOSTANDARD LVCMOS33 [get_ports {GPIO_EMIO_tri_io[7]}]
set_property IOSTANDARD LVCMOS33 [get_ports {GPIO_EMIO_tri_io[6]}]
set_property IOSTANDARD LVCMOS33 [get_ports {GPIO_EMIO_tri_io[5]}]
set_property IOSTANDARD LVCMOS33 [get_ports {GPIO_EMIO_tri_io[4]}]
