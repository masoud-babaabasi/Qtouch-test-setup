

/*============================================================================
This file is part of QTouch Modular Library example application.

Important Note: Do not edit this file manually.
                Use QTouch Configurator within Atmel Start to apply any
                modifications to this file.

Usage License: Refer license.h file for license information
Support: Visit http://www.microchip.com/support/hottopics.aspx
               to create MySupport case.

------------------------------------------------------------------------------
Copyright (c) 2021 Microchip. All rights reserved.
------------------------------------------------------------------------------
============================================================================*/

/*----------------------------------------------------------------------------
  include files
----------------------------------------------------------------------------*/
#include "touch.h"

#if KRONOCOMM_ENABLE == 1u
#include "kronocommadaptor.h"
#if KRONOCOMM_UART == 1u

#include "KronoCommuart.h"

#endif

static uint8_t                   writeback_req = 0u;
extern qtm_acquisition_control_t qtlib_acq_set1;
extern qtm_touch_key_control_t   qtlib_key_set1;

extern qtm_surface_cs_control_t   qtm_surface_cs_control1;
extern qtm_surface_contact_data_t qtm_surface_cs_data1;

#if (KRONO_GESTURE_ENABLE == 1u)
extern qtm_gestures_2d_control_t qtm_gestures_2d_control1;
extern qtm_gestures_2d_config_t  qtm_gestures_2d_config;
extern qtm_gestures_2d_data_t    qtm_gestures_2d_data;
#endif
extern qtm_touch_key_data_t         qtlib_key_data_set1[DEF_NUM_SENSORS];
extern qtm_acq_avr_da_node_config_t ptc_seq_node_cfg1[DEF_NUM_CHANNELS];
extern qtm_touch_key_config_t       qtlib_key_configs_set1[DEF_NUM_SENSORS];

static uint8_t cfgRam[CFGRAM_SIZE] = {SURFACE_CS_NUM_KEYS_H, // X
                                      SURFACE_CS_NUM_KEYS_V, // Y
                                      64,                    // oversampling
                                      20,                    // X threshold
                                      20,                    // Y threshold
                                      0,
                                      0,
                                      0,
                                      0,
                                      (uint8_t)SCR_RESOLUTION(SURFACE_CS_RESOL_DB)};

#if (KRONO_GESTURE_ENABLE == 1u)
static uint8_t gestureCfgRam[GESTURECFGRAM_SIZE] = {TAP_RELEASE_TIMEOUT,
                                                    TAP_HOLD_TIMEOUT,
                                                    SWIPE_TIMEOUT,
                                                    HORIZONTAL_SWIPE_DISTANCE_THRESHOLD,
                                                    VERTICAL_SWIPE_DISTANCE_THRESHOLD,
                                                    0,
                                                    TAP_AREA,
                                                    SEQ_TAP_DIST_THRESHOLD,
                                                    EDGE_BOUNDARY,
                                                    WHEEL_POSTSCALER,
                                                    WHEEL_START_QUADRANT_COUNT,
                                                    WHEEL_REVERSE_QUADRANT_COUNT,

                                                    0,

                                                    150};
#endif
static uint8_t coreRam[CORERAM_SIZE] = {VERSION_HI, VERSION_LO, ID_HI, ID_LO, 0u, (MODE_TOUCH | MODE_GESTURE), 1u, 0u};

static uint8_t  touchRam[6];
static uint8_t  touchDelta[SURFACE_CS_NUM_KEYS_V + SURFACE_CS_NUM_KEYS_H];
static uint16_t touchRaw[SURFACE_CS_NUM_KEYS_V + SURFACE_CS_NUM_KEYS_H];
static uint16_t touchRef[SURFACE_CS_NUM_KEYS_V + SURFACE_CS_NUM_KEYS_H];

uint8_t isPowerOfTwo(uint8_t x)
{
	return ((x != 0) && ((x & (~x + 1)) == x));
}

void Krono_UpdateBuffer(void)
{
	static uint8_t initialized           = 0;
	static uint8_t frameCount            = 0;
	static uint8_t previousSurfaceStatus = 0;

	uint8_t new_setting;

	uint8_t temp_i_var, temp_j_var = 0u;

	/* CfGRam update */
	if (initialized == 0) {
		cfgRam[CFGGRAM_OVRSMPLS_ADDR] = 1 << (ptc_seq_node_cfg1[SURFACE_CS_START_KEY_H].node_oversampling);
		cfgRam[CFGGRAM_THRESH_X_ADDR] = qtlib_key_configs_set1[SURFACE_CS_START_KEY_H].channel_threshold;
		cfgRam[CFGGRAM_THRESH_Y_ADDR] = qtlib_key_configs_set1[SURFACE_CS_START_KEY_V].channel_threshold;
		initialized                   = 1;
	} else if (0u != writeback_req) {
		/* Check if any valid parameters have been written to RAM Buffer */
		/* Oversampling */
		new_setting = cfgRam[CFGGRAM_OVRSMPLS_ADDR];
		if (new_setting != (uint8_t)(1u << (ptc_seq_node_cfg1[SURFACE_CS_START_KEY_H].node_oversampling))) {
			/* Find the highest bit of the new oversampling setting */
			for (temp_i_var = 0u; temp_i_var < 8u; temp_i_var++) {
				temp_j_var = (1u << (7u - temp_i_var));
				temp_j_var &= new_setting;
				if (0u != temp_j_var) {
					/* This bit set */
					new_setting = (7u - temp_i_var);
					temp_i_var  = 99u;
				}
			}
			/* Check max for device */
			if (new_setting > 6u) {
				new_setting = 6u;
			}
			/* Write actual setting back to RAM Buffer */
			cfgRam[CFGGRAM_OVRSMPLS_ADDR] = (uint8_t)(1u << new_setting);

			/* Write oversampling to each node in config */
			for (temp_i_var = 0; temp_i_var < SURFACE_CS_NUM_KEYS_H; temp_i_var++) {
				ptc_seq_node_cfg1[SURFACE_CS_START_KEY_H + temp_i_var].node_oversampling = new_setting;
			}

			for (temp_i_var = 0; temp_i_var < SURFACE_CS_NUM_KEYS_V; temp_i_var++) {
				ptc_seq_node_cfg1[SURFACE_CS_START_KEY_V + temp_i_var].node_oversampling = new_setting;
			}
			touch_init(); /* Re-initialize the touch library */
		}
		/* X Threshold */
		if (cfgRam[CFGGRAM_THRESH_X_ADDR] != qtlib_key_configs_set1[SURFACE_CS_START_KEY_H].channel_threshold) {
			for (temp_i_var = 0; temp_i_var < SURFACE_CS_NUM_KEYS_H; temp_i_var++) {
				qtlib_key_configs_set1[SURFACE_CS_START_KEY_H + temp_i_var].channel_threshold = cfgRam[3];
			}
		}
		/* Y Threshold */
		if (cfgRam[CFGGRAM_THRESH_Y_ADDR] != qtlib_key_configs_set1[SURFACE_CS_START_KEY_V].channel_threshold) {
			for (temp_i_var = 0; temp_i_var < SURFACE_CS_NUM_KEYS_V; temp_i_var++) {
				qtlib_key_configs_set1[SURFACE_CS_START_KEY_V + temp_i_var].channel_threshold = cfgRam[4];
			}
		}
		/* Resolution */
		if (cfgRam[CFGGRAM_RESOL_ADDR]
		    != SCR_RESOLUTION(qtm_surface_cs_control1.qtm_surface_cs_config->resol_deadband)) {
			qtm_surface_cs_control1.qtm_surface_cs_config->resol_deadband &= 0x0Fu;
			qtm_surface_cs_control1.qtm_surface_cs_config->resol_deadband
			    |= ((cfgRam[CFGGRAM_RESOL_ADDR] & 0x0Fu) << 4u);
		}
#if (KRONO_GESTURE_ENABLE == 1u)
		/* Gesture Config */
		qtm_gestures_2d_control1.qtm_gestures_2d_config->tapReleaseTimeout
		    = gestureCfgRam[GESTCFGRAM_TAP_RELEASE_TIMEOUT_ADDR];
		qtm_gestures_2d_control1.qtm_gestures_2d_config->tapHoldTimeout
		    = gestureCfgRam[GESTCFGRAM_TAP_HOLD_TIMEOUT_ADDR];
		qtm_gestures_2d_control1.qtm_gestures_2d_config->swipeTimeout = gestureCfgRam[GESTCFGRAM_SWIPE_TIMEOUT_ADDR];
		qtm_gestures_2d_control1.qtm_gestures_2d_config->xSwipeDistanceThreshold
		    = gestureCfgRam[GESTCFGRAM_XSWIPE_DIST_ADDR];
		qtm_gestures_2d_control1.qtm_gestures_2d_config->ySwipeDistanceThreshold
		    = gestureCfgRam[GESTCFGRAM_YSWIPE_DIST_ADDR];
		qtm_gestures_2d_control1.qtm_gestures_2d_config->edgeSwipeDistanceThreshold
		    = gestureCfgRam[GESTCFGRAM_EDGSWIPE_DIST_ADDR];
		qtm_gestures_2d_control1.qtm_gestures_2d_config->tapDistanceThreshold = gestureCfgRam[GESTCFGRAM_TAP_DIST_ADDR];
		qtm_gestures_2d_control1.qtm_gestures_2d_config->seqTapDistanceThreshold
		    = gestureCfgRam[GESTCFGRAM_MULTI_TAP_DIST_ADDR];
		qtm_gestures_2d_control1.qtm_gestures_2d_config->edgeBoundary = gestureCfgRam[GESTCFGRAM_EDGE_BOUND_ADDR];
		qtm_gestures_2d_control1.qtm_gestures_2d_config->wheelPostscaler
		    = gestureCfgRam[GESTCFGRAM_WHEEL_POSTSCAL_ADDR];
		qtm_gestures_2d_control1.qtm_gestures_2d_config->wheelStartQuadrantCount
		    = gestureCfgRam[GESTCFGRAM_WHEEL_STARTQUAD_ADDR];
		qtm_gestures_2d_control1.qtm_gestures_2d_config->wheelReverseQuadrantCount
		    = gestureCfgRam[GESTCFGRAM_WHEEL_REVQUAD_ADDR];
		qtm_gestures_2d_control1.qtm_gestures_2d_config->pinchZoomThreshold
		    = gestureCfgRam[GESTCFGRAM_PINCHZOOM_THRS_ADDR];
		// qtlib_key_configs_set1[16].channel_threshold = gestureCfgRam[GESTCFGRAM_PALMDETECTION_THRS_ADDR]; // the palm
		// detection threshold will be moved into the gesture parameter in the future

#endif
		writeback_req = 0u;
	}

		/* Write to RAM buffers */

		/* Update TouchRam */
#if (KRONO_GESTURE_ENABLE == 1u)
	touchRam[5] = qtm_gestures_2d_data.gestures_info;

	touchRam[4] = qtm_gestures_2d_data.gestures_which_gesture;
#endif
	touchRam[0] = 0x00;

#if (KRONO_GESTURE_ENABLE == 1u)
	if (qtm_gestures_2d_data.gestures_status) {
		touchRam[0] |= TOUCHSTATE_GES;
		SetIRQPin();
	}
#endif

	{
		if (qtm_surface_cs_control1.qtm_surface_contact_data[0].qt_surface_status & TOUCH_ACTIVE) {
			touchRam[0] |= TOUCHSTATE_TCH;
			SetIRQPin();
		}

		else
			touchRam[0] &= TOUCHSTATE_nTCH;
	}

	touchRam[0] |= frameCount;
	frameCount += 0x10;

#if (KRONO_GESTURE_ENABLE == 1u)
	{
		touchRam[1] = (uint8_t)((*(qtm_gestures_2d_config.horiz_position0) >> 4) & 0x00FF);
		touchRam[2] = (uint8_t)((*(qtm_gestures_2d_config.vertical_position0) >> 4) & 0x00FF);
		touchRam[3] = 0;
		touchRam[3] = (uint8_t)((*(qtm_gestures_2d_config.horiz_position0) & 0x000F) << 4); /* Append LSB for X */
		touchRam[3] |= (uint8_t)((*(qtm_gestures_2d_config.vertical_position0) & 0x000F));  /* Append MSB for Y */
	}
#else
	{
		touchRam[1] = (uint8_t)((qtm_surface_cs_control1.qtm_surface_contact_data->h_position >> 4) & 0x00FF);
		touchRam[2] = (uint8_t)((qtm_surface_cs_control1.qtm_surface_contact_data->v_position >> 4) & 0x00FF);
		touchRam[3] = 0;
		touchRam[3] = (uint8_t)((qtm_surface_cs_control1.qtm_surface_contact_data->h_position & 0x000F)
		                        << 4); /* Append LSB for X */
		touchRam[3] |= (uint8_t)(
		    (qtm_surface_cs_control1.qtm_surface_contact_data->v_position & 0x000F)); /* Append MSB for Y */
	}
#endif
	/* Update Touch Raw Reference and Delta */
	/* QTM start with vertical axis */
	for (uint8_t i = 0; i < SURFACE_CS_NUM_KEYS_H + SURFACE_CS_NUM_KEYS_V; i++) {
		if (i < SURFACE_CS_NUM_KEYS_H) {
			touchRaw[i]   = qtlib_key_data_set1[SURFACE_CS_START_KEY_H + i].node_data_struct_ptr->node_acq_signals;
			touchRef[i]   = qtlib_key_data_set1[SURFACE_CS_START_KEY_H + i].channel_reference;
			touchDelta[i] = (touchRaw[i] > touchRef[i]) ? (touchRaw[i] - touchRef[i]) : 0;
		} else {
			touchRaw[i] = qtlib_key_data_set1[SURFACE_CS_START_KEY_V + i - SURFACE_CS_NUM_KEYS_H]
			                  .node_data_struct_ptr->node_acq_signals;
			touchRef[i]   = qtlib_key_data_set1[SURFACE_CS_START_KEY_V + i - SURFACE_CS_NUM_KEYS_H].channel_reference;
			touchDelta[i] = (touchRaw[i] > touchRef[i]) ? (touchRaw[i] - touchRef[i]) : 0;
		}
	}

	/* Force re-calibration of the whole system */
	if (coreRam[CORERAM_CMD_ADDR] & 0x01) {
		touch_init();
		coreRam[CORERAM_CMD_ADDR] &= 0xFE;
	}

	if (qtm_surface_cs_data1.qt_surface_status != previousSurfaceStatus)

	{
		SetIRQPin();
	}

	{
		uart_send_touch_gesture_data();
	}

	previousSurfaceStatus = qtm_surface_cs_data1.qt_surface_status;
}

void InitIRQPin(void)
{
}

void SetIRQPin(void)
{
}

void ClearIRQPin(void)
{
}

uint8_t Krono_memory_map_read(uint8_t mem_map_address)
{
	uint16_t temp_read_16bit;
	uint8_t  temp_addr_calc;
	uint8_t  return_this_byte = 0u;

	if (mem_map_address < (VADDR_CORERAM + CORERAM_SIZE)) {
		return_this_byte = coreRam[mem_map_address];
	} else if ((mem_map_address >= VADDR_TOUCHRAM) && (mem_map_address < (VADDR_TOUCHRAM + TOUCHRAM_SIZE))) {
		return_this_byte = touchRam[mem_map_address - VADDR_TOUCHRAM];
	} else if ((mem_map_address >= VADDR_CFGRAM) && (mem_map_address < (VADDR_CFGRAM + CFGRAM_SIZE))) {
		return_this_byte = cfgRam[mem_map_address - VADDR_CFGRAM];
	}
#if (KRONO_GESTURE_ENABLE == 1u)
	else if ((mem_map_address >= VADDR_GESTURECFGRAM)
	         && (mem_map_address < (VADDR_GESTURECFGRAM + GESTURECFGRAM_SIZE))) {
		return_this_byte = gestureCfgRam[mem_map_address - VADDR_GESTURECFGRAM];
	}
#endif
	else if ((mem_map_address >= VADDR_SVRAM)
	         && (mem_map_address < (VADDR_SVRAM + (SURFACE_CS_NUM_KEYS_V + SURFACE_CS_NUM_KEYS_H)))) {
		return_this_byte = touchDelta[mem_map_address - VADDR_SVRAM];
	} else if ((mem_map_address >= VADDR_RAWVALUES)
	           && (mem_map_address < (VADDR_RAWVALUES + ((SURFACE_CS_NUM_KEYS_V + SURFACE_CS_NUM_KEYS_H) << 1u)))) {
		temp_addr_calc  = mem_map_address - VADDR_RAWVALUES;
		temp_read_16bit = touchRaw[temp_addr_calc >> 1u];
		if (0u == (temp_addr_calc % 2)) {
			/* Even number -> LSB */
			return_this_byte = (uint8_t)(temp_read_16bit & 0x00FFu);
		} else {
			/* Odd number -> MSB */
			return_this_byte = (uint8_t)((temp_read_16bit & 0xFF00u) >> 8u);
		}
	} else if ((mem_map_address >= VADDR_BASEVALUES)
	           && (mem_map_address < (VADDR_BASEVALUES + ((SURFACE_CS_NUM_KEYS_V + SURFACE_CS_NUM_KEYS_H) << 1u)))) {
		temp_addr_calc  = mem_map_address - VADDR_BASEVALUES;
		temp_read_16bit = touchRef[temp_addr_calc >> 1u];
		if (0u == (temp_addr_calc % 2)) {
			/* Even number -> LSB */
			return_this_byte = (uint8_t)(temp_read_16bit & 0x00FFu);
		} else {
			/* Odd number -> MSB */
			return_this_byte = (uint8_t)((temp_read_16bit & 0xFF00u) >> 8u);
		}
	} else {
		/* Address not valid */
	}

	return return_this_byte;
}

uint8_t Krono_memory_map_write(uint8_t mem_map_address, uint8_t write_what)
{
	uint8_t return_this_byte = 0u;

	/* CMD Byte */
	if (mem_map_address == 4u) {
		coreRam[mem_map_address] = write_what;
	}
	/* Touch Config - No access to NUM_KEYS_V or NUM_KEYS_H */
	else if ((mem_map_address >= (VADDR_CFGRAM + 2u)) && (mem_map_address < (VADDR_CFGRAM + CFGRAM_SIZE))) {
		cfgRam[mem_map_address - VADDR_CFGRAM] = write_what;
	}
#if (KRONO_GESTURE_ENABLE == 1u)
	/* Gesture Config - All access */
	else if ((mem_map_address >= VADDR_GESTURECFGRAM)
	         && (mem_map_address < (VADDR_GESTURECFGRAM + GESTURECFGRAM_SIZE))) {
		gestureCfgRam[mem_map_address - VADDR_GESTURECFGRAM] = write_what;
	}
#endif
	else {
		/* Address not valid */
		return_this_byte = 1u;
	}

	if (0u == return_this_byte) {
		writeback_req = 1u;
	}

	return return_this_byte;
}

#endif