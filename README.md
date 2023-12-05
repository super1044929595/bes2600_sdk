# BES2600IHC
基于BES2600IHC SDK个人架构搭建

STDF: Standard Framwork, 标准架构
stdf/stdf_app：应用层，产品功能
stdf/stdf_bsp：板载驱动层，touch，入耳检测，外挂charger，NTC，
stdf/stdf_hal：硬件抽象层，i2c，spi，中断，内置charger
stdf/stdf_os：操作系统，用户自己封装的系统接口，比如消息
stdf/stdf_sdk：sdk的回调和重新封装的api
