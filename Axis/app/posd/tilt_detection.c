/*
 * Copyright (c) 2015 Axis Communications AB
 */

/****************** INCLUDE FILES SECTION ***********************************/
#include <glib.h>
#include <glib-object.h>
#include <glib/gstdio.h>
#include <gio/gio.h>
#include <stdlib.h>
#include <event2.h>
#include <syslog.h>
#include "tilt_detection.h"
#include "posd.h"
#include "posd_state.h"
#include "datacache_posd.h"
#include "kiss_fft.h"
#include <time.h>
#include "extract_acc_features.h"
#include "svm.h"
#ifdef HOST
#include "checktests/stubs/positioning_stubs.h"
#include "checktests/stubs/stubbed_dbus.h"
#include "checktests/stubs/stubbed_events.h"
#include "checktests/check_testmacros.h"
#endif
/****************** CONSTANT AND MACRO SECTION ******************************/
#define TAMPER_GRACE_PERIOD 200 //milliseconds times UPDATE_INTERVAL_SLOW_IN_MSEC
#define UPDATE_INTERVAL_IN_MSEC (5)

#define ILLEGAL_LATERAL_VALUE -91
#define ILLEGAL_LONGITUDINAL_VALUE -181
#define MOVING_AVERAGE_FILTER_LEN 10
#define NUM_WINDOWS 2
#define WINDOW_LEN 128
#define SAMPLES_TO_COLLECT (WINDOW_LEN*NUM_WINDOWS)
#define NUM_PEAKS 5
#define NUM_MAIN_PEAKS 2
#define MINIMUM_DOMINANT_PEAK_FREQUENCY 2
#define MAX_HALF_LOBE_WIDTH 3
#define PEAK_DAMP_FACTOR 0.1

//User serviceable parts:
#define ENERGY_POSITIVE_THRESHOLD (tilt_detection_get_sensitivity()*(-0.202f)+25.202f)
#define ENERGY_NEGATIVE_THRESHOLD 0.25f
#define POWER_RATIO_THRESHOLD 0.80f
#define PEAK_RATIO_THRESHOLD 1.25f
#define NOISE_FILTER_OBSERVATION_COUNT 5
#define NOISE_FILTER_PASS_COUNT 3
#define UPDATE_INTERVAL_SLOW_IN_MSEC UPDATE_INTERVAL_IN_MSEC*5

//#define force_test
//insert values manually below, e.g. cat data.txt | xargs echo | tr " " ", "
//int force_val[SAMPLES_TO_COLLECT] = {};
#ifdef force_test
#define print_stats
#undef DBG
#define DBG(x) x
#endif
//define debug
#undef DBG
#define DBG(x) x

#ifdef print_stats
static int sum_events = 0;
static int false_positives = 0;
#endif
//#define MONKEY_TESTING
#ifdef MONKEY_TESTING
# undef TAMPER_GRACE_PERIOD
# define TAMPER_GRACE_PERIOD 1
#undef UPDATE_INTERVAL_IN_MSEC
#define UPDATE_INTERVAL_IN_MSEC 0
#define STARTUP_LATERAL lateral-8
#undef SAMPLES_TO_COLLECT
#define SAMPLES_TO_COLLECT 32
#undef WINDOW_LEN
#define WINDOW_LEN 16
#else
#define STARTUP_LATERAL lateral;
#endif
//define constants for activity after sampling
#define ANALYZE_THREAD 0 //old classifier
#define SAVE_SAMPLES 1 //save samples to file
#define CLASSIFY_TAMPERING 2 //new classifier
#define SAMPLING_ACTIVITY 2 // choice of activity



/* Introspection data for the service we are exporting */
static const gchar introspection_xml[] =
  "<node>"
  "  <interface name='com.axis.Positioning.TiltDetection'>"
  "    <method name='SetTiltDetectionEnabled'>"
  "      <arg type='b' name='activate' direction='in'/>"
  "    </method>"
  "    <method name='GetTiltDetectionEnabled'>"
  "      <arg type='b' name='activated' direction='out'/>"
  "    </method>"
  "    <method name='SetTriggerAngle'>"
  "      <arg type='i' name='triggerangle' direction='in'/>"
  "    </method>"
  "    <method name='GetTriggerAngle'>"
  "      <arg type='i' name='triggerangle' direction='out'/>"
  "    </method>"
  "    <method name='TriggerTiltDetectedEvent'>"
  "    </method>"
  "  </interface>"
  "</node>";

#define CHECK_TILT_IS_INITIALIZED_OR_RETURN(func, ret) \
  if (tilt_user_data == NULL || \
      tilt_user_data->initialized == FALSE) { \
    syslog (LOG_WARNING, \
      "Tilt detection needs to be initialized before calling %s", func); \
    return ret; \
  }

#define CHECK_TILT_IS_ENABLED_OR_RETURN(func, ret) \
  if (tilt_user_data == NULL || \
      tilt_user_data->enabled == FALSE) { \
    DBG(syslog (LOG_DEBUG, \
      "Tilt detection needs to be enabled before calling %s", func)); \
    return ret; \
  }

/****************** TYPE DEFINITION SECTION *********************************/
/**
 * @sample_log: An int array of size SAMPLES_TO_COLLECT containing
 * tilt data relative to the startup tilt angle.
 */
typedef struct {
  gint *collected_tilt_samples;
  device_sample** collected_samples;
  GMutex *report_mutex;
  gboolean *tamper_triggered;
  gint acc_x_at_startup;
  gint acc_y_at_startup;
  gint acc_z_at_startup;
} thread_data_type;



/**
 * tilt_detection_data_type:
 * @dbus_owner_id: The id used to stop the dbus service.
 * @trigger_angle: the trigger angle identifier.
 * @sensitivity: the sensitivity identifier.
 * @enabled: #TRUE if tilt detection enabled, else #FALSE.
 * @initialized: #TRUE if tilt detection has been initialized, else #FALSE.
 * @lateral: current number of lateral degrees of tilt
 * @startup_lateral: baseline lateral degrees of tilt to compare against
 * @longitudinal: current number of longitudinal degrees of tilt
 * @startup_longitudinal: baseline longitudinal degrees of tilt to compare against
 */
typedef struct {
  guint dbus_owner_id;
  guint trigger_angle;
  guint sensitivity;
  gboolean enabled;
  gboolean initialized;
  gint lateral;
  gint startup_lateral;
  gint longitudinal;
  gint startup_longitudinal;
  guint update_counter;
  guint samples;
  gboolean tamper_triggered;
  gboolean should_record_samples;
  //gint *collected_samples;
  device_sample** collected_samples;
  gboolean observing;
  gint pass_count;
  gint observation_count;
  GMutex report_mutex;
  gboolean sample_speed_fast;
  gint acc_x_at_startup;
  gint acc_y_at_startup;
  gint acc_z_at_startup;
} tilt_detection_data_type;



/****************** GLOBAL VARIABLE DECLARATION SECTION *********************/

/****************** LOCALE VARIABLE DECLARATION SECTION**********************/
static tilt_detection_data_type *tilt_user_data = NULL;
static GDBusNodeInfo *introspection_data = NULL;
static svm_linear_model_data_type* svm_model = NULL;

/* Event2 variables */
static EventProducer *producer = NULL;
static guint declaration_id = 0;

/****************** LOCAL FUNCTION DECLARATION SECTION **********************/
/**
 * handle_dbus_method_call:
 * @connection: a GDBusConnection (not used).
 * @sender: the bus name if the remote caller (not used).
 * @object_path: the object path of the invoked method (not used).
 * @interface_name: the interface name of the invoked method (not used).
 * @method_name: the name of the invoked method.
 * @parameters: a GVariant containing the method call parameters.
 * @invocation: the object to send the return value to.
 * @user_data: tilt detection session data.
 *
 * Function for handling incoming dbus method calls.
 */
static void
handle_dbus_method_call(G_GNUC_UNUSED GDBusConnection *connection,
                        G_GNUC_UNUSED const gchar *sender,
                        G_GNUC_UNUSED const gchar *object_path,
                        G_GNUC_UNUSED const gchar *interface_name,
                        const gchar *method_name,
                        GVariant *parameters,
                        GDBusMethodInvocation *invocation,
                        gpointer user_data);

/**
 * handle_dbus_get_property:
 * @connection: a GDBusConnection (not used).
 * @sender: the bus name if the remote caller (not used).
 * @object_path: the object path of the invoked method (not used).
 * @interface_name: the interface name of the property (not used).
 * @property_name: the name of the property to get the value of (not used).
 * @error: error return location (not used).
 * @user_data: tilt detection session data (not used).
 *
 * Function for handling dbus get property calls.
 *
 * Returns: a #GVariant for the requested property, else #NULL.
 */
static GVariant *
handle_dbus_get_property(G_GNUC_UNUSED GDBusConnection *connection,
                         G_GNUC_UNUSED const gchar *sender,
                         G_GNUC_UNUSED const gchar *object_path,
                         G_GNUC_UNUSED const gchar *interface_name,
                         G_GNUC_UNUSED const gchar *property_name,
                         G_GNUC_UNUSED GError **error,
                         G_GNUC_UNUSED gpointer user_data);

/**
 * handle_dbus_set_property:
 * @connection: a GDBusConnection (not used).
 * @sender: the bus name if the remote caller (not used).
 * @object_path: the object path of the invoked method (not used).
 * @interface_name: the interface name of the property (not used).
 * @property_name: the name of the property to set (not used).
 * @value: the value to set the property to (not used).
 * @error: error return location (not used).
 * @user_data: tilt detection session data (not used).
 *
 * Function for handling dbus set property calls.
 *
 * Returns: #TRUE is the value was set, else #FALSE.
 */
static gboolean
handle_dbus_set_property(G_GNUC_UNUSED GDBusConnection *connection,
                         G_GNUC_UNUSED const gchar *sender,
                         G_GNUC_UNUSED const gchar *object_path,
                         G_GNUC_UNUSED const gchar *interface_name,
                         G_GNUC_UNUSED const gchar *property_name,
                         G_GNUC_UNUSED GVariant *value,
                         G_GNUC_UNUSED GError **error,
                         G_GNUC_UNUSED gpointer user_data);

/**
 * on_bus_acuired:
 * @connection: a GDBusConnection.
 * @name: the name that was requested to own (not used).
 * @user_data: tilt detection session data.
 *
 * Function that is called when the requested bus is acquired.
 */
static void
on_bus_acquired(GDBusConnection *connection,
                G_GNUC_UNUSED const gchar *name,
                gpointer user_data);

/**
 * on_name_acuired:
 * @connection: a GDBusConnection (not used).
 * @name: the name that is now owned (not used).
 * @user_data: tilt detection session data (not used).
 *
 * Function that is called when the requested bus name is acquired.
 */
static void
on_name_acquired(G_GNUC_UNUSED GDBusConnection *connection,
                 G_GNUC_UNUSED const gchar *name,
                 G_GNUC_UNUSED gpointer user_data);

/**
 * on_name_lost:
 * @connection: a GDBusConnection (not used).
 * @name: the name that not longer being owned (not used).
 * @user_data: tilt detection session data (not used).
 *
 * Function that is called when the requested bus name is lost.
 */
static void
on_name_lost(G_GNUC_UNUSED GDBusConnection *connection,
             G_GNUC_UNUSED const gchar *name,
             G_GNUC_UNUSED gpointer user_data);

/**
 * initialize_tilt_detection_event:
 *
 * Initialize function for tilt detection event handling.
 *
 * Returns: #TRUE if successful, else #FALSE.
 */
static gboolean
initialize_tilt_detection_event(void);

/**
 * tilt_trigger_tampering:
 *
 * Trigger a tampering event and reset the base tilt
 * degrees value.
 */
static void
tilt_trigger_tampering(void);

/**
 * compare_peaks_decending:
 *
 * Takes two floats wrapped in peak_data_t and compares them.
 *
 * Returns: an integer less than, equal to, or greater than
 *          zero if the first argument is considered to be
 *          respectively less than, equal to, or greater
 *          than the second.
 */
static int compare_peaks_decending (const void * x, const void * y);

/**
 * get_lateral_difference:
 *
 * Compare an angle with a reference angle.
 *
 * @angle: the current angle, between 0 and 180
 * @reference_angle: the reference angle, between 0 and 180
 *
 * Returns: the angular difference between the two angles,
 *          and account for rollover.
 */
static gint
get_lateral_difference(gint angle, gint reference_angle);

/**
 * get_longitudinal_difference:
 *
 * Compare an angle with a reference angle.
 *
 * @angle: the current angle, between 0 and 359
 * @reference_angle: the reference angle, between 0 and 359
 *
 * Returns: the angular difference between the two angles,
 *          and account for rollover.
 */
G_GNUC_UNUSED static gint
get_longitudinal_difference(gint angle, gint reference_angle);

/**
 * send_tilt_detection_event:
 *
 * Send a tilt detection event.
 *
 * Returns: #TRUE if event was sent, else #FALSE.
 */
static gboolean
send_tilt_detection_event(void);

/**
 * get_tilt:
 *
 * Returns: the amount of tilt in degrees, between -90 and 90.
 */
static gint get_tilt(void);

/**
 * get_rotation:
 *
 * Returns: the amount of rotation in degrees, between -180 and 180.
 */
static gint get_rotation(void);

/**
 * collect_samples:
 * @data: Pointer not utilized.
 *
 * Timer based function that samples tilt over
 * time and evaluates them for interest, and
 * collects them into a buffer when deemed valuable.
 *
 * Returns: #TRUE to continue sampling
 */
static gboolean
collect_samples(G_GNUC_UNUSED gpointer data);

/**
 * Thread start function for tilt data analyzation.
 * This thread will analyze, take the decision of tamper/no tamper
 * and notify action engine of the tamper event if needed.
 * This thread will free the input data.
 *
 * @thread_data: a struct containing the information needed for
 *               the thread to do its job, like sample data.
 *
 * Returns: NULL.
 */
static void *
analyze_samples(gpointer thread_data);

/**
 * Thread start function for tilt data saving.
 * This thread will free the input data.
 *
 * @thread_data: a struct containing the information needed for
 *               the thread to do its job, like sample data.
 *
 * Returns: NULL.
 */
static void *
save_samples_to_file(gpointer thread_data);

/**
 * Start a new thread that saves the data to file
 * The new thread will start in save_samples_to_file and it'll
 * take ownership of the thread_data_type struct provided to it.
 */
static void
start_save_to_file_thread(void);

/**
 * Thread start function classifying tampering
 * This thread will free the input data.
 *
 * @thread_data: a struct containing the information needed for
 *               the thread to do its job, like sample data.
 *
 * Returns: NULL.
 */
static void *
classify_tampering(gpointer thread_data);

/**
 * Start a new thread that analyzes the data collected to classify tampering events.
 * The new thread will start in classify_tampering, and it'll
 * take ownership of the thread_data_type struct provided to it.
 */
static void
start_classify_tampering_thread(void);


/**
 * Filter a dataset with a moving average.
 *
 * @data: an array of integers to filter
 * @size: the number of integers to filter
 *
 * Returns: an array of filtered floats, of size #size
 */
float *get_moving_average(int *data, int size);

/**
 * Get the energy present in a set of float values,
 * represented as a window.
 * A window is a subset of a larger data set.
 *
 * @window: an array of floats to analyze for energy content.
 * @size: the number of floats to analyze.
 *
 * Returns: the energy present in the window
 */
float get_energy(float *window, int size);

/**
 * Analyze a window for dominant frequencies.
 *
 * @window: an array of floats to analyze for dominant frequencies.
 * @size: the number of floats to analyze.
 *
 * Returns: %TRUE if there is a dominant frequency present in the
 *          window, false if not.
 */
gboolean has_dominant_frequency(float *window, int size);

/**
 * Classify a given situation based on energy and dominant frequencies.
 *
 * The amount of energy and the presence of a dominant frequency in each
 * window will influence how the dataset is classified.
 *
 * @energy_w1: the energy contained in window 1
 * @energy_w2: the energy contained in window 2
 * @dominant_freq_w1: true if there is a dominant frequency present in window 1
 * @dominant_freq_w2: true if there is a dominant frequency present in window 2
 *
 * Returns: the classification.
 */
int classify(float energy_w1, float energy_w2, gboolean dominant_freq_w1, gboolean dominant_freq_w2);


/**
 * Calculates the spectrum content of a given data set.
 *
 * @window: an array of floats to calculate the spectrum for.
 * @size: the number of floats to analyze.
 *
 * Returns: an array of %size floats representing the spectrum content.
 */
float *calculate_spectrum(float* window, int size);

/**
 * peak_data_t:
 * @peak_index: the index of a peak that's located somewhere in some data set.
 * @peak_value: the value of that peak.
 */
typedef struct {
    int peak_index;
    float peak_value;
} peak_data_t;

/**
 * Find the peaks in a spectrum.
 * Identify and sort peaks in decending order, from a given spectrum.
 *
 * @spectrum: an array of floats representing the spectrum to search through.
 * @size: the number of floats to analyze.
 *
 * Returns: an array of %size peak_data_t, containing the largest peak
 *          first, and the smallest peak last.
 */
peak_data_t *find_peaks(float *spectrum, int size, int *num_peaks_found);

/**
 * Check an array of peak data for the presence of a dominant peak.
 *
 * @spectrum: an array of floats representing the spectrum to search through.
 * @size: the number of floats to analyze.
 * @peaks: the set of peaks to check.
 * @num_peaks: the number of peaks contained in the set of peaks.
 *
 * Returns: #TRUE if there is a dominant peak present in the set
 * of peaks, false if there is no dominant peak.
 */
gboolean detect_dominant_peaks(float *spectrum, int size, peak_data_t * peaks, int num_peaks);


/**
 * Free the device_samples in the data
 */
static void free_collected_samples(device_sample** data, int nbrSamples);

/* Vtable used by Dbus.*/
static const GDBusInterfaceVTable interface_vtable =
{
  handle_dbus_method_call,
  handle_dbus_get_property,
  handle_dbus_set_property,
  { NULL }
};

/****************** LOCAL FUNCTION DEFINITION SECTION ***********************/

/**
 * handle_dbus_method_call:
 * @connection: a GDBusConnection (not used).
 * @sender: the bus name if the remote caller (not used).
 * @object_path: the object path of the invoked method (not used).
 * @interface_name: the interface name of the invoked method (not used).
 * @method_name: the name of the invoked method.
 * @parameters: a GVariant containing the method call parameters.
 * @invocation: the object to send the return value to.
 * @user_data: tilt detection session data.
 *
 * Function for handling incoming dbus method calls.
 */
static void
handle_dbus_method_call(G_GNUC_UNUSED GDBusConnection *connection,
                        G_GNUC_UNUSED const gchar *sender,
                        G_GNUC_UNUSED const gchar *object_path,
                        G_GNUC_UNUSED const gchar *interface_name,
                        const gchar *method_name,
                        GVariant *parameters,
                        GDBusMethodInvocation *invocation,
                        gpointer user_data)
{
  tilt_detection_data_type *sess_data = (tilt_detection_data_type*)user_data;

  if (g_strcmp0(method_name, "SetTriggerAngle") == 0) {
    guint trigger_angle;
    g_variant_get(parameters, "(i)", &trigger_angle);
    DBG(syslog (LOG_DEBUG, "Received: %d\n", trigger_angle));
    if (tilt_detection_set_trigger_angle(trigger_angle)) {
      g_dbus_method_invocation_return_value(invocation, NULL);
    } else {
      g_dbus_method_invocation_return_dbus_error(invocation,
                                                 "org.freedesktop.DBus.Error.InvalidArgs",
                                                 "Valid trigger angle range is 1 - 90");
    }
  } else if (g_strcmp0(method_name, "GetTriggerAngle") == 0) {
    g_dbus_method_invocation_return_value(invocation,
                                          g_variant_new("(i)",
                                          sess_data->trigger_angle));
  } else if (g_strcmp0(method_name, "SetSensitivity") == 0) {
    guint sensitivity;
    g_variant_get(parameters, "(i)", &sensitivity);
    DBG(syslog (LOG_DEBUG, "Received: %d\n", sensitivity));
    if (tilt_detection_set_sensitivity(sensitivity)) {
      g_dbus_method_invocation_return_value(invocation, NULL);
    } else {
      g_dbus_method_invocation_return_dbus_error(invocation,
                                                 "org.freedesktop.DBus.Error.InvalidArgs",
                                                 "Valid sensitivity range is 1 - 100");
    }
  } else if (g_strcmp0(method_name, "GetSensitivity") == 0) {
    g_dbus_method_invocation_return_value(invocation,
                                          g_variant_new("(i)",
                                          sess_data->sensitivity));
  } else if (g_strcmp0(method_name, "GetTiltDetectionEnabled") == 0) {
    g_dbus_method_invocation_return_value(invocation,
                                          g_variant_new("(b)",
                                          sess_data->enabled));
  } else if (g_strcmp0(method_name, "SetTiltDetectionEnabled") == 0) {
    gboolean enabled;
    g_variant_get(parameters, "(b)", &enabled);
    tilt_detection_set_enabled(enabled);
    DBG(syslog (LOG_DEBUG, "Received: %d\n", enabled));
    g_dbus_method_invocation_return_value(invocation, NULL);
  } else if (g_strcmp0(method_name, "TriggerTiltDetectedEvent") == 0) {
    /* Trigger tilt detection event. Used for testing. */
    tilt_detection_trigger_event();
    g_dbus_method_invocation_return_value(invocation, NULL);
  } else {
    g_dbus_method_invocation_return_dbus_error(invocation,
                                               "org.freedesktop.DBus.Error.UnknownMethod",
                                               "An unknown method was called");
  }
}
/**
 * handle_dbus_get_property:
 * @connection: a GDBusConnection (not used).
 * @sender: the bus name if the remote caller (not used).
 * @object_path: the object path of the invoked method (not used).
 * @interface_name: the interface name of the property (not used).
 * @property_name: the name of the property to get the value of (not used).
 * @error: error return location (not used).
 * @user_data: tilt detection session data (not used).
 *
 * Function for handling dbus get property calls.
 *
 * Returns: a #GVariant for the requested property, else #NULL.
 */

static GVariant *
handle_dbus_get_property(G_GNUC_UNUSED GDBusConnection *connection,
                         G_GNUC_UNUSED const gchar *sender,
                         G_GNUC_UNUSED const gchar *object_path,
                         G_GNUC_UNUSED const gchar *interface_name,
                         G_GNUC_UNUSED const gchar *property_name,
                         G_GNUC_UNUSED GError **error,
                         G_GNUC_UNUSED gpointer user_data)
{
  /* Do nothing. */
  return NULL;
}

/**
 * handle_dbus_set_property:
 * @connection: a GDBusConnection (not used).
 * @sender: the bus name if the remote caller (not used).
 * @object_path: the object path of the invoked method (not used).
 * @interface_name: the interface name of the property (not used).
 * @property_name: the name of the property to set (not used).
 * @value: the value to set the property to (not used).
 * @error: error return location (not used).
 * @user_data: tilt detection session data (not used).
 *
 * Function for handling dbus set property calls.
 *
 * Returns: #TRUE is the value was set, else #FALSE.
 */
static gboolean
handle_dbus_set_property(G_GNUC_UNUSED GDBusConnection *connection,
                         G_GNUC_UNUSED const gchar *sender,
                         G_GNUC_UNUSED const gchar *object_path,
                         G_GNUC_UNUSED const gchar *interface_name,
                         G_GNUC_UNUSED const gchar *property_name,
                         G_GNUC_UNUSED GVariant *value,
                         G_GNUC_UNUSED GError **error,
                         G_GNUC_UNUSED gpointer user_data)
{
  /* Do nothing. */
  return TRUE;
}

/**
 * on_bus_acuired:
 * @connection: a GDBusConnection.
 * @name: the name that was requested to own (not used).
 * @user_data: tilt detection session data.
 *
 * Function that is called when the requested bus is acquired.
 */
static void
on_bus_acquired(GDBusConnection *connection,
                G_GNUC_UNUSED const gchar *name,
                gpointer user_data)
{
  guint registration_id;

  registration_id =
    g_dbus_connection_register_object(connection,
                                      "/com/axis/Positioning/TiltDetection",
                                      introspection_data->interfaces[0],
                                      &interface_vtable,
                                      user_data,  /* user_data */
                                      NULL,  /* user_data_free_func */
                                      NULL); /* GError** */
  if (registration_id <= 0) {
    syslog (LOG_WARNING, "%s - Erroneous registration_id", __FUNCTION__);
  }
  g_assert(registration_id > 0);
}

/**
 * on_name_acuired:
 * @connection: a GDBusConnection(not used).
 * @name: the name that is now owned(not used).
 * @user_data: tilt detection session data(not used).
 *
 * Function that is called when the requested bus name is acquired.
 */
static void
on_name_acquired(G_GNUC_UNUSED GDBusConnection *connection,
                 G_GNUC_UNUSED const gchar *name,
                 G_GNUC_UNUSED gpointer user_data)
{
  /* Do nothing. */
}

/**
 * on_name_lost:
 * @connection: a GDBusConnection (not used).
 * @name: the name that not longer being owned (not used).
 * @user_data: tilt detection session data (not used).
 *
 * Function that is called when the requested bus name is lost.
 */
static void
on_name_lost(G_GNUC_UNUSED GDBusConnection *connection,
             G_GNUC_UNUSED const gchar *name,
             G_GNUC_UNUSED gpointer user_data)
{
  /* Do nothing. */
}

/**
 * tilt_update:
 * @lateral: current number of lateral degrees of tilt
 * @longitudinal: current number of longitudinal degrees of tilt
 *
 * Update function for tilt detection.
 *
 * Returns: #TRUE if the update was successful, otherwise #FALSE
 */
gboolean
tilt_update(gint lateral, gint longitudinal)
{

 if (tilt_user_data == NULL ||
    tilt_user_data->initialized == FALSE) {
    /* Not activated, just return */
    goto exit;
  }

  if (tilt_user_data->startup_lateral == ILLEGAL_LATERAL_VALUE-1) {
    tilt_user_data->startup_lateral = STARTUP_LATERAL;
    DBG(syslog (LOG_INFO, "Startup tilt detection angle: %d\n", tilt_user_data->startup_lateral));
  }
  if (tilt_user_data->startup_longitudinal == ILLEGAL_LONGITUDINAL_VALUE-1) {
    tilt_user_data->startup_longitudinal = longitudinal;
    DBG(syslog (LOG_INFO, "Startup rotation detection angle: %d\n", tilt_user_data->startup_longitudinal));
    DBG(syslog (LOG_INFO, "Tilt tampering trigger angle: %d, sensitivity: %d=%f\n", tilt_detection_get_trigger_angle(), tilt_detection_get_sensitivity(), ENERGY_POSITIVE_THRESHOLD));
  }
  return TRUE;
exit:
  return FALSE;
}

/**
 * get_tilt:
 *
 * Returns: the amount of tilt in degrees, between -90 and 90.
 */
static gint get_tilt(void) {
  gint tilt = pos_lib_get_tilt ();
  if (tilt < -90 || tilt > 90) {
    syslog (LOG_WARNING, "Invalid tilt value returned from positioning lib: %d", tilt);
    return ILLEGAL_LATERAL_VALUE;
  }

  /* Convert [-90..90] into the range [0..180] */
  return tilt + 90;
}




/**
 * get_rotation:
 *
 * Returns: the amount of rotation in degrees, between 0 and 360.
 */
G_GNUC_UNUSED static gint get_rotation(void) {
  gdouble rotation = pos_lib_get_rotation ();
  if (rotation < 0 || rotation > 360) {
    syslog (LOG_WARNING, "Invalid rotational value returned from positioning lib: %f", rotation);
    return ILLEGAL_LONGITUDINAL_VALUE;
  }

  return (gint)rotation;
}

static void
start_record_samples(device_sample* initial_value)
{
  tilt_user_data->should_record_samples = TRUE;
  tilt_user_data->observing = TRUE;
  tilt_user_data->samples = 0; //incremented at the end of collect_samples()
  tilt_user_data->collected_samples = malloc(SAMPLES_TO_COLLECT * sizeof(device_sample*));
  tilt_user_data->collected_samples[0] = initial_value;
}

static void
stop_record_samples(void)
{
  tilt_user_data->samples = UINT_MAX;
  tilt_user_data->should_record_samples = FALSE;
}


/**
 * Start a new thread that analyzes the data collected.
 * The new thread will start in analyze_samples(), and it'll
 * take ownership of the thread_data_type struct provided to it.
 */

static void
start_analyze_thread(void)
{
  pthread_t tid;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  //main loop does not want to join, so set the detached attr:
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

  DBG(syslog (LOG_INFO, "Done sampling, starting analyze thread"));
  //malloc data struct. The new thread will free it:
  thread_data_type *thread_data = (thread_data_type*)malloc(sizeof(thread_data_type));
  thread_data->report_mutex = &tilt_user_data->report_mutex;
  thread_data->tamper_triggered = &tilt_user_data->tamper_triggered;
  thread_data->collected_samples = tilt_user_data->collected_samples;
  tilt_user_data->collected_samples = NULL;

  //start thread
  pthread_create(&tid, &attr, analyze_samples, (void*)thread_data);

  stop_record_samples();
  tilt_user_data->observation_count = 0;
  tilt_user_data->pass_count = 0;
}


/**
 * Start a new thread that saves the collected data to file.
 * The new thread will start in save_samples_to_file(), and it'll
 * take ownership of the thread_data_type struct provided to it.
 */
static void
start_save_to_file_thread(void)
{
  pthread_t tid; 
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  //main loop does not want to join, so set the detached attr:
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

  //syslog (LOG_INFO, "Done sampling, starting save to file thread");
  //malloc data struct. The new thread will free it:
  thread_data_type *thread_data = (thread_data_type*)malloc(sizeof(thread_data_type));
  thread_data->report_mutex = &tilt_user_data->report_mutex;
  thread_data->tamper_triggered = &tilt_user_data->tamper_triggered;
  thread_data->collected_samples = tilt_user_data->collected_samples;
  thread_data->acc_x_at_startup = tilt_user_data->acc_x_at_startup;
  thread_data->acc_y_at_startup = tilt_user_data->acc_y_at_startup;
  thread_data->acc_z_at_startup = tilt_user_data->acc_z_at_startup;
  tilt_user_data->collected_samples = NULL;

  //start thread
  pthread_create(&tid, &attr, save_samples_to_file, (void*)thread_data);

  stop_record_samples();
  tilt_user_data->observation_count = 0;
  tilt_user_data->pass_count = 0;
}


/**
 * Start a new thread that classifies the collected data.
 * The new thread will start in classify_tampering, and it'll
 * take ownership of the thread_data_type struct provided to it.
 */
static void
start_classify_tampering_thread(void)
{
  pthread_t tid; 
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  //main loop does not want to join, so set the detached attr:
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

  //malloc data struct. The new thread will free it:
  thread_data_type *thread_data = (thread_data_type*)malloc(sizeof(thread_data_type));
  thread_data->report_mutex = &tilt_user_data->report_mutex;
  thread_data->tamper_triggered = &tilt_user_data->tamper_triggered;
  thread_data->collected_samples = tilt_user_data->collected_samples;
  thread_data->acc_x_at_startup = tilt_user_data->acc_x_at_startup;
  thread_data->acc_y_at_startup = tilt_user_data->acc_y_at_startup;
  thread_data->acc_z_at_startup = tilt_user_data->acc_z_at_startup;
  tilt_user_data->collected_samples = NULL;

  //start thread
  pthread_create(&tid, &attr, classify_tampering, (void*)thread_data);

  stop_record_samples();
  tilt_user_data->observation_count = 0;
  tilt_user_data->pass_count = 0;
}

/**
 * Start a new thread that analyzes the data collected.
 * The new thread will start in analyze_samples(), and it'll
 * take ownership of the thread_data_type struct provided to it.
 */
static void free_collected_samples(device_sample** data, int nbrSamples)
{
	int i;
	for(i = 0; i < nbrSamples; i++)
	{
		free(data[i]);
	}
}

static void
reset_data_recording(void)
{
  stop_record_samples();
  if (tilt_user_data->collected_samples != NULL) {
    //free(tilt_user_data->collected_samples);
    free_collected_samples(tilt_user_data->collected_samples, tilt_user_data->samples);
    tilt_user_data->collected_samples = NULL;
  }
  tilt_user_data->pass_count = 0;
  tilt_user_data->observation_count = 0;
}

static void
save_sample(gint sample)
{
  //tilt_user_data->collected_samples[tilt_user_data->samples] = sample;
 tilt_user_data->collected_samples[tilt_user_data->samples] = pos_lib_get_accelerometer_raw_data();
  if (tilt_user_data->observing) {
    if (abs(sample) >= tilt_detection_get_trigger_angle()) {
      tilt_user_data->pass_count++;
    }
    tilt_user_data->observation_count++;
    if (tilt_user_data->observation_count == NOISE_FILTER_OBSERVATION_COUNT) {
      tilt_user_data->observing = FALSE;
      if (tilt_user_data->pass_count < NOISE_FILTER_PASS_COUNT) {
        DBG(syslog (LOG_INFO, "Ignoring noise below threshold"));
        reset_data_recording();
      }
    }
  }
}

static gboolean
tamper_triggered_from_work_thread(void)
{
  gboolean triggered;
  g_mutex_lock(&tilt_user_data->report_mutex);
  triggered = tilt_user_data->tamper_triggered;
  if (tilt_user_data->tamper_triggered == TRUE) {
    tilt_user_data->tamper_triggered = FALSE;
    DBG(syslog (LOG_INFO, "Tilt has been triggered, starting grace period"));
    #ifdef print_stats
    false_positives++;
    DBG(syslog (LOG_INFO, "False positives: %d", false_positives));
    #endif
  }
  g_mutex_unlock(&tilt_user_data->report_mutex);
  return triggered;
}

static gboolean
grace_period_active(void)
{
  gboolean grace_active = TRUE;

  //prevent sample counter wrap around:
  if (tilt_user_data->update_counter == UINT_MAX) {
    tilt_user_data->update_counter = TAMPER_GRACE_PERIOD;
  }
  if (tilt_user_data->update_counter == TAMPER_GRACE_PERIOD-1) {
    DBG(syslog (LOG_INFO, "Grace is ending, updating startup tilt angle"));
#ifndef MONKEY_TESTING
    tilt_user_data->startup_lateral = get_tilt();
    tilt_user_data->startup_longitudinal = get_rotation();
#endif
    grace_active = FALSE;
  }

  tilt_user_data->update_counter++;

  if (tilt_user_data->update_counter >= TAMPER_GRACE_PERIOD) {
    grace_active = FALSE;
  }

  return grace_active;
}

/**
 * collect_samples:
 * @data: Pointer not utilized.
 *
 * Timer based function that samples tilt over
 * time and evaluates them for interest, and
 * collects them into a buffer when deemed valuable.
 *
 * Returns: #TRUE to continue sampling
 */
static gboolean
collect_samples(G_GNUC_UNUSED gpointer data)
{
  //printf("asdasd\n");
  //g_printf("hej\n");
  //syslog (LOG_DEBUG, "ENTERING COLLECT SAMPLE");
  //DBG(syslog (LOG_DEBUG, "ENTERING COLLECT SAMPLE"));
  gboolean ret = TRUE;
#ifdef force_test
  int i;
  start_record_samples(force_val[0]);
  for(i=1; i<SAMPLES_TO_COLLECT; i++) {
    if (tilt_user_data->should_record_samples && tilt_user_data->samples < SAMPLES_TO_COLLECT) {
      save_sample(force_val[i]);
      tilt_user_data->samples++;
    }
  }
  if (tilt_user_data->collected_samples != NULL) start_analyze_thread();
  return FALSE;
#endif

  if (tilt_user_data->startup_lateral == ILLEGAL_LATERAL_VALUE-1) return TRUE;
  gint tilt = get_tilt();
  //TODO: handle ILLEGAL_LATERAL_VALUE returned from get_tilt() here.

  if (tamper_triggered_from_work_thread()) {
    tilt_trigger_tampering(); //this will also start the grace period
    reset_data_recording();
  }

  if (grace_period_active() == TRUE) {
    return TRUE;
  }

  //get relative tilt change from absolute tilt angle:
  tilt = get_lateral_difference(tilt, tilt_user_data->startup_lateral);

  if (tilt_user_data->should_record_samples) {
    if (tilt_user_data->samples < SAMPLES_TO_COLLECT) {
      save_sample(tilt);
    } else { //done collecting, send to worker thread
	switch(SAMPLING_ACTIVITY) 
	{
	  	case ANALYZE_THREAD  :
	      		start_analyze_thread();
	      	break; 
		case SAVE_SAMPLES  :
	      		start_save_to_file_thread();
	      	break; 
		case CLASSIFY_TAMPERING  :
	      		start_classify_tampering_thread();
	      	break; 
	}
    }
  } else {
    if (abs(tilt) >= tilt_detection_get_trigger_angle()) {
      DBG(syslog (LOG_INFO, "Detected tilt above threshold, starting sampling"));
      //start_record_samples(tilt);
      start_record_samples(pos_lib_get_accelerometer_raw_data());
      ret = FALSE;
      tilt_user_data->sample_speed_fast = TRUE;
      g_timeout_add (UPDATE_INTERVAL_IN_MSEC, collect_samples, NULL);
    }
  }

  if (tilt_user_data->samples == UINT_MAX) {
      tilt_user_data->samples = 0;
  } else {
      tilt_user_data->samples++;
  }

  if (tilt_user_data->collected_samples == NULL &&
     tilt_user_data->sample_speed_fast == TRUE) {
    ret = FALSE;
    tilt_user_data->sample_speed_fast = FALSE;
    DBG(syslog (LOG_INFO, "Going back to slow sampling"));
    g_timeout_add (UPDATE_INTERVAL_SLOW_IN_MSEC, collect_samples, NULL);
  }
  return ret;
}

static void* save_samples_to_file(gpointer thread_data){
	DBG(syslog (LOG_INFO, "Starting to save samples to file"));
		
	thread_data_type *data = (thread_data_type*)thread_data; 
	
	
	//gint *samples = data->collected_samples;
	device_sample** samples = data->collected_samples;
	char buffer[32]; // The filename buffer.
    	// Put "file" then k then ".txt" in to filename.
    	snprintf(buffer, sizeof(char) * 32, "acc_sample%i", (int)time(NULL));
	FILE* filename = g_fopen (buffer,"w");
	g_fprintf(filename,"startup(x,y,z):(%d,%d,%d)\n",data->acc_x_at_startup, data->acc_y_at_startup, data->acc_z_at_startup);
	int i;
	for(i=0;i<SAMPLES_TO_COLLECT; i++){
		g_fprintf(filename,"%d %d %d\n",samples[i]->x, samples[i]->y,samples[i]->z);
	
	}
	fclose(filename);
	
	//free(data->collected_samples);
	free_collected_samples(data->collected_samples, SAMPLES_TO_COLLECT);	
	free(data);
	DBG(syslog (LOG_INFO, "Data has been saved"));
	//exit(0);
	return NULL;
}



static void* classify_tampering(gpointer thread_data){
	DBG(syslog (LOG_INFO, "Starting tampering classification"));
	
	
	thread_data_type *data = (thread_data_type*)thread_data; 
	
	
	//gint *samples = data->collected_samples;
	device_sample** samples = data->collected_samples;
	
/*
	//temp for debug, store sample for comparison in matlab
	char bufferRaw[32]; // The filename buffer.
    	// Put "file" then k then ".txt" in to filename.
    	snprintf(bufferRaw, sizeof(char) * 32, "acc_sample%i", (int)time(NULL));
	FILE* filename = g_fopen (bufferRaw,"w");
	g_fprintf(filename,"startup(x,y,z):(%d,%d,%d)\n",data->acc_x_at_startup, data->acc_y_at_startup, data->acc_z_at_startup);
	int i;
	for(i=0;i<SAMPLES_TO_COLLECT; i++){
		g_fprintf(filename,"%d %d %d\n",samples[i]->x, samples[i]->y,samples[i]->z);
	
	}
	fclose(filename);
*/
	//onödigt ? kolla senare
	int i;
	double* sample_z = malloc(sizeof(double)*SAMPLES_TO_COLLECT);
	double* sample_x = malloc(sizeof(double)*SAMPLES_TO_COLLECT);
	double* sample_y = malloc(sizeof(double)*SAMPLES_TO_COLLECT);	
	for(i=0;i<SAMPLES_TO_COLLECT; i++){
		sample_z[i]=samples[i]->z - data->acc_z_at_startup;
		sample_x[i]=samples[i]->x - data->acc_x_at_startup;
		sample_y[i]=samples[i]->y - data->acc_y_at_startup;
	}
	
	t1 = clock(); 
	double* features = extract_features(sample_x,sample_y,sample_z,SAMPLES_TO_COLLECT);
	t2 = clock(); 
	float diff = ((float)(t2 - t1) / 1000000.0F ) * 1000;   
    	DBG(syslog (LOG_INFO, "tampering classifying time: %f",diff)); 

	double score = predict(features,svm_model,get_nbr_features());
	
	if(score > 0)
	{
		DBG(syslog (LOG_INFO, "tampering with score %f\n",score));
		tilt_trigger_tampering();
	}else
	{
		DBG(syslog (LOG_INFO, "not tampering with score %f\n",score));
	}
	//free all	
	free(features);
	free_collected_samples(data->collected_samples, SAMPLES_TO_COLLECT);	
	free(data);
	free(sample_x);
	free(sample_y);
	free(sample_z);
	return NULL;
}


// The steps in the algorithm for detecting break-in are:
// 1. When a disturbance is detected, the tilt data from the accelerometer
//    is collected for a duration of length (NUM_WINDOWS * WINDOW_LEN) ms.
//    The raw data is smoothed with a moving average filter of length
//    MOVING_AVERAGE_FILTER_LEN.
//    NOTE: Currently, the data is collected for a duration of 1 second,
//          and there are two windows (periods) of duration 0.5 second.
//
// 2. The output of the moving average filter is used to calculate the
//    energy in each of the two windows. The energy is simply the squared
//    sum of the samples over each window.
//
// 3. The periodogram (spectrum) of the samples (filtered output) is
//    determined for each of the two windows.
//    The spectral estimate is then used to determine if there is a
//    significant "high" frequency present. The presence of such a frequency
//    was found to be highly likely in a "door slam" situation.
//
// 4. The energy of the two windows and the presence/absence of a "high"
//    frequency component over the two windows are used to classify whether
//    or not a break-in has occurred.

static void *
analyze_samples(gpointer thread_data)
{
  thread_data_type *data = (thread_data_type*)thread_data;
  gint *tilt = data->collected_tilt_samples;
  int classification = 0;
  float *data_set = get_moving_average(tilt, SAMPLES_TO_COLLECT);
  float energy_w1 = get_energy(data_set, WINDOW_LEN);
  float energy_w2 = get_energy(&data_set[WINDOW_LEN], WINDOW_LEN);
  gboolean w1_has_dom_freq = FALSE;
  if (energy_w1 > ENERGY_POSITIVE_THRESHOLD) {
    w1_has_dom_freq = has_dominant_frequency(data_set, WINDOW_LEN);
  }
  gboolean w2_has_dom_freq = FALSE;
  if (energy_w2 > ENERGY_POSITIVE_THRESHOLD) {
    w2_has_dom_freq = has_dominant_frequency(&data_set[WINDOW_LEN], WINDOW_LEN);
  }
  free(data_set);

  DBG(syslog (LOG_INFO, "Energy w1 %f, w2 %f", energy_w1, energy_w2));

  classification = classify(energy_w1, energy_w2, w1_has_dom_freq, w2_has_dom_freq);

  switch(classification) {
    case 0: //Tamper
      DBG(syslog (LOG_INFO, "Event classified as tamper"));

#ifdef print_stats
      syslog (LOG_INFO, "Raw tilt data for event %d:", sum_events);
      int i;
      for(i=0;i+8<=SAMPLES_TO_COLLECT;i+=8) g_printf("%d %d %d %d %d %d %d %d\n", data->collected_samples[i+0], data->collected_samples[i+1], data->collected_samples[i+2], data->collected_samples[i+3], data->collected_samples[i+4], data->collected_samples[i+5], data->collected_samples[i+6], data->collected_samples[i+7]);
#endif
      g_mutex_lock(&tilt_user_data->report_mutex);
      if (*data->tamper_triggered == TRUE) {
        DBG(syslog (LOG_INFO, "Multiple tamper events detected in short succession. Dropping this one."));
      } else {
        *data->tamper_triggered = TRUE;
      }
      g_mutex_unlock(&tilt_user_data->report_mutex);

    break;

    case 1: //Possibly tamper
    break;

    case 2: //Possibly door slam
    break;

    default:
      DBG(syslog (LOG_INFO, "Event is not classified. Ignoring."));
  }

#ifdef print_stats
  g_mutex_lock(&tilt_user_data->report_mutex);
  syslog (LOG_INFO, "Total number of events proccessed: %d", sum_events);
  sum_events++;
  g_mutex_unlock(&tilt_user_data->report_mutex);
#endif

  free(data->collected_samples);
  free(data);
  return NULL;
}


/**
 * get_moving_average:
 * @data: raw tilt data array
 * @size: size of the tilt data array
 *
 * Implements a moving average filter. The length of the filter
 * is given by MOVING_AVERAGE_FILTER_LEN
 *
 * Returns: Filtered data (float array)
 */
float *get_moving_average(int *data, int size)
{
  int sum = 0;
  int tail;
  int i;
  int n;
  float *filter_output = (float*)malloc(NUM_WINDOWS*WINDOW_LEN*sizeof(float));
  float coeff = 1.0 / (float) MOVING_AVERAGE_FILTER_LEN;
  if (size < (NUM_WINDOWS*WINDOW_LEN)) {
    // The length of input must be NUM_WINDOWS*WINDOW_LEN.
    // If not, return NULL indicating error!
    return NULL;
  }

  // Initialize
  for (i = 0; i < MOVING_AVERAGE_FILTER_LEN; i++) {
    sum += data[i];
    filter_output[i] = (float) sum * coeff;
  }
  tail = data[0];

  // n is the number of outputs left to be calculated.
  n = size - MOVING_AVERAGE_FILTER_LEN;

  for (i = 0; i < n; i++) {
    sum -= tail;
    sum += data[MOVING_AVERAGE_FILTER_LEN+i];
    filter_output[MOVING_AVERAGE_FILTER_LEN+i] = (float) sum * coeff;
    tail = data[i+1];
  }

  syslog(LOG_INFO, "Moving average output %f", filter_output[0]);
  return filter_output;
}

float get_energy(float *window, int size)
{
  //TODO: Add code to prevent this function from returning inf or NaN.
  int i;
  float squared_sum = 0.0f;
  if (size < 1) {
    return 0.0f;
  } else if (size == 1) {
    return window[0];
  } else {
    for (i = 0;i < size; i++) {
        if (window[i] > 1215000.0f) DBG(syslog(LOG_WARNING, "Bad value in window[%d] %f", i, window[i]));
      squared_sum += (window[i]*window[i]);
    }
  }
  if (squared_sum == -1.0f) { //will yield NaN
    DBG(syslog(LOG_WARNING, "WARNING: squared_sum was -1"));
    squared_sum = 0.1f;
  }
  return sqrt(squared_sum);
}

gboolean has_dominant_frequency(float *window, int size)
{
  gboolean dominant_freq_found = FALSE;
  int num_peaks_found = 0;
  float *spectrum = calculate_spectrum(window, size);
  peak_data_t *peaks = find_peaks(spectrum, size/2, &num_peaks_found);
  dominant_freq_found = detect_dominant_peaks(spectrum, size/2, peaks, num_peaks_found);
  free(spectrum);
  free(peaks);
  return dominant_freq_found;
}

int classify(float energy_w1, float energy_w2, gboolean dominant_freq_w1, gboolean dominant_freq_w2)
{
  gboolean case1 = FALSE;
  gboolean case2 = FALSE;
  gboolean case3 = FALSE;
  gboolean case4 = FALSE;
  gboolean case5 = FALSE;
  gboolean case6 = FALSE;
  gboolean case7 = FALSE;
  float norm_energy_w2 = energy_w2/(energy_w1+energy_w2);

  /* No dominant frequency, and no energy in window-1
     This is most likely "no disturbance" */
  if (energy_w1 < ENERGY_POSITIVE_THRESHOLD && dominant_freq_w1 == FALSE) {
    DBG(syslog (LOG_INFO, "Energy in window 1 was too low."));
    case1 = TRUE;
  }

  /* Energy in window-1 is > threshold, but no
     dominant frequency -> most likely a break-in */
  if (energy_w1 > ENERGY_POSITIVE_THRESHOLD && dominant_freq_w1 == FALSE) {
    DBG(syslog (LOG_INFO, "Window 1: Energy > Threshold, No dominant freq present"));
    case2 = TRUE;
  }

  /* Energy in window-1 is < threshold, dominant frequency ->
     not sure what that is */
  if (energy_w1 < ENERGY_POSITIVE_THRESHOLD && dominant_freq_w1 == TRUE) {
    case3 = TRUE;
    DBG(syslog (LOG_INFO, "C3 true, dont know what to do!"));
  }

  /* Energy and dominant frequency present in window-1.
     This is likely a door slam or similar. */
  if (energy_w1 > ENERGY_POSITIVE_THRESHOLD && dominant_freq_w1 == TRUE) {
    DBG(syslog (LOG_INFO, "Window 1: Energy > Threshold, Dominant freq. present"));
    case4 = TRUE;
  }

  /* Energy in window-2 < Energy in window-1 *AND*
     energy in window-2 < threshold and a dominant frequency
     is absent. This situation is likely in a door slam
     when the oscillations have died out completely. */
  if (energy_w2 < energy_w1 && norm_energy_w2 < ENERGY_NEGATIVE_THRESHOLD && dominant_freq_w2 == FALSE) {
    DBG(syslog (LOG_INFO, "Energy_w2 < Energy_w1, Ratio < Threshold, No dom. freq."));
    case5 = TRUE;
  }

  /* Similar to above, but dominant frequency is presnt. But,
     perhaps the oscillations havent died out completely.
     Still a likely door slam! */
  if (energy_w2 < energy_w1 && norm_energy_w2 < ENERGY_NEGATIVE_THRESHOLD && dominant_freq_w2 == TRUE) {
    DBG(syslog (LOG_INFO, "Window 2: Energy < Energy(W1), Ratio < Threshold, Dom freq. present"));
    case6 = TRUE;
  }

  /* Energy in window-2 > threshold irrespective dominant frequency.
     Likely to be a break-in */
  if (norm_energy_w2 > ENERGY_NEGATIVE_THRESHOLD) {
    DBG(syslog (LOG_INFO, "Window 2: Ratio > Threshold."));
    case7 = TRUE;
  }

  if (case1 == TRUE || case3 == TRUE) {
    DBG(syslog (LOG_INFO, "Nothing to see! Move on!"));
  }

  if (case4 == TRUE && (case5 == TRUE || case6 == TRUE)) {
    DBG(syslog (LOG_INFO, "Door slammed!"));
  } else if (case4 == TRUE) {
    DBG(syslog (LOG_INFO, "Possible door slam, but unsure!"));
    return 2;
  }
  if (case2 == TRUE && (case7 == TRUE || case5 == TRUE)) {
    DBG(syslog (LOG_INFO, "Alarm! Break-in!!!"));
    return 0;
  } else if (case2 == TRUE) {
    DBG(syslog (LOG_INFO, "Possible break-in, but unsure!"));
    return 1;
  }

  return -1;
}

float *calculate_spectrum(float* window, int size)
{
  //TODO: sanity check size before allocation.
  int i;
  float *spectrum = (float *) malloc((size/2)*sizeof(float));
  kiss_fft_cpx *inp_data = (kiss_fft_cpx *)malloc(size*sizeof(kiss_fft_cpx));
  kiss_fft_cpx *fft_out = (kiss_fft_cpx *)malloc(size*sizeof(kiss_fft_cpx));
  kiss_fft_cfg kiss_fft_state;
  float coeff;

  // calculate and remove mean
  float mean = 0;
  for (i = 0; i < size; i++) {
    mean += window[i];
  }
  mean = mean/(float)size;

  for (i = 0; i < size; i++) {
    window[i] -= mean;
  }

  // Prepare the data. Only real parts are present. Complex part is 0.
  for (i = 0; i < size; i++) {
    inp_data[i].r = window[i];
    inp_data[i].i = 0.0f;
  }

  // Compute the FFT
  kiss_fft_state = kiss_fft_alloc(size, 0, NULL, NULL);
  kiss_fft(kiss_fft_state, inp_data, fft_out);

  // actually, we are interested only in 0..Fs/2 (and not the -Fs/2...0 part)
  // compute the periodogram (1/NFFT |Y(omega)|^2).
  coeff = 1.0/(float) size;
  for (i = 0; i < size/2; i++) {
    spectrum[i] = (fft_out[i].r*fft_out[i].r + fft_out[i].i*fft_out[i].i)*coeff;
  }

  free(kiss_fft_state);
  free(fft_out);
  free(inp_data);

  return spectrum;
}

peak_data_t *find_peaks(float *spectrum, int size, int *num_peaks_found)
{
  int i;
  int count = 0;
  peak_data_t *peaks = (peak_data_t *)malloc(NUM_PEAKS*sizeof(peak_data_t));

  // Check if the first bin is the biggest (for example, a large "DC" present)
  for (i = 1; i < size; i++) {
    if (spectrum[0] < spectrum[i]) {
      break;
    }
  }

  if (i == size) {
    peaks[count].peak_index = 0;
    peaks[count].peak_value = spectrum[0];
    count++;
  }

  for (i = 1; i < size-1; i++) {
    if (spectrum[i-1] < spectrum[i] && spectrum[i+1] < spectrum[i]) {
      peaks[count].peak_index = i;
      peaks[count].peak_value = spectrum[i];
      count++;
    }
    if (count >= NUM_PEAKS) {
      break;
    }
  }
  *num_peaks_found = count;
  qsort(peaks, count, sizeof(peak_data_t), compare_peaks_decending);
  return peaks;
}

gboolean detect_dominant_peaks(float *spectrum, int size, peak_data_t * peaks, int num_peaks)
{
  int i;
  int k;
  int start;
  int stop;
  int bin_count = 0;
  gboolean indices[WINDOW_LEN/2];
  float ratio;
  float total_power = 0.0f;
  float peak_power = 0.0f;
  float pk_ratio = 0.0f;

  if (num_peaks > 1) {
    int num_peaks_to_collect = MIN(num_peaks, NUM_MAIN_PEAKS);
    for (i = 0; i < size; i++) {
      indices[i] = FALSE;
    }

    DBG(syslog(LOG_INFO, "detect_dominant peaks[0] value %f index %d (%d Hz)", peaks[0].peak_value, peaks[0].peak_index, ((peaks[0].peak_index-1)*(1000/UPDATE_INTERVAL_IN_MSEC)/WINDOW_LEN)));
    DBG(syslog(LOG_INFO, "detect_dominant peaks[1] value %f index %d (%d Hz)", peaks[1].peak_value, peaks[1].peak_index, ((peaks[1].peak_index-1)*(1000/UPDATE_INTERVAL_IN_MSEC)/WINDOW_LEN)));
    // Calculate the total power (only half of the spectrum is considered)
    for (i = 0; i < size; i++) {
      total_power += spectrum[i];
    }

    // Calculate the power contained in the highest and next highest peaks.
    // We collect the power in the peak bin and +/- HALF_LOBE_WIDTH neighbors.
    for (k = 0; k < num_peaks_to_collect; k++) {
      if (peaks[k].peak_index >= MINIMUM_DOMINANT_PEAK_FREQUENCY) {
        start = MAX(peaks[k].peak_index - MAX_HALF_LOBE_WIDTH, 2);
        stop = MIN(peaks[k].peak_index + MAX_HALF_LOBE_WIDTH, (size-1));

        // Always begin from peak index and move downwards in both directions
        // to avoid situations where a false index is taken into the summation
        for (i = peaks[k].peak_index; i >= start; i--) {
          if (spectrum[i] > PEAK_DAMP_FACTOR*peaks[k].peak_value) {
            indices[i] = TRUE;
            bin_count++;
            if (spectrum[i] <= spectrum[i-1]) {
              break;
            }
          } else {
            break;
          }
        }

        for (i = peaks[k].peak_index + 1; i <= stop; i++) {
          if (spectrum[i] > PEAK_DAMP_FACTOR*peaks[k].peak_value) {
            indices[i] = TRUE;
            bin_count++;
            if (spectrum[i] <= spectrum[i+1]) {
              break;
            }
          } else {
            break;
          }
        }
      }
    }
    for (i = 0; i < size; i++) {
      if (indices[i]) {
        peak_power += spectrum[i];
        DBG(syslog(LOG_INFO, "Power peak %d", i));
      }
    }

    ratio = peak_power/total_power;

    if (peaks[0].peak_index > 3 && peaks[1].peak_index <= 3) {
      pk_ratio = peaks[0].peak_value/peaks[1].peak_value;
    }

    DBG(syslog(LOG_INFO, "total_power: %3.4f, peak_power: %3.4f, power_ratio: %3.4f, peak_ratio: %3.4f", total_power, peak_power, ratio, pk_ratio));

    if (ratio > POWER_RATIO_THRESHOLD || pk_ratio > PEAK_RATIO_THRESHOLD) {
      return TRUE;
    }
  }
  return FALSE;
}

/**
 * tilt_trigger_tampering:
 *
 * Trigger a tampering event and reset the base tilt
 * degrees value.
 */
static void
tilt_trigger_tampering(void)
{
  send_tilt_detection_event();
  tilt_user_data->update_counter = 0;
}

/**
 * compare_peaks_decending:
 *
 * Takes two floats wrapped in peak_data_t and compares them.
 *
 * Returns: an integer less than, equal to, or greater than
 *          zero if the first argument is considered to be
 *          respectively less than, equal to, or greater
 *          than the second.
 */
static int compare_peaks_decending (const void * x, const void * y)
{
  float a = ((peak_data_t*)x)->peak_value;
  float b = ((peak_data_t*)y)->peak_value;
  if (a < b) {
    return 1;
  } else if (a > b) {
    return -1;
  }
  return 0;
}

/**
 * get_lateral_difference:
 *
 * Compare an angle with a reference angle.
 *
 * @angle: the current angle, between 0 and 180
 * @reference_angle: the reference angle, between 0 and 180
 *
 * Returns: the angular difference between the two angles
 */
static gint
get_lateral_difference(gint angle, gint reference_angle)
{
  gint diff_lat = angle - reference_angle; /* lat angle is always between 0 and 180 */
  if (angle <= ILLEGAL_LATERAL_VALUE) {
    return 0;
  }
  return diff_lat;
}

/**
 * get_longitudinal_difference:
 *
 * Compare an angle with a reference angle.
 *
 * @angle: the current angle, between 0 and 359
 * @reference_angle: the reference angle, between 0 and 359
 *
 * Returns: the angular difference between the two angles,
 *          and account for rollover.
 */
static gint
get_longitudinal_difference(gint angle, gint reference_angle)
{
  gint diff_lon = abs(angle - reference_angle); /* lon is between 0 and 359 */
  if (angle <= ILLEGAL_LONGITUDINAL_VALUE) {
    return 0;
  }
  if (diff_lon % 360 > 180) {
    diff_lon = diff_lon - 360; /* this accounts for rollover between 359 and 0 etc */
  }
  return diff_lon;
}

/**
 * initialize_tilt_detection_event:
 *
 * Initialize function for tilt detection event handling.
 */
static
gboolean initialize_tilt_detection_event(void)
{
  EventDeclaration *declaration = NULL;
  /* Hard code channel for now. In the future we might e.g. in an
   * encoder want to specify what camera that was abused.*/
  gint channel = 1;

  /* Create an event producer for the tilt detection event. */
  producer = event_producer_new("Posd");
  if (producer == NULL) {
    syslog (LOG_ERR, "Failed to create event producer");
    return FALSE;
  }

 /* Declare the event. */
 declaration = event_declaration_new(FALSE);
 if (declaration == NULL) {
    syslog (LOG_ERR, "Failed to create event declaration");
    return FALSE;
  }

  /* Keys. */
  event_declaration_add_key_value(declaration, "channel", &channel,
                                 VALUE_TYPE_INT);
  event_declaration_add_key_value(declaration, ONVIF_KEY_TOPIC2,
                                  "TiltDetected", VALUE_TYPE_STRING);
  event_declaration_add_key_value(declaration, ONVIF_KEY_TOPIC0,
                                  "Device", VALUE_TYPE_STRING);
  event_declaration_add_key_value(declaration, ONVIF_KEY_TOPIC1,
                                  "Tampering", VALUE_TYPE_STRING);

  /* Nice names. */
  event_declaration_set_nice_names(declaration, "channel",
                                   "Channel", NULL);
  event_declaration_set_nice_names(declaration, ONVIF_KEY_TOPIC2,
                                    NULL, "Tilt Detected");
  event_declaration_set_nice_names(declaration, ONVIF_KEY_TOPIC0,
                                   NULL, "Device");
  event_declaration_set_nice_names(declaration, ONVIF_KEY_TOPIC1,
                                   NULL, "Tampering");

  /* Set name spaces. */
  event_declaration_set_name_space(declaration, ONVIF_KEY_TOPIC0,
                                   ONVIF_NAMESPACE);
  event_declaration_set_name_space(declaration, ONVIF_KEY_TOPIC1,
                                   AXIS_NAMESPACE);
  event_declaration_set_name_space(declaration, ONVIF_KEY_TOPIC2,
                                   AXIS_NAMESPACE);

  /* Key tags. */
  event_declaration_add_key_tag(declaration, "channel", "onvif-source");

  declaration_id = event_producer_declare(producer, declaration);
  event_declaration_free(declaration);
  if (declaration_id == 0) {
    syslog (LOG_ERR, "Failed to declare event producer");
    return FALSE;
  }
  return TRUE;
}

/**
 * send_tilt_detection_event:
 *
 * Send a tilt detection event.
 *
 * Returns: #TRUE if event was sent, else #FALSE.
 */
static gboolean
send_tilt_detection_event(void)
{
  Event *event = NULL;
  CHECK_TILT_IS_ENABLED_OR_RETURN (__FUNCTION__, FALSE);

  /* Prepare the event. */
  event = event_producer_prepare_event(producer, declaration_id);
  if (event == NULL) {
    syslog (LOG_ERR, "Failed to create the tilt detection event");
    return FALSE;
  }

#ifdef HOST
  fake_event();
#endif

  /* Send the event. */
  DBG(syslog (LOG_DEBUG, "Sending Tilt Detection event"));
  event_producer_send_event_async(producer, event);
  event_free(event);
  return TRUE;
}

/****************** EXPORTED FUNCTION DEFINITION SECTION ********************/
/**
 * tilt_detection_init:
 *
 * Init function for tilt detection API. Must be called before using
 * any of the other functions.
 */
void
tilt_detection_init(void)
{
  if (tilt_user_data != NULL) {
    syslog (LOG_WARNING,
        "%s called, needed to deallocate previous user_data.", __FUNCTION__);
    g_free (tilt_user_data);
  }
  DBG(syslog (LOG_INFO, "INIT"));
  tilt_user_data = g_malloc0(sizeof(*tilt_user_data));
  tilt_user_data->initialized = FALSE;
  tilt_user_data->trigger_angle = DEFAULT_TILT_DETECTION_TRIGGER_ANGLE;
  tilt_user_data->sensitivity = DEFAULT_TILT_DETECTION_SENSITIVITY;
  tilt_user_data->startup_lateral = ILLEGAL_LATERAL_VALUE-1;
  tilt_user_data->startup_longitudinal = ILLEGAL_LONGITUDINAL_VALUE-1;
  tilt_user_data->update_counter = TAMPER_GRACE_PERIOD;
  tilt_user_data->samples = UINT_MAX;
  tilt_user_data->tamper_triggered = FALSE;
  tilt_user_data->should_record_samples = FALSE;
  tilt_user_data->collected_samples = NULL;
  tilt_user_data->observing = FALSE;
  tilt_user_data->pass_count = 0;
  tilt_user_data->observation_count = 0;
  tilt_user_data->sample_speed_fast = FALSE;
  g_mutex_init(&tilt_user_data->report_mutex);

  device_sample* raw_startup =  pos_lib_get_accelerometer_raw_data ();
  tilt_user_data->acc_x_at_startup = raw_startup->x;
  tilt_user_data->acc_y_at_startup = raw_startup->y;
  tilt_user_data->acc_z_at_startup = raw_startup->z;
  free(raw_startup);

  // READ SVM MODEL HERE
  svm_model = read_linear_svm_model("/tmp/svm_params", get_nbr_features());

  if (posd_get_tilt_detection_params(
      &tilt_user_data->enabled,
      &tilt_user_data->trigger_angle,
      &tilt_user_data->sensitivity) == FALSE)
  {
    syslog (LOG_WARNING, "tilt_user_data->enabled == FALSE");
    return;
  }

  if (datacache_posd_init ()) {
    syslog (LOG_ERR, "Failed to initialize datacache posd");
  }

  introspection_data = g_dbus_node_info_new_for_xml(introspection_xml,
                                                     NULL);
  if (introspection_data == NULL) {
    syslog (LOG_WARNING, "%s - Erroneous introspection_data", __FUNCTION__);
  }
  g_assert(introspection_data != NULL);
  tilt_user_data->dbus_owner_id = g_bus_own_name(G_BUS_TYPE_SYSTEM,
                                                   "com.axis.Positioning.TiltDetection",
                                                   G_BUS_NAME_OWNER_FLAGS_NONE,
                                                   on_bus_acquired,
                                                   on_name_acquired,
                                                   on_name_lost,
                                                   tilt_user_data,
                                                   NULL);

  (void)initialize_tilt_detection_event();

  g_timeout_add (UPDATE_INTERVAL_SLOW_IN_MSEC, collect_samples, NULL);

  tilt_user_data->initialized = TRUE;
}

/**
 * tilt_detection_finalize:
 *
 * Finalize function. Must be called when finished using tilt detection.
 */
void
tilt_detection_finalize(void)
{
  CHECK_TILT_IS_INITIALIZED_OR_RETURN (__FUNCTION__, );

  g_mutex_lock(&tilt_user_data->report_mutex);
  g_mutex_unlock(&tilt_user_data->report_mutex);
  g_mutex_clear(&tilt_user_data->report_mutex);

  datacache_posd_finalize ();

  if (producer != NULL) {
    event_producer_free(producer);
    producer = NULL;
  }

  g_bus_unown_name(tilt_user_data->dbus_owner_id);
  g_dbus_node_info_unref(introspection_data);

  if (tilt_user_data != NULL) {
    g_free(tilt_user_data);
    tilt_user_data = NULL;
  }

  if(svm_model != NULL)
  {
    g_free(svm_model->beta);
    g_free(svm_model);
  }
}

/**
 * tilt_detection_get_enabled:
 *
 * Function for retreiving the enable status.
 *
 * Returns: #TRUE if tilt detection is enabled, else #FALSE.
 */
gboolean
tilt_detection_get_enabled(void)
{
  CHECK_TILT_IS_INITIALIZED_OR_RETURN (__FUNCTION__, FALSE);

  return tilt_user_data->enabled;
}

/**
 * tilt_detection_set_enabled:
 * @enabled: #TRUE if tilt detection shall be enabled, else #FALSE
 *
 * Function for enabling / disabling tilt detection.
 */
void
tilt_detection_set_enabled(const gboolean enabled)
{
  CHECK_TILT_IS_INITIALIZED_OR_RETURN (__FUNCTION__, );

  tilt_user_data->enabled = enabled;
  (void)posd_set_tilt_detection_params(tilt_user_data->enabled,
                                        tilt_user_data->trigger_angle,
                                        tilt_user_data->sensitivity);
}

/**
 * tilt_detection_get_trigger_angle:
 *
 * Function for retrieving the trigger angle.
 *
 * Returns: The trigger angle.
 */
guint
tilt_detection_get_trigger_angle(void)
{
  CHECK_TILT_IS_INITIALIZED_OR_RETURN (__FUNCTION__, 0);

  return tilt_user_data->trigger_angle;
}

/**
 * tilt_detection_get_sensitivity:
 *
 * Function for retrieving the tilt sensitivity.
 *
 * Returns: The sensitivity.
 */
guint
tilt_detection_get_sensitivity(void)
{
  CHECK_TILT_IS_INITIALIZED_OR_RETURN (__FUNCTION__, UINT_MAX);

  return tilt_user_data->sensitivity;
}

/**
 * tilt_detection_set_trigger_angle:
 * @angle: The trigger angle to be used
 *
 * Function for setting the trigger angle.
 *
 * Returns: #TRUE if the trigger angle was set, else #FALSE.
 */
gboolean
tilt_detection_set_trigger_angle(const guint angle)
{
  CHECK_TILT_IS_INITIALIZED_OR_RETURN (__FUNCTION__, FALSE);

  if (angle > 90 || angle <= 0) {
   syslog (LOG_WARNING, "Unable to set trigger angle (angle incorrectly declared or out of range).");
   return FALSE;
  }

  tilt_user_data->trigger_angle = angle;
  (void)posd_set_tilt_detection_params(tilt_user_data->enabled,
                                        tilt_user_data->trigger_angle,
                                        tilt_user_data->sensitivity);

  return TRUE;
}

/**
 * tilt_detection_set_sensitivity:
 * @sensitivity: The sensitivity to be used
 *
 * Function for setting the sensitivity.
 *
 * Returns: #TRUE if the sensitivity was set, else #FALSE.
 */
gboolean
tilt_detection_set_sensitivity(const guint sensitivity)
{
  CHECK_TILT_IS_INITIALIZED_OR_RETURN (__FUNCTION__, FALSE);

  if (sensitivity < 1 || sensitivity > 100) {
   syslog (LOG_WARNING, "Unable to set sensitivity (sensitivity incorrectly declared or out of range).");
   return FALSE;
  }

  tilt_user_data->sensitivity = sensitivity;
  (void)posd_set_tilt_detection_params(tilt_user_data->enabled,
                                        tilt_user_data->trigger_angle,
                                        tilt_user_data->sensitivity);

  return TRUE;
}

/**
 * tilt_detection_trigger_event:
 *
 * This function shall be called when a tilt detection event2.
 * is to be triggered.
 */
void
tilt_detection_trigger_event(void)
{
  CHECK_TILT_IS_ENABLED_OR_RETURN (__FUNCTION__, );

  if (!datacache_posd_ignore_tilt ()) {
    send_tilt_detection_event();
  }
}
