/*
 * Copyright (C) 2025 Linux Studio Plugins Project <https://lsp-plug.in/>
 *           (C) 2025 Vladimir Sadovnikov <sadko4u@gmail.com>
 *
 * This file is part of lsp-plugins-mb-expander
 * Created on: 3 авг. 2021 г.
 *
 * lsp-plugins-mb-expander is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * lsp-plugins-mb-expander is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with lsp-plugins-mb-expander. If not, see <https://www.gnu.org/licenses/>.
 */

#include <lsp-plug.in/plug-fw/meta/ports.h>
#include <lsp-plug.in/shared/meta/developers.h>
#include <lsp-plug.in/dsp-units/util/Sidechain.h>
#include <private/meta/mb_expander.h>

#define LSP_PLUGINS_MB_EXPANDER_VERSION_MAJOR       1
#define LSP_PLUGINS_MB_EXPANDER_VERSION_MINOR       0
#define LSP_PLUGINS_MB_EXPANDER_VERSION_MICRO       25

#define LSP_PLUGINS_MB_EXPANDER_VERSION  \
    LSP_MODULE_VERSION( \
        LSP_PLUGINS_MB_EXPANDER_VERSION_MAJOR, \
        LSP_PLUGINS_MB_EXPANDER_VERSION_MINOR, \
        LSP_PLUGINS_MB_EXPANDER_VERSION_MICRO  \
    )

namespace lsp
{
    namespace meta
    {
        //-------------------------------------------------------------------------
        // Multiband expander
        static const int plugin_classes[]           = { C_EXPANDER, -1 };
        static const int clap_features_mono[]       = { CF_AUDIO_EFFECT, CF_MONO, -1 };
        static const int clap_features_stereo[]     = { CF_AUDIO_EFFECT, CF_STEREO, -1 };

        static const port_item_t mb_exp_sc_type[] =
        {
            { "Internal",       "sidechain.internal"        },
            { "Link",           "sidechain.link"            },
            { NULL, NULL }
        };

        static const port_item_t mb_exp_sc_type_sc[] =
        {
            { "Internal",       "sidechain.internal"        },
            { "External",       "sidechain.external"        },
            { "Link",           "sidechain.link"            },
            { NULL, NULL }
        };

        static const port_item_t mb_exp_sc_modes[] =
        {
            { "Peak",           "sidechain.peak"            },
            { "RMS",            "sidechain.rms"             },
            { "LPF",            "sidechain.lpf"             },
            { "SMA",            "sidechain.sma"             },
            { NULL, NULL }
        };

        static const port_item_t mb_exp_sc_source[] =
        {
            { "Middle",         "sidechain.middle"          },
            { "Side",           "sidechain.side"            },
            { "Left",           "sidechain.left"            },
            { "Right",          "sidechain.right"           },
            { "Min",            "sidechain.min"             },
            { "Max",            "sidechain.max"             },
            { NULL, NULL }
        };

        static const port_item_t mb_exp_sc_split_source[] =
        {
            { "Left/Right",     "sidechain.left_right"      },
            { "Right/Left",     "sidechain.right_left"      },
            { "Mid/Side",       "sidechain.mid_side"        },
            { "Side/Mid",       "sidechain.side_mid"        },
            { "Min",            "sidechain.min"             },
            { "Max",            "sidechain.max"             },
            { NULL, NULL }
        };

        static const port_item_t mb_exp_sc_boost[] =
        {
            { "None",           "sidechain.boost.none"      },
            { "Pink BT",        "sidechain.boost.pink_bt"   },
            { "Pink MT",        "sidechain.boost.pink_mt"   },
            { "Brown BT",       "sidechain.boost.brown_bt"  },
            { "Brown MT",       "sidechain.boost.brown_mt"  },
            { NULL, NULL }
        };

        static const port_item_t mb_global_mb_exp_modes[] =
        {
            { "Classic",        "multiband.classic"         },
            { "Modern",         "multiband.modern"          },
            { "Linear Phase",   "multiband.linear_phase"    },
            { NULL, NULL }
        };

        static const port_item_t mb_exp_modes[] =
        {
            { "Down",       "mb_expand.down_ward"           },
            { "Up",         "mb_expand.up_ward"             },
            { NULL, NULL }
        };

        static const port_item_t exp_sc_bands[] =
        {
            { "Split",          "mb_expand.split" },
            { "Band 0",         "mb_expand.band0" },
            { "Band 1",         "mb_expand.band1" },
            { "Band 2",         "mb_expand.band2" },
            { "Band 3",         "mb_expand.band3" },
            { "Band 4",         "mb_expand.band4" },
            { "Band 5",         "mb_expand.band5" },
            { "Band 6",         "mb_expand.band6" },
            { "Band 7",         "mb_expand.band7" },
            { NULL, NULL }
        };

        static const port_item_t exp_sc_lr_bands[] =
        {
            { "Split Left",     "mb_expand.split_left" },
            { "Split Right",    "mb_expand.split_right" },
            { "Band 0",         "mb_expand.band0" },
            { "Band 1",         "mb_expand.band1" },
            { "Band 2",         "mb_expand.band2" },
            { "Band 3",         "mb_expand.band3" },
            { "Band 4",         "mb_expand.band4" },
            { "Band 5",         "mb_expand.band5" },
            { "Band 6",         "mb_expand.band6" },
            { "Band 7",         "mb_expand.band7" },
            { NULL, NULL }
        };

        static const port_item_t exp_sc_ms_bands[] =
        {
            { "Split Mid",      "mb_expand.split_middle" },
            { "Split Side",     "mb_expand.split_side" },
            { "Band 0",         "mb_expand.band0" },
            { "Band 1",         "mb_expand.band1" },
            { "Band 2",         "mb_expand.band2" },
            { "Band 3",         "mb_expand.band3" },
            { "Band 4",         "mb_expand.band4" },
            { "Band 5",         "mb_expand.band5" },
            { "Band 6",         "mb_expand.band6" },
            { "Band 7",         "mb_expand.band7" },
            { NULL, NULL }
        };

        #define MB_EXP_SHM_LINK_MONO \
            OPT_RETURN_MONO("link", "shml", "Side-chain shared memory link")

        #define MB_EXP_SHM_LINK_STEREO \
            OPT_RETURN_STEREO("link", "shml_", "Side-chain shared memory link")

        #define MB_COMMON(bands) \
            BYPASS, \
            COMBO("mode", "Expander mode", "Mode", 1, mb_global_mb_exp_modes), \
            AMP_GAIN("g_in", "Input gain", mb_expander_metadata::IN_GAIN_DFL, 10.0f), \
            AMP_GAIN("g_out", "Output gain", mb_expander_metadata::OUT_GAIN_DFL, 10.0f), \
            AMP_GAIN("g_dry", "Dry gain", 0.0f, 10.0f), \
            AMP_GAIN("g_wet", "Wet gain", 1.0f, 10.0f), \
            PERCENTS("drywet", "Dry/Wet balance", 100.0f, 0.1f), \
            LOG_CONTROL("react", "FFT reactivity", "Reactivity", U_MSEC, mb_expander_metadata::FFT_REACT_TIME), \
            AMP_GAIN("shift", "Shift gain", 1.0f, 100.0f), \
            LOG_CONTROL("zoom", "Graph zoom", "Zoom", U_GAIN_AMP, mb_expander_metadata::ZOOM), \
            COMBO("envb", "Envelope boost", "Env boost", mb_expander_metadata::FB_DEFAULT, mb_exp_sc_boost), \
            COMBO("bsel", "Band selection", "Band selector", mb_expander_metadata::SC_BAND_DFL, bands)

        #define MB_CHANNEL(id, label, alias) \
            SWITCH("flt" id, "Band filter curves" label, "Show flt" alias, 1.0f), \
            MESH("ag" id, "Expander amplitude graph " label, 2, mb_expander_metadata::FFT_MESH_POINTS)

        #define MB_SPLIT(id, label, alias, enable, freq) \
            SWITCH("cbe" id, "Expander band enable" label, "Split on " alias, enable), \
            LOG_CONTROL_DFL("sf" id, "Split frequency" label, "Split" alias, U_HZ, mb_expander_metadata::FREQ, freq)

        #define MB_BAND_COMMON(id, label, alias, x, total, fe, fs) \
            COMBO("scm" id, "Sidechain mode" label, "SC mode" alias, mb_expander_metadata::SC_MODE_DFL, mb_exp_sc_modes), \
            CONTROL("sla" id, "Sidechain lookahead" label, U_MSEC, mb_expander_metadata::LOOKAHEAD), \
            LOG_CONTROL("scr" id, "Sidechain reactivity" label, "SC react" alias, U_MSEC, mb_expander_metadata::REACTIVITY), \
            AMP_GAIN100("scp" id, "Sidechain preamp" label, GAIN_AMP_0_DB), \
            SWITCH("sclc" id, "Sidechain custom lo-cut" label, "SC LCF on" alias, 0), \
            SWITCH("schc" id, "Sidechain custom hi-cut" label, "SC HCF on" alias, 0), \
            LOG_CONTROL_DFL("sclf" id, "Sidechain lo-cut frequency" label, "SC LCF" alias, U_HZ, mb_expander_metadata::FREQ, fe), \
            LOG_CONTROL_DFL("schf" id, "Sidechain hi-cut frequency" label, "SC HCF" alias, U_HZ, mb_expander_metadata::FREQ, fs), \
            MESH("bfc" id, "Side-chain band frequency chart" label, 2, mb_expander_metadata::MESH_POINTS + 4), \
            \
            COMBO("em" id, "Expander mode" label, "Mode " alias, mb_expander_metadata::EM_DEFAULT, mb_exp_modes), \
            SWITCH("ee" id, "Expander enable" label, "On" alias, 1.0f), \
            SWITCH("bs" id, "Solo band" label, "Solo" alias, 0.0f), \
            SWITCH("bm" id, "Mute band" label, "Mute" alias, 0.0f), \
            LOG_CONTROL("al" id, "Attack threshold" label, "Att thresh" alias, U_GAIN_AMP, mb_expander_metadata::ATTACK_LVL), \
            LOG_CONTROL("at" id, "Attack time" label, "Att time" alias, U_MSEC, mb_expander_metadata::ATTACK_TIME), \
            LOG_CONTROL("rrl" id, "Release threshold" label, "Rel thresh" alias, U_GAIN_AMP, mb_expander_metadata::RELEASE_LVL), \
            LOG_CONTROL("rt" id, "Release time" label, "Rel time" alias, U_MSEC, mb_expander_metadata::RELEASE_TIME), \
            CONTROL("ht" id, "Hold time" label, U_MSEC, mb_expander_metadata::HOLD_TIME), \
            LOG_CONTROL("er" id, "Ratio" label, "Ratio" alias, U_NONE, mb_expander_metadata::RATIO), \
            LOG_CONTROL("kn" id, "Knee" label, "Knee" alias, U_GAIN_AMP, mb_expander_metadata::KNEE), \
            LOG_CONTROL("mk" id, "Makeup gain" label, "Makeup" alias, U_GAIN_AMP, mb_expander_metadata::MAKEUP), \
            HUE_CTL("hue" id, "Hue " label, (float(x) / float(total))), \
            METER("fre" id, "Frequency range end" label, U_HZ,  mb_expander_metadata::OUT_FREQ), \
            MESH("ccg" id, "Expander curve graph" label, 2, mb_expander_metadata::CURVE_MESH_SIZE), \
            METER_OUT_GAIN("rl" id, "Release level" label, 20.0f) \

        #define MB_BAND_METERS(id, label) \
            METER_OUT_GAIN("elm" id, "Envelope level meter" label, GAIN_AMP_P_36_DB), \
            METER_OUT_GAIN("clm" id, "Curve level meter" label, GAIN_AMP_P_36_DB), \
            METER_OUT_GAIN("rlm" id, "Reduction level meter" label, GAIN_AMP_P_24_DB)

        #define MB_MONO_BAND(id, label, alias, x, total, fe, fs) \
            COMBO("sce" id, "External sidechain source" label, "Ext SC src" alias, 0.0f, mb_exp_sc_type), \
            MB_BAND_COMMON(id, label, alias, x, total, fe, fs)

        #define MB_STEREO_BAND(id, label, alias, x, total, fe, fs) \
            COMBO("sce" id, "External sidechain source" label, "Ext SC src" alias, 0.0f, mb_exp_sc_type), \
            COMBO("scs" id, "Sidechain source" label, "SC source" alias, 0, mb_exp_sc_source), \
            COMBO("sscs" id, "Split sidechain source" label, "SC split" alias, 0, mb_exp_sc_split_source), \
            MB_BAND_COMMON(id, label, alias, x, total, fe, fs)

        #define MB_SPLIT_BAND(id, label, alias, x, total, fe, fs) \
            COMBO("sce" id, "External sidechain source" label, "Ext SC src" alias, 0.0f, mb_exp_sc_type), \
            COMBO("scs" id, "Sidechain source" label, "SC source" alias, 0, mb_exp_sc_source), \
            MB_BAND_COMMON(id, label, alias, x, total, fe, fs)

        #define MB_SC_MONO_BAND(id, label, alias, x, total, fe, fs) \
            COMBO("sce" id, "External sidechain source" label, "Ext SC src" alias, 0.0f, mb_exp_sc_type_sc), \
            MB_BAND_COMMON(id, label, alias, x, total, fe, fs)

        #define MB_SC_STEREO_BAND(id, label, alias, x, total, fe, fs) \
            COMBO("sce" id, "External sidechain source" label, "Ext SC src" alias, 0.0f, mb_exp_sc_type_sc), \
            COMBO("scs" id, "Sidechain source" label, "SC source" alias, 0, mb_exp_sc_source), \
            COMBO("sscs" id, "Split sidechain source" label, "SC split" alias, 0, mb_exp_sc_split_source), \
            MB_BAND_COMMON(id, label, alias, x, total, fe, fs)

        #define MB_SC_SPLIT_BAND(id, label, alias, x, total, fe, fs) \
            COMBO("sce" id, "External sidechain source" label, "Ext SC src" alias, 0.0f, mb_exp_sc_type_sc), \
            COMBO("scs" id, "Sidechain source" label, "SC source" alias, 0, mb_exp_sc_source), \
            MB_BAND_COMMON(id, label, alias, x, total, fe, fs)

        #define MB_STEREO_CHANNEL \
            SWITCH("flt", "Band filter curves", "Show filters", 1.0f), \
            MESH("ag_l", "Expander amplitude graph Left", 2, mb_expander_metadata::FFT_MESH_POINTS), \
            MESH("ag_r", "Expander amplitude graph Right", 2, mb_expander_metadata::FFT_MESH_POINTS), \
            SWITCH("ssplit", "Stereo split", "Stereo split", 0.0f)

        #define MB_FFT_METERS(id, label, alias) \
            SWITCH("ife" id, "Input FFT graph enable" label, "FFT In" alias, 1.0f), \
            SWITCH("ofe" id, "Output FFT graph enable" label, "FFT Out" alias, 1.0f), \
            MESH("ifg" id, "Input FFT graph" label, 2, mb_expander_metadata::FFT_MESH_POINTS + 2), \
            MESH("ofg" id, "Output FFT graph" label, 2, mb_expander_metadata::FFT_MESH_POINTS)

        #define MB_CHANNEL_METERS(id, label) \
            METER_GAIN("ilm" id, "Input level meter" label, GAIN_AMP_P_24_DB), \
            METER_GAIN("olm" id, "Output level meter" label, GAIN_AMP_P_24_DB)


        static const port_t mb_expander_mono_ports[] =
        {
            PORTS_MONO_PLUGIN,
            MB_EXP_SHM_LINK_MONO,
            MB_COMMON(exp_sc_bands),
            MB_CHANNEL("", "", ""),
            MB_FFT_METERS("", "", ""),
            MB_CHANNEL_METERS("", ""),

            MB_SPLIT("_1", " 1", " 1", 0.0f, 40.0f),
            MB_SPLIT("_2", " 2", " 2", 1.0f, 100.0f),
            MB_SPLIT("_3", " 3", " 3", 0.0f, 252.0f),
            MB_SPLIT("_4", " 4", " 4", 1.0f, 632.0f),
            MB_SPLIT("_5", " 5", " 5", 0.0f, 1587.0f),
            MB_SPLIT("_6", " 6", " 6", 1.0f, 3984.0f),
            MB_SPLIT("_7", " 7", " 7", 0.0f, 10000.0f),

            MB_MONO_BAND("_0", " 0", " 0", 0, 8, 10.0f, 40.0f),
            MB_MONO_BAND("_1", " 1", " 1", 1, 8, 40.0f, 100.0f),
            MB_MONO_BAND("_2", " 2", " 2", 2, 8, 100.0f, 252.0f),
            MB_MONO_BAND("_3", " 3", " 3", 3, 8, 252.0f, 632.0f),
            MB_MONO_BAND("_4", " 4", " 4", 4, 8, 632.0f, 1587.0f),
            MB_MONO_BAND("_5", " 5", " 5", 5, 8, 1587.0f, 3984.0f),
            MB_MONO_BAND("_6", " 6", " 6", 6, 8, 3984.0f, 10000.0f),
            MB_MONO_BAND("_7", " 7", " 7", 7, 8, 10000.0f, 20000.0f),

            MB_BAND_METERS("_0", " 0"),
            MB_BAND_METERS("_1", " 1"),
            MB_BAND_METERS("_2", " 2"),
            MB_BAND_METERS("_3", " 3"),
            MB_BAND_METERS("_4", " 4"),
            MB_BAND_METERS("_5", " 5"),
            MB_BAND_METERS("_6", " 6"),
            MB_BAND_METERS("_7", " 7"),

            PORTS_END
        };

        static const port_t mb_expander_stereo_ports[] =
        {
            PORTS_STEREO_PLUGIN,
            MB_EXP_SHM_LINK_STEREO,
            MB_COMMON(exp_sc_bands),
            MB_STEREO_CHANNEL,
            MB_FFT_METERS("_l", " Left", " L"),
            MB_CHANNEL_METERS("_l", " Left"),
            MB_FFT_METERS("_r", " Right", " R"),
            MB_CHANNEL_METERS("_r", " Right"),

            MB_SPLIT("_1", " 1", " 1", 0.0f, 40.0f),
            MB_SPLIT("_2", " 2", " 2", 1.0f, 100.0f),
            MB_SPLIT("_3", " 3", " 3", 0.0f, 252.0f),
            MB_SPLIT("_4", " 4", " 4", 1.0f, 632.0f),
            MB_SPLIT("_5", " 5", " 5", 0.0f, 1587.0f),
            MB_SPLIT("_6", " 6", " 6", 1.0f, 3984.0f),
            MB_SPLIT("_7", " 7", " 7", 0.0f, 10000.0f),

            MB_STEREO_BAND("_0", " 0", " 0", 0, 8, 10.0f, 40.0f),
            MB_STEREO_BAND("_1", " 1", " 1", 1, 8, 40.0f, 100.0f),
            MB_STEREO_BAND("_2", " 2", " 2", 2, 8, 100.0f, 252.0f),
            MB_STEREO_BAND("_3", " 3", " 3", 3, 8, 252.0f, 632.0f),
            MB_STEREO_BAND("_4", " 4", " 4", 4, 8, 632.0f, 1587.0f),
            MB_STEREO_BAND("_5", " 5", " 5", 5, 8, 1587.0f, 3984.0f),
            MB_STEREO_BAND("_6", " 6", " 6", 6, 8, 3984.0f, 10000.0f),
            MB_STEREO_BAND("_7", " 7", " 7", 7, 8, 10000.0f, 20000.0f),

            MB_BAND_METERS("_0l", " 0 Left"),
            MB_BAND_METERS("_1l", " 1 Left"),
            MB_BAND_METERS("_2l", " 2 Left"),
            MB_BAND_METERS("_3l", " 3 Left"),
            MB_BAND_METERS("_4l", " 4 Left"),
            MB_BAND_METERS("_5l", " 5 Left"),
            MB_BAND_METERS("_6l", " 6 Left"),
            MB_BAND_METERS("_7l", " 7 Left"),

            MB_BAND_METERS("_0r", " 0 Right"),
            MB_BAND_METERS("_1r", " 1 Right"),
            MB_BAND_METERS("_2r", " 2 Right"),
            MB_BAND_METERS("_3r", " 3 Right"),
            MB_BAND_METERS("_4r", " 4 Right"),
            MB_BAND_METERS("_5r", " 5 Right"),
            MB_BAND_METERS("_6r", " 6 Right"),
            MB_BAND_METERS("_7r", " 7 Right"),

            PORTS_END
        };

        static const port_t mb_expander_lr_ports[] =
        {
            PORTS_STEREO_PLUGIN,
            MB_EXP_SHM_LINK_STEREO,
            MB_COMMON(exp_sc_lr_bands),
            MB_CHANNEL("_l", " Left", " L"),
            MB_CHANNEL("_r", " Right", " R"),
            MB_FFT_METERS("_l", " Left", " L"),
            MB_CHANNEL_METERS("_l", " Left"),
            MB_FFT_METERS("_r", " Right", " R"),
            MB_CHANNEL_METERS("_r", " Right"),

            MB_SPLIT("_1l", " 1 Left", " 1 L", 0.0f, 40.0f),
            MB_SPLIT("_2l", " 2 Left", " 2 L", 1.0f, 100.0f),
            MB_SPLIT("_3l", " 3 Left", " 3 L", 0.0f, 252.0f),
            MB_SPLIT("_4l", " 4 Left", " 4 L", 1.0f, 632.0f),
            MB_SPLIT("_5l", " 5 Left", " 5 L", 0.0f, 1587.0f),
            MB_SPLIT("_6l", " 6 Left", " 6 L", 1.0f, 3984.0f),
            MB_SPLIT("_7l", " 7 Left", " 7 L", 0.0f, 10000.0f),

            MB_SPLIT("_1r", " 1 Right", " 1 R", 0.0f, 40.0f),
            MB_SPLIT("_2r", " 2 Right", " 2 R", 1.0f, 100.0f),
            MB_SPLIT("_3r", " 3 Right", " 3 R", 0.0f, 252.0f),
            MB_SPLIT("_4r", " 4 Right", " 4 R", 1.0f, 632.0f),
            MB_SPLIT("_5r", " 5 Right", " 5 R", 0.0f, 1587.0f),
            MB_SPLIT("_6r", " 6 Right", " 6 R", 1.0f, 3984.0f),
            MB_SPLIT("_7r", " 7 Right", " 7 R", 0.0f, 10000.0f),

            MB_SPLIT_BAND("_0l", " 0 Left", " 0 L", 0, 8, 10.0f, 40.0f),
            MB_SPLIT_BAND("_1l", " 1 Left", " 1 L", 1, 8, 40.0f, 100.0f),
            MB_SPLIT_BAND("_2l", " 2 Left", " 2 L", 2, 8, 100.0f, 252.0f),
            MB_SPLIT_BAND("_3l", " 3 Left", " 3 L", 3, 8, 252.0f, 632.0f),
            MB_SPLIT_BAND("_4l", " 4 Left", " 4 L", 4, 8, 632.0f, 1587.0f),
            MB_SPLIT_BAND("_5l", " 5 Left", " 5 L", 5, 8, 1587.0f, 3984.0f),
            MB_SPLIT_BAND("_6l", " 6 Left", " 6 L", 6, 8, 3984.0f, 10000.0f),
            MB_SPLIT_BAND("_7l", " 7 Left", " 7 L", 7, 8, 10000.0f, 20000.0f),

            MB_SPLIT_BAND("_0r", " 0 Right", " 0 R", 0, 8, 10.0f, 40.0f),
            MB_SPLIT_BAND("_1r", " 1 Right", " 1 R", 1, 8, 40.0f, 100.0f),
            MB_SPLIT_BAND("_2r", " 2 Right", " 2 R", 2, 8, 100.0f, 252.0f),
            MB_SPLIT_BAND("_3r", " 3 Right", " 3 R", 3, 8, 252.0f, 632.0f),
            MB_SPLIT_BAND("_4r", " 4 Right", " 4 R", 4, 8, 632.0f, 1587.0f),
            MB_SPLIT_BAND("_5r", " 5 Right", " 5 R", 5, 8, 1587.0f, 3984.0f),
            MB_SPLIT_BAND("_6r", " 6 Right", " 6 R", 6, 8, 3984.0f, 10000.0f),
            MB_SPLIT_BAND("_7r", " 7 Right", " 7 R", 7, 8, 10000.0f, 20000.0f),

            MB_BAND_METERS("_0l", " 0 Left"),
            MB_BAND_METERS("_1l", " 1 Left"),
            MB_BAND_METERS("_2l", " 2 Left"),
            MB_BAND_METERS("_3l", " 3 Left"),
            MB_BAND_METERS("_4l", " 4 Left"),
            MB_BAND_METERS("_5l", " 5 Left"),
            MB_BAND_METERS("_6l", " 6 Left"),
            MB_BAND_METERS("_7l", " 7 Left"),

            MB_BAND_METERS("_0r", " 0 Right"),
            MB_BAND_METERS("_1r", " 1 Right"),
            MB_BAND_METERS("_2r", " 2 Right"),
            MB_BAND_METERS("_3r", " 3 Right"),
            MB_BAND_METERS("_4r", " 4 Right"),
            MB_BAND_METERS("_5r", " 5 Right"),
            MB_BAND_METERS("_6r", " 6 Right"),
            MB_BAND_METERS("_7r", " 7 Right"),

            PORTS_END
        };

        static const port_t mb_expander_ms_ports[] =
        {
            PORTS_STEREO_PLUGIN,
            MB_EXP_SHM_LINK_STEREO,
            MB_COMMON(exp_sc_ms_bands),
            MB_CHANNEL("_m", " Mid", " M"),
            MB_CHANNEL("_s", " Side", " S"),
            MB_FFT_METERS("_m", " Mid", " M"),
            MB_CHANNEL_METERS("_l", " Left"),
            MB_FFT_METERS("_s", " Side", " S"),
            MB_CHANNEL_METERS("_r", " Right"),

            MB_SPLIT("_1m", " 1 Mid", " 1 M", 0.0f, 40.0f),
            MB_SPLIT("_2m", " 2 Mid", " 2 M", 1.0f, 100.0f),
            MB_SPLIT("_3m", " 3 Mid", " 3 M", 0.0f, 252.0f),
            MB_SPLIT("_4m", " 4 Mid", " 4 M", 1.0f, 632.0f),
            MB_SPLIT("_5m", " 5 Mid", " 5 M", 0.0f, 1587.0f),
            MB_SPLIT("_6m", " 6 Mid", " 6 M", 1.0f, 3984.0f),
            MB_SPLIT("_7m", " 7 Mid", " 7 M", 0.0f, 10000.0f),

            MB_SPLIT("_1s", " 1 Side", " 1 S", 0.0f, 40.0f),
            MB_SPLIT("_2s", " 2 Side", " 2 S", 1.0f, 100.0f),
            MB_SPLIT("_3s", " 3 Side", " 3 S", 0.0f, 252.0f),
            MB_SPLIT("_4s", " 4 Side", " 4 S", 1.0f, 632.0f),
            MB_SPLIT("_5s", " 5 Side", " 5 S", 0.0f, 1587.0f),
            MB_SPLIT("_6s", " 6 Side", " 6 S", 1.0f, 3984.0f),
            MB_SPLIT("_7s", " 7 Side", " 7 S", 0.0f, 10000.0f),

            MB_SPLIT_BAND("_0m", " 0 Mid", " 0 M", 0, 8, 10.0f, 40.0f),
            MB_SPLIT_BAND("_1m", " 1 Mid", " 1 M", 1, 8, 40.0f, 100.0f),
            MB_SPLIT_BAND("_2m", " 2 Mid", " 2 M", 2, 8, 100.0f, 252.0f),
            MB_SPLIT_BAND("_3m", " 3 Mid", " 3 M", 3, 8, 252.0f, 632.0f),
            MB_SPLIT_BAND("_4m", " 4 Mid", " 4 M", 4, 8, 632.0f, 1587.0f),
            MB_SPLIT_BAND("_5m", " 5 Mid", " 5 M", 5, 8, 1587.0f, 3984.0f),
            MB_SPLIT_BAND("_6m", " 6 Mid", " 6 M", 6, 8, 3984.0f, 10000.0f),
            MB_SPLIT_BAND("_7m", " 7 Mid", " 7 M", 7, 8, 10000.0f, 20000.0f),

            MB_SPLIT_BAND("_0s", " 0 Side", " 0 S", 0, 8, 10.0f, 40.0f),
            MB_SPLIT_BAND("_1s", " 1 Side", " 1 S", 1, 8, 40.0f, 100.0f),
            MB_SPLIT_BAND("_2s", " 2 Side", " 2 S", 2, 8, 100.0f, 252.0f),
            MB_SPLIT_BAND("_3s", " 3 Side", " 3 S", 3, 8, 252.0f, 632.0f),
            MB_SPLIT_BAND("_4s", " 4 Side", " 4 S", 4, 8, 632.0f, 1587.0f),
            MB_SPLIT_BAND("_5s", " 5 Side", " 5 S", 5, 8, 1587.0f, 3984.0f),
            MB_SPLIT_BAND("_6s", " 6 Side", " 6 S", 6, 8, 3984.0f, 10000.0f),
            MB_SPLIT_BAND("_7s", " 7 Side", " 7 S", 7, 8, 10000.0f, 20000.0f),

            MB_BAND_METERS("_0m", " 0 Mid"),
            MB_BAND_METERS("_1m", " 1 Mid"),
            MB_BAND_METERS("_2m", " 2 Mid"),
            MB_BAND_METERS("_3m", " 3 Mid"),
            MB_BAND_METERS("_4m", " 4 Mid"),
            MB_BAND_METERS("_5m", " 5 Mid"),
            MB_BAND_METERS("_6m", " 6 Mid"),
            MB_BAND_METERS("_7m", " 7 Mid"),

            MB_BAND_METERS("_0s", " 0 Side"),
            MB_BAND_METERS("_1s", " 1 Side"),
            MB_BAND_METERS("_2s", " 2 Side"),
            MB_BAND_METERS("_3s", " 3 Side"),
            MB_BAND_METERS("_4s", " 4 Side"),
            MB_BAND_METERS("_5s", " 5 Side"),
            MB_BAND_METERS("_6s", " 6 Side"),
            MB_BAND_METERS("_7s", " 7 Side"),

            PORTS_END
        };

        static const port_t sc_mb_expander_mono_ports[] =
        {
            PORTS_MONO_PLUGIN,
            PORTS_MONO_SIDECHAIN,
            MB_EXP_SHM_LINK_MONO,
            MB_COMMON(exp_sc_bands),
            MB_CHANNEL("", "", ""),
            MB_FFT_METERS("", "", ""),
            MB_CHANNEL_METERS("", ""),

            MB_SPLIT("_1", " 1", " 1", 0.0f, 40.0f),
            MB_SPLIT("_2", " 2", " 2", 1.0f, 100.0f),
            MB_SPLIT("_3", " 3", " 3", 0.0f, 252.0f),
            MB_SPLIT("_4", " 4", " 4", 1.0f, 632.0f),
            MB_SPLIT("_5", " 5", " 5", 0.0f, 1587.0f),
            MB_SPLIT("_6", " 6", " 6", 1.0f, 3984.0f),
            MB_SPLIT("_7", " 7", " 7", 0.0f, 10000.0f),

            MB_SC_MONO_BAND("_0", " 0", " 0", 0, 8, 10.0f, 40.0f),
            MB_SC_MONO_BAND("_1", " 1", " 1", 1, 8, 40.0f, 100.0f),
            MB_SC_MONO_BAND("_2", " 2", " 2", 2, 8, 100.0f, 252.0f),
            MB_SC_MONO_BAND("_3", " 3", " 3", 3, 8, 252.0f, 632.0f),
            MB_SC_MONO_BAND("_4", " 4", " 4", 4, 8, 632.0f, 1587.0f),
            MB_SC_MONO_BAND("_5", " 5", " 5", 5, 8, 1587.0f, 3984.0f),
            MB_SC_MONO_BAND("_6", " 6", " 6", 6, 8, 3984.0f, 10000.0f),
            MB_SC_MONO_BAND("_7", " 7", " 7", 7, 8, 10000.0f, 20000.0f),

            MB_BAND_METERS("_0", " 0"),
            MB_BAND_METERS("_1", " 1"),
            MB_BAND_METERS("_2", " 2"),
            MB_BAND_METERS("_3", " 3"),
            MB_BAND_METERS("_4", " 4"),
            MB_BAND_METERS("_5", " 5"),
            MB_BAND_METERS("_6", " 6"),
            MB_BAND_METERS("_7", " 7"),

            PORTS_END
        };

        static const port_t sc_mb_expander_stereo_ports[] =
        {
            PORTS_STEREO_PLUGIN,
            PORTS_STEREO_SIDECHAIN,
            MB_EXP_SHM_LINK_STEREO,
            MB_COMMON(exp_sc_bands),
            MB_STEREO_CHANNEL,
            MB_FFT_METERS("_l", " Left", " L"),
            MB_CHANNEL_METERS("_l", " Left"),
            MB_FFT_METERS("_r", " Right", " R"),
            MB_CHANNEL_METERS("_r", " Right"),

            MB_SPLIT("_1", " 1", " 1", 0.0f, 40.0f),
            MB_SPLIT("_2", " 2", " 2", 1.0f, 100.0f),
            MB_SPLIT("_3", " 3", " 3", 0.0f, 252.0f),
            MB_SPLIT("_4", " 4", " 4", 1.0f, 632.0f),
            MB_SPLIT("_5", " 5", " 5", 0.0f, 1587.0f),
            MB_SPLIT("_6", " 6", " 6", 1.0f, 3984.0f),
            MB_SPLIT("_7", " 7", " 7", 0.0f, 10000.0f),

            MB_SC_STEREO_BAND("_0", " 0", " 0", 0, 8, 10.0f, 40.0f),
            MB_SC_STEREO_BAND("_1", " 1", " 1", 1, 8, 40.0f, 100.0f),
            MB_SC_STEREO_BAND("_2", " 2", " 2", 2, 8, 100.0f, 252.0f),
            MB_SC_STEREO_BAND("_3", " 3", " 3", 3, 8, 252.0f, 632.0f),
            MB_SC_STEREO_BAND("_4", " 4", " 4", 4, 8, 632.0f, 1587.0f),
            MB_SC_STEREO_BAND("_5", " 5", " 5", 5, 8, 1587.0f, 3984.0f),
            MB_SC_STEREO_BAND("_6", " 6", " 6", 6, 8, 3984.0f, 10000.0f),
            MB_SC_STEREO_BAND("_7", " 7", " 7", 7, 8, 10000.0f, 20000.0f),

            MB_BAND_METERS("_0l", " 0 Left"),
            MB_BAND_METERS("_1l", " 1 Left"),
            MB_BAND_METERS("_2l", " 2 Left"),
            MB_BAND_METERS("_3l", " 3 Left"),
            MB_BAND_METERS("_4l", " 4 Left"),
            MB_BAND_METERS("_5l", " 5 Left"),
            MB_BAND_METERS("_6l", " 6 Left"),
            MB_BAND_METERS("_7l", " 7 Left"),

            MB_BAND_METERS("_0r", " 0 Right"),
            MB_BAND_METERS("_1r", " 1 Right"),
            MB_BAND_METERS("_2r", " 2 Right"),
            MB_BAND_METERS("_3r", " 3 Right"),
            MB_BAND_METERS("_4r", " 4 Right"),
            MB_BAND_METERS("_5r", " 5 Right"),
            MB_BAND_METERS("_6r", " 6 Right"),
            MB_BAND_METERS("_7r", " 7 Right"),

            PORTS_END
        };

        static const port_t sc_mb_expander_lr_ports[] =
        {
            PORTS_STEREO_PLUGIN,
            PORTS_STEREO_SIDECHAIN,
            MB_EXP_SHM_LINK_STEREO,
            MB_COMMON(exp_sc_lr_bands),
            MB_CHANNEL("_l", " Left", " L"),
            MB_CHANNEL("_r", " Right", " R"),
            MB_FFT_METERS("_l", " Left", " L"),
            MB_CHANNEL_METERS("_l", " Left"),
            MB_FFT_METERS("_r", " Right", " R"),
            MB_CHANNEL_METERS("_r", " Right"),

            MB_SPLIT("_1l", " 1 Left", " 1 L", 0.0f, 40.0f),
            MB_SPLIT("_2l", " 2 Left", " 2 L", 1.0f, 100.0f),
            MB_SPLIT("_3l", " 3 Left", " 3 L", 0.0f, 252.0f),
            MB_SPLIT("_4l", " 4 Left", " 4 L", 1.0f, 632.0f),
            MB_SPLIT("_5l", " 5 Left", " 5 L", 0.0f, 1587.0f),
            MB_SPLIT("_6l", " 6 Left", " 6 L", 1.0f, 3984.0f),
            MB_SPLIT("_7l", " 7 Left", " 7 L", 0.0f, 10000.0f),

            MB_SPLIT("_1r", " 1 Right", " 1 R", 0.0f, 40.0f),
            MB_SPLIT("_2r", " 2 Right", " 2 R", 1.0f, 100.0f),
            MB_SPLIT("_3r", " 3 Right", " 3 R", 0.0f, 252.0f),
            MB_SPLIT("_4r", " 4 Right", " 4 R", 1.0f, 632.0f),
            MB_SPLIT("_5r", " 5 Right", " 5 R", 0.0f, 1587.0f),
            MB_SPLIT("_6r", " 6 Right", " 6 R", 1.0f, 3984.0f),
            MB_SPLIT("_7r", " 7 Right", " 7 R", 0.0f, 10000.0f),

            MB_SC_SPLIT_BAND("_0l", " 0 Left", " 0 L", 0, 8, 10.0f, 40.0f),
            MB_SC_SPLIT_BAND("_1l", " 1 Left", " 1 L", 1, 8, 40.0f, 100.0f),
            MB_SC_SPLIT_BAND("_2l", " 2 Left", " 2 L", 2, 8, 100.0f, 252.0f),
            MB_SC_SPLIT_BAND("_3l", " 3 Left", " 3 L", 3, 8, 252.0f, 632.0f),
            MB_SC_SPLIT_BAND("_4l", " 4 Left", " 4 L", 4, 8, 632.0f, 1587.0f),
            MB_SC_SPLIT_BAND("_5l", " 5 Left", " 5 L", 5, 8, 1587.0f, 3984.0f),
            MB_SC_SPLIT_BAND("_6l", " 6 Left", " 6 L", 6, 8, 3984.0f, 10000.0f),
            MB_SC_SPLIT_BAND("_7l", " 7 Left", " 7 L", 7, 8, 10000.0f, 20000.0f),

            MB_SC_SPLIT_BAND("_0r", " 0 Right", " 0 R", 0, 8, 10.0f, 40.0f),
            MB_SC_SPLIT_BAND("_1r", " 1 Right", " 1 R", 1, 8, 40.0f, 100.0f),
            MB_SC_SPLIT_BAND("_2r", " 2 Right", " 2 R", 2, 8, 100.0f, 252.0f),
            MB_SC_SPLIT_BAND("_3r", " 3 Right", " 3 R", 3, 8, 252.0f, 632.0f),
            MB_SC_SPLIT_BAND("_4r", " 4 Right", " 4 R", 4, 8, 632.0f, 1587.0f),
            MB_SC_SPLIT_BAND("_5r", " 5 Right", " 5 R", 5, 8, 1587.0f, 3984.0f),
            MB_SC_SPLIT_BAND("_6r", " 6 Right", " 6 R", 6, 8, 3984.0f, 10000.0f),
            MB_SC_SPLIT_BAND("_7r", " 7 Right", " 7 R", 7, 8, 10000.0f, 20000.0f),

            MB_BAND_METERS("_0l", " 0 Left"),
            MB_BAND_METERS("_1l", " 1 Left"),
            MB_BAND_METERS("_2l", " 2 Left"),
            MB_BAND_METERS("_3l", " 3 Left"),
            MB_BAND_METERS("_4l", " 4 Left"),
            MB_BAND_METERS("_5l", " 5 Left"),
            MB_BAND_METERS("_6l", " 6 Left"),
            MB_BAND_METERS("_7l", " 7 Left"),

            MB_BAND_METERS("_0r", " 0 Right"),
            MB_BAND_METERS("_1r", " 1 Right"),
            MB_BAND_METERS("_2r", " 2 Right"),
            MB_BAND_METERS("_3r", " 3 Right"),
            MB_BAND_METERS("_4r", " 4 Right"),
            MB_BAND_METERS("_5r", " 5 Right"),
            MB_BAND_METERS("_6r", " 6 Right"),
            MB_BAND_METERS("_7r", " 7 Right"),

            PORTS_END
        };

        static const port_t sc_mb_expander_ms_ports[] =
        {
            PORTS_STEREO_PLUGIN,
            PORTS_STEREO_SIDECHAIN,
            MB_EXP_SHM_LINK_STEREO,
            MB_COMMON(exp_sc_ms_bands),
            MB_CHANNEL("_m", " Mid", " M"),
            MB_CHANNEL("_s", " Side", " S"),
            MB_FFT_METERS("_m", " Mid", " M"),
            MB_CHANNEL_METERS("_l", " Left"),
            MB_FFT_METERS("_s", " Side", " S"),
            MB_CHANNEL_METERS("_r", " Right"),

            MB_SPLIT("_1m", " 1 Mid", " 1 M", 0.0f, 40.0f),
            MB_SPLIT("_2m", " 2 Mid", " 2 M", 1.0f, 100.0f),
            MB_SPLIT("_3m", " 3 Mid", " 3 M", 0.0f, 252.0f),
            MB_SPLIT("_4m", " 4 Mid", " 4 M", 1.0f, 632.0f),
            MB_SPLIT("_5m", " 5 Mid", " 5 M", 0.0f, 1587.0f),
            MB_SPLIT("_6m", " 6 Mid", " 6 M", 1.0f, 3984.0f),
            MB_SPLIT("_7m", " 7 Mid", " 7 M", 0.0f, 10000.0f),

            MB_SPLIT("_1s", " 1 Side", " 1 S", 0.0f, 40.0f),
            MB_SPLIT("_2s", " 2 Side", " 2 S", 1.0f, 100.0f),
            MB_SPLIT("_3s", " 3 Side", " 3 S", 0.0f, 252.0f),
            MB_SPLIT("_4s", " 4 Side", " 4 S", 1.0f, 632.0f),
            MB_SPLIT("_5s", " 5 Side", " 5 S", 0.0f, 1587.0f),
            MB_SPLIT("_6s", " 6 Side", " 6 S", 1.0f, 3984.0f),
            MB_SPLIT("_7s", " 7 Side", " 7 S", 0.0f, 10000.0f),

            MB_SC_SPLIT_BAND("_0m", " 0 Mid", " 0 M", 0, 8, 10.0f, 40.0f),
            MB_SC_SPLIT_BAND("_1m", " 1 Mid", " 1 M", 1, 8, 40.0f, 100.0f),
            MB_SC_SPLIT_BAND("_2m", " 2 Mid", " 2 M", 2, 8, 100.0f, 252.0f),
            MB_SC_SPLIT_BAND("_3m", " 3 Mid", " 3 M", 3, 8, 252.0f, 632.0f),
            MB_SC_SPLIT_BAND("_4m", " 4 Mid", " 4 M", 4, 8, 632.0f, 1587.0f),
            MB_SC_SPLIT_BAND("_5m", " 5 Mid", " 5 M", 5, 8, 1587.0f, 3984.0f),
            MB_SC_SPLIT_BAND("_6m", " 6 Mid", " 6 M", 6, 8, 3984.0f, 10000.0f),
            MB_SC_SPLIT_BAND("_7m", " 7 Mid", " 7 M", 7, 8, 10000.0f, 20000.0f),

            MB_SC_SPLIT_BAND("_0s", " 0 Side", " 0 S", 0, 8, 10.0f, 40.0f),
            MB_SC_SPLIT_BAND("_1s", " 1 Side", " 1 S", 1, 8, 40.0f, 100.0f),
            MB_SC_SPLIT_BAND("_2s", " 2 Side", " 2 S", 2, 8, 100.0f, 252.0f),
            MB_SC_SPLIT_BAND("_3s", " 3 Side", " 3 S", 3, 8, 252.0f, 632.0f),
            MB_SC_SPLIT_BAND("_4s", " 4 Side", " 4 S", 4, 8, 632.0f, 1587.0f),
            MB_SC_SPLIT_BAND("_5s", " 5 Side", " 5 S", 5, 8, 1587.0f, 3984.0f),
            MB_SC_SPLIT_BAND("_6s", " 6 Side", " 6 S", 6, 8, 3984.0f, 10000.0f),
            MB_SC_SPLIT_BAND("_7s", " 7 Side", " 7 S", 7, 8, 10000.0f, 20000.0f),

            MB_BAND_METERS("_0m", " 0 Mid"),
            MB_BAND_METERS("_1m", " 1 Mid"),
            MB_BAND_METERS("_2m", " 2 Mid"),
            MB_BAND_METERS("_3m", " 3 Mid"),
            MB_BAND_METERS("_4m", " 4 Mid"),
            MB_BAND_METERS("_5m", " 5 Mid"),
            MB_BAND_METERS("_6m", " 6 Mid"),
            MB_BAND_METERS("_7m", " 7 Mid"),

            MB_BAND_METERS("_0s", " 0 Side"),
            MB_BAND_METERS("_1s", " 1 Side"),
            MB_BAND_METERS("_2s", " 2 Side"),
            MB_BAND_METERS("_3s", " 3 Side"),
            MB_BAND_METERS("_4s", " 4 Side"),
            MB_BAND_METERS("_5s", " 5 Side"),
            MB_BAND_METERS("_6s", " 6 Side"),
            MB_BAND_METERS("_7s", " 7 Side"),

            PORTS_END
        };

        const meta::bundle_t mb_expander_bundle =
        {
            "mb_expander",
            "Multiband Expander",
            B_MB_DYNAMICS,
            "TR_Ox7U_a84",
            "This plugin performs multiband expansion of input signsl. Flexible sidechain\ncontrol configuration provided. As opposite to most available multiband\nexpanders, this expander provides numerous special functions: 'modern'\noperating mode, 'Sidechain boost', 'Lookahead' option and up to 8 frequency\nbands for processing."
        };

        // Multiband Expander
        const meta::plugin_t  mb_expander_mono =
        {
            "Multi-band Expander Mono x8",
            "Multiband Expander Mono x8",
            "MB Expander Mono",
            "MBE8M",
            &developers::v_sadovnikov,
            "mb_expander_mono",
            {
                LSP_LV2_URI("mb_expander_mono"),
                LSP_LV2UI_URI("mb_expander_mono"),
                "mygo",
                LSP_VST3_UID("mbe8m   mygo"),
                LSP_VST3UI_UID("mbe8m   mygo"),
                LSP_LADSPA_MB_EXPANDER_BASE + 0,
                LSP_LADSPA_URI("mb_expander_mono"),
                LSP_CLAP_URI("mb_expander_mono"),
                LSP_GST_UID("mb_expander_mono"),
            },
            LSP_PLUGINS_MB_EXPANDER_VERSION,
            plugin_classes,
            clap_features_mono,
            E_INLINE_DISPLAY | E_DUMP_STATE,
            mb_expander_mono_ports,
            "dynamics/expander/multiband/mono.xml",
            NULL,
            mono_plugin_port_groups,
            &mb_expander_bundle
        };

        const meta::plugin_t  mb_expander_stereo =
        {
            "Multi-band Expander Stereo x8",
            "Multiband Expander Stereo x8",
            "MB Expander Stereo",
            "MBE8S",
            &developers::v_sadovnikov,
            "mb_expander_stereo",
            {
                LSP_LV2_URI("mb_expander_stereo"),
                LSP_LV2UI_URI("mb_expander_stereo"),
                "hobt",
                LSP_VST3_UID("mbe8s   hobt"),
                LSP_VST3UI_UID("mbe8s   hobt"),
                LSP_LADSPA_MB_EXPANDER_BASE + 1,
                LSP_LADSPA_URI("mb_expander_stereo"),
                LSP_CLAP_URI("mb_expander_stereo"),
                LSP_GST_UID("mb_expander_stereo"),
            },
            LSP_PLUGINS_MB_EXPANDER_VERSION,
            plugin_classes,
            clap_features_stereo,
            E_INLINE_DISPLAY | E_DUMP_STATE,
            mb_expander_stereo_ports,
            "dynamics/expander/multiband/stereo.xml",
            NULL,
            stereo_plugin_port_groups,
            &mb_expander_bundle
        };

        const meta::plugin_t  mb_expander_lr =
        {
            "Multi-band Expander LeftRight x8",
            "Multiband Expander LeftRight x8",
            "MB Expander L/R",
            "MBE8LR",
            &developers::v_sadovnikov,
            "mb_expander_lr",
            {
                LSP_LV2_URI("mb_expander_lr"),
                LSP_LV2UI_URI("mb_expander_lr"),
                "bfmk",
                LSP_VST3_UID("mbe8lr  bfmk"),
                LSP_VST3UI_UID("mbe8lr  bfmk"),
                LSP_LADSPA_MB_EXPANDER_BASE + 2,
                LSP_LADSPA_URI("mb_expander_lr"),
                LSP_CLAP_URI("mb_expander_lr"),
                LSP_GST_UID("mb_expander_lr"),
            },
            LSP_PLUGINS_MB_EXPANDER_VERSION,
            plugin_classes,
            clap_features_stereo,
            E_INLINE_DISPLAY | E_DUMP_STATE,
            mb_expander_lr_ports,
            "dynamics/expander/multiband/lr.xml",
            NULL,
            stereo_plugin_port_groups,
            &mb_expander_bundle
        };

        const meta::plugin_t  mb_expander_ms =
        {
            "Multi-band Expander MidSide x8",
            "Multiband Expander MidSide x8",
            "MB Expander M/S",
            "MBE8MS",
            &developers::v_sadovnikov,
            "mb_expander_ms",
            {
                LSP_LV2_URI("mb_expander_ms"),
                LSP_LV2UI_URI("mb_expander_ms"),
                "ulte",
                LSP_VST3_UID("mbe8ms  ulte"),
                LSP_VST3UI_UID("mbe8ms  ulte"),
                LSP_LADSPA_MB_EXPANDER_BASE + 3,
                LSP_LADSPA_URI("mb_expander_ms"),
                LSP_CLAP_URI("mb_expander_ms"),
                LSP_GST_UID("mb_expander_ms"),
            },
            LSP_PLUGINS_MB_EXPANDER_VERSION,
            plugin_classes,
            clap_features_stereo,
            E_INLINE_DISPLAY | E_DUMP_STATE,
            mb_expander_ms_ports,
            "dynamics/expander/multiband/ms.xml",
            NULL,
            stereo_plugin_port_groups,
            &mb_expander_bundle
        };


        const meta::plugin_t  sc_mb_expander_mono =
        {
            "Sidechain Multi-band Expander Mono x8",
            "Sidechain Multiband Expander Mono x8",
            "SC MB Expander Mono",
            "SCMBE8M",
            &developers::v_sadovnikov,
            "sc_mb_expander_mono",
            {
                LSP_LV2_URI("sc_mb_expander_mono"),
                LSP_LV2UI_URI("sc_mb_expander_mono"),
                "szkf",
                LSP_VST3_UID("scmbe8m szkf"),
                LSP_VST3UI_UID("scmbe8m szkf"),
                LSP_LADSPA_MB_EXPANDER_BASE + 4,
                LSP_LADSPA_URI("sc_mb_expander_mono"),
                LSP_CLAP_URI("sc_mb_expander_mono"),
                LSP_GST_UID("sc_mb_expander_mono"),
            },
            LSP_PLUGINS_MB_EXPANDER_VERSION,
            plugin_classes,
            clap_features_mono,
            E_INLINE_DISPLAY | E_DUMP_STATE,
            sc_mb_expander_mono_ports,
            "dynamics/expander/multiband/mono.xml",
            NULL,
            mono_plugin_sidechain_port_groups,
            &mb_expander_bundle
        };

        const meta::plugin_t  sc_mb_expander_stereo =
        {
            "Sidechain Multi-band Expander Stereo x8",
            "Sidechain Multiband Expander Stereo x8",
            "SC MB Expander Stereo",
            "SCMBE8S",
            &developers::v_sadovnikov,
            "sc_mb_expander_stereo",
            {
                LSP_LV2_URI("sc_mb_expander_stereo"),
                LSP_LV2UI_URI("sc_mb_expander_stereo"),
                "f0qr",
                LSP_VST3_UID("scmbe8s f0qr"),
                LSP_VST3UI_UID("scmbe8s f0qr"),
                LSP_LADSPA_MB_EXPANDER_BASE + 5,
                LSP_LADSPA_URI("sc_mb_expander_stereo"),
                LSP_CLAP_URI("sc_mb_expander_stereo"),
                LSP_GST_UID("sc_mb_expander_stereo"),
            },
            LSP_PLUGINS_MB_EXPANDER_VERSION,
            plugin_classes,
            clap_features_stereo,
            E_INLINE_DISPLAY | E_DUMP_STATE,
            sc_mb_expander_stereo_ports,
            "dynamics/expander/multiband/stereo.xml",
            NULL,
            stereo_plugin_sidechain_port_groups,
            &mb_expander_bundle
        };

        const meta::plugin_t  sc_mb_expander_lr =
        {
            "Sidechain Multi-band Expander LeftRight x8",
            "Sidechain Multiband Expander LeftRight x8",
            "SC MB Expander L/R",
            "SCMBE8LR",
            &developers::v_sadovnikov,
            "sc_mb_expander_lr",
            {
                LSP_LV2_URI("sc_mb_expander_lr"),
                LSP_LV2UI_URI("sc_mb_expander_lr"),
                "kxdv",
                LSP_VST3_UID("scmbe8lrkxdv"),
                LSP_VST3UI_UID("scmbe8lrkxdv"),
                LSP_LADSPA_MB_EXPANDER_BASE + 6,
                LSP_LADSPA_URI("sc_mb_expander_lr"),
                LSP_CLAP_URI("sc_mb_expander_lr"),
                LSP_GST_UID("sc_mb_expander_lr"),
            },
            LSP_PLUGINS_MB_EXPANDER_VERSION,
            plugin_classes,
            clap_features_stereo,
            E_INLINE_DISPLAY | E_DUMP_STATE,
            sc_mb_expander_lr_ports,
            "dynamics/expander/multiband/lr.xml",
            NULL,
            stereo_plugin_sidechain_port_groups,
            &mb_expander_bundle
        };

        const meta::plugin_t  sc_mb_expander_ms =
        {
            "Sidechain Multi-band Expander MidSide x8",
            "Sidechain Multiband Expander MidSide x8",
            "SC MB Expander M/S",
            "SCMBE8MS",
            &developers::v_sadovnikov,
            "sc_mb_expander_ms",
            {
                LSP_LV2_URI("sc_mb_expander_ms"),
                LSP_LV2UI_URI("sc_mb_expander_ms"),
                "wkge",
                LSP_VST3_UID("scmbe8mswkge"),
                LSP_VST3UI_UID("scmbe8mswkge"),
                LSP_LADSPA_MB_EXPANDER_BASE + 7,
                LSP_LADSPA_URI("sc_mb_expander_ms"),
                LSP_CLAP_URI("sc_mb_expander_ms"),
                LSP_GST_UID("sc_mb_expander_ms"),
            },
            LSP_PLUGINS_MB_EXPANDER_VERSION,
            plugin_classes,
            clap_features_stereo,
            E_INLINE_DISPLAY | E_DUMP_STATE,
            sc_mb_expander_ms_ports,
            "dynamics/expander/multiband/ms.xml",
            NULL,
            stereo_plugin_sidechain_port_groups,
            &mb_expander_bundle
        };
    } /* namespace meta */
} /* namespace lsp */
