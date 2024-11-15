void GT911_Int() {
  uint8_t config_Checksum = 0;

  IIC_Init();
  GT911_Reset_Sequence();

  GT911_RD_Reg(GT_CFGS_REG, (uint8_t *)&s_GT911_CfgParams[0], 186);

  for (uint8_t i = 0; i < sizeof(s_GT911_CfgParams) - 2; i++) {
    config_Checksum += s_GT911_CfgParams[i];
  }

  if (s_GT911_CfgParams[184] != ((~config_Checksum + 1) & 0xff)) {
    s_GT911_CfgParams[1] = GT911_MAX_WIDTH & 0xff;
    s_GT911_CfgParams[2] = GT911_MAX_WIDTH >> 8;
    s_GT911_CfgParams[3] = GT911_MAX_HEIGHT & 0xff;
    s_GT911_CfgParams[4] = GT911_MAX_HEIGHT >> 8;
    s_GT911_CfgParams[185] = 1;

    config_Checksum = 0;
    for (uint8_t i = 0; i < sizeof(s_GT911_CfgParams) - 2; i++) {
      config_Checksum += s_GT911_CfgParams[i];
    }
    s_GT911_CfgParams[184] = (~config_Checksum) + 1;

    GT911_WR_Reg(GT_CFGS_REG, (uint8_t *)s_GT911_CfgParams, sizeof(s_GT911_CfgParams));
  }

  GT911_ReadStatue();
}
