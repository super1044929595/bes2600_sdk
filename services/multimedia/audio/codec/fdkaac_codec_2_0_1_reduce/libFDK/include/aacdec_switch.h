#ifndef __AACDEC_SWITCH_H__
#define __AACDEC_SWITCH_H__

#define FDKaacDEC_DISABLE_ADIF

#define FDKaacDEC_ONLY_A2DP_MCP1
#ifdef FDKaacDEC_ONLY_A2DP_MCP1
#define FDKaacDEC_DISABLE_ADTS
#endif
#define FDKaacDEC_DISABLE_DRM
//#define FDKaacDEC_ENABLE_CONCEAL
//#define FDKaacDEC_ENABLE_SBR

#endif