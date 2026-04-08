//Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
//Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
//--------------------------------------------------------------------------------
//Tool Version: Vivado v.2023.1 (win64) Build 3865809 Sun May  7 15:05:29 MDT 2023
//Date        : Wed Apr  8 12:22:07 2026
//Host        : TONY-MSI running 64-bit major release  (build 9200)
//Command     : generate_target system_wrapper.bd
//Design      : system_wrapper
//Purpose     : IP block netlist
//--------------------------------------------------------------------------------
`timescale 1 ps / 1 ps

module system_wrapper
   (DDR_addr,
    DDR_ba,
    DDR_cas_n,
    DDR_ck_n,
    DDR_ck_p,
    DDR_cke,
    DDR_cs_n,
    DDR_dm,
    DDR_dq,
    DDR_dqs_n,
    DDR_dqs_p,
    DDR_odt,
    DDR_ras_n,
    DDR_reset_n,
    DDR_we_n,
    FIXED_IO_ddr_vrn,
    FIXED_IO_ddr_vrp,
    FIXED_IO_mio,
    FIXED_IO_ps_clk,
    FIXED_IO_ps_porb,
    FIXED_IO_ps_srstb,
    GPIO_EMIO_tri_io,
    aud_adc_data,
    aud_adc_lrc,
    aud_bclk,
    aud_dac_data,
    aud_dac_lrc,
    aud_iic_scl_io,
    aud_iic_sda_io,
    aud_mclk);
  inout [14:0]DDR_addr;
  inout [2:0]DDR_ba;
  inout DDR_cas_n;
  inout DDR_ck_n;
  inout DDR_ck_p;
  inout DDR_cke;
  inout DDR_cs_n;
  inout [3:0]DDR_dm;
  inout [31:0]DDR_dq;
  inout [3:0]DDR_dqs_n;
  inout [3:0]DDR_dqs_p;
  inout DDR_odt;
  inout DDR_ras_n;
  inout DDR_reset_n;
  inout DDR_we_n;
  inout FIXED_IO_ddr_vrn;
  inout FIXED_IO_ddr_vrp;
  inout [53:0]FIXED_IO_mio;
  inout FIXED_IO_ps_clk;
  inout FIXED_IO_ps_porb;
  inout FIXED_IO_ps_srstb;
  inout [7:0]GPIO_EMIO_tri_io;
  input aud_adc_data;
  output [0:0]aud_adc_lrc;
  output [0:0]aud_bclk;
  output [0:0]aud_dac_data;
  output [0:0]aud_dac_lrc;
  inout aud_iic_scl_io;
  inout aud_iic_sda_io;
  output aud_mclk;

  wire [14:0]DDR_addr;
  wire [2:0]DDR_ba;
  wire DDR_cas_n;
  wire DDR_ck_n;
  wire DDR_ck_p;
  wire DDR_cke;
  wire DDR_cs_n;
  wire [3:0]DDR_dm;
  wire [31:0]DDR_dq;
  wire [3:0]DDR_dqs_n;
  wire [3:0]DDR_dqs_p;
  wire DDR_odt;
  wire DDR_ras_n;
  wire DDR_reset_n;
  wire DDR_we_n;
  wire FIXED_IO_ddr_vrn;
  wire FIXED_IO_ddr_vrp;
  wire [53:0]FIXED_IO_mio;
  wire FIXED_IO_ps_clk;
  wire FIXED_IO_ps_porb;
  wire FIXED_IO_ps_srstb;
  wire [0:0]GPIO_EMIO_tri_i_0;
  wire [1:1]GPIO_EMIO_tri_i_1;
  wire [2:2]GPIO_EMIO_tri_i_2;
  wire [3:3]GPIO_EMIO_tri_i_3;
  wire [4:4]GPIO_EMIO_tri_i_4;
  wire [5:5]GPIO_EMIO_tri_i_5;
  wire [6:6]GPIO_EMIO_tri_i_6;
  wire [7:7]GPIO_EMIO_tri_i_7;
  wire [0:0]GPIO_EMIO_tri_io_0;
  wire [1:1]GPIO_EMIO_tri_io_1;
  wire [2:2]GPIO_EMIO_tri_io_2;
  wire [3:3]GPIO_EMIO_tri_io_3;
  wire [4:4]GPIO_EMIO_tri_io_4;
  wire [5:5]GPIO_EMIO_tri_io_5;
  wire [6:6]GPIO_EMIO_tri_io_6;
  wire [7:7]GPIO_EMIO_tri_io_7;
  wire [0:0]GPIO_EMIO_tri_o_0;
  wire [1:1]GPIO_EMIO_tri_o_1;
  wire [2:2]GPIO_EMIO_tri_o_2;
  wire [3:3]GPIO_EMIO_tri_o_3;
  wire [4:4]GPIO_EMIO_tri_o_4;
  wire [5:5]GPIO_EMIO_tri_o_5;
  wire [6:6]GPIO_EMIO_tri_o_6;
  wire [7:7]GPIO_EMIO_tri_o_7;
  wire [0:0]GPIO_EMIO_tri_t_0;
  wire [1:1]GPIO_EMIO_tri_t_1;
  wire [2:2]GPIO_EMIO_tri_t_2;
  wire [3:3]GPIO_EMIO_tri_t_3;
  wire [4:4]GPIO_EMIO_tri_t_4;
  wire [5:5]GPIO_EMIO_tri_t_5;
  wire [6:6]GPIO_EMIO_tri_t_6;
  wire [7:7]GPIO_EMIO_tri_t_7;
  wire aud_adc_data;
  wire [0:0]aud_adc_lrc;
  wire [0:0]aud_bclk;
  wire [0:0]aud_dac_data;
  wire [0:0]aud_dac_lrc;
  wire aud_iic_scl_i;
  wire aud_iic_scl_io;
  wire aud_iic_scl_o;
  wire aud_iic_scl_t;
  wire aud_iic_sda_i;
  wire aud_iic_sda_io;
  wire aud_iic_sda_o;
  wire aud_iic_sda_t;
  wire aud_mclk;

  IOBUF GPIO_EMIO_tri_iobuf_0
       (.I(GPIO_EMIO_tri_o_0),
        .IO(GPIO_EMIO_tri_io[0]),
        .O(GPIO_EMIO_tri_i_0),
        .T(GPIO_EMIO_tri_t_0));
  IOBUF GPIO_EMIO_tri_iobuf_1
       (.I(GPIO_EMIO_tri_o_1),
        .IO(GPIO_EMIO_tri_io[1]),
        .O(GPIO_EMIO_tri_i_1),
        .T(GPIO_EMIO_tri_t_1));
  IOBUF GPIO_EMIO_tri_iobuf_2
       (.I(GPIO_EMIO_tri_o_2),
        .IO(GPIO_EMIO_tri_io[2]),
        .O(GPIO_EMIO_tri_i_2),
        .T(GPIO_EMIO_tri_t_2));
  IOBUF GPIO_EMIO_tri_iobuf_3
       (.I(GPIO_EMIO_tri_o_3),
        .IO(GPIO_EMIO_tri_io[3]),
        .O(GPIO_EMIO_tri_i_3),
        .T(GPIO_EMIO_tri_t_3));
  IOBUF GPIO_EMIO_tri_iobuf_4
       (.I(GPIO_EMIO_tri_o_4),
        .IO(GPIO_EMIO_tri_io[4]),
        .O(GPIO_EMIO_tri_i_4),
        .T(GPIO_EMIO_tri_t_4));
  IOBUF GPIO_EMIO_tri_iobuf_5
       (.I(GPIO_EMIO_tri_o_5),
        .IO(GPIO_EMIO_tri_io[5]),
        .O(GPIO_EMIO_tri_i_5),
        .T(GPIO_EMIO_tri_t_5));
  IOBUF GPIO_EMIO_tri_iobuf_6
       (.I(GPIO_EMIO_tri_o_6),
        .IO(GPIO_EMIO_tri_io[6]),
        .O(GPIO_EMIO_tri_i_6),
        .T(GPIO_EMIO_tri_t_6));
  IOBUF GPIO_EMIO_tri_iobuf_7
       (.I(GPIO_EMIO_tri_o_7),
        .IO(GPIO_EMIO_tri_io[7]),
        .O(GPIO_EMIO_tri_i_7),
        .T(GPIO_EMIO_tri_t_7));
  IOBUF aud_iic_scl_iobuf
       (.I(aud_iic_scl_o),
        .IO(aud_iic_scl_io),
        .O(aud_iic_scl_i),
        .T(aud_iic_scl_t));
  IOBUF aud_iic_sda_iobuf
       (.I(aud_iic_sda_o),
        .IO(aud_iic_sda_io),
        .O(aud_iic_sda_i),
        .T(aud_iic_sda_t));
  system system_i
       (.DDR_addr(DDR_addr),
        .DDR_ba(DDR_ba),
        .DDR_cas_n(DDR_cas_n),
        .DDR_ck_n(DDR_ck_n),
        .DDR_ck_p(DDR_ck_p),
        .DDR_cke(DDR_cke),
        .DDR_cs_n(DDR_cs_n),
        .DDR_dm(DDR_dm),
        .DDR_dq(DDR_dq),
        .DDR_dqs_n(DDR_dqs_n),
        .DDR_dqs_p(DDR_dqs_p),
        .DDR_odt(DDR_odt),
        .DDR_ras_n(DDR_ras_n),
        .DDR_reset_n(DDR_reset_n),
        .DDR_we_n(DDR_we_n),
        .FIXED_IO_ddr_vrn(FIXED_IO_ddr_vrn),
        .FIXED_IO_ddr_vrp(FIXED_IO_ddr_vrp),
        .FIXED_IO_mio(FIXED_IO_mio),
        .FIXED_IO_ps_clk(FIXED_IO_ps_clk),
        .FIXED_IO_ps_porb(FIXED_IO_ps_porb),
        .FIXED_IO_ps_srstb(FIXED_IO_ps_srstb),
        .GPIO_EMIO_tri_i({GPIO_EMIO_tri_i_7,GPIO_EMIO_tri_i_6,GPIO_EMIO_tri_i_5,GPIO_EMIO_tri_i_4,GPIO_EMIO_tri_i_3,GPIO_EMIO_tri_i_2,GPIO_EMIO_tri_i_1,GPIO_EMIO_tri_i_0}),
        .GPIO_EMIO_tri_o({GPIO_EMIO_tri_o_7,GPIO_EMIO_tri_o_6,GPIO_EMIO_tri_o_5,GPIO_EMIO_tri_o_4,GPIO_EMIO_tri_o_3,GPIO_EMIO_tri_o_2,GPIO_EMIO_tri_o_1,GPIO_EMIO_tri_o_0}),
        .GPIO_EMIO_tri_t({GPIO_EMIO_tri_t_7,GPIO_EMIO_tri_t_6,GPIO_EMIO_tri_t_5,GPIO_EMIO_tri_t_4,GPIO_EMIO_tri_t_3,GPIO_EMIO_tri_t_2,GPIO_EMIO_tri_t_1,GPIO_EMIO_tri_t_0}),
        .aud_adc_data(aud_adc_data),
        .aud_adc_lrc(aud_adc_lrc),
        .aud_bclk(aud_bclk),
        .aud_dac_data(aud_dac_data),
        .aud_dac_lrc(aud_dac_lrc),
        .aud_iic_scl_i(aud_iic_scl_i),
        .aud_iic_scl_o(aud_iic_scl_o),
        .aud_iic_scl_t(aud_iic_scl_t),
        .aud_iic_sda_i(aud_iic_sda_i),
        .aud_iic_sda_o(aud_iic_sda_o),
        .aud_iic_sda_t(aud_iic_sda_t),
        .aud_mclk(aud_mclk));
endmodule
