# AirPortOpenBSD

**基于OpenBSD项目的Mac操作系统Wi-Fi适配器内核扩展**


## 文档说明

此驱动尽可能的保证了OpenBSD代码的完整性。

已迁移的驱动包含了以下：

iwm、iwx、iwn、ipw、iwi、wpi、bwfm、bwi、rtwn。

其中iwm、iwx已通过测试，其他驱动有待验证。


## 参考

在构建这个驱动程序时，我依赖于:

- [openbsd/src](https://github.com/openbsd/src)
- [mercurysquad/Voodoo80211](https://github.com/mercurysquad/Voodoo80211)
- [AppleIntelWiFi/Black80211-Catalina](https://github.com/AppleIntelWiFi/Black80211-Catalina)
- [rpeshkov/black80211](https://github.com/rpeshkov/black80211)
- [Mieze/IntelMausiEthernet](https://github.com/Mieze/IntelMausiEthernet)
- [www.kernel.org](https://www.kernel.org)
- [OpenIntelWireless/itlwm](https://github.com/OpenIntelWireless/itlwm)


