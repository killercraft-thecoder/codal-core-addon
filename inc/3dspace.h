#include "CoordinateSystem.h"
#include "types.h"
#include "Accelerometer.h"
#include "CodalComponent.h"
#include "Compass.h"
#include "CodalDmesg.h"
#include "Event.h"

#define ENABLE_FALL_SPEED_DECTION 0 // switch to 1 to enable.
#define DEVICE_ID_SPACE3D_FALL_REPORT 0x2002

typedef struct __attribute__((packed))
{
    int32_t device_x;
    int32_t device_y;
    int32_t device_z;
    int32_t device_roll;  // X
    int32_t device_pitch; // Y
    int32_t device_yaw;   // Z
} SPACE_3D;

typedef struct __attribute__((packed))
{
    int32_t CENTER_X;
    int32_t CENTER_Y;
    int32_t CENTER_Z;
    int32_t CENTER_ROLL;
    int32_t CENTER_PITCH;
    int32_t CENTER_YAW;
} SPACE_CENTER;

/**
 * A Sample of where the device is in space
 */
typedef SPACE_3D DEVICE_POS_SAMPLE;

inline codal::CoordinateSpace CORD_SPACE = new codal::CoordinateSpace(codal::CoordinateSystem::SIMPLE_CARTESIAN);

#define DEVICE_ID_SPACE3D 0x2001
#define SPIN_THRESHOLD 100

class Space3D : public codal::CodalComponent
{
private:
    codal::Accelerometer &accel;
    codal::Compass &comp;
    bool hasComp;
    bool calibrated;
    SPACE_3D currentState;
    SPACE_CENTER centerState;
    int sampleRate;
    float radialAccel;
    float vx;
    float x;
    float vy;
    float y;
    float vz;
    float z;
    int16_t _id;
    uint32_t last_time;
    float velocity_z; // Esimated Fall Speed in m/s²
    void registerGestureHandlers()
    {
        messageBus.listen(accel.getId(), DEVICE_ID_GESTURE, this->onGestureDetected);
    }
    void onGestureDetected(codal::Event e)
    {
        uint16_t gesture = e.value;

        switch (gesture)
        {
        case ACCELEROMETER_EVT_SHAKE:
        case ACCELEROMETER_EVT_2G:
        case ACCELEROMETER_EVT_3G:
        case ACCELEROMETER_EVT_6G:
        case ACCELEROMETER_EVT_8G:
            // Update current state
            this->update();

            // Estimate radial acceleration (centrifugal force)
            {
                int x = currentState.device_x;
                int y = currentState.device_y;
                float radialAccel = sqrtf((float)(x * x + y * y));

                this->radialAccel = radialAccel;
            }
            break;

        case ACCELEROMETER_EVT_TILT_LEFT:
        case ACCELEROMETER_EVT_TILT_RIGHT:
        case ACCELEROMETER_EVT_FACE_UP:
        case ACCELEROMETER_EVT_FACE_DOWN:
            break;
        case ACCELEROMETER_EVT_FREEFALL:
#if defined(ENABLE_FALL_SPEED_DECTION)
            uint32_t now = system_timer_current_time();
            float dt = (now - this->last_time) / 1000.0f; // ms to seconds
            this->last_time = now;

            float az = accel.getZ() * 9.81f / 1000.0f; // convert milli-g to m/s²
            float net_az = az - 9.81f;                 // remove gravity

            this->velocity_z += net_az * dt;

            DMESG("Fall speed: %.2f m/s\n", velocity_z);
            Event e(DEVICE_ID_SPACE3D_FALL_REPORT, velocity_z);
            e.fire();
#endif
            break;
        default:
            // Unknown or unhandled gesture
            break;
        }
    }
    bool trackMotion = false; // Optional feature toggle
    float getYaw()
    {
        if (this->hasComp)
        {
            // TODO: Maybe add handling for this case to make it more accurate?
        }

        // Compute angle of lateral acceleration vector in degrees
        float angle = atan2(currentState.device_y, currentState.device_x) * 180.0f / M_PI;
        if (angle == nullptr)
        {
            return 0;
        }
        // Optional: suppress noisy readings if radialAccel is too low
        if (radialAccel < SPIN_THRESHOLD)
            return 0.0f;

        // Normalize angle to range [0, 360)
        if (angle < 0)
            angle += 360.0f;

        return angle;
    }

public:
    // rate is ms per tick , not hz.
    Space3D(codal::Accelerometer &accelerometer, int rate = 25)
        : codal::CodalComponent(DEVICE_ID_SPACE3D)(DEVICE_ID_SPACE3D), accel(accelerometer), sampleRate(rate), calibrated(false)
    {
        currentState = {0, 0, 0, 0, 0, 0};
        centerState = {0, 0, 0, 0, 0, 0};
        this->setup();
        this->registerGestureHandlers();
        this->calibrateCenter();
        this->calibrated = true;
        this->update();
        this->comp = NULL;
        this->hasComp = false;
    }
    // rate is ms per tick , not hz.
    Space3D(codal::Accelerometer &accelerometer, codal::Compass &comp, int rate = 25)
        : codal::CodalComponent(DEVICE_ID_SPACE3D), accel(accelerometer), sampleRate(rate)
    {
        currentState = {0, 0, 0, 0, 0, 0};
        centerState = {0, 0, 0, 0, 0, 0};
        this->setup();
        this->registerGestureHandlers();
        this->calibrateCenter();
        this->calibrated = true;
        this->update();
        this->comp = comp;
        this->hasComp = true;
        this->comp.calibrate();
    }
    // rate is ms per tick , not hz.
    Space3D(codal::Compass &comp, int rate = 25)
        : codal::CodalComponent(DEVICE_ID_SPACE3D), sampleRate(rate)
    {
        this->setup();
        this->accel = new codal::Accelerometer(CORD_SPACE);
        currentState = {0, 0, 0, 0, 0, 0};
        centerState = {0, 0, 0, 0, 0, 0};
        this->registerGestureHandlers();
        this->calibrateCenter();
        this->calibrated = true;
        this->update();
        this->comp = comp;
        this->hasComp = true;
        this->comp.calibrate();
    }
    // rate is ms per tick , not hz.
    Space3D(int rate = 25)
        : codal::Component(DEVICE_ID_SPACE3D), sampleRate(rate)
    {
        this->accel = new codal::Accelerometer(CORD_SPACE);
        currentState = {0, 0, 0, 0, 0, 0};
        centerState = {0, 0, 0, 0, 0, 0};
        this->setup();
        this->registerGestureHandlers();
        this->calibrateCenter();
        this->calibrated = true;
        this->update();
        this->comp = NULL;
        this->hasComp = false;
    }

    void motionTracking(bool enable)
    {
        this->motionTracking = enable;
        if (enable)
        {
            this->x = 0;
            this->y = 0;
            this->z = 0;
            this->vx = 0;
            this->vy = 0;
            this->vz = 0;
        }
    }
    void setup()
    {
        if (this->_id == nullptr)
        {
            // means its not initalized yet
            this->_id = 0;
        }
        system_timer_event_every(this->sampleRate, DEVICE_ID_SPACE3D, this->_id);
        this->_id += 1; // make sure next time it is diffrent
    }
    void eventReceived(Event e) override
    {
        if (e.source == DEVICE_ID_SPACE3D && e.value == this->_id)
        {
            this->update();
        }
    }

    // Override from Component
    virtual int update(bool ignorecal) override
    {
        if (ignorecal == false && this->calibrated == false)
        {
            return DEVICE_CALIBRATION_IN_PROGRESS;
        }
        currentState.device_x = accel.getX() - this->centerState.CENTER_X;
        currentState.device_y = accel.getY() - this->centerState.CENTER_Y;
        currentState.device_z = accel.getZ() - this->centerState.CENTER_Z;
        currentState.device_roll = accel.getRoll() - this->centerState.CENTER_ROLL;
        currentState.device_pitch = accel.getPitch() - this->centerState.CENTER_PITCH;
        if (this->hasComp && (this->comp.getFieldStrength() > 20))
        {
            currentState.device_yaw = this->comp.heading() - this->centerState.CENTER_YAW;
        }
        else
        {
            currentState.device_yaw = this->getYaw() - this->centerState.CENTER_YAW; // estimate
        }

        if (this->trackMotion)
        {
            uint64_t now = system_timer_current_time();
            float dt = (now - this->lastUpdateTime) / 1000.0f; // convert ms to seconds
            this->lastUpdateTime = now;

            // Convert milli-g to m/s²
            float ax = currentState.device_x * 9.81f / 1000.0f;
            float ay = currentState.device_y * 9.81f / 1000.0f;
            float az = currentState.device_z * 9.81f / 1000.0f;
            if (ax == nullptr || ay == nullptr || az == nullptr)
            {
                return DEVICE_INVALID_STATE;
            }
            if (dt > 0.2)
            {
                ax *= 0.5f;
                ay *= 0.5f;
                az *= 0.5f;
            }
            // Integrate acceleration to velocity
            this->vx += ax * dt;
            this->vy += ay * dt;
            this->vz += az * dt;

            // Integrate velocity to position
            this->x += vx * dt;
            this->y += vy * dt;
            this->z += vz * dt;
        }

        return DEVICE_OK;
    }

    // Accessors
    inline const SPACE_3D &getCurrentState() const
    {
        return currentState;
    }

    inline const SPACE_CENTER &getCenterState() const
    {
        return centerState;
    }
    /**
     * Recalibrates the Space3d system.
     *
     * If a compass is available and enabled, it triggers a compass calibration
     * to reset magnetic heading. Then it recalibrates the center orientation
     * for position and rotation in space reference using accelerometer data.
     *
     * This method ensures that both compass-based and accel-based yaw tracking
     * start from a stable reference point.
     *
     * @returns {int} Always returns 0 (success).
     */
    int recalibrate()
    {
        this->calibrated = false;
        if (this->hasComp)
        {
            int code = this->comp.calibrate();
            if (code == DEVICE_I2C_ERROR)
            {
                return DEVICE_I2C_ERROR;
            }
            if (code == DEVICE_CALIBRATION_REQUIRED)
            {
                return DEVICE_CALIBRATION_REQUIRED;
            }
        }
        this->calibrateCenter();
        this->calibrated = true;
        return DEVICE_OK;
    }

    // Calibration
    inline void calibrateCenter()
    {

        this->update(true);
        centerState.CENTER_X = currentState.device_x;
        centerState.CENTER_Y = currentState.device_y;
        centerState.CENTER_Z = currentState.device_z;
        centerState.CENTER_ROLL = currentState.device_roll;
        centerState.CENTER_PITCH = currentState.device_pitch;
        centerState.CENTER_YAW = currentState.device_yaw;
    }

    inline int setSampleRate(int rate)
    {
        if (!this->calibrated)
        {
            return DEVICE_CALIBRATION_IN_PROGRESS;
        }
        sampleRate = rate;
        this->setup();
        return DEVICE_OK;
    }
    // rate is ms per tick , not hz.  , defualt rate is:40hz , or 25ms per tick.
    inline int getSampleRate() const
    {
        return sampleRate;
    }
    // rate is ms per tick , not hz.
    inline DEVICE_POS_SAMPLE &getSample()
    {
        return &this->currentState;
    }

    ~Space3D()
    {
        delete this->accel;
        if (this->hasComp)
            delete this->comp;
    }
};
