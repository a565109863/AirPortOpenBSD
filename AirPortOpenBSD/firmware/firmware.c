//
//  firmware.c
//  AirPortOpenBSD

//  Created by Zhong-Mac on 2020/7/18.
//  Copyright © 2020 Zhong-Mac. All rights reserved.

#include "firmware.h"

const struct firmware firmwares[] = {
    {FIRMWARE("rtwn-rtl8192cfwU_B", rtwn_rtl8192cfwU_B, rtwn_rtl8192cfwU_B_size)},
    {FIRMWARE("iwi-bss", iwi_bss, iwi_bss_size)},
    {FIRMWARE("iwx-Qu-c0-jf-b0-48", iwx_Qu_c0_jf_b0_48, iwx_Qu_c0_jf_b0_48_size)},
    {FIRMWARE("iwm-8265-34", iwm_8265_34, iwm_8265_34_size)},
    {FIRMWARE("iwn-5150", iwn_5150, iwn_5150_size)},
    {FIRMWARE("iwm-3160-17", iwm_3160_17, iwm_3160_17_size)},
    {FIRMWARE("brcmfmac4356-pcie.bin", brcmfmac4356_pcie_bin, brcmfmac4356_pcie_bin_size)},
    {FIRMWARE("iwn-6005", iwn_6005, iwn_6005_size)},
    {FIRMWARE("iwn-4965", iwn_4965, iwn_4965_size)},
    {FIRMWARE("iwx-QuZ-a0-hr-b0-48", iwx_QuZ_a0_hr_b0_48, iwx_QuZ_a0_hr_b0_48_size)},
    {FIRMWARE("rtwn-rtl8723fw", rtwn_rtl8723fw, rtwn_rtl8723fw_size)},
    {FIRMWARE("iwn-6050", iwn_6050, iwn_6050_size)},
    {FIRMWARE("iwx-QuQnj-b0-jf-b0-48", iwx_QuQnj_b0_jf_b0_48, iwx_QuQnj_b0_jf_b0_48_size)},
    {FIRMWARE("brcmfmac4350-pcie.bin", brcmfmac4350_pcie_bin, brcmfmac4350_pcie_bin_size)},
    {FIRMWARE("iwx-Qu-b0-jf-b0-48", iwx_Qu_b0_jf_b0_48, iwx_Qu_b0_jf_b0_48_size)},
    {FIRMWARE("iwn-1000", iwn_1000, iwn_1000_size)},
    {FIRMWARE("rtwn-rtl8188efw", rtwn_rtl8188efw, rtwn_rtl8188efw_size)},
    {FIRMWARE("iwn-100", iwn_100, iwn_100_size)},
    {FIRMWARE("iwn-5000", iwn_5000, iwn_5000_size)},
    {FIRMWARE("iwm-9260-34", iwm_9260_34, iwm_9260_34_size)},
    {FIRMWARE("ipw-monitor", ipw_monitor, ipw_monitor_size)},
    {FIRMWARE("brcmfmac4350c2-pcie.bin", brcmfmac4350c2_pcie_bin, brcmfmac4350c2_pcie_bin_size)},
    {FIRMWARE("iwi-monitor", iwi_monitor, iwi_monitor_size)},
    {FIRMWARE("iwx-cc-a0-48", iwx_cc_a0_48, iwx_cc_a0_48_size)},
    {FIRMWARE("iwx-Qu-c0-hr-b0-48", iwx_Qu_c0_hr_b0_48, iwx_Qu_c0_hr_b0_48_size)},
    {FIRMWARE("iwm-9000-34", iwm_9000_34, iwm_9000_34_size)},
    {FIRMWARE("rtwn-rtl8723fw_B", rtwn_rtl8723fw_B, rtwn_rtl8723fw_B_size)},
    {FIRMWARE("iwm-3168-29", iwm_3168_29, iwm_3168_29_size)},
    {FIRMWARE("brcmfmac43602-pcie.bin", brcmfmac43602_pcie_bin, brcmfmac43602_pcie_bin_size)},
    {FIRMWARE("bwi-airforce", bwi_airforce, bwi_airforce_size)},
    {FIRMWARE("ipw-bss", ipw_bss, ipw_bss_size)},
    {FIRMWARE("iwn-6030", iwn_6030, iwn_6030_size)},
    {FIRMWARE("iwn-2000", iwn_2000, iwn_2000_size)},
    {FIRMWARE("iwm-7265D-29", iwm_7265D_29, iwm_7265D_29_size)},
    {FIRMWARE("iwx-QuZ-a0-jf-b0-48", iwx_QuZ_a0_jf_b0_48, iwx_QuZ_a0_jf_b0_48_size)},
    {FIRMWARE("iwn-6000", iwn_6000, iwn_6000_size)},
    {FIRMWARE("iwn-2030", iwn_2030, iwn_2030_size)},
    {FIRMWARE("iwx-QuQnj-b0-hr-b0-48", iwx_QuQnj_b0_hr_b0_48, iwx_QuQnj_b0_hr_b0_48_size)},
    {FIRMWARE("iwx-Qu-b0-hr-b0-48", iwx_Qu_b0_hr_b0_48, iwx_Qu_b0_hr_b0_48_size)},
    {FIRMWARE("wpi-3945abg", wpi_3945abg, wpi_3945abg_size)},
    {FIRMWARE("iwn-105", iwn_105, iwn_105_size)},
    {FIRMWARE("iwm-7265-17", iwm_7265_17, iwm_7265_17_size)},
    {FIRMWARE("brcmfmac4371-pcie.bin", brcmfmac4371_pcie_bin, brcmfmac4371_pcie_bin_size)},
    {FIRMWARE("iwn-135", iwn_135, iwn_135_size)},
    {FIRMWARE("iwm-7260-17", iwm_7260_17, iwm_7260_17_size)},
    {FIRMWARE("iwm-8000C-34", iwm_8000C_34, iwm_8000C_34_size)},
    {FIRMWARE("rtwn-rtl8192cfwU", rtwn_rtl8192cfwU, rtwn_rtl8192cfwU_size)},
    {FIRMWARE("ipw-ibss", ipw_ibss, ipw_ibss_size)},
    {FIRMWARE("iwi-ibss", iwi_ibss, iwi_ibss_size)},
};
const int firmwares_total = 49;
