/*
 * BlueALSA - utils.c
 * Copyright (c) 2016-2022 Arkadiusz Bokowy
 *
 * This file is a part of bluez-alsa.
 *
 * This project is licensed under the terms of the MIT license.
 *
 */

#include "utils.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <bluetooth/bluetooth.h>

#if ENABLE_LDAC
# include "ldacBT.h"
#endif

#include "hfp.h"
#include "shared/a2dp-codecs.h"
#include "shared/defs.h"
#include "shared/log.h"

/**
 * Extract HCI device ID from the BlueZ D-Bus object path.
 *
 * @param path BlueZ D-Bus object path.
 * @return On success this function returns ID of the HCI device.
 *   Otherwise, -1 is returned. */
int g_dbus_bluez_object_path_to_hci_dev_id(const char *path) {
	if ((path = strstr(path, "/hci")) == NULL || path[4] == '\0')
		return -1;
	return atoi(&path[4]);
}

/**
 * Extract BT address from the BlueZ D-Bus object path.
 *
 * @param path BlueZ D-Bus object path.
 * @param addr Address where the parsed BT address will be stored.
 * @return On success this function returns pointer to the BT address. On
 *   error, NULL is returned. */
bdaddr_t *g_dbus_bluez_object_path_to_bdaddr(const char *path, bdaddr_t *addr) {

	char tmp[sizeof("00:00:00:00:00:00")] = { 0 };
	size_t i;

	if ((path = strstr(path, "/dev_")) != NULL)
		strncpy(tmp, path + 5, sizeof(tmp) - 1);

	for (i = 0; i < sizeof(tmp); i++)
		if (tmp[i] == '_')
			tmp[i] = ':';

	if (str2ba(tmp, addr) == -1)
		return NULL;

	return addr;
}

/**
 * Sanitize D-Bus object path.
 *
 * @param path D-Bus object path.
 * @return Pointer to the object path string. */
char *g_variant_sanitize_object_path(char *path) {

	char *tmp = path - 1;

	while (*(++tmp) != '\0')
		if (!(*tmp == '/' || isalnum(*tmp)))
			*tmp = '_';

	return path;
}

/**
 * Convenience wrapper around g_variant_is_of_type().
 *
 * @param value Variant for validation.
 * @param type Expected variant type.
 * @param name Variant name for logging.
 * @return If variant matches type, this function returns true. */
bool g_variant_validate_value(GVariant *value, const GVariantType *type,
		const char *name) {
	if (g_variant_is_of_type(value, type))
		return true;
	warn("Invalid variant type: %s: %s != %s", name,
			g_variant_get_type_string(value), (const char *)type);
	return false;
}

/**
 * Convert a pointer to BT address to a hash value.
 *
 * @param v A pointer to bdaddr_t structure.
 * @return Hash value compatible with GHashTable. */
unsigned int g_bdaddr_hash(const void *v) {
	const uint16_t *b = (uint16_t *)((const bdaddr_t *)v)->b;
	return (b[0] | ((uint32_t)b[1]) << 16) * b[2];
}

/**
 * Compare two BT addresses.
 *
 * @param v1 A pointer to first bdaddr_t structure.
 * @param v2 A pointer to second bdaddr_t structure.
 * @return Comparision value compatible with GHashTable. */
gboolean g_bdaddr_equal(const void *v1, const void *v2) {
	return bacmp(v1, v2) == 0;
}

#if DEBUG
/**
 * Get BlueALSA transport type debug name.
 *
 * @param t Transport structure.
 * @return Human-readable string. */
const char *ba_transport_debug_name(const struct ba_transport *t) {
	const enum ba_transport_profile profile = t->profile;
	const uint16_t codec_id = t->codec_id;
	switch (profile) {
	case BA_TRANSPORT_PROFILE_NONE:
		return "NONE";
	case BA_TRANSPORT_PROFILE_A2DP_SOURCE:
		switch (codec_id) {
		case A2DP_CODEC_SBC:
			return "A2DP Source (SBC)";
#if ENABLE_MPEG
		case A2DP_CODEC_MPEG12:
			return "A2DP Source (MP3)";
#endif
#if ENABLE_AAC
		case A2DP_CODEC_MPEG24:
			return "A2DP Source (AAC)";
#endif
#if ENABLE_APTX
		case A2DP_CODEC_VENDOR_APTX:
			return "A2DP Source (aptX)";
#endif
#if ENABLE_APTX_HD
		case A2DP_CODEC_VENDOR_APTX_HD:
			return "A2DP Source (aptX HD)";
#endif
#if ENABLE_FASTSTREAM
		case A2DP_CODEC_VENDOR_FASTSTREAM:
			return "A2DP Source (FastStream)";
#endif
#if ENABLE_LC3PLUS
		case A2DP_CODEC_VENDOR_LC3PLUS:
			return "A2DP Source (LC3plus)";
#endif
#if ENABLE_LDAC
		case A2DP_CODEC_VENDOR_LDAC:
			return "A2DP Source (LDAC)";
#endif
		default:
			return "A2DP Source";
		}
	case BA_TRANSPORT_PROFILE_A2DP_SINK:
		switch (codec_id) {
		case A2DP_CODEC_SBC:
			return "A2DP Sink (SBC)";
#if ENABLE_MPEG
		case A2DP_CODEC_MPEG12:
			return "A2DP Sink (MP3)";
#endif
#if ENABLE_AAC
		case A2DP_CODEC_MPEG24:
			return "A2DP Sink (AAC)";
#endif
#if ENABLE_APTX
		case A2DP_CODEC_VENDOR_APTX:
			return "A2DP Sink (aptX)";
#endif
#if ENABLE_APTX_HD
		case A2DP_CODEC_VENDOR_APTX_HD:
			return "A2DP Sink (aptX HD)";
#endif
#if ENABLE_FASTSTREAM
		case A2DP_CODEC_VENDOR_FASTSTREAM:
			return "A2DP Sink (FastStream)";
#endif
#if ENABLE_LC3PLUS
		case A2DP_CODEC_VENDOR_LC3PLUS:
			return "A2DP Sink (LC3plus)";
#endif
#if ENABLE_LDAC
		case A2DP_CODEC_VENDOR_LDAC:
			return "A2DP Sink (LDAC)";
#endif
		default:
			return "A2DP Sink";
		}
	case BA_TRANSPORT_PROFILE_HFP_HF:
		switch (codec_id) {
		case HFP_CODEC_CVSD:
			return "HFP Hands-Free (CVSD)";
		case HFP_CODEC_MSBC:
			return "HFP Hands-Free (mSBC)";
		default:
			return "HFP Hands-Free";
		}
	case BA_TRANSPORT_PROFILE_HFP_AG:
		switch (codec_id) {
		case HFP_CODEC_CVSD:
			return "HFP Audio Gateway (CVSD)";
		case HFP_CODEC_MSBC:
			return "HFP Audio Gateway (mSBC)";
		default:
			return "HFP Audio Gateway";
		}
	case BA_TRANSPORT_PROFILE_HSP_HS:
		return "HSP Headset";
	case BA_TRANSPORT_PROFILE_HSP_AG:
		return "HSP Audio Gateway";
	}
	debug("Unknown transport type: %#x %#x", profile, codec_id);
	return "N/A";
}
#endif

#if ENABLE_MP3LAME
/**
 * Get maximum possible bit-rate for the given bit-rate mask.
 *
 * @param mask MPEG-1 layer III bit-rate mask.
 * @return Bit-rate in kilobits per second. */
int a2dp_mpeg1_mp3_get_max_bitrate(uint16_t mask) {

	static int bitrates[] = { 320, 256, 224, 192, 160, 128, 112, 96, 80, 64, 56, 48, 40, 32 };
	size_t i = 0;

	while (i < ARRAYSIZE(bitrates)) {
		if (mask & (1 << (14 - i)))
			return bitrates[i];
		i++;
	}

	return -1;
}
#endif

#if ENABLE_MP3LAME
/**
 * Get string representation of LAME error code.
 *
 * @param error LAME encoder error code.
 * @return Human-readable string. */
const char *lame_encode_strerror(int err) {
	switch (err) {
	case -1:
		return "Too small output buffer";
	case -2:
		return "Out of memory";
	case -3:
		return "Params not initialized";
	case -4:
		return "Psycho acoustic error";
	default:
		debug("Unknown error code: %#x", err);
		return "Unknown error";
	}
}
#endif

#if ENABLE_AAC
/**
 * Get string representation of the FDK-AAC decoder error code.
 *
 * @param err FDK-AAC decoder error code.
 * @return Human-readable string. */
const char *aacdec_strerror(AAC_DECODER_ERROR err) {
	switch (err) {
	case AAC_DEC_OK:
		return "Success";
	case AAC_DEC_OUT_OF_MEMORY:
		return "Out of memory";
	case AAC_DEC_TRANSPORT_SYNC_ERROR:
		return "Transport sync error";
	case AAC_DEC_NOT_ENOUGH_BITS:
		return "Not enough bits";
	case AAC_DEC_INVALID_HANDLE:
		return "Invalid handle";
	case AAC_DEC_UNSUPPORTED_AOT:
		return "Unsupported AOT";
	case AAC_DEC_UNSUPPORTED_FORMAT:
		return "Unsupported format";
	case AAC_DEC_UNSUPPORTED_ER_FORMAT:
		return "Unsupported ER format";
	case AAC_DEC_UNSUPPORTED_EPCONFIG:
		return "Unsupported EP format";
	case AAC_DEC_UNSUPPORTED_MULTILAYER:
		return "Unsupported multilayer";
	case AAC_DEC_UNSUPPORTED_CHANNELCONFIG:
		return "Unsupported channels";
	case AAC_DEC_UNSUPPORTED_SAMPLINGRATE:
		return "Unsupported sample rate";
	case AAC_DEC_INVALID_SBR_CONFIG:
		return "Unsupported SBR";
	case AAC_DEC_SET_PARAM_FAIL:
		return "Unsupported parameter";
	case AAC_DEC_NEED_TO_RESTART:
		return "Restart required";
	case AAC_DEC_TRANSPORT_ERROR:
		return "Transport error";
	case AAC_DEC_PARSE_ERROR:
		return "Parse error";
	case AAC_DEC_UNSUPPORTED_EXTENSION_PAYLOAD:
		return "Unsupported extension payload";
	case AAC_DEC_DECODE_FRAME_ERROR:
		return "Bitstream corrupted";
	case AAC_DEC_CRC_ERROR:
		return "CRC mismatch";
	case AAC_DEC_INVALID_CODE_BOOK:
		return "Invalid codebook";
	case AAC_DEC_UNSUPPORTED_PREDICTION:
		return "Unsupported prediction";
	case AAC_DEC_UNSUPPORTED_CCE:
		return "Unsupported CCE";
	case AAC_DEC_UNSUPPORTED_LFE:
		return "Unsupported LFE";
	case AAC_DEC_UNSUPPORTED_GAIN_CONTROL_DATA:
		return "Unsupported gain control data";
	case AAC_DEC_UNSUPPORTED_SBA:
		return "Unsupported SBA";
	case AAC_DEC_TNS_READ_ERROR:
		return "TNS read error";
	case AAC_DEC_RVLC_ERROR:
		return "RVLC decode error";
	case AAC_DEC_ANC_DATA_ERROR:
		return "Ancillary data error";
	case AAC_DEC_TOO_SMALL_ANC_BUFFER:
		return "Too small ancillary buffer";
	case AAC_DEC_TOO_MANY_ANC_ELEMENTS:
		return "Too many ancillary elements";
	default:
		debug("Unknown error code: %#x", err);
		return "Unknown error";
	}
}
#endif

#if ENABLE_AAC
/**
 * Get string representation of the FDK-AAC encoder error code.
 *
 * @param err FDK-AAC encoder error code.
 * @return Human-readable string. */
const char *aacenc_strerror(AACENC_ERROR err) {
	switch (err) {
	case AACENC_OK:
		return "Success";
	case AACENC_INVALID_HANDLE:
		return "Invalid handle";
	case AACENC_MEMORY_ERROR:
		return "Out of memory";
	case AACENC_UNSUPPORTED_PARAMETER:
		return "Unsupported parameter";
	case AACENC_INVALID_CONFIG:
		return "Invalid config";
	case AACENC_INIT_ERROR:
		return "Initialization error";
	case AACENC_INIT_AAC_ERROR:
		return "AAC library initialization error";
	case AACENC_INIT_SBR_ERROR:
		return "SBR library initialization error";
	case AACENC_INIT_TP_ERROR:
		return "Transport library initialization error";
	case AACENC_INIT_META_ERROR:
		return "Metadata library initialization error";
	case AACENC_ENCODE_ERROR:
		return "Encoding error";
	case AACENC_ENCODE_EOF:
		return "End of file";
	default:
		debug("Unknown error code: %#x", err);
		return "Unknown error";
	}
}
#endif

#if ENABLE_LC3PLUS
/**
 * Get string representation of the LC3plus error code.
 *
 * @param err LC3plus error code.
 * @return Human-readable string. */
const char *lc3plus_strerror(LC3PLUS_Error err) {
	switch (err) {
	case LC3PLUS_OK:
		return "Success";
	case LC3PLUS_ERROR:
		return "Generic error";
	case LC3PLUS_DECODE_ERROR:
		return "Decode error";
	case LC3PLUS_NULL_ERROR:
		return "Invalid argument";
	case LC3PLUS_SAMPLERATE_ERROR:
		return "Invalid sample rate";
	case LC3PLUS_CHANNELS_ERROR:
		return "Invalid channel config";
	case LC3PLUS_BITRATE_ERROR:
		return "Invalid bitrate";
	case LC3PLUS_NUMBYTES_ERROR:
		return "Invalid number of bytes";
	case LC3PLUS_EPMODE_ERROR:
		return "Invalid EP mode";
	case LC3PLUS_FRAMEMS_ERROR:
		return "Invalid frame length";
	case LC3PLUS_ALIGN_ERROR:
		return "Unaligned pointer";
	case LC3PLUS_HRMODE_ERROR:
		return "Invalid hi-resolution mode";
	case LC3PLUS_BITRATE_UNSET_ERROR:
	case LC3PLUS_BITRATE_SET_ERROR:
		return "Bitrate set error";
	case LC3PLUS_HRMODE_BW_ERROR:
		return "Conflict hi-resolution mode and bandwidth switching";
	case LC3PLUS_PLCMODE_ERROR:
		return "Invalid PLC method";
	case LC3PLUS_EPMR_ERROR:
		return "Invalid EPMR value";
	case LC3PLUS_WARNING:
		return "Generic warning";
	case LC3PLUS_BW_WARNING:
		return "Invalid cutoff frequency";
	case LC3PLUS_PADDING_ERROR:
		return "Padding error";
	case FRAMESIZE_ERROR:
		return "Framesize error";
	default:
		debug("Unknown error code: %#x", err);
		return "Unknown error";
	}
}
#endif

#if ENABLE_LDAC
/**
 * Get string representation of the LDAC error code.
 *
 * @param err LDAC error code.
 * @return Human-readable string. */
const char *ldacBT_strerror(int err) {
	int code = LDACBT_HANDLE_ERR(err);
	switch (code != 0 ? code : LDACBT_API_ERR(err)) {
	case LDACBT_ERR_NONE:
		return "Success";
	case LDACBT_ERR_FATAL_HANDLE:
		return "Invalid handle";
	case LDACBT_ERR_HANDLE_NOT_INIT:
		return "Handle not initialized";
	case LDACBT_ERR_ENC_INIT_ALLOC:
	case LDACBT_ERR_DEC_INIT_ALLOC:
		return "Out of memory";
	case LDACBT_ERR_ASSERT_SAMPLING_FREQ:
	case LDACBT_ERR_ASSERT_SUP_SAMPLING_FREQ:
	case LDACBT_ERR_CHECK_SAMPLING_FREQ:
		return "Invalid sample rate";
	case LDACBT_ERR_ASSERT_CHANNEL_CONFIG:
	case LDACBT_ERR_CHECK_CHANNEL_CONFIG:
		return "Invalid channel config";
	case LDACBT_ERR_ASSERT_FRAME_LENGTH:
	case LDACBT_ERR_ASSERT_SUP_FRAME_LENGTH:
	case LDACBT_ERR_ASSERT_FRAME_STATUS:
	case LDACBT_ERR_FRAME_LENGTH_OVER:
	case LDACBT_ERR_FRAME_ALIGN_OVER:
		return "Invalid frame";
	case LDACBT_ERR_ASSERT_NSHIFT:
		return "Invalid N-shift";
	case LDACBT_ERR_ASSERT_CHANNEL_MODE:
		return "Invalid channel mode";
	case LDACBT_ERR_ENC_ILL_GRADMODE:
	case LDACBT_ERR_ENC_ILL_GRADPAR_A:
	case LDACBT_ERR_ENC_ILL_GRADPAR_B:
	case LDACBT_ERR_ENC_ILL_GRADPAR_C:
	case LDACBT_ERR_ENC_ILL_GRADPAR_D:
		return "Invalid gradient parameter";
	case LDACBT_ERR_ENC_ILL_NBANDS:
		return "Invalid N-bands";
	case LDACBT_ERR_PACK_BLOCK_FAILED:
		return "Block packing error";
	case LDACBT_ERR_INPUT_BUFFER_SIZE:
		return "Too small input buffer";
	case LDACBT_ERR_UNPACK_BLOCK_FAILED:
	case LDACBT_ERR_UNPACK_BLOCK_ALIGN:
	case LDACBT_ERR_UNPACK_FRAME_ALIGN:
		return "Block unpacking error";
	case LDACBT_ERR_ILL_SYNCWORD:
		return "Invalid sync-word";
	case LDACBT_ERR_ILL_SMPL_FORMAT:
		return "Invalid sample format";
	case LDACBT_ERR_ILL_PARAM:
		return "Invalid parameter";
	case LDACBT_ERR_ILL_EQMID:
		return "Unsupported EQMID";
	case LDACBT_ERR_ILL_SAMPLING_FREQ:
		return "Unsupported sample rate";
	case LDACBT_ERR_ILL_NUM_CHANNEL:
		return "Unsupported channels";
	case LDACBT_ERR_ILL_MTU_SIZE:
		return "Unsupported MTU";
	case LDACBT_ERR_ALTER_EQMID_LIMITED:
		return "EQMID limited";
	case LDACBT_ERR_DEC_CONFIG_UPDATED:
		return "Configuration updated";
	default:
		debug("Unknown error code: %#x (API: %u, handle: %u, block: %u)",
				err, LDACBT_API_ERR(err), LDACBT_HANDLE_ERR(err), LDACBT_BLOCK_ERR(err));
		return "Unknown error";
	}
}
#endif
