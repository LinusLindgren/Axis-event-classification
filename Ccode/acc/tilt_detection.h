/*
 * Copyright (c) 2013 Axis Communications AB
 */
#ifndef TILT_DETECTION_H
#define TILT_DETECTION_H

/**
 * tilt_detection_init:
 *
 * Init function for tilt detection API. Must be called before using
 * any of the other functions.
 */
void
tilt_detection_init(void);

/**
 * tilt_detection_finalize:
 *
 * Finalize function. Must be called when finished using tilt detection.
 */
void
tilt_detection_finalize(void);

/**
 * orientation_update:
 * @data: Pointer not utilized.
 *
 * Update function for orientation API.
 *
 * Returns: #TRUE
 */
gboolean
tilt_update(gint lateral, gint longitudinal);

/**
 * tilt_detection_get_enabled:
 *
 * Function for retreiving the enable status.
 *
 * Returns: TRUE if tilt detection is enabled, else FALSE.
 */
gboolean
tilt_detection_get_enabled(void);

/**
 * tilt_detection_set_enabled:
 * @enabled: TRUE if tilt detection shall be enabled, else FALSE
 *
 * Function for enabling / disabling tilt detection.
 */
void
tilt_detection_set_enabled(const gboolean enabled);

/**
 * tilt_detection_get_trigger_angle:
 *
 * Function for retreiving the trigger angle.
 *
 * Returns: The trigger angle.
 */
guint
tilt_detection_get_trigger_angle(void);

/**
 * tilt_detection_get_sensitivity:
 *
 * Function for retrieving the tilt sensitivity.
 *
 * Returns: The sensitivity.
 */
guint
tilt_detection_get_sensitivity(void);

/**
 * tilt_detection_set_trigger_angle:
 * @angle: The trigger angle to be used
 *
 * Function for setting the trigger angle.
 *
 * Returns: TRUE is the trigger angle was set, else FALSE.
 */
gboolean
tilt_detection_set_trigger_angle(const guint angle);

/**
 * tilt_detection_set_sensitivity:
 * @angle: The sensitivity to be used
 *
 * Function for setting the sensitivity.
 *
 * Returns: #TRUE if the sensitivity was set, else #FALSE.
 */
gboolean
tilt_detection_set_sensitivity(const guint sensitivity);

/**
 * tilt_detection_trigger_event:
 *
 * This function shall be called when a tilt detection event2
 * is to be triggered.
 */
void
tilt_detection_trigger_event(void);

#endif /* TILT_DETECTION_H */

